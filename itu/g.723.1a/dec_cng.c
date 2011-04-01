/*
**
** File:            "dec_cng.c"
**
** Description:     Comfort noise generation
**                  performed at the decoder part
**
** Functions:       Init_Dec_Cng()
**                  Dec_Cng()
**
**
*/
/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "cst_lbc.h"
#include "tab_lbc.h"
#include "util_lbc.h"
#include "lsp.h"
#include "exc_lbc.h"
#include "basop.h"
#include "util_cng.h"
#include "dec_cng.h"
#include "decod.h"

/*
**
** Function:        Init_Dec_Cng()
**
** Description:     Initialize Dec_Cng static variables
**
** Links to text:
**
** Arguments:       None
**
** Outputs:         None
**
** Return value:    None
**
*/
void Init_Dec_Cng(void)
{
    int i;

    DecCng->PastFtyp = 1;
    DecCng->SidGain = 0;
    for(i=0; i<LpcOrder; i++) DecCng->LspSid[i] = LspDcTable[i] ;
    DecCng->RandSeed = 12345;
    return;
}

/*
**
** Function:           Dec_Cng()
**
** Description:        Receives Ftyp
**                     0  :  for untransmitted frames
**                     2  :  for SID frames
**                     Decodes SID frames
**                     Computes current frame excitation
**                     Computes current frame LSPs
**
** Links to text:
**
** Arguments:
**
**  Word16 Ftyp        Type of silence frame
**  LINEDEF *Line      Coded parameters
**  Word16 *DataExc    Current frame excitation
**  Word16 *QntLpc     Interpolated frame LPC coefficients
**
** Outputs:
**
**  Word16 *DataExc
**  Word16 *QntLpc
**
** Return value:       None
**
*/
void Dec_Cng(Word16 Ftyp, LINEDEF *Line, Word16 *DataExc, Word16 *QntLpc)
{

    Word16 temp;
    int i;

    if(Ftyp == 2) {

 /*
  * SID Frame decoding
  */
        DecCng->SidGain = Dec_SidGain(Line->Sfs[0].Mamp);

        /* Inverse quantization of the LSP */
        Lsp_Inq( DecCng->LspSid, DecStat->PrevLsp, Line->LspId, 0) ;
    }

    else {

/*
 * non SID Frame
 */
        if(DecCng->PastFtyp == 1) {

 /*
  * Case of 1st SID frame erased : quantize-decode
  * energy estimate stored in DecCng->SidGain
  * scaling factor in DecCng->CurGain
  */
            temp = Qua_SidGain(&DecCng->SidGain, &DecCng->CurGain, 0);
            DecCng->SidGain = Dec_SidGain(temp);
        }
    }


    if(DecCng->PastFtyp == 1) {
        DecCng->CurGain = DecCng->SidGain;
    }
    else {
        DecCng->CurGain = extract_h(L_add( L_mult(DecCng->CurGain,0x7000),
                    L_mult(DecCng->SidGain,0x1000) ) ) ;
    }
    Calc_Exc_Rand(DecCng->CurGain, DecStat->PrevExc, DataExc,
                    &DecCng->RandSeed, Line);

    /* Interpolate the Lsp vectors */
    Lsp_Int( QntLpc, DecCng->LspSid, DecStat->PrevLsp ) ;

    /* Copy the LSP vector for the next frame */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        DecStat->PrevLsp[i] = DecCng->LspSid[i] ;

    return;
}

