/*/////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2010 Intel Corporation. All Rights Reserved.
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
// Purpose: G.723.1 speech codec: encode API functions.
//
*/

#include "vadg723.h"
#include "owng723.h"

/* Declaration of local functions */
static void PastAverageFilter_G723(G723Encoder_Obj* encoderObj);
static void GetReflectionCoeff_G723(Ipp16s *pSrcLPC, Ipp16s *pDstReflectCoeff, Ipp16s *pDstReflectCoeffSFS);
static Ipp32s ItakuraDist_G723(Ipp16s *pSrcReflectCoeff, Ipp16s ReflectCoeffSFS, Ipp16s *pSrcAutoCorrs, Ipp16s energy);

static Ipp32s EncoderObjSize(void){
   Ipp32s fltSize;
   Ipp32s objSize = sizeof(G723Encoder_Obj);
   VoiceActivityDetectSize_G723(&fltSize);
   objSize += fltSize; /* VAD decision memory size*/
   return objSize;
}

G723_CODECFUN( APIG723_Status, apiG723Codec_ScratchMemoryAlloc,(Ipp32s *pCodecSize)) {
    if(NULL==pCodecSize)
        return APIG723_StsBadArgErr;
    *pCodecSize = G723_ENCODER_SCRATCH_MEMORY_SIZE;
    return APIG723_StsNoErr;
}

G723_CODECFUN( APIG723_Status, apiG723Encoder_Alloc, (Ipp32s *pCodecSize))
{
   *pCodecSize =  EncoderObjSize();
   return APIG723_StsNoErr;
}
G723_CODECFUN( APIG723_Status, apiG723Encoder_ControlMode,
         (G723Encoder_Obj* encoderObj, Ipp32u mode))
{
   encoderObj->objPrm.mode = mode;

   return APIG723_StsNoErr;
}

G723_CODECFUN( APIG723_Status, apiG723Encoder_Mode,
         (G723Encoder_Obj* encoderObj, Ipp32u mode))
{
   if (G723Encode_VAD_Enabled == mode)
      encoderObj->objPrm.mode |= G723Encode_VAD_Enabled;
   else{
      encoderObj->objPrm.mode |= G723Encode_VAD_Enabled;
      encoderObj->objPrm.mode ^= G723Encode_VAD_Enabled;
   }

   return APIG723_StsNoErr;
}

G723_CODECFUN( APIG723_Status, apiG723Encoder_Init,
              (G723Encoder_Obj* encoderObj, Ipp32u mode))
{
   Ipp32s i;
   Ipp8s* oldMemBuff;

   if(NULL==encoderObj)
      return APIG723_StsBadArgErr;

    oldMemBuff = encoderObj->Mem.base; /* if Reinit */

   ippsZero_16s((Ipp16s*)encoderObj,sizeof(G723Encoder_Obj)>>1) ;

   encoderObj->objPrm.objSize = EncoderObjSize();
   encoderObj->objPrm.mode = mode;
   encoderObj->objPrm.key = G723_ENC_KEY;
   encoderObj->objPrm.rat = 0; /* default 6.3 KBit/s*/

   encoderObj->vadMem  = (Ipp8s*)encoderObj + sizeof(G723Encoder_Obj);

   /* Initialize encoder data structure with zeros */
   ippsZero_16s(encoderObj->ZeroSignal, G723_SBFR_LEN);
   ippsZero_16s(encoderObj->UnitImpulseSignal, G723_SBFR_LEN); encoderObj->UnitImpulseSignal[0] = 0x2000 ; /* unit impulse */

   /* Initialize the previously decoded LSP vector to the DC vector */
   ippsCopy_16s(LSFCTbl,encoderObj->PrevLSF,G723_LPC_ORDER);

   /* Initialize the taming procedure */
   for(i=0; i<5; i++) encoderObj->ExcitationError[i] = 4;
   encoderObj->sSearchTime = 120; /* reset max time */
   /* Initialize the VAD */
   //if(encoderObj->objPrm.mode & G723Encode_VAD_Enabled){
     VoiceActivityDetectInit_G723(encoderObj->vadMem);
     /* Initialize the CNG */
     encoderObj->CurrGain = 0;
     ippsZero_16s(encoderObj->AutoCorrs,AUOTOCORRS_BUFF_SIZE);

     for(i=0; i <= N_AUTOCORRS_BLOCKS; i++) encoderObj->AutoCorrsSFS[i] = 40;
     encoderObj->PrevOpenLoopLags[0] = G723_SBFR_LEN;
     encoderObj->PrevOpenLoopLags[1] = G723_SBFR_LEN;

     ippsZero_16s(encoderObj->LPCSID,G723_LPC_ORDER);
     ippsZero_16s(encoderObj->SIDLSP,G723_LPC_ORDER);
     ippsZero_16s(encoderObj->prevSidLpc,G723_LPC_ORDERP1);
     encoderObj->prevSidLpc[0] = 0x2000;
     ippsZero_16s(encoderObj->ReflectionCoeff,G723_LPC_ORDER+1);
     ippsZero_16s(encoderObj->GainAverEnergies,N_GAIN_AVER_FRMS);

     encoderObj->PastFrameType = G723_ActiveFrm;

     encoderObj->CNGSeed = 12345;
     encoderObj->CasheCounter = 0;
     encoderObj->ReflectionCoeffSFS = 0;
     encoderObj->AverEnerCounter = 0;
     encoderObj->PrevQGain = 0;
     encoderObj->sSidGain = 0;
   //}

   apiG723Encoder_InitBuff(encoderObj,oldMemBuff);

   return APIG723_StsNoErr;
}

