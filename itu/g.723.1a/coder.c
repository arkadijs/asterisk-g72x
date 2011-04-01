/*
**
** File:        "coder.c"
**
** Description:     Top-level source code for G.723.1 dual-rate coder
**
** Functions:       Init_Coder()
**                  Coder()
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
#include "coder.h"
#include "lpc.h"
#include "lsp.h"
#include "exc_lbc.h"
#include "util_lbc.h"
#include "vad.h"
#include "cod_cng.h"
#include "tame.h"
/*
   This file includes the coder main functions
*/

/*
**
** Function:        Init_Coder()
**
** Description:     Initializes non-zero state variables
**          for the coder.
**
** Links to text:   Section 2.21
** 
** Arguments:       None
**
** Outputs:     None
** 
** Return value:    None
**
*/
void  Init_Coder( void)
{
    int   i ;

    /* Initialize encoder data structure with zeros */
    memset(CodStat, 0, sizeof(CODSTATDEF));

    /* Initialize the previously decoded LSP vector to the DC vector */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        CodStat->PrevLsp[i] = LspDcTable[i] ;

    /* Initialize the taming procedure */
    for(i=0; i<SizErr; i++) CodStat->Err[i] = Err0;

    return;
}

/*
**
** Function:        Coder()
**
** Description:     Implements G.723.1 dual-rate coder for    a frame
**          of speech
**
** Links to text:   Section 2
**
** Arguments:
**
**  Word16 DataBuff[]   frame (480 bytes)
**

** Outputs:
**
**  Word16 Vout[]       Encoded frame (20/24 bytes)
**
** Return value:
**
**  Flag            Always True
**
*/
Flag  Coder( Word16 *DataBuff, char *Vout )
{
    int     i,j   ;

    /*
      Local variables
    */
    Word16   UnqLpc[SubFrames*LpcOrder] ;
    Word16   QntLpc[SubFrames*LpcOrder] ;
    Word16   PerLpc[2*SubFrames*LpcOrder] ;

    Word16   LspVect[LpcOrder] ;
    LINEDEF  Line  ;
    PWDEF    Pw[SubFrames]  ;

    Word16   ImpResp[SubFrLen] ;

    Word16  *Dpnt  ;

    Word16  Ftyp = 1 ;

    /*
      Coder Start
    */
    Line.Crc = (Word16) 0 ;

    Rem_Dc( DataBuff ) ;

    /* Compute the Unquantized Lpc set for whole frame */
    Comp_Lpc( UnqLpc, CodStat->PrevDat, DataBuff ) ;

    /* Convert to Lsp */
    AtoLsp( LspVect, &UnqLpc[LpcOrder*(SubFrames-1)], CodStat->PrevLsp ) ;

    /* Compute the Vad */
    Ftyp = (Word16) Comp_Vad( DataBuff ) ;

    /* VQ Lsp vector */
    Line.LspId = Lsp_Qnt( LspVect, CodStat->PrevLsp ) ;

    Mem_Shift( CodStat->PrevDat, DataBuff ) ;

    /* Compute Perceptual filter Lpc coefficients */
    Wght_Lpc( PerLpc, UnqLpc ) ;

    /* Apply the perceptual weighting filter */
    Error_Wght( DataBuff, PerLpc ) ;

    /*
    // Compute Open loop pitch estimates
    */
    Dpnt = (Word16 *) malloc( sizeof(Word16)*(PitchMax+Frame) ) ;

    /* Construct the buffer */
    for ( i = 0 ; i < PitchMax ; i ++ )
        Dpnt[i] = CodStat->PrevWgt[i] ;
    for ( i = 0 ; i < Frame ; i ++ )
        Dpnt[PitchMax+i] = DataBuff[i] ;

    Vec_Norm( Dpnt, (Word16) (PitchMax+Frame) ) ;

    j = PitchMax ;
    for ( i = 0 ; i < SubFrames/2 ; i ++ ) {
        Line.Olp[i] = Estim_Pitch( Dpnt, (Word16) j ) ;
        VadStat->Polp[i+2] = Line.Olp[i] ;
        j += 2*SubFrLen ;
    }

    if(Ftyp != 1) {

        /*
        // Case of inactive signal
        */
        free ( (char *) Dpnt ) ;

        /* Save PrevWgt */
        for ( i = 0 ; i < PitchMax ; i ++ )
            CodStat->PrevWgt[i] = DataBuff[i+Frame-PitchMax] ;



        /* CodCng => Ftyp = 0 (untransmitted) or 2 (SID) */
        Cod_Cng(DataBuff, &Ftyp, &Line, QntLpc);

        /* Update the ringing delays */
        Dpnt = DataBuff;
        for( i = 0 ; i < SubFrames; i++ ) {

            /* Update exc_err */
            Update_Err(Line.Olp[i>>1], Line.Sfs[i].AcLg, Line.Sfs[i].AcGn);

            Upd_Ring( Dpnt, &QntLpc[i*LpcOrder], &PerLpc[i*2*LpcOrder],
                                                        CodStat->PrevErr ) ;
            Dpnt += SubFrLen;
        }
    }

    else {

        /*
        // Case of Active signal  (Ftyp=1)
        */

        /* Compute the Hmw */
        j = PitchMax ;
        for ( i = 0 ; i < SubFrames ; i ++ ) {
            Pw[i] = Comp_Pw( Dpnt, (Word16) j, Line.Olp[i>>1] ) ;
            j += SubFrLen ;
        }

        /* Reload the buffer */
        for ( i = 0 ; i < PitchMax ; i ++ )
            Dpnt[i] = CodStat->PrevWgt[i] ;
        for ( i = 0 ; i < Frame ; i ++ )
            Dpnt[PitchMax+i] = DataBuff[i] ;

        /* Save PrevWgt */
        for ( i = 0 ; i < PitchMax ; i ++ )
            CodStat->PrevWgt[i] = Dpnt[Frame+i] ;

        /* Apply the Harmonic filter */
        j = 0 ;
        for ( i = 0 ; i < SubFrames ; i ++ ) {
            Filt_Pw( DataBuff, Dpnt, (Word16) j , Pw[i] ) ;
            j += SubFrLen ;
        }
        free ( (char *) Dpnt ) ;

        /* Inverse quantization of the LSP */
        Lsp_Inq( LspVect, CodStat->PrevLsp, Line.LspId, Line.Crc ) ;

        /* Interpolate the Lsp vectors */
        Lsp_Int( QntLpc, LspVect, CodStat->PrevLsp ) ;

        /* Copy the LSP vector for the next frame */
        for ( i = 0 ; i < LpcOrder ; i ++ )
            CodStat->PrevLsp[i] = LspVect[i] ;

        /*
        // Start the sub frame processing loop
        */
        Dpnt = DataBuff ;

        for ( i = 0 ; i < SubFrames ; i ++ ) {

            /* Compute full impulse response */
            Comp_Ir( ImpResp, &QntLpc[i*LpcOrder],
                                            &PerLpc[i*2*LpcOrder], Pw[i] ) ;

            /* Subtract the ringing of previous sub-frame */
            Sub_Ring( Dpnt, &QntLpc[i*LpcOrder], &PerLpc[i*2*LpcOrder],
                                                   CodStat->PrevErr, Pw[i] ) ;

            /* Compute adaptive code book contribution */
            Find_Acbk( Dpnt, ImpResp, CodStat->PrevExc, &Line, (Word16) i ) ;

            /* Compute fixed code book contribution */
            Find_Fcbk( Dpnt, ImpResp, &Line, (Word16) i ) ;

            /* Reconstruct the excitation */
            Decod_Acbk( ImpResp, CodStat->PrevExc, Line.Olp[i>>1],
                                    Line.Sfs[i].AcLg, Line.Sfs[i].AcGn ) ;

            for ( j = SubFrLen ; j < PitchMax ; j ++ )
                CodStat->PrevExc[j-SubFrLen] = CodStat->PrevExc[j] ;

            for ( j = 0 ; j < SubFrLen ; j ++ ) {
                Dpnt[j] = shl( Dpnt[j], (Word16) 1 ) ;
                Dpnt[j] = add( Dpnt[j], ImpResp[j] ) ;
                CodStat->PrevExc[PitchMax-SubFrLen+j] = Dpnt[j] ;
            }

            /* Update exc_err */
            Update_Err(Line.Olp[i>>1], Line.Sfs[i].AcLg, Line.Sfs[i].AcGn);

            /* Update the ringing delays */
            Upd_Ring( Dpnt, &QntLpc[i*LpcOrder], &PerLpc[i*2*LpcOrder],
                                                       CodStat->PrevErr ) ;

            Dpnt += SubFrLen ;
        }  /* end of subframes loop */

        /*
        // Save Vad information and reset CNG random generator
        */
        CodCng->PastFtyp = 1;
        CodCng->RandSeed = 12345;

    } /* End of active frame case */

    /* Pack the Line structure */
    Line_Pack( &Line, Vout, Ftyp ) ;

    return (Flag) True ;
}
