/*
**
** File:        "decod.c"
**
** Description:     Top-level source code for G.723.1 dual-rate decoder
**
** Functions:       Init_Decod()
**                  Decod()
**
**
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "typedef.h"
#include "basop.h"
#include "cst_lbc.h"
#include "tab_lbc.h"
#include "lbccodec.h"
#include "decod.h"
#include "util_lbc.h"
#include "lpc.h"
#include "lsp.h"
#include "exc_lbc.h"
#include "dec_cng.h"


/*
**
** Function:        Init_Decod()
** 
** Description:     Initializes non-zero state variables
**          for the decoder.
**
** Links to text:   Section 3.11
** 
** Arguments:       None
** 
** Outputs:     None
** 
** Return value:    None
**
*/
void  Init_Decod( )
{
    int   i  ;

    /* Initialize encoder data structure with zeros */
    memset(DecStat, 0, sizeof(DECSTATDEF));


    /* Initialize the previously decoded LSP vector to the DC vector */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        DecStat->PrevLsp[i] = LspDcTable[i] ;

    /* Initialize the gain scaling unit memory to a constant */
    DecStat->Gain = (Word16) 0x1000 ;

    return;
}


/*
**
** Function:        Decod()
**
** Description:     Implements G.723.1 dual-rate decoder for  a frame
**          of speech
**
** Links to text:   Section 3
**
** Arguments:
**
**  Word16 *DataBuff    Empty buffer
**  Word16 Vinp[]       Encoded frame (22/26 bytes)
**

** Outputs:
**
**  Word16 DataBuff[]   Decoded frame (480 bytes)
**
** Return value:
**
**  Flag            Always True
**
*/