G723_CODECFUN( APIG723_Status, apiG723Encoder_InitBuff,
         (G723Encoder_Obj* encoderObj, Ipp8s *buff))
{
   if(NULL==encoderObj) return APIG723_StsBadArgErr;
   if(NULL==buff) return APIG723_StsBadArgErr;

   if(buff)   encoderObj->Mem.base = buff; // otherwise reinit
   else if (encoderObj->Mem.base == NULL) return APIG723_StsNotInitialized;
   encoderObj->Mem.CurPtr = encoderObj->Mem.base;
   encoderObj->Mem.VecPtr = (Ipp32s *)(encoderObj->Mem.base+G723_ENCODER_SCRATCH_MEMORY_SIZE);

   return APIG723_StsNoErr;
}

void EncoderCNG_G723(G723Encoder_Obj* encoderObj, ParamStream_G723 *Params, Ipp16s *pExcitation, Ipp16s *pDstLPC)
{
   Ipp16s sQuantGain, sTmp;
   Ipp32s i;

   LOCAL_ARRAY(Ipp16s, curCoeff,G723_LPC_ORDER,encoderObj) ;

   for(i=N_GAIN_AVER_FRMS-1;i>=1;i--) encoderObj->GainAverEnergies[i]=encoderObj->GainAverEnergies[i-1];

   /* Calculate the LPC filter */
   ippsLevinsonDurbin_G723_16s( encoderObj->AutoCorrs, &sTmp, encoderObj->GainAverEnergies, curCoeff);

   /* if the first frame of silence => SID frame */
   if(encoderObj->PastFrameType == G723_ActiveFrm) {
      Params->FrameType = G723_SIDFrm;
      encoderObj->AverEnerCounter = 1;
      QuantSIDGain_G723_16s(encoderObj->GainAverEnergies, encoderObj->AutoCorrsSFS, encoderObj->AverEnerCounter,&i);
      sQuantGain=(Ipp16s)i;
   } else {
      encoderObj->AverEnerCounter++;
      if(encoderObj->AverEnerCounter > N_GAIN_AVER_FRMS) encoderObj->AverEnerCounter = N_GAIN_AVER_FRMS;
      QuantSIDGain_G723_16s(encoderObj->GainAverEnergies, encoderObj->AutoCorrsSFS, encoderObj->AverEnerCounter,&i);
      sQuantGain=(Ipp16s)i;

      /* Compute stationarity of current filter versus reference filter */
      if(ItakuraDist_G723(encoderObj->ReflectionCoeff, encoderObj->ReflectionCoeffSFS, encoderObj->AutoCorrs, *encoderObj->GainAverEnergies) == 0) {
         Params->FrameType = G723_SIDFrm; /* SID frame */
      } else {
         sTmp = (Ipp16s)Abs_16s(sQuantGain-encoderObj->PrevQGain);
         if(sTmp > 3) {
            Params->FrameType = G723_SIDFrm;/* SID frame */
         } else {
            Params->FrameType = G723_UntransmittedFrm;/* untransmitted */
         }
      }
   }

   if(Params->FrameType == G723_SIDFrm) { /* Compute SID filter */

      LOCAL_ARRAY(Ipp16s, qIndex,3,encoderObj) ;

      /* Check stationarity */

      PastAverageFilter_G723(encoderObj);


      if(!encoderObj->AdaptEnableFlag) /* adaptation enabled */
         for(i=0;i<G723_LPC_ORDER;i++)
            encoderObj->prevSidLpc[i+1] = (Ipp16s)(-encoderObj->LPCSID[i]);

      GetReflectionCoeff_G723(encoderObj->LPCSID , encoderObj->ReflectionCoeff, &encoderObj->ReflectionCoeffSFS);

      if(ItakuraDist_G723(encoderObj->ReflectionCoeff, encoderObj->ReflectionCoeffSFS, encoderObj->AutoCorrs, *encoderObj->GainAverEnergies) == 0){
         ippsCopy_16s(curCoeff,encoderObj->LPCSID,G723_LPC_ORDER);
         GetReflectionCoeff_G723(curCoeff, encoderObj->ReflectionCoeff, &encoderObj->ReflectionCoeffSFS);
      }

      /* Compute SID frame codes */
      ippsLPCToLSF_G723_16s(encoderObj->LPCSID,encoderObj->PrevLSF,encoderObj->SIDLSP);

      ippsLSFQuant_G723_16s32s(encoderObj->SIDLSP, encoderObj->PrevLSF, &Params->lLSPIdx);
      qIndex[2] =  (Ipp16s)(Params->lLSPIdx & 0xff);
      qIndex[1] =  (Ipp16s)((Params->lLSPIdx>>8) & 0xff);
      qIndex[0] =  (Ipp16s)((Params->lLSPIdx>>16) & 0xff);
      if(ippsLSFDecode_G723_16s(qIndex, encoderObj->PrevLSF,  0, encoderObj->SIDLSP) != ippStsNoErr)
         ippsCopy_16s(encoderObj->PrevLSF,encoderObj->SIDLSP,G723_LPC_ORDER);

      Params->sAmpIndex[0] = sQuantGain;
      encoderObj->PrevQGain = sQuantGain;
      DecodeSIDGain_G723_16s(encoderObj->PrevQGain,&encoderObj->sSidGain);

      LOCAL_ARRAY_FREE(Ipp16s, qIndex,3,encoderObj) ;
   }

   /* Compute new excitation */
   if(encoderObj->PastFrameType == G723_ActiveFrm) {
      encoderObj->CurrGain = encoderObj->sSidGain;
   } else {
      encoderObj->CurrGain = (Ipp16s)(( (encoderObj->CurrGain*0xE000)+
                              (encoderObj->sSidGain*0x2000) )>>16) ;
   }
   {

      LOCAL_ARRAY(Ipp8s, buff,ComfortNoiseExcitation_G723_16s_Buff_Size,encoderObj) ;

      ComfortNoiseExcitation_G723_16s(encoderObj->CurrGain, encoderObj->PrevExcitation, pExcitation,
                    &encoderObj->CNGSeed, Params->PitchLag,Params->AdCdbkLag,(Ipp16s*)Params->AdCdbkGain, Params->currRate, buff, &encoderObj->CasheCounter);

      LOCAL_ARRAY_FREE(Ipp8s, buff,ComfortNoiseExcitation_G723_16s_Buff_Size,encoderObj) ;
   }

   LSPInterpolation(encoderObj->SIDLSP, encoderObj->PrevLSF, pDstLPC);/* Interpolate LSPs */
   ippsCopy_16s(encoderObj->SIDLSP,encoderObj->PrevLSF,G723_LPC_ORDER); /* update prev SID LPC */

   encoderObj->PastFrameType = Params->FrameType;
   LOCAL_ARRAY_FREE(Ipp16s, curCoeff,G723_LPC_ORDER,encoderObj) ;
   return;
}

