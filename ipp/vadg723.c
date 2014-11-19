/*/////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2011 Intel Corporation. All Rights Reserved.
//
//     Intel(R) Integrated Performance Primitives
//     USC - Unified Speech Codec interface library
//
// By downloading and installing USC codec, you hereby agree that the
// accompanying Materials are being provided to you under the terms and
// conditions of the End User License Agreement for the Intel(R) Integrated
// Performance Primitives product previously accepted by you. Please refer
// to the file ippEULA.rtf or ippEULA.txt located in the root directory of your Intel(R) IPP
// product installation for more information.
//
// A speech coding standards promoted by ITU, ETSI, 3GPP and other
// organizations. Implementations of these standards, or the standard enabled
// platforms may require licenses from various entities, including
// Intel Corporation.
//
//
// Purpose: G723 VAD functions.
//
*/

#include "vadg723.h"
#include "aux_fnxs.h"

/*
  Name:       VoiceActivityDetectSize

   Purpose:     VAD decision memory size query
   pVADmem     pointer to the VAD decision memory
*/
void VoiceActivityDetectSize_G723(Ipp32s* pVADsize)
{

   *pVADsize = sizeof(G723VADmemory);
   *pVADsize = (*pVADsize+7)&(~7);
}
/*
  Name:       VoiceActivityDetectInit

   Purpose:     VAD decision memory init
   pVADmem     pointer to the VAD decision memory
*/

void VoiceActivityDetectInit_G723(Ipp8s* pVADmem)
{
   G723VADmemory* vadMem =  (G723VADmemory*)pVADmem;


   vadMem->HangoverCounter = 3;
   vadMem->VADCounter = 0;
   vadMem->PrevEnergy = 0x400;
   vadMem->NoiseLevel = 0x400;

   vadMem->AdaptEnableFlag = 0;

   vadMem->OpenLoopDelay[0] = 1;
   vadMem->OpenLoopDelay[1] = 1;

}

static __ALIGN32 CONST Ipp32s LogAdd_Tbl[11] = {
   300482560, 300482560, 300482560, 300482560,337149952,378273792,424443904,476217344,534315008,599523328,672694272
};
static __ALIGN32 CONST Ipp16s LogMul_Tbl[11] = {
      0,    0,    0,    0, 1119, 1255, 1409, 1580, 1773, 1990, 2233
};