Flag    Decod( Word16 *DataBuff, char *Vinp, Word16 Crc )
{
    int   i,j   ;

    Word32   Senr ;
    Word16   QntLpc[SubFrames*LpcOrder] ;
    Word16   AcbkCont[SubFrLen] ;

    Word16   LspVect[LpcOrder] ;
    Word16   Temp[PitchMax+Frame] ;
    Word16  *Dpnt ;

    LINEDEF  Line ;
    PFDEF    Pf[SubFrames] ;

    Word16   Ftyp;

    /*
    * Decode the packed bitstream for the frame.  (Text: Section 4;
    * pars of sections 2.17, 2.18)
    */
    Line = Line_Unpk( Vinp, &Ftyp, Crc ) ;

    /*
    * Update the frame erasure count (Text: Section 3.10)
    */
    if ( Line.Crc != (Word16) 0 ) {
        if(DecCng->PastFtyp == 1) Ftyp = 1;  /* active */
        else Ftyp = 0;  /* untransmitted */
    }

    if(Ftyp != 1) {

        /* Silence frame : do noise generation */
        Dec_Cng(Ftyp, &Line, DataBuff, QntLpc);
    }

    else {

        /*
        * Update the frame erasure count (Text: Section 3.10)
        */
        if ( Line.Crc != (Word16) 0 )
            DecStat->Ecount = add( DecStat->Ecount, (Word16) 1 ) ;
        else
            DecStat->Ecount = (Word16) 0 ;

        if ( DecStat->Ecount > (Word16) ErrMaxNum )
            DecStat->Ecount = (Word16) ErrMaxNum ;

        /*
        * Decode the LSP vector for subframe 3.  (Text: Section 3.2)
        */
        Lsp_Inq( LspVect, DecStat->PrevLsp, Line.LspId, Line.Crc ) ;

        /*
        * Interpolate the LSP vectors for subframes 0--2.  Convert the
        * LSP vectors to LPC coefficients.  (Text: Section 3.3)
        */
        Lsp_Int( QntLpc, LspVect, DecStat->PrevLsp ) ;

        /* Copy the LSP vector for the next frame */
        for ( i = 0 ; i < LpcOrder ; i ++ )
            DecStat->PrevLsp[i] = LspVect[i] ;

        /*
        * In case of no erasure, update the interpolation gain memory.
        * Otherwise compute the interpolation gain (Text: Section 3.10)
        */
        if ( DecStat->Ecount == (Word16) 0 ) {
            DecStat->InterGain = add( Line.Sfs[SubFrames-2].Mamp,
                                            Line.Sfs[SubFrames-1].Mamp ) ;
            DecStat->InterGain = shr( DecStat->InterGain, (Word16) 1 ) ;
            DecStat->InterGain = FcbkGainTable[DecStat->InterGain] ;
        }
        else
            DecStat->InterGain = mult_r( DecStat->InterGain, (Word16) 0x6000 ) ;


        /*
        * Generate the excitation for the frame
        */
        for ( i = 0 ; i < PitchMax ; i ++ )
            Temp[i] = DecStat->PrevExc[i] ;

        Dpnt = &Temp[PitchMax] ;

        if ( DecStat->Ecount == (Word16) 0 ) {

            for ( i = 0 ; i < SubFrames ; i ++ ) {

                /* Generate the fixed codebook excitation for a
                   subframe. (Text: Section 3.5) */
                Fcbk_Unpk( Dpnt, Line.Sfs[i], Line.Olp[i>>1], (Word16) i ) ;

                /* Generate the adaptive codebook excitation for a
                   subframe. (Text: Section 3.4) */
                Decod_Acbk( AcbkCont, &Temp[SubFrLen*i], Line.Olp[i>>1],
                                    Line.Sfs[i].AcLg, Line.Sfs[i].AcGn ) ;

                /* Add the adaptive and fixed codebook contributions to
                   generate the total excitation. */
                for ( j = 0 ; j < SubFrLen ; j ++ ) {
                    Dpnt[j] = shl( Dpnt[j], (Word16) 1 ) ;
                    Dpnt[j] = add( Dpnt[j], AcbkCont[j] ) ;
                }

                Dpnt += SubFrLen ;
            }

            /* Save the excitation */
            for ( j = 0 ; j < Frame ; j ++ )
                DataBuff[j] = Temp[PitchMax+j] ;

            /* Compute interpolation index. (Text: Section 3.10) */
            /* Use DecCng->SidGain and DecCng->CurGain to store    */
            /* excitation energy estimation                      */
            DecStat->InterIndx = Comp_Info( Temp, Line.Olp[SubFrames/2-1],
                                       &DecCng->SidGain, &DecCng->CurGain ) ;

            /* Compute pitch post filter coefficients.  (Text: Section 3.6) */
            if ( UsePf )
                for ( i = 0 ; i < SubFrames ; i ++ )
                    Pf[i] = Comp_Lpf( Temp, Line.Olp[i>>1], (Word16) i ) ;

            /* Reload the original excitation */
            for ( j = 0 ; j < PitchMax ; j ++ )
                Temp[j] = DecStat->PrevExc[j] ;
            for ( j = 0 ; j < Frame ; j ++ )
                Temp[PitchMax+j] = DataBuff[j] ;

            /* Perform pitch post filtering for the frame.  (Text: Section
               3.6) */
            if ( UsePf )
                for ( i = 0 ; i < SubFrames ; i ++ )
                    Filt_Lpf( DataBuff, Temp, Pf[i], (Word16) i ) ;

            /* Save Lsps --> LspSid */
            for(i=0; i< LpcOrder; i++)
                DecCng->LspSid[i] = DecStat->PrevLsp[i];
        }

        else {

            /* If a frame erasure has occurred, regenerate the
               signal for the frame. (Text: Section 3.10) */
            Regen( DataBuff, Temp, DecStat->InterIndx, DecStat->InterGain,
                                        DecStat->Ecount, &DecStat->Rseed ) ;
        }

        /* Update the previous excitation for the next frame */
        for ( j = 0 ; j < PitchMax ; j ++ )
            DecStat->PrevExc[j] = Temp[Frame+j] ;

        /* Resets random generator for CNG */
        DecCng->RandSeed = 12345;
    }

    /* Save Ftyp information for next frame */
    DecCng->PastFtyp = Ftyp;

    /*
    * Synthesize the speech for the frame
    */
    Dpnt = DataBuff ;
    for ( i = 0 ; i < SubFrames ; i ++ ) {

        /* Compute the synthesized speech signal for a subframe.
         * (Text: Section 3.7)
         */
        Synt( Dpnt, &QntLpc[i*LpcOrder] ) ;

        if ( UsePf ) {

            /* Do the formant post filter. (Text: Section 3.8) */
            Senr = Spf( Dpnt, &QntLpc[i*LpcOrder] ) ;

            /* Do the gain scaling unit.  (Text: Section 3.9) */
            Scale( Dpnt, Senr ) ;
        }

        Dpnt += SubFrLen ;
    }
    return (Flag) True ;
}