void UpdateAutoCorrs_G723(G723Encoder_Obj* encoderObj, const Ipp16s *pSrcAutoCorrs, const Ipp16s *pSrcAutoCorrsSFS)
{

   Ipp32s i, lNsbfr;
   Ipp16s sMinSFS, sTmp;
   Ipp16s m1, m2;

   LOCAL_ARRAY(Ipp32s, lSumAutoCorrs,G723_LPC_ORDER+1,encoderObj) ;

   /* Update Acf and ShAcf */
   for(i=0; i<AUOTOCORRS_BUFF_SIZE-G723_LPC_ORDER-1; i++) encoderObj->AutoCorrs[AUOTOCORRS_BUFF_SIZE-1-i] = encoderObj->AutoCorrs[AUOTOCORRS_BUFF_SIZE-G723_LPC_ORDER-2-i];

   for(i=N_AUTOCORRS_BLOCKS; i>=1; i--) encoderObj->AutoCorrsSFS[i] = encoderObj->AutoCorrsSFS[i-1];

   /* Search the min of pSrcAutoCorrsSFS */
   m1 = (Ipp16s)IPP_MIN(pSrcAutoCorrsSFS[0],pSrcAutoCorrsSFS[1]);
   m2 = (Ipp16s)IPP_MIN(pSrcAutoCorrsSFS[2],pSrcAutoCorrsSFS[3]);
   sMinSFS = (Ipp16s)((IPP_MIN(m1,m2))+14);

   /* Calculate the acfs sum */
   ippsZero_16s((Ipp16s*)lSumAutoCorrs,2*G723_LPC_ORDERP1);

   for(lNsbfr=0; lNsbfr<4; lNsbfr++) {
      sTmp = (Ipp16s)(sMinSFS - pSrcAutoCorrsSFS[lNsbfr]);
      if(sTmp < 0) {
         sTmp = (Ipp16s)(-sTmp);
         for(i=0; i <= G723_LPC_ORDER; i++) {
            lSumAutoCorrs[i] += (pSrcAutoCorrs[lNsbfr*G723_LPC_ORDERP1+i]>>sTmp);
         }
      } else {
         for(i=0; i <= G723_LPC_ORDER; i++) {
            lSumAutoCorrs[i] += (pSrcAutoCorrs[lNsbfr*G723_LPC_ORDERP1+i]<<sTmp);
         }
      }
   }
   /* Normalize */
   sTmp = Exp_32s_Pos(lSumAutoCorrs[0]);
   sTmp = (Ipp16s)(16 - sTmp); if(sTmp < 0) sTmp = 0;

   for(i=0;i<=G723_LPC_ORDER;i++) encoderObj->AutoCorrs[i]=(Ipp16s)(lSumAutoCorrs[i]>>sTmp);
   encoderObj->AutoCorrsSFS[0] = (Ipp16s)(sMinSFS - sTmp);

   LOCAL_ARRAY_FREE(Ipp32s, lSumAutoCorrs,G723_LPC_ORDER+1,encoderObj) ;

   return;
}

void PastAverageFilter_G723(G723Encoder_Obj* encoderObj)
{
   Ipp32s i, j;
   Ipp16s sMinSFS, sTmp;

   LOCAL_ARRAY(Ipp32s, lSumAutoCorrs,G723_LPC_ORDER+1,encoderObj) ;
   LOCAL_ARRAY(Ipp16s, pCorr,G723_LPC_ORDER+1,encoderObj) ;

   /* Search ShAcf min */
   sMinSFS = (Ipp16s)IPP_MIN(encoderObj->AutoCorrsSFS[1],encoderObj->AutoCorrsSFS[2]);
   sMinSFS = (Ipp16s)((IPP_MIN(sMinSFS,encoderObj->AutoCorrsSFS[3]))+14);

   ippsZero_16s((Ipp16s*)lSumAutoCorrs,2*G723_LPC_ORDERP1);

   for(i=1; i <= N_AUTOCORRS_BLOCKS; i ++) {
      sTmp = (Ipp16s)(sMinSFS - encoderObj->AutoCorrsSFS[i]);
      if(sTmp < 0) {
         sTmp=(Ipp16s)(-sTmp);
         for(j=0; j <= G723_LPC_ORDER; j++) {
            lSumAutoCorrs[j] += (encoderObj->AutoCorrs[i*G723_LPC_ORDERP1+j]>>sTmp);
         }
      } else {
         for(j=0; j <= G723_LPC_ORDER; j++) {
            lSumAutoCorrs[j] += (encoderObj->AutoCorrs[i*G723_LPC_ORDERP1+j]<<sTmp);
         }
      }
   }

   /* Normalize */
   sTmp = Exp_32s_Pos(lSumAutoCorrs[0]);
   sTmp = (Ipp16s)(16 - sTmp);
   if(sTmp < 0) sTmp = 0;

   for(i=0; i<G723_LPC_ORDER+1; i++) {
      pCorr[i] = (Ipp16s)(lSumAutoCorrs[i]>>sTmp);
   }

   ippsLevinsonDurbin_G723_16s(pCorr, &sTmp, &sTmp, encoderObj->LPCSID);

   LOCAL_ARRAY_FREE(Ipp16s, pCorr,G723_LPC_ORDER+1,encoderObj) ;
   LOCAL_ARRAY_FREE(Ipp32s, lSumAutoCorrs,G723_LPC_ORDER+1,encoderObj) ;

   return;
}