void VoiceActivityDetect_G723(const Ipp16s *pSrc, const Ipp16s *pNoiseLPC,
         const Ipp16s *pOpenLoopDelay, Ipp32s SineWaveDetector, Ipp32s *pVADDecision, Ipp32s *pAdaptEnableFlag, Ipp8s* pVADmem, Ipp16s *AlignBuff)
{
   G723VADmemory* vadMem =  (G723VADmemory*)pVADmem;
   Ipp32s i, j, lTmp0, lTmp1;
   Ipp16s sTmp0, sTmp1, sNmult, MinOLP;

   Ipp32s  VADResult = 1 ;

   vadMem->OpenLoopDelay[2] = pOpenLoopDelay[0] ;
   vadMem->OpenLoopDelay[3] = pOpenLoopDelay[1] ;

   /* Find Minimum pitch period */
   MinOLP = G723_MAX_PITCH ;
   for ( i = 0 ; i < 4 ; i ++ ) {
       if ( MinOLP > vadMem->OpenLoopDelay[i] )
           MinOLP = vadMem->OpenLoopDelay[i] ;
   }

   /* How many olps are multiplies of their min */
   sNmult = 0 ;
   for ( i = 0 ; i < 4 ; i++ ) {
       sTmp1 = MinOLP ;
       for ( j = 0 ; j < 8 ; j++ ) {
           sTmp0 = (Ipp16s)(sTmp1 - vadMem->OpenLoopDelay[i]);
           if(sTmp0 < 0) sTmp0 = (Ipp16s)(-sTmp0);
           if ( sTmp0 <= 3 ) sNmult++ ;
           sTmp1 = (Ipp16s)(sTmp1 + MinOLP);
       }
   }

   /* Update adaptation enable counter if not periodic and not sine and clip it.*/
   if ( (sNmult == 4) || (SineWaveDetector == -1) )
      if(vadMem->AdaptEnableFlag > 5) vadMem->AdaptEnableFlag = 6;
      else vadMem->AdaptEnableFlag += 2;
   else
      if ( vadMem->AdaptEnableFlag < 1 ) vadMem->AdaptEnableFlag = 0;
      else vadMem->AdaptEnableFlag--;

   /* Inverse filter the data */

   ippsResidualFilter_AMRWB_16s_Sfs(pNoiseLPC, G723_LPC_ORDER, &pSrc[G723_SBFR_LEN], AlignBuff, G723_FRM_LEN-G723_SBFR_LEN, 14);
   ippsDotProd_16s32s_Sfs(AlignBuff,AlignBuff,G723_FRM_LEN-G723_SBFR_LEN,&lTmp1,-1);

    /* Scale the residual energy */
    lTmp1 = MulC_32s(2913,  lTmp1 ) ;
   /* Clip noise level */
   if ( vadMem->NoiseLevel > vadMem->PrevEnergy ) {
      lTmp0 = vadMem->PrevEnergy - (vadMem->PrevEnergy>>2);
      vadMem->NoiseLevel = lTmp0 + (vadMem->NoiseLevel>>2);
   }


   /* Update the noise level */
   if ( !vadMem->AdaptEnableFlag ) {
      vadMem->NoiseLevel = vadMem->NoiseLevel + (vadMem->NoiseLevel>>5);
   } else { /* Decay NoiseLevel */
      vadMem->NoiseLevel = vadMem->NoiseLevel - (vadMem->NoiseLevel>>11);
   }

   /* Update previous energy */
   vadMem->PrevEnergy = lTmp1 ;

   /* CLip Noise Level */
   if ( vadMem->NoiseLevel < 0x80 )
       vadMem->NoiseLevel = 0x80 ;
   if ( vadMem->NoiseLevel > 0x1ffff )
       vadMem->NoiseLevel = 0x1ffff ;

   /* Compute the threshold */
   lTmp0 = vadMem->NoiseLevel<<13;
   sTmp0 = Norm_32s_Pos_I(&lTmp0);
   sTmp1 = (Ipp16s)((lTmp0>>15)& 0x7e00);

   lTmp0 = LogAdd_Tbl[sTmp0] - sTmp1 * LogMul_Tbl[sTmp0];
   sTmp1 = (Ipp16s)(lTmp0 >> 15);


   sTmp0 = (Ipp16s)(vadMem->NoiseLevel>>2);
   lTmp0 = sTmp0*sTmp1;
   lTmp0 >>= 10;

   /* threshold */
   if ( lTmp0 > lTmp1 )
       VADResult = 0 ;

   /* update counters */
   if ( VADResult ) {
       vadMem->VADCounter++ ;
       vadMem->HangoverCounter++ ;
   } else {
       vadMem->VADCounter-- ;
       if ( vadMem->VADCounter < 0 )
           vadMem->VADCounter = 0 ;
   }
   if ( vadMem->VADCounter >= 2 ) {
       vadMem->HangoverCounter = 6 ;
       if ( vadMem->VADCounter >= 3 )
           vadMem->VADCounter = 3 ;
   }
   if ( vadMem->HangoverCounter ) {
       VADResult = 1 ;
       if ( vadMem->VADCounter == 0 )
           vadMem->HangoverCounter -- ;
   }
   /* Update periodicity detector */
   vadMem->OpenLoopDelay[0] = vadMem->OpenLoopDelay[2] ;
   vadMem->OpenLoopDelay[1] = vadMem->OpenLoopDelay[3] ;

   *pAdaptEnableFlag = vadMem->AdaptEnableFlag; /* adaptation enable counter  */
   *pVADDecision = VADResult; /* VAD decision : 0 - noise, 1 - voice  */

}