void GetReflectionCoeff_G723(Ipp16s *pSrcLPC, Ipp16s *pDstReflectCoeff, Ipp16s *pDstReflectCoeffSFS)
{
   Ipp32s i, j;
   Ipp16s SFS;
   Ipp32s lCorr;

   ippsDotProd_16s32s_Sfs(pSrcLPC,pSrcLPC,G723_LPC_ORDER,&lCorr,-1);
   lCorr = lCorr >> 1;
   lCorr = lCorr + 0x04000000;
   SFS = (Ipp16s)(Exp_32s_Pos(lCorr) - 2);
   *pDstReflectCoeffSFS = SFS;
   if(SFS > 0) {
      lCorr = ShiftL_32s(lCorr, SFS);
      pDstReflectCoeff[0] = Cnvrt_NR_32s16s(lCorr);

      for(i=1; i<=G723_LPC_ORDER; i++) {
         lCorr = -(pSrcLPC[i-1]<<13);
         for(j=0; j<G723_LPC_ORDER-i; j++) {
            lCorr = Add_32s(lCorr, pSrcLPC[j]*pSrcLPC[j+i]);
         }
         lCorr = (Ipp32s)(ShiftL_32s(lCorr, (Ipp16u)(SFS+1)));
         pDstReflectCoeff[i] = Cnvrt_NR_32s16s(lCorr);
      }
   } else {
      SFS = (Ipp16s)(-SFS);
      lCorr = lCorr>>SFS;
      pDstReflectCoeff[0] = Cnvrt_NR_32s16s(lCorr);

      for(i=1; i<=G723_LPC_ORDER; i++) {
         lCorr = -(pSrcLPC[i-1]<<13);
         for(j=0; j<G723_LPC_ORDER-i; j++) {
            lCorr = Add_32s(lCorr, pSrcLPC[j]*pSrcLPC[j+i]);
         }
         lCorr = Mul2_32s(lCorr)>>(SFS);
         pDstReflectCoeff[i] = Cnvrt_NR_32s16s(lCorr);
      }
   }
   return;
}


Ipp32s ItakuraDist_G723(Ipp16s *pSrcReflectCoeff, Ipp16s ReflectCoeffSFS, Ipp16s *pSrcAutoCorrs, Ipp16s energy)
{
    Ipp32s i, lSum, lThresh;

    lSum = 0;
    for(i=0; i <= G723_LPC_ORDER; i++) {
        lSum += pSrcReflectCoeff[i] * (pSrcAutoCorrs[i]>>2);
    }

    lThresh = Cnvrt_32s16s(((energy * 7000)+0x4000)>>15) + energy;
    lThresh <<= (ReflectCoeffSFS + 8);

    /* The condition (lSum < lThresh) has been corrected to the new condition (lSum <= lThresh). */
    /* The g723.1 speech codec has worked with digital silence incorrectly by use the old condition. */
    return ((lSum <= lThresh));
}

G723_CODECFUN(  APIG723_Status, apiG723Encode,
         (G723Encoder_Obj* encoderObj,const Ipp16s* src, Ipp16s rat, Ipp8s* pDstBitStream ))
{
   Ipp32s  i, lNSbfr;

   LOCAL_ALIGN_ARRAY(16, Ipp16s, HPFltSignal,G723_FRM_LEN, encoderObj);
   LOCAL_ALIGN_ARRAY(16, Ipp16s, AlignTmpVec,G723_MAX_PITCH+G723_FRM_LEN, encoderObj);
   LOCAL_ALIGN_ARRAY(16, Ipp16s, CurrLPC,4*G723_LPC_ORDER, encoderObj) ;
   LOCAL_ALIGN_ARRAY(16, Ipp16s, CurrQLPC,4*(G723_LPC_ORDER+1), encoderObj) ;
   LOCAL_ALIGN_ARRAY(16, Ipp16s, WeightedLPC,8*G723_LPC_ORDER, encoderObj) ;
   LOCAL_ALIGN_ARRAY(16, Ipp16s, CurrLSF,G723_LPC_ORDER, encoderObj) ;
   LOCAL_ARRAY(GainInfo_G723, GainInfo,4, encoderObj) ;

   ParamStream_G723 CurrentParams;

   Ipp16s *pData;
   Ipp16s isNotSineWave;

   CurrentParams.FrameType = G723_ActiveFrm;

   if(NULL==encoderObj || NULL==src || NULL ==pDstBitStream)
      return APIG723_StsBadArgErr;
   if(encoderObj->objPrm.objSize <= 0)
      return APIG723_StsNotInitialized;
   if(G723_ENC_KEY != encoderObj->objPrm.key)
      return APIG723_StsBadCodecType;

   if(rat < 0 || rat > 1) {
      rat = (Ipp16s)encoderObj->objPrm.rat;
   } else {
      encoderObj->objPrm.rat = rat;
   }

   CurrentParams.currRate = (G723_Rate)rat;
   if ( CurrentParams.currRate == G723_Rate53)     encoderObj->sSearchTime = 120; /* reset max time */

   CurrentParams.isBadFrame = (Ipp16s) 0 ;

   if ( encoderObj->objPrm.mode&G723Encode_HF_Enabled ) {
      /*    High-pass filtering.   Section 2.3 */
      ippsHighPassFilter_G723_16s(src,HPFltSignal,encoderObj->HPFltMem);
   } else {
      ippsRShiftC_16s(src,1,HPFltSignal,G723_FRM_LEN);
   }

   /* Compute the Unquantized Lpc */
   {

       Ipp16s  sTmp;

       LOCAL_ALIGN_ARRAY(16, Ipp16s, AutoCorrs,(G723_LPC_ORDER+1)*4,encoderObj) ;
       LOCAL_ARRAY(Ipp16s, AutoCorrsSFS,4,encoderObj) ;

       ippsCopy_16s(encoderObj->SignalDelayLine,AlignTmpVec,G723_HALFFRM_LEN);
       ippsCopy_16s(HPFltSignal,&AlignTmpVec[G723_HALFFRM_LEN],G723_FRM_LEN);

       ippsAutoCorr_G723_16s(AlignTmpVec,&AutoCorrsSFS[0],AutoCorrs);
       ippsAutoCorr_G723_16s(&AlignTmpVec[G723_SBFR_LEN],&AutoCorrsSFS[1],&AutoCorrs[G723_LPC_ORDERP1]);
       ippsAutoCorr_G723_16s(&AlignTmpVec[2*G723_SBFR_LEN],&AutoCorrsSFS[2],&AutoCorrs[2*G723_LPC_ORDERP1]);
       ippsAutoCorr_G723_16s(&AlignTmpVec[3*G723_SBFR_LEN],&AutoCorrsSFS[3],&AutoCorrs[3*G723_LPC_ORDERP1]);

       /* LPC calculation for all subframes */

       ippsLevinsonDurbin_G723_16s( AutoCorrs, &encoderObj->SineWaveDetector, &sTmp , CurrLPC);
       ippsLevinsonDurbin_G723_16s( &AutoCorrs[G723_LPC_ORDERP1], &encoderObj->SineWaveDetector, &sTmp , &CurrLPC[G723_LPC_ORDER]);
       ippsLevinsonDurbin_G723_16s( &AutoCorrs[2*G723_LPC_ORDERP1], &encoderObj->SineWaveDetector, &sTmp , &CurrLPC[2*G723_LPC_ORDER]);
       ippsLevinsonDurbin_G723_16s( &AutoCorrs[3*G723_LPC_ORDERP1], &encoderObj->SineWaveDetector, &sTmp , &CurrLPC[3*G723_LPC_ORDER]);

       /* Update sine detector */
       UpdateSineDetector(&encoderObj->SineWaveDetector, &isNotSineWave);
       /* Update CNG Acf memories */
       UpdateAutoCorrs_G723(encoderObj, AutoCorrs, AutoCorrsSFS);

       LOCAL_ARRAY_FREE(Ipp16s, AutoCorrsSFS,4,encoderObj) ;
       LOCAL_ALIGN_ARRAY_FREE(16, Ipp16s, AutoCorrs,(G723_LPC_ORDER+1)*4,encoderObj) ;

   }
   /* Convert to Lsp */
   ippsLPCToLSF_G723_16s(&CurrLPC[G723_LPC_ORDER*3], encoderObj->PrevLSF, CurrLSF);

   /* Compute the Vad */
   CurrentParams.FrameType = G723_ActiveFrm;
   if( encoderObj->objPrm.mode&G723Encode_VAD_Enabled){
      VoiceActivityDetect_G723(HPFltSignal, (Ipp16s*)&encoderObj->prevSidLpc,(Ipp16s*)&encoderObj->PrevOpenLoopLags,
         isNotSineWave,&i,&encoderObj->AdaptEnableFlag,encoderObj->vadMem,AlignTmpVec);
      CurrentParams.FrameType = (G723_FrameType)i;
   }

   /* VQ Lsp vector */

   ippsLSFQuant_G723_16s32s(CurrLSF, encoderObj->PrevLSF, &CurrentParams.lLSPIdx);

   ippsCopy_16s(&encoderObj->SignalDelayLine[G723_SBFR_LEN],AlignTmpVec,G723_SBFR_LEN);
   ippsCopy_16s(&HPFltSignal[2*G723_SBFR_LEN],encoderObj->SignalDelayLine,2*G723_SBFR_LEN);
   ippsCopy_16s(HPFltSignal,&AlignTmpVec[G723_SBFR_LEN],3*G723_SBFR_LEN);
   ippsCopy_16s(AlignTmpVec,HPFltSignal,G723_FRM_LEN);

   /* Compute Perceptual filter Lpc coefficients */

    for ( i = 0 ; i < 4 ; i ++ ) {
       /* Compute the FIR and IIR coefficient of the perceptual weighting filter */
       ippsMul_NR_16s_Sfs(&CurrLPC[i*G723_LPC_ORDER],PerceptualFltCoeffTbl,
                                    &WeightedLPC[2*i*G723_LPC_ORDER],G723_LPC_ORDER,15);
       ippsMul_NR_16s_Sfs(&CurrLPC[i*G723_LPC_ORDER],&PerceptualFltCoeffTbl[G723_LPC_ORDER],
                                    &WeightedLPC[(2*i+1)*G723_LPC_ORDER],G723_LPC_ORDER,15);
       /* do filtering */
       ippsIIR16s_G723_16s_I(&WeightedLPC[2*i*G723_LPC_ORDER],&HPFltSignal[i*G723_SBFR_LEN],encoderObj->WeightedFltMem);
    }


   /* Compute Open loop pitch estimates */

   ippsCopy_16s(encoderObj->PrevWeightedSignal,AlignTmpVec,G723_MAX_PITCH);
   ippsCopy_16s(HPFltSignal,&AlignTmpVec[G723_MAX_PITCH],G723_FRM_LEN);


   i=3;
   ippsAutoScale_16s_I( AlignTmpVec, G723_MAX_PITCH+G723_FRM_LEN, &i);

   ippsOpenLoopPitchSearch_G723_16s( &AlignTmpVec[G723_MAX_PITCH], &CurrentParams.PitchLag[0]);
   ippsOpenLoopPitchSearch_G723_16s( &AlignTmpVec[G723_MAX_PITCH+(2*G723_SBFR_LEN)], &CurrentParams.PitchLag[1]);

   encoderObj->PrevOpenLoopLags[0] = CurrentParams.PitchLag[0];
   encoderObj->PrevOpenLoopLags[1] = CurrentParams.PitchLag[1];

   if(CurrentParams.FrameType != G723_ActiveFrm) {

      ippsCopy_16s(&HPFltSignal[G723_FRM_LEN-G723_MAX_PITCH],encoderObj->PrevWeightedSignal,G723_MAX_PITCH);
      EncoderCNG_G723(encoderObj, &CurrentParams,HPFltSignal, CurrQLPC);

      /* change the ringing delays */
      pData = HPFltSignal;

      for( i = 0 ; i < 4; i++ ) {

         LOCAL_ARRAY(Ipp32s, V_AccS,G723_SBFR_LEN,encoderObj) ;
         /* Update exc_err */
         ErrorUpdate_G723(encoderObj->ExcitationError, CurrentParams.PitchLag[i>>1], CurrentParams.AdCdbkLag[i], CurrentParams.AdCdbkGain[i],CurrentParams.currRate);

         /* Shift the harmonic noise shaping filter memory */
         ///>>>_ippsCopy_16s(&encoderObj->FltSignal[G723_SBFR_LEN],encoderObj->FltSignal,G723_MAX_PITCH-G723_SBFR_LEN);
         ippsMove_16s(&encoderObj->FltSignal[G723_SBFR_LEN],encoderObj->FltSignal,G723_MAX_PITCH-G723_SBFR_LEN);
         /* Combined filtering */
         ippsCopy_16s(encoderObj->RingSynthFltMem,encoderObj->RingWeightedFltMem,G723_LPC_ORDER);
         ippsSynthesisFilter_G723_16s32s(&CurrQLPC[i*(G723_LPC_ORDER+1)],
                pData,V_AccS,encoderObj->RingSynthFltMem);

         ippsIIR16s_G723_32s16s_Sfs(&WeightedLPC[i*2*G723_LPC_ORDER],V_AccS,0,
                &(encoderObj->FltSignal[G723_MAX_PITCH-G723_SBFR_LEN]),encoderObj->RingWeightedFltMem);

         pData += G723_SBFR_LEN;
         LOCAL_ARRAY_FREE(Ipp32s, V_AccS,G723_SBFR_LEN,encoderObj) ;
      }
   }

   else {

      /* Active frame */
      ippsHarmonicSearch_G723_16s(CurrentParams.PitchLag[0], &AlignTmpVec[G723_MAX_PITCH], &GainInfo[0].sDelay, &GainInfo[0].sGain);
      ippsHarmonicSearch_G723_16s(CurrentParams.PitchLag[0], &AlignTmpVec[G723_MAX_PITCH+G723_SBFR_LEN], &GainInfo[1].sDelay, &GainInfo[1].sGain);
      ippsHarmonicSearch_G723_16s(CurrentParams.PitchLag[1], &AlignTmpVec[G723_MAX_PITCH+2*G723_SBFR_LEN], &GainInfo[2].sDelay, &GainInfo[2].sGain);
      ippsHarmonicSearch_G723_16s(CurrentParams.PitchLag[1], &AlignTmpVec[G723_MAX_PITCH+3*G723_SBFR_LEN], &GainInfo[3].sDelay, &GainInfo[3].sGain);

      ippsCopy_16s(encoderObj->PrevWeightedSignal,AlignTmpVec,G723_MAX_PITCH);
      ippsCopy_16s(HPFltSignal,&AlignTmpVec[G723_MAX_PITCH],G723_FRM_LEN);

      ippsCopy_16s(&AlignTmpVec[G723_FRM_LEN],encoderObj->PrevWeightedSignal,G723_MAX_PITCH);

      ippsHarmonicFilter_NR_16s((Ipp16s)(-GainInfo[0].sGain),GainInfo[0].sDelay,&AlignTmpVec[G723_MAX_PITCH],
                                                          HPFltSignal,G723_SBFR_LEN);
      ippsHarmonicFilter_NR_16s((Ipp16s)(-GainInfo[1].sGain),GainInfo[1].sDelay,&AlignTmpVec[G723_MAX_PITCH+G723_SBFR_LEN],
                                                          &HPFltSignal[G723_SBFR_LEN],G723_SBFR_LEN);
      ippsHarmonicFilter_NR_16s((Ipp16s)(-GainInfo[2].sGain),GainInfo[2].sDelay,&AlignTmpVec[G723_MAX_PITCH+2*G723_SBFR_LEN],
                                                          &HPFltSignal[2*G723_SBFR_LEN],G723_SBFR_LEN);
      ippsHarmonicFilter_NR_16s((Ipp16s)(-GainInfo[3].sGain),GainInfo[3].sDelay,&AlignTmpVec[G723_MAX_PITCH+3*G723_SBFR_LEN],
                                                          &HPFltSignal[3*G723_SBFR_LEN],G723_SBFR_LEN);

      {
         LOCAL_ARRAY(Ipp16s, qIndex,3,encoderObj) ;
         /* Inverse quantization of the LSP */
         qIndex[2] =  (Ipp16s)(CurrentParams.lLSPIdx & 0xff);
         qIndex[1] =  (Ipp16s)((CurrentParams.lLSPIdx>>8) & 0xff);
         qIndex[0] =  (Ipp16s)((CurrentParams.lLSPIdx>>16) & 0xff);
         if(ippsLSFDecode_G723_16s(qIndex, encoderObj->PrevLSF, CurrentParams.isBadFrame, CurrLSF) != ippStsNoErr)
            ippsCopy_16s(encoderObj->PrevLSF,CurrLSF,G723_LPC_ORDER);
         LOCAL_ARRAY_FREE(Ipp16s, qIndex,3,encoderObj) ;
      }

      /* Interpolate the Lsp vectors */
      LSPInterpolation(CurrLSF, encoderObj->PrevLSF, CurrQLPC) ;

      /* Copy the LSP vector for the next frame */
      ippsCopy_16s(CurrLSF,encoderObj->PrevLSF,G723_LPC_ORDER);

      /* sub frame processing */
      pData = HPFltSignal ;

      for ( lNSbfr = 0 ; lNSbfr < 4 ; lNSbfr ++ ) {

         LOCAL_ALIGN_ARRAY(16, Ipp16s, SynDl,G723_LPC_ORDER,encoderObj) ; /* synthesis filter delay line */
         LOCAL_ALIGN_ARRAY(16, Ipp16s, RingWgtDl,2*G723_LPC_ORDER,encoderObj) ;/* formant perceptual weighting filter delay line */
         LOCAL_ALIGN_ARRAY(16, Ipp32s, V_AccS,G723_SBFR_LEN,encoderObj) ;
         LOCAL_ALIGN_ARRAY(16, Ipp16s, ImpResp,G723_SBFR_LEN,encoderObj) ;

         /* Compute full impulse response */
         ippsZero_16s(SynDl,G723_LPC_ORDER); /* synthesis filter zero delay */

         ippsSynthesisFilter_G723_16s32s(&CurrQLPC[lNSbfr*(G723_LPC_ORDER+1)],encoderObj->UnitImpulseSignal,V_AccS,SynDl);

         {

            LOCAL_ALIGN_ARRAY(16, Ipp16s, Temp,G723_MAX_PITCH+G723_SBFR_LEN,encoderObj) ;

            ippsZero_16s(RingWgtDl,2*G723_LPC_ORDER);/* formant perceptual weighting filter zero delay */
            ippsIIR16s_G723_32s16s_Sfs(&WeightedLPC[lNSbfr*2*G723_LPC_ORDER],V_AccS,1,
                                                         &Temp[G723_MAX_PITCH],RingWgtDl);

            ippsZero_16s(Temp,G723_MAX_PITCH);/* harmonic filter zero delay */
            ippsHarmonicFilter_NR_16s((Ipp16s)(-GainInfo[lNSbfr].sGain),GainInfo[lNSbfr].sDelay,&Temp[G723_MAX_PITCH],ImpResp,G723_SBFR_LEN);

            LOCAL_ALIGN_ARRAY_FREE(16, Ipp16s, Temp,G723_MAX_PITCH+G723_SBFR_LEN,encoderObj) ;
         }

         /* Subtract the ringing of previous sub-frame */
         ippsCopy_16s(encoderObj->RingSynthFltMem,SynDl,G723_LPC_ORDER);
         /*  Synthesis filter of zero input  */
         ippsSynthesisFilter_G723_16s32s(&CurrQLPC[lNSbfr*(G723_LPC_ORDER+1)],encoderObj->ZeroSignal,V_AccS,SynDl);

         ippsCopy_16s(encoderObj->RingSynthFltMem,RingWgtDl,G723_LPC_ORDER);/* FIR same as for synth filter */
         ippsCopy_16s(&encoderObj->RingWeightedFltMem[G723_LPC_ORDER],&RingWgtDl[G723_LPC_ORDER],G723_LPC_ORDER);/* IIR part*/
         ippsIIR16s_G723_32s16s_Sfs(&WeightedLPC[lNSbfr*2*G723_LPC_ORDER],V_AccS,0,
                                    &(encoderObj->FltSignal[G723_MAX_PITCH]),RingWgtDl);

         /* Do the harmonic noise shaping filter with subtraction the result
           from the harmonic noise weighted vector.*/
         ippsHarmonicNoiseSubtract_G723_16s_I((Ipp16s)(-GainInfo[lNSbfr].sGain),GainInfo[lNSbfr].sDelay,
                                              &(encoderObj->FltSignal[G723_MAX_PITCH]),pData);
         /* Shift the harmonic noise shaping filter memory */
         ///>>>_ippsCopy_16s(&encoderObj->FltSignal[G723_SBFR_LEN],encoderObj->FltSignal,G723_MAX_PITCH-G723_SBFR_LEN);
         ippsMove_16s(&encoderObj->FltSignal[G723_SBFR_LEN],encoderObj->FltSignal,G723_MAX_PITCH-G723_SBFR_LEN);

         /*  Adaptive codebook contribution to exitation residual.  Section 2.14. */
         {
            Ipp16s  sCloseLag;
            Ipp16s sPitchLag = CurrentParams.PitchLag[lNSbfr>>1] ;

            LOCAL_ALIGN_ARRAY(16, Ipp16s, RezBuf,G723_SBFR_LEN+4,encoderObj) ;

            if ( (lNSbfr & 1 ) == 0 ) { /* For even frames only */
                if ( sPitchLag ==  G723_MIN_PITCH ) sPitchLag++;
                if ( sPitchLag > (G723_MAX_PITCH-5) ) sPitchLag = G723_MAX_PITCH-5 ;
            }

            ippsAdaptiveCodebookSearch_G723(sPitchLag, pData, ImpResp, encoderObj->PrevExcitation, encoderObj->ExcitationError,
               &sCloseLag, &CurrentParams.AdCdbkGain[lNSbfr], (Ipp16s)lNSbfr, isNotSineWave, SA_Rate[CurrentParams.currRate]);

            /* Modify sPitchLag for even sub frames */
            if ( (lNSbfr & 1 ) ==  0 ) {
                sPitchLag = (Ipp16s)(sPitchLag - 1 + sCloseLag) ;
                sCloseLag = 1 ;
            }
            CurrentParams.AdCdbkLag[lNSbfr] = sCloseLag ;
            CurrentParams.PitchLag[lNSbfr>>1] = sPitchLag ;

            ippsDecodeAdaptiveVector_G723_16s(sPitchLag, sCloseLag, CurrentParams.AdCdbkGain[lNSbfr], encoderObj->PrevExcitation, RezBuf, SA_Rate[CurrentParams.currRate]);

            /* subtract the contribution of the pitch predictor decoded to obtain the residual */
            ExcitationResidual_G723_16s(RezBuf,ImpResp,pData,encoderObj);

            LOCAL_ALIGN_ARRAY_FREE(16, Ipp16s, RezBuf,G723_SBFR_LEN+4,encoderObj) ;
         }
         /* Compute fixed code book contribution */
         FixedCodebookSearch_G723_16s(encoderObj, &CurrentParams, pData, ImpResp, (Ipp16s) lNSbfr) ;

         ippsDecodeAdaptiveVector_G723_16s(CurrentParams.PitchLag[lNSbfr>>1], CurrentParams.AdCdbkLag[lNSbfr], CurrentParams.AdCdbkGain[lNSbfr], encoderObj->PrevExcitation,
                                                         ImpResp, SA_Rate[CurrentParams.currRate]);

         ///>>>_ippsCopy_16s(&encoderObj->PrevExcitation[G723_SBFR_LEN],encoderObj->PrevExcitation,G723_MAX_PITCH-G723_SBFR_LEN);
         ippsMove_16s(&encoderObj->PrevExcitation[G723_SBFR_LEN],encoderObj->PrevExcitation,G723_MAX_PITCH-G723_SBFR_LEN);

         for ( i = 0 ; i < G723_SBFR_LEN ; i ++ ) {
            pData[i] = Cnvrt_32s16s( Mul2_16s(pData[i])+ImpResp[i]);
         }
         ippsCopy_16s(pData,&encoderObj->PrevExcitation[G723_MAX_PITCH-G723_SBFR_LEN],G723_SBFR_LEN);

         /* Update exc_err */
         ErrorUpdate_G723(encoderObj->ExcitationError, CurrentParams.PitchLag[lNSbfr>>1], CurrentParams.AdCdbkLag[lNSbfr], CurrentParams.AdCdbkGain[lNSbfr],CurrentParams.currRate);

         /* Update the ringing delays by passing excitation through the combined filter.*/
         for(i=0; i<G723_LPC_ORDER; i++){
            encoderObj->RingWeightedFltMem[i] = encoderObj->RingSynthFltMem[i]; /* FIR same as for synth filter */
         }
         ippsSynthesisFilter_G723_16s32s(&CurrQLPC[lNSbfr*(G723_LPC_ORDER+1)],
               pData,V_AccS,encoderObj->RingSynthFltMem);
         ippsIIR16s_G723_32s16s_Sfs(&WeightedLPC[lNSbfr*2*G723_LPC_ORDER],V_AccS,0,
                                    &(encoderObj->FltSignal[G723_MAX_PITCH-G723_SBFR_LEN]),encoderObj->RingWeightedFltMem);
         pData += G723_SBFR_LEN ;

         LOCAL_ALIGN_ARRAY_FREE(16, Ipp16s, ImpResp,G723_SBFR_LEN,encoderObj) ;
         LOCAL_ALIGN_ARRAY_FREE(16, Ipp32s, V_AccS,G723_SBFR_LEN,encoderObj) ;
         LOCAL_ALIGN_ARRAY_FREE(16, Ipp16s, RingWgtDl,2*G723_LPC_ORDER,encoderObj) ;/* formant perceptual weighting filter delay line */
         LOCAL_ALIGN_ARRAY_FREE(16, Ipp16s, SynDl,G723_LPC_ORDER,encoderObj) ; /* synthesis filter delay line */
      }  /* end of subframes loop */

      encoderObj->PastFrameType = G723_ActiveFrm;
      encoderObj->CNGSeed = 12345;
      encoderObj->CasheCounter = 0;

   }

   /* Pack to the bitstream */
   SetParam2Bitstream(encoderObj, &CurrentParams, pDstBitStream);

   CLEAR_SCRATCH_MEMORY(encoderObj);

   return APIG723_StsNoErr;

}
