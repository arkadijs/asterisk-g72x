/*/////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2004-2012 Intel Corporation. All Rights Reserved.
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
// Purpose: G.729 floating-point speech codec: internal functions.
//
*/

#include <math.h>
#include <stdlib.h>
#include "vadg729fp.h"
#include "aux_fnxs.h"
#include "owng729fp.h"

#define         sqr(a)          ((a)*(a))

/* Hamming_cos window for LPC analysis.           */

static __ALIGN32 CONST Ipp32f HammingWindow[WINDOW_LEN] = {        /* hamming-cosine window */
0.08000000f, 0.08005703f, 0.08022812f, 0.08051321f,
0.08091225f, 0.08142514f, 0.08205172f, 0.08279188f,
0.08364540f, 0.08461212f, 0.08569173f, 0.08688401f,
0.08818865f, 0.08960532f, 0.09113365f, 0.09277334f,
0.09452391f, 0.09638494f, 0.09835598f, 0.10043652f,
0.10262608f, 0.10492408f, 0.10732999f, 0.10984316f,
0.11246302f, 0.11518890f, 0.11802010f, 0.12095598f,
0.12399574f, 0.12713866f, 0.13038395f, 0.13373083f,
0.13717847f, 0.14072597f, 0.14437246f, 0.14811710f,
0.15195890f, 0.15589692f, 0.15993017f, 0.16405767f,
0.16827843f, 0.17259133f, 0.17699537f, 0.18148938f,
0.18607232f, 0.19074300f, 0.19550033f, 0.20034306f,
0.20527001f, 0.21027996f, 0.21537170f, 0.22054392f,
0.22579536f, 0.23112471f, 0.23653066f, 0.24201185f,
0.24756692f, 0.25319457f, 0.25889328f, 0.26466170f,
0.27049842f, 0.27640197f, 0.28237087f, 0.28840363f,
0.29449883f, 0.30065489f, 0.30687031f, 0.31314352f,
0.31947297f, 0.32585713f, 0.33229437f, 0.33878314f,
0.34532180f, 0.35190874f, 0.35854232f, 0.36522087f,
0.37194279f, 0.37870640f, 0.38550997f, 0.39235184f,
0.39923036f, 0.40614375f, 0.41309035f, 0.42006844f,
0.42707625f, 0.43411207f, 0.44117412f, 0.44826069f,
0.45537004f, 0.46250033f, 0.46964988f, 0.47681686f,
0.48399949f, 0.49119604f, 0.49840465f, 0.50562358f,
0.51285106f, 0.52008528f, 0.52732444f, 0.53456670f,
0.54181033f, 0.54905349f, 0.55629444f, 0.56353134f,
0.57076240f, 0.57798582f, 0.58519983f, 0.59240264f,
0.59959245f, 0.60676748f, 0.61392599f, 0.62106609f,
0.62818617f, 0.63528436f, 0.64235890f, 0.64940804f,
0.65643007f, 0.66342324f, 0.67038584f, 0.67731601f,
0.68421221f, 0.69107264f, 0.69789559f, 0.70467937f,
0.71142232f, 0.71812278f, 0.72477907f, 0.73138952f,
0.73795253f, 0.74446648f, 0.75092971f, 0.75734061f,
0.76369762f, 0.76999915f, 0.77624369f, 0.78242958f,
0.78855544f, 0.79461962f, 0.80062068f, 0.80655706f,
0.81242740f, 0.81823015f, 0.82396388f, 0.82962728f,
0.83521879f, 0.84073710f, 0.84618086f, 0.85154873f,
0.85683930f, 0.86205131f, 0.86718345f, 0.87223446f,
0.87720311f, 0.88208807f, 0.88688827f, 0.89160240f,
0.89622939f, 0.90076804f, 0.90521723f, 0.90957582f,
0.91384280f, 0.91801709f, 0.92209762f, 0.92608339f,
0.92997342f, 0.93376678f, 0.93746245f, 0.94105959f,
0.94455731f, 0.94795465f, 0.95125085f, 0.95444512f,
0.95753652f, 0.96052444f, 0.96340811f, 0.96618676f,
0.96885973f, 0.97142631f, 0.97388595f, 0.97623801f,
0.97848189f, 0.98061699f, 0.98264289f, 0.98455900f,
0.98636484f, 0.98806006f, 0.98964417f, 0.99111670f,
0.99247742f, 0.99372596f, 0.99486196f, 0.99588519f,
0.99679530f, 0.99759221f, 0.99827564f, 0.99884540f,
0.99930143f, 0.99964350f, 0.99987161f, 0.99998569f,
1.00000000f, 0.99921930f, 0.99687845f, 0.99298108f,
0.98753333f, 0.98054361f, 0.97202289f, 0.96198452f,
0.95044410f, 0.93741965f, 0.92293155f, 0.90700239f,
0.88965708f, 0.87092263f, 0.85082841f, 0.82940567f,
0.80668795f, 0.78271067f, 0.75751126f, 0.73112911f,
0.70360541f, 0.67498308f, 0.64530689f, 0.61462307f,
0.58297962f, 0.55042595f, 0.51701277f, 0.48279238f,
0.44781810f, 0.41214463f, 0.37582767f, 0.33892387f,
0.30149087f, 0.26358715f, 0.22527184f, 0.18660481f,
0.14764643f, 0.10845750f, 0.06909923f, 0.02963307f};

void ownAutoCorr_G729_32f(Ipp32f *pSrc, Ipp32s len, Ipp32f *pDst, Ipp32f *pExtBuff)
{
   ippsMul_32f(pSrc, HammingWindow, pExtBuff, WINDOW_LEN);

   ippsAutoCorr_32f(pExtBuff, WINDOW_LEN, pDst, len+1);

   if (pDst[0]<(Ipp32f)1.0) pDst[0]=(Ipp32f)1.0;

   return;
}

void ownACOS_G729_32f(Ipp32f *pSrc, Ipp32f *pDst, Ipp32s len)
{

   Ipp32s i;

   for (i=0; i<len; i++ )
      pDst[i] = (Ipp32f)acos(pSrc[i]);
   return;
}

void ownCOS_G729_32f(Ipp32f *pSrc, Ipp32f *pDst, Ipp32s len)
{

   Ipp32s i;

   for (i=0; i<len; i++ )
      pDst[i] = (Ipp32f)cos(pSrc[i]);
   return;
}


Ipp32f ownAdaptiveCodebookGainCoeff_G729_32f(Ipp32f *pSrcTargetVector, Ipp32f *pSrcFltAdaptivCdbkVec,
               Ipp32f *pDstCorrCoeff, Ipp32s len)
{
    Ipp32f fCorr, fEnergy, fGain;
    Ipp64f sum;

    /* energy of filtered excitation */
    ippsDotProd_32f64f(pSrcFltAdaptivCdbkVec, pSrcFltAdaptivCdbkVec, len, &sum);
    fEnergy = (Ipp32f)(sum + 0.01);

    ippsDotProd_32f64f(pSrcTargetVector, pSrcFltAdaptivCdbkVec, len, &sum);
    fCorr = (Ipp32f)sum;

    pDstCorrCoeff[0] = fEnergy;
    pDstCorrCoeff[1] = -2.0f*fCorr +0.01f;

    /* find pitch fGain and bound it by [0,1.2] */

    fGain = fCorr/fEnergy;

    CLIP_TO_LOWLEVEL(fGain,0.f);
    CLIP_TO_UPLEVEL(fGain,GAIN_PIT_MAX);

    return fGain;
}


void WeightLPCCoeff_G729(Ipp32f *pSrcLPC, Ipp32f valWeightingFactor, Ipp32s len, Ipp32f *pDstWeightedLPC)
{
  Ipp32f fFactor;
  Ipp32s i;

  pDstWeightedLPC[0]=pSrcLPC[0];
  fFactor=valWeightingFactor;
  for (i = 1; i < len; i++)
  {
    pDstWeightedLPC[i] = fFactor*pSrcLPC[i];
    fFactor *= valWeightingFactor;
  }
  pDstWeightedLPC[len] = fFactor*pSrcLPC[len];
  return;
}

void AdaptiveCodebookGainCoeff_G729_32f( Ipp32f *pSrcTargetVector, Ipp32f *pSrcFltAdaptiveCodebookVector,
                                         Ipp32f *pSrcFltInnovation, Ipp32f *pDstCoeff)
{
   Ipp64f sum;

   ippsDotProd_32f64f(pSrcFltInnovation, pSrcFltInnovation, SUBFR_LEN, &sum);
   pDstCoeff[2] = (Ipp32f)(sum + 0.01);

   ippsDotProd_32f64f(pSrcTargetVector, pSrcFltInnovation, SUBFR_LEN, &sum);
   pDstCoeff[3] = (Ipp32f)(-2.0*(sum + 0.01));

   ippsDotProd_32f64f(pSrcFltAdaptiveCodebookVector, pSrcFltInnovation, SUBFR_LEN, &sum);
   pDstCoeff[4] = (Ipp32f)(2.0*(sum + 0.01));

   return;

}

Ipp32s ownAdaptiveCodebookSearch_G729A_32f(Ipp32f *pSrcExc, Ipp32f *pSrcTargetVector, Ipp32f *pSrcImpulseResponse,
  Ipp32s minPitchDelay, Ipp32s maxPitchDelay, Ipp32s nSbfr, Ipp32s *fracPartPitchDelay, Ipp32f *pExtBuff)
{
  Ipp32s  pitchPeriod;
  Ipp32f *pCorr;
  Ipp32f *pTmpExcitation;
  Ipp64f  corr, max;
  Ipp32s delayLine[2];

  pCorr = &pExtBuff[0];
  pTmpExcitation = &pExtBuff[SUBFR_LEN];

  /* Compute  correlations of input response with the target vector.*/

  ippsCrossCorr_32f(pSrcImpulseResponse, SUBFR_LEN, pSrcTargetVector, SUBFR_LEN, pCorr,  SUBFR_LEN, 0);

  /* Find maximum integer delay */

  ippsCrossCorrLagMax_32f64f(pCorr, &pSrcExc[-maxPitchDelay], SUBFR_LEN, maxPitchDelay-minPitchDelay, &max, &pitchPeriod);
  pitchPeriod = (maxPitchDelay-minPitchDelay-pitchPeriod) + minPitchDelay;

  /* Test fractions */

  /* Fraction 0 */
  delayLine[0] = pitchPeriod;
  delayLine[1] = 0;
  ippsDecodeAdaptiveVector_G729_32f_I(delayLine, pSrcExc);
  ippsDotProd_32f64f( pCorr, pSrcExc, SUBFR_LEN, &max);
  *fracPartPitchDelay = 0;

  /* If first subframe and lag > 84 do not search fractional pitch */

  if( (nSbfr == 0) && (pitchPeriod > 84) )
    return pitchPeriod;

  ippsCopy_32f(pSrcExc, pTmpExcitation, SUBFR_LEN);

  /* Fraction -1/3 */

  delayLine[1] = -1;

  ippsDecodeAdaptiveVector_G729_32f_I(delayLine, pSrcExc);
  ippsDotProd_32f64f( pCorr, pSrcExc, SUBFR_LEN, &corr);
  if(corr > max){
    max = corr;
    *fracPartPitchDelay = -1;
    ippsCopy_32f(pSrcExc, pTmpExcitation, SUBFR_LEN);
  }

  /* Fraction +1/3 */

  delayLine[1] = 1;
  ippsDecodeAdaptiveVector_G729_32f_I(delayLine, pSrcExc);
  ippsDotProd_32f64f( pCorr, pSrcExc, SUBFR_LEN, &corr);
  if(corr > max){
    *fracPartPitchDelay = 1;
  }
  else
    ippsCopy_32f(pTmpExcitation, pSrcExc, SUBFR_LEN);

  return pitchPeriod;
}

Ipp32s  ExtractBitsG729FP( const Ipp8u **pBits, Ipp32s *nBit, Ipp32s Count )
{
    Ipp32s  i ;
    Ipp32s  Rez = 0L ;

    for ( i = 0 ; i < Count ; i ++ ){
        Ipp32s  fTmp ;
        fTmp = ((*pBits)[(i + *nBit)>>3] >> (7 - ((i + *nBit) & 0x0007)) ) & 1;
        Rez <<= 1 ;
        Rez += fTmp ;
    }

    *pBits += (Count + *nBit)>>3;
    *nBit = (Count + *nBit) & 0x0007;

    return Rez ;
}

void PWGammaFactor_G729(Ipp32f *pGamma1, Ipp32f *pGamma2, Ipp32f *pIntLSF, Ipp32f *CurrLSF,
                   Ipp32f *ReflectCoeff, Ipp32s   *isFlat, Ipp32f *PrevLogAreaRatioCoeff)
{
   Ipp32f   logAreaRatioCoeff[4];
   Ipp32f  *logAreaRatioCoeffNew;
   Ipp32f  *lsf;
   Ipp32f   minDist, fTmp;
   Ipp32s   i, k;

   logAreaRatioCoeffNew = &logAreaRatioCoeff[2];

   /* Convert reflection coefficients to the Log Area Ratio coefficient*/
   for (i=0; i<2; i++)
      logAreaRatioCoeffNew[i] = (Ipp32f)log10( (Ipp64f)( ( 1.0f + ReflectCoeff[i]) / (1.0f - ReflectCoeff[i])));

   /* Interpolation of lar for the 1st subframe */
   for (i=0; i<2; i++) {
      logAreaRatioCoeff[i] = 0.5f * (logAreaRatioCoeffNew[i] + PrevLogAreaRatioCoeff[i]);
      PrevLogAreaRatioCoeff[i] = logAreaRatioCoeffNew[i];
   }

   for (k=0; k<2; k++) {    /* LOOP : gamma2 for 1st to 2nd subframes */
      if (*isFlat != 0) {
         if ((logAreaRatioCoeff[2*k] <LAR_THRESH1 )&&(logAreaRatioCoeff[2*k+1] > LAR_THRESH3)) *isFlat = 0;
      } else {
         if ((logAreaRatioCoeff[2*k] > LAR_THRESH2)||(logAreaRatioCoeff[2*k+1] < LAR_THRESH4)) *isFlat = 1;
      }

      if (*isFlat == 0) {
         /* Second criterion based on the minimum distance between two successives lsfs. */
         pGamma1[k] = GAMMA1_TILTED;
         if (k == 0) lsf = pIntLSF;
         else lsf = CurrLSF;

         minDist = lsf[1] - lsf[0];
         for (i=1; i<LPC_ORDER-1; i++) {
            fTmp = lsf[i+1] - lsf[i];
            if (fTmp < minDist) minDist = fTmp;
         }

         pGamma2[k] =  GAMMA2_TILTED_SCALE * minDist + GAMMA2_TILTED_SHIFT;

         if (pGamma2[k] > GAMMA2_TILTED_MAX) pGamma2[k] = GAMMA2_TILTED_MAX;
         if (pGamma2[k] < GAMMA2_TILTED_MIN) pGamma2[k] = GAMMA2_TILTED_MIN;
      } else {
         pGamma1[k] = GAMMA1_FLAT;
         pGamma2[k] = GAMMA2_FLAT;
      }
   }
   return;
}


void CodewordImpConv_G729_32f(Ipp32s index, const Ipp32f *pSrc1,const Ipp32f *pSrc2,Ipp32f *pDst)
{
   Ipp32s i;
   Ipp32s lPos0, lPos1, lPos2, lPos3; /* position*/
   Ipp32s lSign0, lSign1, lSign2, lSign3;     /*signs: 1,-1*/

   lPos0 = index & 0x7;
   lPos1 = (index>>3) & 0x7;
   lPos2 = (index>>6) & 0x7;
   lPos3 = index>>9;

   lPos0 = (lPos0<<2)+lPos0;                      /* lPos0*5;*/
   lPos1 = (lPos1<<2)+lPos1+1;                    /* 1+lPos1*5;*/
   lPos2 = (lPos2<<2)+lPos2+2;                    /* 2+lPos2*5;*/
   lPos3 = ((lPos3>>1)<<2)+(lPos3>>1)+(lPos3&1)+3;  /* 3+(lPos3>>1)*5+(lPos3&1);*/

   if (lPos0>lPos1) {i=lPos0; lPos0=lPos1; lPos1=i; }
   if (lPos2>lPos3) {i=lPos2; lPos2=lPos3; lPos3=i; }
   if (lPos0>lPos2) {i=lPos0; lPos0=lPos2; lPos2=i; }
   if (lPos1>lPos3) {i=lPos1; lPos1=lPos3; lPos3=i; }
   if (lPos1>lPos2) {i=lPos1; lPos1=lPos2; lPos2=i; }

   lSign0 = (pSrc1[lPos0] > 0)? 1:-1;
   lSign1 = (pSrc1[lPos1] > 0)? 1:-1;
   lSign2 = (pSrc1[lPos2] > 0)? 1:-1;
   lSign3 = (pSrc1[lPos3] > 0)? 1:-1;

   for (i=0; i<lPos0; i++)
      pDst[i]=0;
   for (; i<lPos1; i++)
      pDst[i]=lSign0*pSrc2[i-lPos0];
   for (; i<lPos2; i++)
      pDst[i]=lSign0*pSrc2[i-lPos0]+lSign1*pSrc2[i-lPos1];
   for (; i<lPos3; i++)
      pDst[i]=lSign0*pSrc2[i-lPos0]+lSign1*pSrc2[i-lPos1]+lSign2*pSrc2[i-lPos2];
   for (; i<SUBFR_LEN; i++)
      pDst[i]=lSign0*pSrc2[i-lPos0]+lSign1*pSrc2[i-lPos1]+lSign2*pSrc2[i-lPos2]+lSign3*pSrc2[i-lPos3];
}

void MSDGetSize(Ipp32s *pDstSize)
{
   *pDstSize = sizeof(CNGmemory);
   return;
}

void MSDInit(Ipp8s *msdMem)
{
   MusDetectMemory *msdState = (MusDetectMemory *)msdMem;

   ippsZero_16s((Ipp16s*)msdState,sizeof(MusDetectMemory)>>1) ;
   ippsZero_32f(msdState->MeanRC,10);
   msdState->lMusicCounter=0;
   msdState->fMusicCounter=0.0f;
   msdState->lZeroMusicCounter=0;
   msdState->fMeanPitchGain =0.5f;
   msdState->lPFlagCounter=0;
   msdState->fMeanPFlagCounter=0.0;
   msdState->lConscPFlagCounter=0;
   msdState->lRCCounter=0;
   msdState->fMeanFullBandEnergy =0.0f;

   return;

}

void MusicDetection_G729E_32f(G729FPEncoder_Obj *encoderObj, G729Codec_Type codecType, Ipp32f Energy,
                              Ipp32f *ReflectCoeff, Ipp32s *VadDecision, Ipp32f LLenergy, Ipp8s *msdMem,Ipp32f *pExtBuff)
{

    Ipp32s i;
    Ipp32f fSum1, fSum2,fStandartDeviation;
    Ipp16s VoicingStrenght1, VoicingStrenght2, VoicingStrenght;
    Ipp32f fError, fEnergy , fSpectralDifference, *pTmpVec;
    Ipp32f fThreshold;
    MusDetectMemory *msdState = (MusDetectMemory *)msdMem;

    pTmpVec = &pExtBuff[0]; /*10 elements*/

    fError = 1.0f;
    for (i=0; i< 4; i++) fError *= (1.0f - ReflectCoeff[i]*ReflectCoeff[i]);
    ippsSub_32f(msdState->MeanRC, ReflectCoeff, pTmpVec, 10);
    ippsDotProd_32f(pTmpVec, pTmpVec, 10, &fSpectralDifference);

    fEnergy = 10.0f*(Ipp32f)log10(fError*Energy/240.0f +IPP_MINABS_32F);

    if( *VadDecision == VAD_NOISE ){
       ippsInterpolateC_G729_32f(msdState->MeanRC, 0.9f, ReflectCoeff, 0.1f, msdState->MeanRC, 10);
       msdState->fMeanFullBandEnergy = 0.9f * msdState->fMeanFullBandEnergy + 0.1f * fEnergy;
    }

    fSum1 = 0.0f;
    fSum2 = 0.0f;
    for(i=0; i<5; i++){
        fSum1 += (Ipp32f) encoderObj->LagBuffer[i];
        fSum2 +=  encoderObj->PitchGainBuffer[i];
    }

    fSum1 = fSum1/5.0f;
    fSum2 = fSum2/5.0f;
    fStandartDeviation =0.0f;
    for(i=0; i<5; i++) fStandartDeviation += sqr(((Ipp32f) encoderObj->LagBuffer[i] - fSum1));
    fStandartDeviation = (Ipp32f)sqrt(fStandartDeviation/4.0f);

    msdState->fMeanPitchGain = 0.8f * msdState->fMeanPitchGain + 0.2f * fSum2;
    /* See I.5.1.1 Pitch lag smoothness and voicing strenght indicator.*/
    if ( codecType == G729D_CODEC)
        fThreshold = 0.73f;
    else
        fThreshold = 0.63f;

    if ( msdState->fMeanPitchGain > fThreshold)
        VoicingStrenght2 = 1;
    else
        VoicingStrenght2 = 0;

    if ( fStandartDeviation < 1.30f && msdState->fMeanPitchGain > 0.45f )
        VoicingStrenght1 = 1;
    else
        VoicingStrenght1 = 0;

    VoicingStrenght= (Ipp16s)( ((Ipp16s)encoderObj->prevVADDec & (Ipp16s)(VoicingStrenght1 | VoicingStrenght2))| (Ipp16s)(VoicingStrenght2));

    if( ReflectCoeff[1] <= 0.45f && ReflectCoeff[1] >= 0.0f && msdState->fMeanPitchGain < 0.5f)
        msdState->lRCCounter++;
    else
        msdState->lRCCounter =0;

    if( encoderObj->prevLPCMode== 1 && (*VadDecision == VAD_VOICE))
        msdState->lMusicCounter++;

    if ((encoderObj->sFrameCounter%64) == 0 ){
        if( encoderObj->sFrameCounter == 64)
            msdState->fMusicCounter = (Ipp32f)msdState->lMusicCounter;
        else
            msdState->fMusicCounter = 0.9f*msdState->fMusicCounter + 0.1f*(Ipp32f)msdState->lMusicCounter;
    }

    if( msdState->lMusicCounter == 0)
        msdState->lZeroMusicCounter++;
    else
        msdState->lZeroMusicCounter = 0;

    if( msdState->lZeroMusicCounter > 500 || msdState->lRCCounter > 150) msdState->fMusicCounter = 0.0f;

    if ((encoderObj->sFrameCounter%64) == 0)
        msdState->lMusicCounter = 0;

    if( VoicingStrenght== 1 )
        msdState->lPFlagCounter++;

    if ((encoderObj->sFrameCounter%64) == 0 ){
        if( encoderObj->sFrameCounter == 64)
            msdState->fMeanPFlagCounter = (Ipp32f)msdState->lPFlagCounter;
        else{
            if( msdState->lPFlagCounter > 25)
                msdState->fMeanPFlagCounter = 0.98f * msdState->fMeanPFlagCounter + 0.02f * msdState->lPFlagCounter;
            else if( msdState->lPFlagCounter > 20)
                msdState->fMeanPFlagCounter = 0.95f * msdState->fMeanPFlagCounter + 0.05f * msdState->lPFlagCounter;
            else
                msdState->fMeanPFlagCounter = 0.90f * msdState->fMeanPFlagCounter + 0.10f * msdState->lPFlagCounter;
        }
    }

    if( msdState->lPFlagCounter == 0)
        msdState->lConscPFlagCounter++;
    else
        msdState->lConscPFlagCounter = 0;

    if( msdState->lConscPFlagCounter > 100 || msdState->lRCCounter > 150) msdState->fMeanPFlagCounter = 0.0f;

    if ((encoderObj->sFrameCounter%64) == 0)
        msdState->lPFlagCounter = 0;

    if (codecType == G729E_CODEC){
        if( fSpectralDifference > 0.15f && (fEnergy -msdState->fMeanFullBandEnergy)> 4.0f && (LLenergy> 50.0) )
            *VadDecision =VAD_VOICE;
        else if( (fSpectralDifference > 0.38f || (fEnergy -msdState->fMeanFullBandEnergy)> 4.0f  ) && (LLenergy> 50.0f))
            *VadDecision =VAD_VOICE;
        else if( (msdState->fMeanPFlagCounter >= 10.0f || msdState->fMusicCounter >= 5.0f || encoderObj->sFrameCounter < 64)&& (LLenergy> 7.0))
            *VadDecision =VAD_VOICE;
    }
    return;
}

void PitchTracking_G729FPE(Ipp32s *pitchDelay, Ipp32s *fracPitchDelay, Ipp32s *prevPitchDelay, Ipp32s *stat_N,
                         Ipp32s *lStatPitch2PT, Ipp32s *lStatFracPT)
{
    Ipp32s pitchDistance, minDist, lPitchMult;
    Ipp32s j, distSign;

    pitchDistance = (*pitchDelay) - (*prevPitchDelay);
    if(pitchDistance < 0) {
        distSign = 0;
        pitchDistance = - pitchDistance;
    } else {
        distSign = 1;
    }

    /* Test pitch stationarity */
    if (pitchDistance < 5) {
        (*stat_N)++;
        if (*stat_N > 7) *stat_N = 7 ;
        *lStatPitch2PT = *pitchDelay;
        *lStatFracPT = *fracPitchDelay;
    } else {
        /* Find multiples or sub-multiples */
        minDist =  pitchDistance;
        if( distSign == 0) {
            lPitchMult = 2 * (*pitchDelay);
            for (j=2; j<5; j++) {
                pitchDistance = abs(lPitchMult - (*prevPitchDelay));
                if (pitchDistance <= minDist) {
                    minDist = pitchDistance;
                }
                lPitchMult += (*pitchDelay);
            }
        } else {
            lPitchMult = 2 * (*prevPitchDelay);
            for (j=2; j<5; j++) {
                pitchDistance = abs(lPitchMult - (*pitchDelay));
                if (pitchDistance <= minDist) {
                    minDist = pitchDistance;
                }
                lPitchMult += (*prevPitchDelay);
            }
        }
        if (minDist < 5) {   /* Multiple or sub-multiple detected */
            if (*stat_N > 0) {
                *pitchDelay      = *lStatPitch2PT;
                *fracPitchDelay = *lStatFracPT;
            }
            *stat_N -= 1;
            if (*stat_N < 0) *stat_N = 0 ;
        } else {
            *stat_N = 0;    /* No (sub-)multiple detected  => Pitch transition */
            *lStatPitch2PT = *pitchDelay;
            *lStatFracPT = *fracPitchDelay;
        }
    }

    *prevPitchDelay = *pitchDelay;

    return;
}

void OpenLoopPitchSearch_G729_32f(const Ipp32f *pSrc, Ipp32s* lBestLag)
{
   Ipp32f fTmp;
   Ipp64f dTmp;

   Ipp32f fMax1, fMax2, fMax3;
   Ipp32s max1Idx, max2Idx, max3Idx;

   /*  Find a maximum for three sections and compare the maxima
       of each section by favoring small lag.      */

   /*  First section:  lag delay = PITCH_LAG_MAX to 80                   */
   ippsAutoCorrLagMax_32f(pSrc, FRM_LEN, 80,PITCH_LAG_MAX+1, &fMax1, &max1Idx);
   /*  Second section: lag delay = 79 to 40                              */
   ippsAutoCorrLagMax_32f(pSrc, FRM_LEN, 40,80, &fMax2, &max2Idx);
   /*  Third section:  lag delay = 39 to 20                              */
   ippsAutoCorrLagMax_32f(pSrc, FRM_LEN, PITCH_LAG_MIN,40, &fMax3, &max3Idx);

   ippsDotProd_32f64f(&pSrc[-max1Idx], &pSrc[-max1Idx], FRM_LEN, &dTmp);
   fTmp = (Ipp32f) (1.0f / sqrt(dTmp+0.01f));
   fMax1 = (Ipp32f)(fMax1) * fTmp;        /* max/sqrt(energy)  */

   ippsDotProd_32f64f(&pSrc[-max2Idx], &pSrc[-max2Idx], FRM_LEN, &dTmp);

   fTmp = (Ipp32f) (1.0f / sqrt(dTmp+0.01));
   fMax2 = (Ipp32f)(fMax2) * fTmp; /* max/sqrt(energy)  */

   /* Calc energy */
   ippsDotProd_32f64f(&pSrc[-max3Idx], &pSrc[-max3Idx], FRM_LEN, &dTmp);
   /* 1/sqrt(energy)    */
   fTmp = 1.0f / (Ipp32f)sqrt(dTmp+0.01);
   fMax3 = (Ipp32f)(fMax3) * fTmp;        /* max/sqrt(energy)  */

   /* Compare the 3 sections maxima and choose the small one. */
   if ( fMax1 * PITCH_THRESH < fMax2 ) {
      fMax1 = fMax2;
      max1Idx = max2Idx;
   }

   if ( fMax1 * PITCH_THRESH < fMax3 )  max1Idx = max3Idx;

   *lBestLag = max1Idx;

   return;
}

Ipp32s TestErrorContribution_G729(Ipp32s valPitchDelay, Ipp32s valFracPitchDelay, Ipp32f *ExcErr)
{

    Ipp32s j, lTmp, l1, l2, lTaming;
    Ipp32f maxErrExc;

    lTmp = (valFracPitchDelay > 0) ? (valPitchDelay+1) : valPitchDelay;

    j = lTmp - SUBFR_LEN - INTER_PITCH_LEN;
    if(j < 0) j = 0;
    l1 =  (Ipp32s) (j * INV_SUBFR_LEN);

    j = lTmp + INTER_PITCH_LEN - 2;
    l2 =  (Ipp32s) (j * INV_SUBFR_LEN);

    maxErrExc = -1.f;
    lTaming = 0 ;
    for(j=l2; j>=l1; j--) {
        if(ExcErr[j] > maxErrExc) maxErrExc = ExcErr[j];
    }
    if(maxErrExc > THRESH_ERR) {
        lTaming = 1;
    }
    return(lTaming);
}

void UpdateExcErr_G729(Ipp32f valPitchGain, Ipp32s valPitchDelay, Ipp32f *pExcErr)
{
    Ipp32s i, l1, l2, n;
    Ipp32f fMax, fTmp;

    fMax = -1.f;

    n = valPitchDelay- SUBFR_LEN;
    if(n < 0) {
        fTmp = 1.f + valPitchGain * pExcErr[0];
        if(fTmp > fMax) fMax = fTmp;
        fTmp = 1.f + valPitchGain * fTmp;
        if(fTmp > fMax) fMax = fTmp;
    } else {
        l1 = (Ipp32s) (n * INV_SUBFR_LEN);

        i = valPitchDelay - 1;
        l2 = (Ipp32s) (i * INV_SUBFR_LEN);

        for(i = l1; i <= l2; i++) {
            fTmp = 1.f + valPitchGain * pExcErr[i];
            if(fTmp > fMax) fMax = fTmp;
        }
    }

    for(i=3; i>=1; i--) pExcErr[i] = pExcErr[i-1];
    pExcErr[0] = fMax;

    return;
}

void isBackwardModeDominant_G729(Ipp32s *isBackwardModeDominant, Ipp32s LPCMode, Ipp32s *pCounterBackward, Ipp32s *pCounterForward)
{

    Ipp32s lTmp, lCounter;

    if (LPCMode == 0) (*pCounterForward)++;
    else (*pCounterBackward)++;

    lCounter = *pCounterBackward + *pCounterForward;

    if (lCounter == 100) {
        lCounter = lCounter >> 1;
        *pCounterBackward = (*pCounterBackward) >> 1;
        *pCounterForward = (*pCounterForward) >> 1;
    }

    *isBackwardModeDominant = 0;
    if (lCounter >= 10) {
        lTmp = (*pCounterForward) << 2;
        if (*pCounterBackward > lTmp) *isBackwardModeDominant = 1;
    }

    return;
}

Ipp32f CalcEnergy_dB_G729(Ipp32f *pSrc, Ipp32s len)
{
   Ipp64f  dEnergy;
   Ipp32f fEnergydB;
   Ipp32s n, k, lTmp;

   ippsDotProd_32f64f(pSrc, pSrc, len, &dEnergy);
   dEnergy += 0.0001;
   fEnergydB = (Ipp32f)log10(dEnergy);
   n = (Ipp32s) (fEnergydB * INVERSE_LOG2);
   if(n >= 4) {
      if(dEnergy > 2147483647.) dEnergy = 93.1814;
      else {
         k = (Ipp32s)dEnergy;
         lTmp = -(1 << (n-4));
         k &= lTmp;
         dEnergy = 10. * log10((Ipp32f)k);
      }
   }
   else dEnergy = 0.005;

   return (Ipp32f)dEnergy;
}

void InterpolatedBackwardFilter_G729(Ipp32f *pSrcDstLPCBackwardFlt, Ipp32f *pSrcPrevFilter, Ipp32f *pSrcDstIntCoeff)
{
    Ipp32s i;
    Ipp32f s1, s2;
    Ipp32f *pBwdLPC;
    Ipp32f fIntFactor;

    pBwdLPC = pSrcDstLPCBackwardFlt + BWD_LPC_ORDERP1;

    /* Calculate the interpolated filters  */
    fIntFactor = *pSrcDstIntCoeff - 0.1f;
    if( fIntFactor < 0) fIntFactor = 0;

    for (i=0; i<BWD_LPC_ORDERP1; i++) {
        s1 = pBwdLPC[i] * (1.f - fIntFactor);
        s2 = pSrcPrevFilter[i] * fIntFactor;
        pBwdLPC[i] = s1 + s2;
    }
    //ippsInterpolateC_G729_32f(pBwdLPC, (1.f - fIntFactor), pSrcPrevFilter, fIntFactor, pBwdLPC, BWD_LPC_ORDERP1);

    for (i=0; i<BWD_LPC_ORDERP1; i++) {
        pSrcDstLPCBackwardFlt[i] = 0.5f * (pBwdLPC[i] + pSrcPrevFilter[i]);
    }
    //ippsInterpolateC_G729_32f(pBwdLPC, 0.5f, pSrcPrevFilter, 0.5f, pBwdLPC, BWD_LPC_ORDERP1);

    *pSrcDstIntCoeff = fIntFactor;
    return;
}

/* anti-sparseness post-processing */
static __ALIGN32 CONST Ipp32f ImpLow[SUBFR_LEN]={
   0.4483f, 0.3515f, 0.0387f,-0.0843f,-0.1731f, 0.2293f,-0.0011f,
   -0.0857f,-0.0928f, 0.1472f, 0.0901f,-0.2571f, 0.1155f, 0.0444f,
   0.0665f,-0.2636f, 0.2457f,-0.0642f,-0.0444f, 0.0237f, 0.0338f,
   -0.0728f, 0.0688f,-0.0111f,-0.0206f,-0.0642f, 0.1845f,-0.1734f,
   0.0327f, 0.0953f,-0.1544f, 0.1621f,-0.0711f,-0.1138f, 0.2113f,
   -0.1187f, 0.0206f,-0.0542f, 0.0009f,0.3096f
};

static __ALIGN32 CONST Ipp32f ImpMiddle[SUBFR_LEN]={
   0.9239f, 0.1169f, -0.1232f, 0.0907f, -0.0320f, -0.0306f, 0.0756f,
   -0.0929f, 0.0859f, -0.0681f, 0.0535f, -0.0492f, 0.0523f, -0.0542f,
   0.0471f, -0.0308f, 0.0131f, -0.0052f, 0.0144f, -0.0386f, 0.0664f,
   -0.0826f, 0.0770f, -0.0495f, 0.0105f, 0.0252f, -0.0467f, 0.0526f,
   -0.0506f, 0.0519f, -0.0630f, 0.0807f, -0.0934f, 0.0884f, -0.0604f,
   0.0170f, 0.0238f, -0.0418f, 0.0257f, 0.0200f
};

static __ALIGN32 CONST Ipp32f ImpHigh[SUBFR_LEN]={
   1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};


void PHDGetSize(Ipp32s *pDstSize)
{
   *pDstSize = sizeof(PHDmemory);
   return;
}

void PHDInit(Ipp8s *phdMem)
{
   PHDmemory *phdState = (PHDmemory *)phdMem;

   ippsZero_32f(phdState->gainMem,6);
   phdState->prevDispState = 0;
   phdState->prevCbGain = 0.;
   phdState->onset = 0;
}

void PhaseDispersionUpdate_G729D(Ipp32f valPitchGain, Ipp32f valCodebookGain, Ipp8s *phdMem)
{
    Ipp32s i;
    PHDmemory *phdState = (PHDmemory *)phdMem;

    for (i = 5; i > 0; i--) phdState->gainMem[i] = phdState->gainMem[i-1];
    phdState->gainMem[0] = valPitchGain;
    phdState->prevDispState = 2;
    phdState->prevCbGain = valCodebookGain;
    phdState->onset = 0;

    return;
}

void PhaseDispersion_G729D(Ipp32f *pSrcExcSignal, Ipp32f *pDstFltExcSignal, Ipp32f valCodebookGain,
                           Ipp32f valPitchGain, Ipp32f *pSrcDstInnovation, Ipp8s *phdMem,Ipp8s *pExtBuff)
{
    Ipp32s  i;
    PHDmemory *phdState = (PHDmemory *)phdMem;

    Ipp32f *pScaledLTP;
    Ipp32f *pMemory;
    Ipp32s *pPos;
    Ipp32s numNonZeroElem, nPulse, i1, lPos;
    Ipp32s phDispState;
    const Ipp32f *pTable=NULL;

    pScaledLTP = (Ipp32f *)pExtBuff;
    pMemory = (Ipp32f *)(pExtBuff + SUBFR_LEN*sizeof(Ipp32f));
    pPos = (Ipp32s *)(pMemory + SUBFR_LEN*sizeof(Ipp32f));

    /* anti-sparseness post-processing */
    ippsAdaptiveCodebookContribution_G729_32f(valCodebookGain, pSrcDstInnovation, pSrcExcSignal, pScaledLTP);
    ippsCopy_32f(pSrcDstInnovation,pMemory,SUBFR_LEN);
    ippsZero_32f(pSrcDstInnovation,SUBFR_LEN);

    numNonZeroElem=0;
    for (i=0; i<SUBFR_LEN; i++) {
        if (pMemory[i]) /*Can't change to if(fabs(pMemory[i]) < IPP_MINABS_32F)*/
            pPos[numNonZeroElem++] = i;
    }
    if (valPitchGain <= 0.6f) {
        phDispState = 0;
    } else if ( (valPitchGain > 0.6f) && (valPitchGain < 0.9f) ) {
        phDispState = 1;
    } else {
        phDispState = 2;
    }

    for (i = 5; i > 0; i--) {
        phdState->gainMem[i]=phdState->gainMem[i-1];
    }
    phdState->gainMem[0] = valPitchGain;

    if (valCodebookGain > 2.0f * phdState->prevCbGain)
        phdState->onset = 2;
    else {
        if (phdState->onset) phdState->onset -= 1;
    }

    i1=0;
    for (i = 0; i < 6; i++) {
        if (phdState->gainMem[i] < 0.6f) i1 += 1;
    }
    if (i1 > 2 && !phdState->onset) phDispState = 0;

    if (phDispState - phdState->prevDispState > 1 && !phdState->onset) phDispState -= 1;

    if (phdState->onset) {
        if (phDispState < 2) phDispState++;
    }

    phdState->prevDispState=phDispState;
    phdState->prevCbGain = valCodebookGain;

    if (phDispState == 0) {
       pTable = ImpLow;
    } else if (phDispState == 1) {
       pTable = ImpMiddle;
    } else if (phDispState == 2) {
       pTable = ImpHigh;
    }

    for (nPulse=0; nPulse<numNonZeroElem; nPulse++) {
      lPos = pPos[nPulse];
      for (i=lPos; i<SUBFR_LEN; i++)
         pSrcDstInnovation[i] += pMemory[lPos] * pTable[i-lPos];
      for (i=0; i < lPos; i++)
         pSrcDstInnovation[i] += pMemory[lPos] * pTable[SUBFR_LEN-lPos+i];
    }

    ippsAdaptiveCodebookContribution_G729_32f(-valCodebookGain, pSrcDstInnovation, pScaledLTP, pDstFltExcSignal);

    return;
}

static void GlobalStationnarityAdaptation_G729E(G729FPEncoder_Obj* encoderObj, Ipp32f valBackwardPredGain, Ipp32f valForwardPredGain, Ipp32s valLPCMode)
{
    Ipp16s sTmp;
    /* First adaptation based on previous backward / forward decisions */
    if (valLPCMode == 1) { /* Backward stationnary mode */

        (encoderObj->sBWDStatInd)++;
        CLIP_TO_UPLEVEL(encoderObj->sBWDStatInd,21);
        if(encoderObj->sValBWDStatInd < 32517) encoderObj->sValBWDStatInd  += 250;
        else encoderObj->sValBWDStatInd = 32767;

        /* after 20 backward frames => increase stat */
        if (encoderObj->sBWDStatInd == 20) {
            if(encoderObj->sGlobalStatInd < 30267) encoderObj->sGlobalStatInd += 2500;
            else encoderObj->sGlobalStatInd = 32767;
        }
        else if (encoderObj->sBWDStatInd > 20) encoderObj->sGlobalStatInd += 500;

    }

    else if ((valLPCMode == 0)&&(encoderObj->prevLPCMode == 1)) { /* Backward -> Forward transition */

        /* Transition occurs after less than 20 backward frames => decrease stat */
        if (encoderObj->sBWDStatInd < 20) {
            sTmp = (Ipp16s)(5000 - encoderObj->sValBWDStatInd);
            encoderObj->sGlobalStatInd = (Ipp16s)(encoderObj->sGlobalStatInd-sTmp);
        }

        /* Reset consecutive backward frames counter */
        encoderObj->sBWDStatInd = 0;
        encoderObj->sValBWDStatInd = 0;

    }

    /* Second adaptation based on prediction gains */

    if (encoderObj->sGlobalStatInd < 13000) {

        if      (valBackwardPredGain > valForwardPredGain + TH4) encoderObj->sGlobalStatInd += 3200;
        else if (valBackwardPredGain > valForwardPredGain + TH3) encoderObj->sGlobalStatInd += 2400;
        else if (valBackwardPredGain > valForwardPredGain + TH2) encoderObj->sGlobalStatInd += 1600;
        else if (valBackwardPredGain > valForwardPredGain + TH1) encoderObj->sGlobalStatInd +=  800;
        else if (valBackwardPredGain > valForwardPredGain)       encoderObj->sGlobalStatInd +=  400;

    }

    if      (valBackwardPredGain < valForwardPredGain -  TH5) encoderObj->sGlobalStatInd -= 6400;
    else if (valBackwardPredGain < valForwardPredGain -  TH4) encoderObj->sGlobalStatInd -= 3200;
    else if (valBackwardPredGain < valForwardPredGain -  TH3) encoderObj->sGlobalStatInd -= 1600;
    else if (valBackwardPredGain < valForwardPredGain -  TH2) encoderObj->sGlobalStatInd -=  800;
    else if (valBackwardPredGain < valForwardPredGain -  TH1) encoderObj->sGlobalStatInd -=  400;

    CLIP_TO_UPLEVEL(encoderObj->sGlobalStatInd,32000);
    CLIP_TO_LOWLEVEL(encoderObj->sGlobalStatInd,0);
    return;
}

void SetLPCMode_G729FPE(G729FPEncoder_Obj* encoderObj, Ipp32f *pSrcSignal, Ipp32f *pSrcForwardLPCFilter,
                      Ipp32f *pSrcBackwardLPCFilter, Ipp32s *pDstLPCMode, Ipp32f *pSrcLSP,Ipp32f *pExtBuff)
{

    Ipp32s  i;
    Ipp32f *pLPCFlt, *PPtr;
    Ipp32f fGap, forwardPredGain, backwardPredGain, intBackwardPredGain;
    Ipp32f LSPThreshold, LSPDist, fTmp;
    Ipp32f ener_DB_pSrcSignal;

    PPtr = &pExtBuff[0]; /*FRM_LEN elements*/

    ener_DB_pSrcSignal = CalcEnergy_dB_G729(pSrcSignal, FRM_LEN);

    pLPCFlt = pSrcBackwardLPCFilter + BWD_LPC_ORDERP1;
    /* Calc backward filter prediction gain (without interpolation ) */
    ippsConvBiased_32f(pLPCFlt,BWD_LPC_ORDER+1,pSrcSignal,FRM_LEN+BWD_LPC_ORDER,PPtr,FRM_LEN,BWD_LPC_ORDER);
    backwardPredGain = ener_DB_pSrcSignal - CalcEnergy_dB_G729(PPtr, FRM_LEN);

    /* Interpolated backward filter for the first sub-frame       */
    InterpolatedBackwardFilter_G729(pSrcBackwardLPCFilter, encoderObj->PrevFlt, &encoderObj->fInterpolationCoeff);

    /* Calc interpolated backward filter prediction gain */
    ippsConvBiased_32f(pSrcBackwardLPCFilter,BWD_LPC_ORDER+1,pSrcSignal,SUBFR_LEN+BWD_LPC_ORDER,PPtr,SUBFR_LEN,BWD_LPC_ORDER);
    ippsConvBiased_32f(pLPCFlt,BWD_LPC_ORDER+1,&pSrcSignal[SUBFR_LEN],SUBFR_LEN+BWD_LPC_ORDER,&PPtr[SUBFR_LEN],SUBFR_LEN,BWD_LPC_ORDER);
    intBackwardPredGain = ener_DB_pSrcSignal - CalcEnergy_dB_G729(PPtr, FRM_LEN);

    /* Calc forward filter prediction gain */
    ippsConvBiased_32f(pSrcForwardLPCFilter,LPC_ORDER+1,pSrcSignal,SUBFR_LEN+LPC_ORDER,PPtr,SUBFR_LEN,LPC_ORDER);
    ippsConvBiased_32f(&pSrcForwardLPCFilter[LPC_ORDERP1],LPC_ORDER+1,&pSrcSignal[SUBFR_LEN],SUBFR_LEN+LPC_ORDER,&PPtr[SUBFR_LEN],SUBFR_LEN,LPC_ORDER);
    forwardPredGain = ener_DB_pSrcSignal - CalcEnergy_dB_G729(PPtr, FRM_LEN);

    /* Choose: backward/forward mode.*/
    /* 1st criterion with prediction gains. The global stationarity index
         is used to adapt the threshold value " GAP ".*/

    /* Do the threshold adaptation according to the global stationarity indicator */
    fGap = (Ipp32f)(encoderObj->sGlobalStatInd) * GAP_FACT;
    fGap += 1.f;

    if ( (intBackwardPredGain > forwardPredGain - fGap)&& (backwardPredGain > forwardPredGain - fGap)&&
         (backwardPredGain > 0.f)               && (intBackwardPredGain > 0.f) ) *pDstLPCMode = 1;
    else *pDstLPCMode = 0;

    if (encoderObj->sGlobalStatInd < 13000) *pDstLPCMode = 0; /* => Forward mode imposed */

    /* 2nd criterion with a distance between 2 successive LSP vectors */
    /* Computation of the LPC distance */
    LSPDist = 0;
    for(i=0; i<LPC_ORDER; i++){
        fTmp = encoderObj->OldLSP[i] - pSrcLSP[i];
        LSPDist += fTmp * fTmp;
    }

    /* Adaptation of the LSPs thresholds */
    if (encoderObj->sGlobalStatInd < 32000) {
        LSPThreshold = 0.f;
    }
    else {
        LSPThreshold = 0.03f;
    }

    /* Switching backward -> forward forbidden in case of a LPC stationnary */
    if ((LSPDist < LSPThreshold) &&(*pDstLPCMode == 0)&&(encoderObj->prevLPCMode == 1)
         &&(backwardPredGain > 0.f)&&(intBackwardPredGain > 0.f)) {
        *pDstLPCMode = 1;
    }

    /* Low energy frame => Forward mode chosen */

    if (ener_DB_pSrcSignal < THRES_ENERGY) {
        *pDstLPCMode = 0;
        if (encoderObj->sGlobalStatInd > 13000) encoderObj->sGlobalStatInd = 13000;
    }
    else isBackwardModeDominant_G729(&encoderObj->isBWDDominant, *pDstLPCMode,&encoderObj->sBWDFrmCounter,&encoderObj->sFWDFrmCounter);

    /* Adaptation of the global stationarity indicator */
    if (ener_DB_pSrcSignal >= THRES_ENERGY) GlobalStationnarityAdaptation_G729E(encoderObj,backwardPredGain, forwardPredGain, *pDstLPCMode);
    if(*pDstLPCMode == 0) encoderObj->fInterpolationCoeff = 1.1f;
    return;
}

static void NormalizedCorrelation(Ipp32f *pSrcExc, Ipp32f *pSrcTargetVector, Ipp32f *pSrcImpulseResponse,
                                  Ipp32s len, Ipp32s lagMin, Ipp32s lagMax, Ipp32f *pDstCorr, Ipp32f *pTmpFltPastExc)
{
   Ipp32s i, k;
   Ipp64f dEnergy, dTmp;

   k = -lagMin;

   ippsConvBiased_32f( &pSrcExc[k], len , pSrcImpulseResponse, len ,  pTmpFltPastExc, len ,0);

   /* loop for every possible period */
   for (i = lagMin; i < lagMax; i++) {
      /* Compute energy of pFltExc[] */
      ippsDotProd_32f64f(pTmpFltPastExc, pTmpFltPastExc, len, &dEnergy);

      /* Compute correlation between pSrcTargetVector[] and pFltExc[] */
      ippsDotProd_32f64f(pSrcTargetVector, pTmpFltPastExc, len, &dTmp);

      /* Normalize correlation = correlation * (1/sqrt(energy)) */
      pDstCorr[i] = (Ipp32f)(dTmp)/((Ipp32f)sqrt(dEnergy+0.01));

      /* modify the filtered excitation pFltExc[] for the next iteration */
      k--;
      ippsFilteredExcitation_G729_32f( pSrcImpulseResponse, pTmpFltPastExc, len, pSrcExc[k]);
   }

   /* Compute energy of pFltExc[] */
   ippsDotProd_32f64f(pTmpFltPastExc, pTmpFltPastExc, len, &dEnergy);

   /* Compute correlation between pSrcTargetVector[] and pFltExc[] */
   ippsDotProd_32f64f(pSrcTargetVector, pTmpFltPastExc, len, &dTmp);

   /* Normalize correlation = correlation * (1/sqrt(energy)) */
   pDstCorr[lagMax] = (Ipp32f)(dTmp)/((Ipp32f)sqrt(dEnergy+0.01));

   return;

}

static __ALIGN32 CONST Ipp32f ReorderInter_3[2*3*INTERPOL4_LEN] = {
 0.015738f,-0.047624f, 0.084078f, 0.900839f,
 0.084078f,-0.047624f, 0.015738f, 0.000000f,
 0.000000f, 0.016285f,-0.105570f, 0.760084f,
 0.424082f,-0.121120f, 0.031217f,-0.005925f,
-0.005925f, 0.031217f,-0.121120f, 0.424082f,
 0.760084f,-0.105570f, 0.016285f, 0.000000f
};

static Ipp32f Interpolation_3(Ipp32f *pSrc, Ipp32s lFrac)
{
   Ipp32s i;
   Ipp32f sum, *x1;
   const Ipp32f *c;

   x1 = &pSrc[-(INTERPOL4_LEN-1)];
   if (lFrac < 0) {
      x1--;
      lFrac += UP_SAMPLING;
   }
   c = &ReorderInter_3[2*INTERPOL4_LEN*lFrac];

   sum = 0.0f;
   for(i=0; i< 2*INTERPOL4_LEN; i++)
      sum+= x1[i] * c[i];

   return sum;
}

Ipp32s AdaptiveCodebookSearch_G729_32f(Ipp32f *pSrcExc, Ipp32f *pSrcTargetVector, Ipp32f *pSrcImpulseResponse, Ipp32s len,
                        Ipp32s minLag, Ipp32s maxLag, Ipp32s valSubframeNum, Ipp32s *pDstFracPitch, G729Codec_Type codecType,Ipp32f *pExtBuff)
{
   Ipp32s i, lFracPart;
   Ipp32s lBestLag, lMin, lMax;
   Ipp32f max;
   Ipp32f fIntCorr;
   Ipp32f *pFltExc;     /* filtered past excitation */
   Ipp32f *pCorr;
   Ipp32s midLag;

   pFltExc = &pExtBuff[0];

   /* Find interval to compute normalized correlation */
   lMin = minLag - INTERPOL4_LEN;
   lMax = maxLag + INTERPOL4_LEN;

   pCorr = &pExtBuff[SUBFR_LEN - lMin]; /*Actually (10+2*INTERPOL4_LEN) from [SUBFR_LEN]. pCorr[lMin:lMax]*/

   /* Compute normalized correlation between target and filtered excitation */
   NormalizedCorrelation(pSrcExc, pSrcTargetVector, pSrcImpulseResponse, len, lMin, lMax, pCorr,pFltExc);

   /* find integer pitch */
   max = pCorr[minLag];
   lBestLag  = minLag;

   for(i= minLag+1; i<=maxLag; i++) {
      if( pCorr[i] >= max) {
         max = pCorr[i];
         lBestLag = i;
      }
   }

   /* If first subframe and lBestLag > 84 do not search fractionnal pitch */

   if( (valSubframeNum == 0) && (lBestLag > 84) ) {
      *pDstFracPitch = 0;
      return(lBestLag);
   }

   /* test the fractions around lBestLag and choose the one which maximizes the interpolated normalized correlation */

   if (codecType == G729D_CODEC) {    /* 6.4 kbps */
      if (valSubframeNum == 0) {
         max  = Interpolation_3(&pCorr[lBestLag], -2);
         lFracPart = -2;

         for (i = -1; i <= 2; i++) {
            fIntCorr = Interpolation_3(&pCorr[lBestLag], i);
            if(fIntCorr > max) {
               max = fIntCorr;
               lFracPart = i;
            }
         }
      } else {
         midLag = maxLag - 4;
         if ((lBestLag == midLag - 1) || lBestLag == midLag) {
            max  = Interpolation_3(&pCorr[lBestLag], -2);
            lFracPart = -2;

            for (i = -1; i <= 2; i++) {
               fIntCorr = Interpolation_3(&pCorr[lBestLag], i);
               if(fIntCorr > max) {
                  max = fIntCorr;
                  lFracPart = i;
               }
            }
         } else if (lBestLag == midLag - 2) {
            max  = Interpolation_3(&pCorr[lBestLag], 0);
            lFracPart = 0;

            for (i = 1; i <= 2; i++) {
               fIntCorr = Interpolation_3(&pCorr[lBestLag], i);
               if(fIntCorr > max) {
                  max = fIntCorr;
                  lFracPart = i;
               }
            }
         } else if (lBestLag == midLag + 1) {
            max  = Interpolation_3(&pCorr[lBestLag], -2);
            lFracPart = -2;

            for (i = -1; i <= 0; i++) {
               fIntCorr = Interpolation_3(&pCorr[lBestLag], i);
               if(fIntCorr > max) {
                  max = fIntCorr;
                  lFracPart = i;
               }
            }
         } else
            lFracPart = 0;
      }
   } else {
      max  = Interpolation_3(&pCorr[lBestLag], -2);
      lFracPart = -2;

      for (i = -1; i <= 2; i++) {
         fIntCorr = Interpolation_3(&pCorr[lBestLag], i);
         if(fIntCorr > max) {
            max = fIntCorr;
            lFracPart = i;
         }
      }
   }

   /* limit the fraction value in the interval [-1,0,1] */

   if (lFracPart == -2) {
      lFracPart = 1;
      lBestLag -= 1;
   }
   if (lFracPart == 2) {
      lFracPart = -1;
      lBestLag += 1;
   }

   *pDstFracPitch = lFracPart;

   return lBestLag;
}

void CNGGetSize(Ipp32s *pDstSize)
{
   *pDstSize = sizeof(CNGmemory);
   return;
}

void CNGInit(Ipp8s *cngMem)
{
   CNGmemory *cngState = (CNGmemory *)cngMem;
   ippsZero_16s((Ipp16s*)cngState,sizeof(CNGmemory)>>1) ;

   ippsZero_32f(cngState->SumAutoCorrs,SUMAUTOCORRS_SIZE);
   ippsZero_32f(cngState->AutoCorrs,AUTOCORRS_SIZE);
   ippsZero_32f(cngState->Energies,GAINS_NUM);

   cngState->fCurrGain = 0.;
   cngState->lAutoCorrsCounter = 0;
   cngState->lFltChangeFlag = 0;
}

static Ipp32f gaussGen(Ipp16s *sCNGSeed)
{
    Ipp32s i, lTmp;

    lTmp = 0;
    for(i=0; i<12; i++) {
        lTmp += Rand_16s(sCNGSeed);
    }
    lTmp >>= 7;
    return((Ipp32f)lTmp * 0.001953125f);
}

void ComfortNoiseExcitation_G729(Ipp32f fCurrGain, Ipp32f *exc, Ipp16s *sCNGSeed, Ipp32s flag_cod, Ipp32f *ExcitationError, Ipp8s *phdMem, Ipp8s *pExtBuff)
{
   Ipp32f *pGaussianExc;
   Ipp32s   *pos;
   Ipp32f *sign;
   Ipp32f *CurrentExcitation;
   Ipp32f fgp, fEner, fFact, fInterExc, k, discriminant, x1, x2, absMinRoot;
   Ipp32s   i, n, pitchDelay, lFrac;
   Ipp16s sGp, sTmp1, sTmp2;
   Ipp32s *delayLine;
   //Ipp64f ener_tmp;

   pGaussianExc = (Ipp32f *)pExtBuff;
   pos = (Ipp32s *)(pExtBuff + SUBFR_LEN*sizeof(Ipp32f));
   sign = (Ipp32f *)((Ipp8s *)pos + 4*sizeof(Ipp32s));
   delayLine = (Ipp32s *)((Ipp8s *)sign + 4*sizeof(Ipp32f));

   /* Loop on subframes */
   CurrentExcitation = exc;
   for (n = 0;  n < NUN_SUBFRAMES; n++) {
      /* Fenerate random adaptive codebook and fixed codebook parameters */
      sTmp1   = Rand_16s(sCNGSeed);
      lFrac    = (Ipp32s)(sTmp1 & (Ipp16s)0x0003) - 1;
      if(lFrac == 2) lFrac = 0;
      sTmp1 >>= 2;
      pitchDelay      = (Ipp32s)(sTmp1 & (Ipp16s)0x003F) + 40;
      sTmp1 >>= 6;
      sTmp2   = (Ipp16s)(sTmp1 & (Ipp16s)0x0007);
      pos[0]  = 5 * (Ipp32s)sTmp2;
      sTmp1 >>= 3;
      sTmp2   = (Ipp16s)(sTmp1 & (Ipp16s)0x0001);
      sign[0] = 2.f * (Ipp32f)sTmp2 - 1.f;
      sTmp1 >>= 1;
      sTmp2   = (Ipp16s)(sTmp1 & (Ipp16s)0x0007);
      pos[1]  = 5 * (Ipp32s)sTmp2 + 1;
      sTmp1 >>= 3;
      sTmp2   = (Ipp16s)(sTmp1 & (Ipp16s)0x0001);
      sign[1] = 2.f * (Ipp32f)sTmp2 - 1.f;
      sTmp1   = Rand_16s(sCNGSeed);
      sTmp2   = (Ipp16s)(sTmp1 & (Ipp16s)0x0007);
      pos[2]  = 5 * (Ipp32s)sTmp2 + 1;
      sTmp1 >>= 3;
      sTmp2   = (Ipp16s)(sTmp1 & (Ipp16s)0x0001);
      sign[2] = 2.f * (Ipp32f)sTmp2 - 1.f;
      sTmp1 >>= 1;
      sTmp2   = (Ipp16s)(sTmp1 & (Ipp16s)0x000F);
      pos[3]  = (Ipp32s)(sTmp2 & (Ipp16s)0x0001) + 3; /* j+3*/
      sTmp2 >>= 1;
      sTmp2  &= (Ipp16s)0x0007;
      pos[3] += 5 * (Ipp32s)sTmp2;
      sTmp1 >>= 4;
      sTmp2   = (Ipp16s)(sTmp1 & (Ipp16s)0x0001);
      sign[3] = 2.f * (Ipp32f)sTmp2 - 1.f;
      sGp      = (Ipp16s)(Rand_16s(sCNGSeed) & (Ipp16s)0x1FFF); /* < 0.5  */
      fgp      = (Ipp32f)sGp / 16384.f;

      /* Generate gaussian excitation */
      /*ippsRandomNoiseExcitation_G729B_16s32f(sCNGSeed, excg, SUBFR_LEN);
      ippsDotProd_32f64f(excg, excg, SUBFR_LEN,&ener_tmp);*/
      fEner = 0.f;
      for(i=0; i<SUBFR_LEN; i++) {
         pGaussianExc[i] = gaussGen(sCNGSeed);
         fEner += pGaussianExc[i] * pGaussianExc[i];
      }

      /*  Compute fFact = 0.5 x fCurrGain * sqrt(40 / fEner) */
      fFact = NORM_GAUSS * fCurrGain;
      fFact /= (Ipp32f)sqrt(fEner);
      /* Multiply excg by fFact */
      for(i=0; i<SUBFR_LEN; i++) {
         pGaussianExc[i] *= fFact;
      }
      //ippsMulC_32f_I(fFact, excg, SUBFR_LEN);

      /* generate random  adaptive excitation */
      delayLine[0] = pitchDelay;
      delayLine[1] = lFrac;
      ippsDecodeAdaptiveVector_G729_32f_I(delayLine, CurrentExcitation);

      /* CurrentExcitation is an adaptive + gaussian excitation */
      fEner = 0.f;
      for(i=0; i<SUBFR_LEN; i++) {
         CurrentExcitation[i] *= fgp;
         CurrentExcitation[i] += pGaussianExc[i];
         fEner += CurrentExcitation[i] * CurrentExcitation[i];
      }
      /* Compute fixed code gain */
      /* Solve EQ(X) = 4 X**2 + 2 b X + c, where b = fInterExc*/
      fInterExc = 0.f;
      for(i=0; i<4; i++) {
         fInterExc += CurrentExcitation[pos[i]] * sign[i];
      }

      /* Compute k = fCurrGain x fCurrGain x SUBFR_LEN */
      k = fCurrGain * fCurrGain * SUBFR_LEN;

      /* Compute discriminant = b^2 - 4*c, where c=fEner-k*/
      discriminant = fInterExc * fInterExc - 4.f * (fEner - k);

      if(discriminant < 0.f) {
         /* adaptive excitation = 0 */
         ippsCopy_32f(pGaussianExc, CurrentExcitation, SUBFR_LEN);
         fInterExc = 0.f;
         for(i=0; i<4; i++) {
            fInterExc += CurrentExcitation[pos[i]] * sign[i];
         }
         /* Compute discriminant = b^2 - 4*c, where c = - k x (1- alpha^2)*/
         discriminant = fInterExc * fInterExc + K_MUL_COEFF * k;
         fgp = 0.f;
      }

      discriminant = (Ipp32f)sqrt(discriminant);
      /* First root*/
      x1 = (discriminant - fInterExc) * 0.25f;
      /* Second root*/
      x2 = - (discriminant + fInterExc) * 0.25f;
      absMinRoot = ((Ipp32f)fabs(x1) < (Ipp32f)fabs(x2)) ? x1 : x2;
      if(absMinRoot >= 0.f) {
         CLIP_TO_UPLEVEL(absMinRoot,CNG_MAX_GAIN);
      } else {
         CLIP_TO_LOWLEVEL(absMinRoot,(-CNG_MAX_GAIN));
      }

      /* Update cur_exc with ACELP excitation */
      for(i=0; i<4; i++) {
         CurrentExcitation[pos[i]] += absMinRoot * sign[i];
      }

      if(flag_cod != DECODER) UpdateExcErr_G729(fgp, pitchDelay,ExcitationError);
      else {
         if(absMinRoot >= 0.f) PhaseDispersionUpdate_G729D(fgp,absMinRoot,phdMem);
         else PhaseDispersionUpdate_G729D(fgp,-absMinRoot,phdMem);
      }
      CurrentExcitation += SUBFR_LEN;
   } /* end of loop on subframes */

   return;
}



static __ALIGN32 CONST Ipp32f fFact[GAINS_NUM+1] =
        {(Ipp32f)0.003125, (Ipp32f)0.00078125, (Ipp32f)0.000390625};

static Ipp32s quantEnergy(Ipp32f fEner, Ipp32f *pDstQEnergy)
{
    Ipp32f fEnergydB;
    Ipp32s index;

    if(fEner <= MIN_ENER) {  /* MIN_ENER <=> -8dB */
        *pDstQEnergy = (Ipp32f)-12.;
        return(0);
    }

    fEnergydB = (Ipp32f)10. * (Ipp32f)log10(fEner);

    if(fEnergydB <= (Ipp32f)-8.) {
        *pDstQEnergy = (Ipp32f)-12.;
        return(0);
    }

    if(fEnergydB >= (Ipp32f)65.) {
        *pDstQEnergy = (Ipp32f)66.;
        return(31);
    }

    if(fEnergydB <= (Ipp32f)14.) {
        index = (Ipp32s)((fEnergydB + (Ipp32f)10.) * 0.25);
        if (index < 1) index = 1;
        *pDstQEnergy = (Ipp32f)4. * (Ipp32f)index - (Ipp32f)8.;
        return(index);
    }

    index = (Ipp32s)((fEnergydB - (Ipp32f)3.) * 0.5);
    if (index < 6) index = 6;
    *pDstQEnergy = (Ipp32f)2. * (Ipp32f)index + (Ipp32f)4.;
    return(index);
}

void QuantSIDGain_G729B(Ipp32f *fEner, Ipp32s lNumSavedEnergies, Ipp32f *enerq, Ipp32s *idx)
{
    Ipp32s i;
    Ipp32f averageEnergy;

    /* Quantize energy saved for frame erasure case*/
    if(lNumSavedEnergies == 0) {
        averageEnergy = (*fEner) * fFact[0];
    } else {
        /* Compute weighted average of energies*/
        averageEnergy = (Ipp32f)0.;
        for(i=0; i<lNumSavedEnergies; i++) {
            averageEnergy += fEner[i];
        }
        averageEnergy *= fFact[lNumSavedEnergies];
    }

    *idx = quantEnergy(averageEnergy, enerq);

    return;
}

static __ALIGN32 CONST Ipp32f coef_G729[2][2] = {
{(Ipp32f)31.134575,
(Ipp32f) 1.612322},

{(Ipp32f) 0.481389,
(Ipp32f) 0.053056}
};

static __ALIGN32 CONST Ipp32f thr1_G729[SIZECODEBOOK1-NUM_CAND1] = {
(Ipp32f)0.659681,
(Ipp32f)0.755274,
(Ipp32f)1.207205,
(Ipp32f)1.987740
};

static __ALIGN32 CONST Ipp32f thr2_G729[SIZECODEBOOK2-NUM_CAND2] = {
(Ipp32f)0.429912,
(Ipp32f)0.494045,
(Ipp32f)0.618737,
(Ipp32f)0.650676,
(Ipp32f)0.717949,
(Ipp32f)0.770050,
(Ipp32f)0.850628,
(Ipp32f)0.932089
};

static __ALIGN32 CONST Ipp32s map1_G729[SIZECODEBOOK1] = {
 5, 1, 4, 7, 3, 0, 6, 2};

static __ALIGN32 CONST Ipp32s map2_G729[SIZECODEBOOK2] = {
 4, 6, 0, 2,12,14, 8,10,15,11, 9,13, 7, 3, 1, 5};

static __ALIGN32 CONST Ipp32s imap1_G729[SIZECODEBOOK1] = {
 5, 1, 7, 4, 2, 0, 6, 3};
static __ALIGN32 CONST Ipp32s imap2_G729[SIZECODEBOOK2] = {
 2,14, 3,13, 0,15, 1,12, 6,10, 7, 9, 4,11, 5, 8};

static __ALIGN32 CONST Ipp32f gbk1_G729[SIZECODEBOOK1][2] = {
{0.000010f, 0.185084f},
{0.094719f, 0.296035f},
{0.111779f, 0.613122f},
{0.003516f, 0.659780f},
{0.117258f, 1.134277f},
{0.197901f, 1.214512f},
{0.021772f, 1.801288f},
{0.163457f, 3.315700f}};

static __ALIGN32 CONST Ipp32f gbk2_G729[SIZECODEBOOK2][2] = {
{0.050466f, 0.244769f},
{0.121711f, 0.000010f},
{0.313871f, 0.072357f},
{0.375977f, 0.292399f},
{0.493870f, 0.593410f},
{0.556641f, 0.064087f},
{0.645363f, 0.362118f},
{0.706138f, 0.146110f},
{0.809357f, 0.397579f},
{0.866379f, 0.199087f},
{0.923602f, 0.599938f},
{0.925376f, 1.742757f},
{0.942028f, 0.029027f},
{0.983459f, 0.414166f},
{1.055892f, 0.227186f},
{1.158039f, 0.724592f}};

static __ALIGN32 CONST Ipp32f coef_G729D[2][2] = {
{ 36.632507f, 2.514171f},
{  0.399259f, 0.073709f}
};

static __ALIGN32 CONST Ipp32f thr1_G729D[SIZECODEBOOK1_ANNEXD-NUM_CAND1_ANNEXD] = {
  1.210869f,
  2.401702f
};

static __ALIGN32 CONST Ipp32f thr2_G729D[SIZECODEBOOK2_ANNEXD-NUM_CAND2_ANNEXD] = {
  0.525915f,
  0.767320f
};

static __ALIGN32 CONST Ipp32s map1_G729D[SIZECODEBOOK1_ANNEXD] = { 0, 4, 6, 5, 2, 1, 7, 3 };

static __ALIGN32 CONST Ipp32s map2_G729D[SIZECODEBOOK2_ANNEXD] = { 0, 4, 3, 7, 5, 1, 6, 2 };

static __ALIGN32 CONST Ipp32s imap1_G729D[SIZECODEBOOK1_ANNEXD] = { 0, 5, 4, 7, 1, 3, 2, 6 };



static __ALIGN32 CONST Ipp32s imap2_G729D[SIZECODEBOOK2_ANNEXD] = { 0, 5, 7, 2, 1, 4, 6, 3 };


static __ALIGN32 CONST Ipp32f gbk1_G729D[SIZECODEBOOK1_ANNEXD][2] = {
{ 0.357003f, 0.00000f},
{ 0.178752f, 0.065771f},
{ 0.575276f, 0.166704f},
{ 0.370335f, 0.371903f},
{ 0.220734f, 0.411803f},
{ 0.193548f, 0.566385f},
{ 0.238962f, 0.785625f},
{ 0.304379f, 1.360714f}
};
static __ALIGN32 CONST Ipp32f gbk2_G729D[SIZECODEBOOK2_ANNEXD][2] = {
{ 0.00000f, 0.254841f},
{ 0.243384f, 0.00000f},
{ 0.273293f, 0.447009f},
{ 0.480707f, 0.477384f},
{ 0.628117f, 0.694884f},
{ 0.660905f, 1.684719f},
{ 0.729735f, 0.655223f},
{ 1.002375f, 0.959743f}
};

static __ALIGN32 CONST Ipp32f PredCoeff[4] = {
(Ipp32f)0.68,
(Ipp32f)0.58,
(Ipp32f)0.34,
(Ipp32f)0.19
};

static void   GainCodebookPreselect_G729D(Ipp32f *pBestGains, Ipp32s *pCand, Ipp32f fGainCode)
{
    Ipp32f   x,y ;

    x = (pBestGains[1]-(coef_G729D[0][0]*pBestGains[0]+coef_G729D[1][1])*fGainCode) * INV_COEF_ANNEXD ;
    y = (coef_G729D[1][0]*(-coef_G729D[0][1]+pBestGains[0]*coef_G729D[0][0])*fGainCode
        -coef_G729D[0][0]*pBestGains[1]) * INV_COEF_ANNEXD ;

    if(fGainCode>(Ipp32f)0.0){
        /* pre select first index */
        pCand[0] = 0 ;
        do{
            if(y>thr1_G729D[pCand[0]]*fGainCode) (pCand[0])++ ;
            else               break ;
        } while((pCand[0])<(SIZECODEBOOK1_ANNEXD-NUM_CAND1_ANNEXD)) ;
        /* pre select second index */
        pCand[1] = 0 ;
        do{
            if(x>thr2_G729D[pCand[1]]*fGainCode) (pCand[1])++ ;
            else               break ;
        } while((pCand[1])<(SIZECODEBOOK2_ANNEXD-NUM_CAND2_ANNEXD)) ;
    }
    else{
        /* pre select first index */
        pCand[0] = 0 ;
        do{
            if(y<thr1_G729D[pCand[0]]*fGainCode) (pCand[0])++ ;
            else               break ;
        } while((pCand[0])<(SIZECODEBOOK1_ANNEXD-NUM_CAND1_ANNEXD)) ;
        /* pre select second index */
        pCand[1] = 0 ;
        do{
            if(x<thr2_G729D[pCand[1]]*fGainCode) (pCand[1])++ ;
            else               break ;
        } while((pCand[1])<(SIZECODEBOOK2_ANNEXD-NUM_CAND2_ANNEXD)) ;
    }

    return ;
}

static void   GainCodebookPreselect_G729(Ipp32f *pBestGains, Ipp32s *pCand, Ipp32f fGainCode)
{
    Ipp32f   x,y ;

    x = (pBestGains[1]-(coef_G729[0][0]*pBestGains[0]+coef_G729[1][1])*fGainCode) * INV_COEF_BASE ;
    y = (coef_G729[1][0]*(-coef_G729[0][1]+pBestGains[0]*coef_G729[0][0])*fGainCode
        -coef_G729[0][0]*pBestGains[1]) * INV_COEF_BASE ;

    if(fGainCode>(Ipp32f)0.0){
        /* pre select first index */
        pCand[0] = 0 ;
        do{
            if(y>thr1_G729[pCand[0]]*fGainCode) (pCand[0])++ ;
            else               break ;
        } while((pCand[0])<(SIZECODEBOOK1-NUM_CAND1)) ;
        /* pre select second index */
        pCand[1] = 0 ;
        do{
            if(x>thr2_G729[pCand[1]]*fGainCode) (pCand[1])++ ;
            else               break ;
        } while((pCand[1])<(SIZECODEBOOK2-NUM_CAND2)) ;
    }
    else{
        /* pre select first index */
        pCand[0] = 0 ;
        do{
            if(y<thr1_G729[pCand[0]]*fGainCode) (pCand[0])++ ;
            else               break ;
        } while((pCand[0])<(SIZECODEBOOK1-NUM_CAND1)) ;
        /* pre select second index */
        pCand[1] = 0 ;
        do{
            if(x<thr2_G729[pCand[1]]*fGainCode) (pCand[1])++ ;
            else               break ;
        } while((pCand[1])<(SIZECODEBOOK2-NUM_CAND2)) ;
    }

    return ;
}

static void GainUpdate_G729_32f(Ipp32f *pastQntEnergies, Ipp32f fGainCode)
{
    Ipp32s j;

    /* update table of past quantized energies */
    for (j = 3; j > 0; j--)
        pastQntEnergies[j] = pastQntEnergies[j-1];
    pastQntEnergies[0] = (Ipp32f)20.0*(Ipp32f)log10((Ipp64f)fGainCode);

    return;
}

static void GainPredict_G729_32f(Ipp32f *pastQntEnr, Ipp32f *FixedCodebookExc, Ipp32s len,Ipp32f *fGainCode)
{
    Ipp32f fEnergy, predCodeboookGain;
    Ipp32s i;
    Ipp64f dTmp;

    predCodeboookGain = MEAN_ENER ;

    /* innovation energy */
    ippsDotProd_32f64f(FixedCodebookExc, FixedCodebookExc, len, &dTmp);
    fEnergy = (Ipp32f)dTmp+0.01f;

    fEnergy = (Ipp32f)10.0 * (Ipp32f)log10(fEnergy /(Ipp32f)len);

    predCodeboookGain -= fEnergy;

    /* predicted energy */
    for (i=0; i<4; i++) predCodeboookGain += PredCoeff[i]*pastQntEnr[i];

    /* predicted codebook gain */
    *fGainCode = predCodeboookGain;
    *fGainCode = (Ipp32f)pow((Ipp64f)10.0,(Ipp64f)(*fGainCode/20.0));   /* predicted gain */

    return;
}

Ipp32s GainQuant_G729(Ipp32f *FixedCodebookExc, Ipp32f *pGainCoeff, Ipp32s lSbfrLen, Ipp32f *pitchGain, Ipp32f *codeGain,
                     Ipp32s tamingflag, Ipp32f *PastQuantEnergy, G729Codec_Type codecType,Ipp8s *pExtBuff)
{
    Ipp32s *pCand,*index;
    Ipp32f fGainCode ;
    Ipp32f *pBestGain,fTmp;
    Ipp64f dGainCode;
    Ipp32s lQIndex;

    pBestGain = (Ipp32f *)pExtBuff;
    pCand = (Ipp32s *)(pExtBuff + 2*sizeof(Ipp32f));
    index = (Ipp32s *)((Ipp8s *)pCand + 2*sizeof(Ipp32s));

    GainPredict_G729_32f( PastQuantEnergy, FixedCodebookExc, lSbfrLen, &fGainCode);

    /*-- pre-selection --*/
    fTmp = (Ipp32f)-1./((Ipp32f)4.*pGainCoeff[0]*pGainCoeff[2]-pGainCoeff[4]*pGainCoeff[4]) ;
    pBestGain[0] = ((Ipp32f)2.*pGainCoeff[2]*pGainCoeff[1]-pGainCoeff[3]*pGainCoeff[4])*fTmp ;
    pBestGain[1] = ((Ipp32f)2.*pGainCoeff[0]*pGainCoeff[3]-pGainCoeff[1]*pGainCoeff[4])*fTmp ;

    if (tamingflag == 1){
       CLIP_TO_UPLEVEL(pBestGain[0],MAX_GAIN_TIMING2);
    }
    /* Presearch for gain codebook */
    if(codecType==G729D_CODEC) {
      GainCodebookPreselect_G729D(pBestGain,pCand,fGainCode) ;

      ippsGainCodebookSearch_G729D_32f(pGainCoeff, fGainCode, pCand, index, tamingflag);

      *pitchGain  = gbk1_G729D[index[0]][0]+gbk2_G729D[index[1]][0] ;
      dGainCode = (Ipp64f)(gbk1_G729D[index[0]][1]+gbk2_G729D[index[1]][1]);

      *codeGain =  (Ipp32f)(dGainCode) * fGainCode;

      if(dGainCode < 0.2) dGainCode = 0.2;

      lQIndex = map1_G729D[index[0]]*SIZECODEBOOK2_ANNEXD+map2_G729D[index[1]];
    } else {
      GainCodebookPreselect_G729(pBestGain,pCand,fGainCode) ;

      ippsGainCodebookSearch_G729_32f(pGainCoeff, fGainCode, pCand, index, tamingflag);

      *pitchGain  = gbk1_G729[index[0]][0]+gbk2_G729[index[1]][0] ;
      dGainCode = (Ipp64f)(gbk1_G729[index[0]][1]+gbk2_G729[index[1]][1]);
      *codeGain =  (Ipp32f)(dGainCode) * fGainCode;

      lQIndex = map1_G729[index[0]]*SIZECODEBOOK2+map2_G729[index[1]];
    }
    /* Update table of past quantized energies */
    GainUpdate_G729_32f( PastQuantEnergy, (Ipp32f)dGainCode);

    return lQIndex;
}

void DecodeGain_G729(Ipp32s index, Ipp32f *FixedCodebookExc, Ipp32s lSbfrLen, Ipp32f *pitchGain, Ipp32f *codeGain, Ipp32s rate, Ipp32f *PastQuantEnergy)
{


    Ipp32s idxs[2];
    Ipp32f fGainCode;
    Ipp64f dGainCode;

    GainPredict_G729_32f( PastQuantEnergy, FixedCodebookExc, lSbfrLen, &fGainCode);

    /* Decode pitch and codebook gain. */

    if(rate==G729D_MODE) {
       idxs[0] = imap1_G729D[index>>NCODE2_B_ANNEXD] ;
       idxs[1] = imap2_G729D[index & (SIZECODEBOOK2_ANNEXD-1)] ;

       *pitchGain = gbk1_G729D[idxs[0]][0]+gbk2_G729D[idxs[1]][0];
       dGainCode = (Ipp64f)gbk1_G729D[idxs[0]][1]+gbk2_G729D[idxs[1]][1];

       *codeGain =  (Ipp32f)(dGainCode) * fGainCode;

       if(dGainCode < 0.2) dGainCode = 0.2;
    } else {
       idxs[0] = imap1_G729[index>>NCODE2_BITS] ;
       idxs[1] = imap2_G729[index & (SIZECODEBOOK2-1)] ;

       *pitchGain = gbk1_G729[idxs[0]][0]+gbk2_G729[idxs[1]][0];
       dGainCode = (Ipp64f)gbk1_G729[idxs[0]][1]+gbk2_G729[idxs[1]][1];

       *codeGain =  (Ipp32f) (dGainCode * fGainCode);
    }

    /* Update table of past quantized energies */
    GainUpdate_G729_32f( PastQuantEnergy, (Ipp32f)dGainCode);

    return;
}

void PSTGetSize(Ipp32s *pDstSize)
{
   *pDstSize = sizeof(PSTmemory);
   return;
}

void PSTInit(Ipp8s *pstMem)
{
   PSTmemory *pstState = (PSTmemory *)pstMem;

   ippsZero_32f(pstState->ResidualMemory,RESISDUAL_MEMORY);
   ippsZero_32f(pstState->STPMemory,BWD_LPC_ORDER);
   ippsZero_32f(pstState->STPNumCoeff,SHORTTERM_POSTFLT_LEN_E);
   ippsZero_32f(pstState->ZeroMemory,BWD_LPC_ORDER);

   pstState->gainPrec = 1.;
}

static __ALIGN32 CONST Ipp32f STPTbl[SIZE_SHORT_INT_FLT_MEMORY] = {
(Ipp32f) -0.005772, (Ipp32f)  0.087669, (Ipp32f)  0.965882, (Ipp32f) -0.048753,
(Ipp32f) -0.014793, (Ipp32f)  0.214886, (Ipp32f)  0.868791, (Ipp32f) -0.065537,
(Ipp32f) -0.028507, (Ipp32f)  0.374334, (Ipp32f)  0.723418, (Ipp32f) -0.060834,
(Ipp32f) -0.045567, (Ipp32f)  0.550847, (Ipp32f)  0.550847, (Ipp32f) -0.045567,
(Ipp32f) -0.060834, (Ipp32f)  0.723418, (Ipp32f)  0.374334, (Ipp32f) -0.028507,
(Ipp32f) -0.065537, (Ipp32f)  0.868791, (Ipp32f)  0.214886, (Ipp32f) -0.014793,
(Ipp32f) -0.048753, (Ipp32f)  0.965882, (Ipp32f)  0.087669, (Ipp32f) -0.005772};

static __ALIGN32 CONST Ipp32f LTPTbl[SIZE_LONG_INT_FLT_MEMORY] = {
(Ipp32f) -0.001246, (Ipp32f)  0.002200, (Ipp32f) -0.004791, (Ipp32f)  0.009621,
(Ipp32f) -0.017685, (Ipp32f)  0.031212, (Ipp32f) -0.057225, (Ipp32f)  0.135470,
(Ipp32f)  0.973955, (Ipp32f) -0.103495, (Ipp32f)  0.048663, (Ipp32f) -0.027090,
(Ipp32f)  0.015280, (Ipp32f) -0.008160, (Ipp32f)  0.003961, (Ipp32f) -0.001827,
(Ipp32f) -0.002388, (Ipp32f)  0.004479, (Ipp32f) -0.009715, (Ipp32f)  0.019261,
(Ipp32f) -0.035118, (Ipp32f)  0.061945, (Ipp32f) -0.115187, (Ipp32f)  0.294161,
(Ipp32f)  0.898322, (Ipp32f) -0.170283, (Ipp32f)  0.083211, (Ipp32f) -0.046645,
(Ipp32f)  0.026210, (Ipp32f) -0.013854, (Ipp32f)  0.006641, (Ipp32f) -0.003099,
(Ipp32f) -0.003277, (Ipp32f)  0.006456, (Ipp32f) -0.013906, (Ipp32f)  0.027229,
(Ipp32f) -0.049283, (Ipp32f)  0.086990, (Ipp32f) -0.164590, (Ipp32f)  0.464041,
(Ipp32f)  0.780309, (Ipp32f) -0.199879, (Ipp32f)  0.100795, (Ipp32f) -0.056792,
(Ipp32f)  0.031761, (Ipp32f) -0.016606, (Ipp32f)  0.007866, (Ipp32f) -0.003740,
(Ipp32f) -0.003770, (Ipp32f)  0.007714, (Ipp32f) -0.016462, (Ipp32f)  0.031849,
(Ipp32f) -0.057272, (Ipp32f)  0.101294, (Ipp32f) -0.195755, (Ipp32f)  0.630993,
(Ipp32f)  0.630993, (Ipp32f) -0.195755, (Ipp32f)  0.101294, (Ipp32f) -0.057272,
(Ipp32f)  0.031849, (Ipp32f) -0.016462, (Ipp32f)  0.007714, (Ipp32f) -0.003770,
(Ipp32f) -0.003740, (Ipp32f)  0.007866, (Ipp32f) -0.016606, (Ipp32f)  0.031761,
(Ipp32f) -0.056792, (Ipp32f)  0.100795, (Ipp32f) -0.199879, (Ipp32f)  0.780309,
(Ipp32f)  0.464041, (Ipp32f) -0.164590, (Ipp32f)  0.086990, (Ipp32f) -0.049283,
(Ipp32f)  0.027229, (Ipp32f) -0.013906, (Ipp32f)  0.006456, (Ipp32f) -0.003277,
(Ipp32f) -0.003099, (Ipp32f)  0.006641, (Ipp32f) -0.013854, (Ipp32f)  0.026210,
(Ipp32f) -0.046645, (Ipp32f)  0.083211, (Ipp32f) -0.170283, (Ipp32f)  0.898322,
(Ipp32f)  0.294161, (Ipp32f) -0.115187, (Ipp32f)  0.061945, (Ipp32f) -0.035118,
(Ipp32f)  0.019261, (Ipp32f) -0.009715, (Ipp32f)  0.004479, (Ipp32f) -0.002388,
(Ipp32f) -0.001827, (Ipp32f)  0.003961, (Ipp32f) -0.008160, (Ipp32f)  0.015280,
(Ipp32f) -0.027090, (Ipp32f)  0.048663, (Ipp32f) -0.103495, (Ipp32f)  0.973955,
(Ipp32f)  0.135470, (Ipp32f) -0.057225, (Ipp32f)  0.031212, (Ipp32f) -0.017685,
(Ipp32f)  0.009621, (Ipp32f) -0.004791, (Ipp32f)  0.002200, (Ipp32f) -0.001246
};

static void HarmonicPostFilter_G729_32f(Ipp32s valPitchDelay, Ipp32f *pSrc, Ipp32f *pDst,
                           Ipp32s *isVoiced, Ipp32f valHarmonicCoeff, Ipp32f *mem);
static void SearchDelay(Ipp32s valPitchDelay, Ipp32f *pSrcSignal, Ipp32s *LTPDelay, Ipp32s *lPhase,
                       Ipp32f *LTPGainNumerator, Ipp32f *LTPGainDemerator, Ipp32f *pSignal, Ipp32s *lOffset, Ipp32f *mem);
static void HarmonicFilter_G729_32f(Ipp32f *pSrcSignal, Ipp32f *pSrcFltSignal, Ipp32f *pDstSignal, Ipp32f valFltGain);
static void TiltCompensation_G729_32f(Ipp32f *pSrcSignal, Ipp32f *pDstSignal, Ipp32f val);
static void Calc1stParcor(Ipp32f *pSrcImpulseResponse, Ipp32f *pDstPartialCorrCoeff, Ipp32s len);
static void GainControl_G729(Ipp32f *pSrcSignal, Ipp32f *pDstSignal, Ipp32f *pSrcDstGain, Ipp32f *mem);

void Post_G729E(G729FPDecoder_Obj *decoderObj, Ipp32s pitchDelay, Ipp32f *pSignal, Ipp32f *pLPC, Ipp32f *pDstFltSignal, Ipp32s *pVoicing,
               Ipp32s len, Ipp32s lenLPC, Ipp32s Vad)
{
    LOCAL_ALIGN_ARRAY(32, Ipp32f, STPDenCoeff, BWD_LPC_ORDERP1,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp32f, LTPSignal, SUBFR_LENP1,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp32f, mem,SIZE_SEARCH_DEL_MEMORY+2*(FRAC_DELAY_RES-1),decoderObj);
    Ipp32f *pLTPSignal;
    Ipp32f f1stParcor,fG0, fTmp;
    Ipp32f *pResidual;
    Ipp32f *pSTPMemory;
    Ipp32s i;
    PSTmemory *pstState = (PSTmemory *)decoderObj->pstMem;

    pResidual = pstState->ResidualMemory + RESISDUAL_MEMORY;
    pSTPMemory = pstState->STPMemory + BWD_LPC_ORDER - 1;

    /* Compute weighted LPC coefficients */
    WeightLPCCoeff_G729(pLPC, decoderObj->g1PST, lenLPC, STPDenCoeff);
    WeightLPCCoeff_G729(pLPC, decoderObj->g2PST, lenLPC, pstState->STPNumCoeff);
    ippsZero_32f(&pstState->STPNumCoeff[lenLPC+1], (BWD_LPC_ORDER-lenLPC));
    /* Compute A(gamma2) residual */
    ippsConvBiased_32f(pstState->STPNumCoeff,lenLPC+1,pSignal,SUBFR_LEN+lenLPC,pResidual,SUBFR_LEN,lenLPC);

    /* Harmonic filtering */
    pLTPSignal = LTPSignal + 1;

    if (Vad > 1)
      HarmonicPostFilter_G729_32f(pitchDelay, pResidual, pLTPSignal, pVoicing, decoderObj->gHarmPST,mem);
    else {
      *pVoicing = 0;
      ippsCopy_32f(pResidual, pLTPSignal, SUBFR_LEN);
    }

    /* Save last output of 1/A(gamma1) (from preceding subframe) */
    LTPSignal[0] = *pSTPMemory;

    /* Controls Ipp16s term pst filter gain and compute f1stParcor   */
    /* compute i.r. of composed filter STPNumCoeff / STPDenCoeff */
    ippsSynthesisFilter_G729_32f(STPDenCoeff, lenLPC, pstState->STPNumCoeff, mem, len, pstState->ZeroMemory);

    /* compute 1st parcor */
    Calc1stParcor(mem, &f1stParcor, len);

    /* compute fG0 */
    fG0 = 0.f;
    for(i=0; i<len; i++) {
        fG0 += (Ipp32f)fabs(mem[i]);
    }

    /* Scale signal input of 1/A(gamma1) */
    if(fG0 > (Ipp32f)1.) {
      fTmp = (Ipp32f)1./fG0;
      ippsMulC_32f(pLTPSignal, fTmp, pLTPSignal, SUBFR_LEN);
    }

    /* 1/A(gamma1) filtering, STPMemory is updated */
    ippsSynthesisFilter_G729_32f(STPDenCoeff, lenLPC, pLTPSignal, pLTPSignal, SUBFR_LEN, &pstState->STPMemory[BWD_LPC_ORDER-lenLPC]);
    ippsCopy_32f(&pLTPSignal[SUBFR_LEN-BWD_LPC_ORDER], pstState->STPMemory, BWD_LPC_ORDER);

    /* Tilt filtering with : (1 + mu z-1) * (1/1-|mu|)*/
    TiltCompensation_G729_32f(LTPSignal, pDstFltSignal, f1stParcor);

    /* Gain control */
    GainControl_G729(pSignal, pDstFltSignal, &pstState->gainPrec,mem);

    /**** Update for next subframe */
    ippsMove_32f(&pstState->ResidualMemory[SUBFR_LEN], &pstState->ResidualMemory[0], RESISDUAL_MEMORY);

    LOCAL_ALIGN_ARRAY_FREE(32, Ipp32f, mem,SIZE_SEARCH_DEL_MEMORY+2*(FRAC_DELAY_RES-1),decoderObj);
    LOCAL_ALIGN_ARRAY_FREE(32, Ipp32f, LTPSignal, SUBFR_LENP1,decoderObj);  /* H0 output signal             */
    LOCAL_ALIGN_ARRAY_FREE(32, Ipp32f, STPDenCoeff, BWD_LPC_ORDERP1,decoderObj);   /* s.t. denominator coeff.      */

    return;
}

static void HarmonicPostFilter_G729_32f(Ipp32s valPitchDelay, Ipp32f *pSrc, Ipp32f *pDst,
                           Ipp32s *isVoiced, Ipp32f valHarmonicCoeff, Ipp32f *mem)
{
    Ipp32s LTPDelay, lPhase;
    Ipp32f LTPGainNumerator, LTPGainDenominator;
    Ipp32f LTPGainNumerator2, LTPGainDenominator2;
    Ipp32f harmFltCoeff;
    Ipp32f *pDelayedSignal;
    Ipp32s lOffset;
    Ipp32f *pSignal = &mem[0];
    Ipp32f *searcgDelMem = &mem[SIZE_SEARCH_DEL_MEMORY];

    /* Sub optimal delay search */
    SearchDelay(valPitchDelay, pSrc, &LTPDelay, &lPhase, &LTPGainNumerator, &LTPGainDenominator,
                            pSignal, &lOffset,searcgDelMem);
    *isVoiced = LTPDelay;

    if(isVarZero(LTPGainNumerator))   {
        ippsCopy_32f(pSrc, pDst, SUBFR_LEN);
    } else {
        if(lPhase == 0) {
            pDelayedSignal = pSrc - LTPDelay;
        } else {
            Ipp64f z;
            /* Filtering with long filter*/
            ippsConvBiased_32f((Ipp32f*)&LTPTbl[(lPhase-1) * LONG_INT_FLT_LEN], LONG_INT_FLT_LEN,
                      &pSrc[LONG_INT_FLT_LEN_BY2-LTPDelay], SUBFR_LEN+LONG_INT_FLT_LEN,
                            pDst, SUBFR_LEN, LONG_INT_FLT_LEN);

            /* Compute fNumerator */
            ippsDotProd_32f64f(pDst, pSrc, SUBFR_LEN, &z);
            LTPGainNumerator2 = (Ipp32f)z;
            if(LTPGainNumerator2 < 0.0f) LTPGainNumerator2 = 0.0f;

            /* Compute den */
            ippsDotProd_32f(pDst, pDst, SUBFR_LEN, &LTPGainDenominator2);

            if(LTPGainNumerator2 * LTPGainNumerator2 * LTPGainDenominator> LTPGainNumerator * LTPGainNumerator * LTPGainDenominator2) {
               /* select long filter */
                LTPGainNumerator = LTPGainNumerator2;
                LTPGainDenominator = LTPGainDenominator2;
                pDelayedSignal = pDst;
            } else {
               /* select Ipp16s filter */
                pDelayedSignal = pSignal + ((lPhase-1) * SUBFR_LENP1 + lOffset);
            }
        }

        if(LTPGainNumerator >= LTPGainDenominator) {
            harmFltCoeff = 1.f/(1.f+ valHarmonicCoeff);
        } else {
            harmFltCoeff = LTPGainDenominator / (LTPGainDenominator + valHarmonicCoeff * LTPGainNumerator);
        }

        /** filtering by H0(z) = harmonic filter **/
        HarmonicFilter_G729_32f(pSrc, pDelayedSignal, pDst, harmFltCoeff);
    }

    return;
}

static void SearchDelay(Ipp32s valPitchDelay, Ipp32f *pSrcSignal, Ipp32s *LTPDelay, Ipp32s *lPhase,
    Ipp32f *LTPGainNumerator, Ipp32f *LTPGainDemerator, Ipp32f *pSignal, Ipp32s *lOffset, Ipp32f *mem)
{
   Ipp32f *pDen0, *pDen1;
   Ipp32s iOfset, i, lLambda;
   Ipp32f fNumerator, fSqNumerator, fDenominator0, fDenominator1;
   Ipp32f fMaxDenominator, maxNumerator, squaredNumeratorMax;
   Ipp32s lMaxPhi;
   Ipp32f *pDelayedSignal;
    Ipp64f dEnergy, dDenominator, dTmp;
    Ipp32f fIntNumerator;

   pDen0 = &mem[0];
   pDen1 = &mem[FRAC_DELAY_RES-1];

   /* Compute current signal energy */
   ippsDotProd_32f64f(pSrcSignal, pSrcSignal, SUBFR_LEN, &dEnergy);
   if(dEnergy < 0.1) {
      *LTPGainNumerator = 0.f;
      *LTPGainDemerator = 1.f;
      *lPhase = 0;
      *LTPDelay = 0;
      return;
   }

    /* Selects best of 3 integer delays: Maximum of 3 numerators around valPitchDelay coder LTP delay*/

    ippsAutoCorrLagMax_32f(pSrcSignal, SUBFR_LEN, valPitchDelay - 1, valPitchDelay + 2, &fIntNumerator, &lLambda);

    if(fIntNumerator <= 0.) {
        *LTPGainNumerator = 0.f;
        *LTPGainDemerator = 1.f;
        *lPhase = 0;
        *LTPDelay   = 0;
        return;
    }

    /* Calculates denominator for lambda_max */
    ippsDotProd_32f64f(&pSrcSignal[-lLambda], &pSrcSignal[-lLambda], SUBFR_LEN, &dDenominator);
    if(dDenominator < 0.1) {
        *LTPGainNumerator = 0.f;
        *LTPGainDemerator = 1.f;
        *lPhase = 0;
        *LTPDelay   = 0;
        return;
    }

    /* Compute pSignal & denominators */
    pDelayedSignal = pSignal;
    fMaxDenominator = (Ipp32f)dDenominator;

    /* loop on lPhase to select the best lPhase around lLambda */
    for(i=0; i<FRAC_DELAY_RES-1; i++) {

        ippsConvBiased_32f((Ipp32f*)&STPTbl[i*SHORT_INT_FLT_LEN], SHORT_INT_FLT_LEN,
                      &pSrcSignal[SHORT_INT_FLT_LEN_BY2 - 1 - lLambda], SUBFR_LEN+1+SHORT_INT_FLT_LEN,
                            pDelayedSignal, SUBFR_LEN+1, SHORT_INT_FLT_LEN);

        /* recursive computation of fDenominator0 (lambda_max+1) and fDenominator1 (lambda_max) */

        /* common part to fDenominator0 and fDenominator1 */
        ippsDotProd_32f64f(&pDelayedSignal[1], &pDelayedSignal[1], SUBFR_LEN-1, &dTmp);

        /* Leftside denominator */
        fDenominator0  = (Ipp32f) (dTmp + pDelayedSignal[0] * pDelayedSignal[0]);
        pDen0[i] = fDenominator0;

        /* Rightside denominator */
        fDenominator1 = (Ipp32f) (dTmp + pDelayedSignal[SUBFR_LEN] * pDelayedSignal[SUBFR_LEN]);
        pDen1[i] = fDenominator1;

        if(fabs(pDelayedSignal[0])>fabs(pDelayedSignal[SUBFR_LEN])) {
           /* Choose left side*/
            if(fDenominator0 > fMaxDenominator) {
                fMaxDenominator = fDenominator0;
            }
        } else {
           /* Choose right side*/
            if(fDenominator1 > fMaxDenominator) {
                fMaxDenominator = fDenominator1;
            }
        }
        pDelayedSignal += SUBFR_LENP1;
    }
    if(fMaxDenominator < 0.1f ) {
        *LTPGainNumerator = 0.f;
        *LTPGainDemerator = 1.f;
        *lPhase    = 0;
        *LTPDelay   = 0;
        return;
    }
    /* Initialization */
    maxNumerator = (Ipp32f)fIntNumerator;
    fMaxDenominator      = (Ipp32f)dDenominator;
    squaredNumeratorMax    =  maxNumerator * maxNumerator;
    lMaxPhi      = 0;
    iOfset         = 1;

    pDelayedSignal     = pSignal;

    /* if fMaxDenominator = 0 or fNumerator!=0 & den=0: will be selected and declared unvoiced */
    /* Loop on lPhase */
    for(i=0; i<(FRAC_DELAY_RES-1); i++) {

        /* computes fNumerator for lambda_max+1 - i/FRAC_DELAY_RES */
        ippsDotProd_32f64f(pSrcSignal, pDelayedSignal, SUBFR_LEN, &dTmp);

        if(dTmp < 0.) fNumerator = (Ipp32f)0.;
        else fNumerator = (Ipp32f)dTmp;
        fSqNumerator = fNumerator * fNumerator;

        /* selection if fNumerator/sqrt(fDenominator0) max */
        if((fSqNumerator * fMaxDenominator) > (squaredNumeratorMax * pDen0[i])) {
            maxNumerator     = fNumerator;
            squaredNumeratorMax        = fSqNumerator;
            fMaxDenominator          = pDen0[i];
            iOfset             = 0;
            lMaxPhi          = i+1;
        }

        /* computes fNumerator for lambda_max - phi/FRAC_DELAY_RES */

        ippsDotProd_32f64f(pSrcSignal, &pDelayedSignal[1], SUBFR_LEN, &dTmp);
        if(dTmp < 0.) fNumerator = (Ipp32f)0.;
        else fNumerator = (Ipp32f)dTmp;

        fSqNumerator = fNumerator * fNumerator;

        /* selection if fNumerator/sqrt(fDenominator1) max */
        if((fSqNumerator * fMaxDenominator) > (squaredNumeratorMax * pDen1[i])) {
            maxNumerator     = fNumerator;
            squaredNumeratorMax        = fSqNumerator;
            fMaxDenominator          = pDen1[i];
            iOfset             = 1;
            lMaxPhi          = i+1;
        }
        pDelayedSignal += (SUBFR_LEN+1);
    }

    if(isVarZero(maxNumerator) || (fMaxDenominator <= 0.1f)) {
        *LTPGainNumerator = 0.f;
        *LTPGainDemerator = 1.f;
        *lPhase = 0;
        *LTPDelay = 0;
        return;
    }

    /* comparison numerator**2 with energy*denominator*Threshold */
    if(squaredNumeratorMax >= (fMaxDenominator * dEnergy * LTPTHRESHOLD)) {
        *LTPDelay   = lLambda + 1 - iOfset;
        *lOffset  = iOfset;
        *lPhase    = lMaxPhi;
        *LTPGainNumerator = maxNumerator;
        *LTPGainDemerator = fMaxDenominator;
    } else {
        *LTPGainNumerator = 0.f;
        *LTPGainDemerator = 1.f;
        *lPhase    = 0;
        *LTPDelay   = 0;
    }
    return;
}

static void HarmonicFilter_G729_32f(Ipp32f *pSrcSignal, Ipp32f *pSrcFltSignal, Ipp32f *pDstSignal, Ipp32f valFltGain)
{
    Ipp32f valFltGainNeg;

    valFltGainNeg = 1.f - valFltGain;

    ippsInterpolateC_G729_32f(pSrcSignal, valFltGain, pSrcFltSignal, valFltGainNeg, pDstSignal, SUBFR_LEN);

    return;
}

static void Calc1stParcor(Ipp32f *pSrcImpulseResponse, Ipp32f *pDstPartialCorrCoeff, Ipp32s len)
{
    Ipp32f firstCorr, secondCorr;
    Ipp64f dTmp;

    /* computation of the autocorrelation of the impulse response*/
    ippsDotProd_32f64f(pSrcImpulseResponse, pSrcImpulseResponse, len, &dTmp);
    firstCorr = (Ipp32f)dTmp;

    ippsDotProd_32f64f(pSrcImpulseResponse, &pSrcImpulseResponse[1], len-1, &dTmp);
    secondCorr = (Ipp32f)dTmp;

    if(isVarZero(firstCorr)) {
        *pDstPartialCorrCoeff = 0.f;
        return;
    }

    /* Compute 1st parcor */
    if(firstCorr < (Ipp32f)fabs(secondCorr) ) {
        *pDstPartialCorrCoeff = 0.0f;
        return;
    }
    *pDstPartialCorrCoeff = - secondCorr / firstCorr;

    return;
}

static void TiltCompensation_G729_32f(Ipp32f *pSrcSignal, Ipp32f *pDstSignal, Ipp32f val)
{
    Ipp32s n;
    Ipp32f fMu, fGamma, fTmp;

    if(val > 0.f) {
        fMu = val * GAMMA3_POSTFLT_P;
    }
    else {
        fMu = val * GAMMA3_POSTFLT_M;
    }
    fGamma = 1.f / (1.f - (Ipp32f)fabs(fMu));

    /* points on pSrcSignal(-1) */
    for(n=0; n<SUBFR_LEN; n++) {
        fTmp = fMu * pSrcSignal[n];
        fTmp += pSrcSignal[n+1];
        pDstSignal[n] = fGamma * fTmp;
    }
    return;
}

void GainControl_G729(Ipp32f *pSrcSignal, Ipp32f *pDstSignal, Ipp32f *pSrcDstGain, Ipp32f *mem)
{
    Ipp64f dInGain, dOutGain, fG0;
    Ipp32f *pTmpVector;
    Ipp64f *pTmpVectorD;

    pTmpVector = &mem[0];
    pTmpVectorD = (Ipp64f *)&mem[SUBFR_LEN];

    /* compute input gain */
    ippsAbs_32f(pSrcSignal,pTmpVector,SUBFR_LEN);
    ippsConvert_32f64f(pTmpVector,pTmpVectorD,SUBFR_LEN);
    ippsSum_64f(pTmpVectorD,SUBFR_LEN,&dInGain);

    if(isVarZero(dInGain)) {
        fG0 = 0.;
    }
    else {
        /* Compute output gain */
       ippsAbs_32f(pDstSignal,pTmpVector,SUBFR_LEN);
       ippsConvert_32f64f(pTmpVector,pTmpVectorD,SUBFR_LEN);
       ippsSum_64f(pTmpVectorD,SUBFR_LEN,&dOutGain);
        if(isVarZero(dOutGain)) {
            *pSrcDstGain = (Ipp32f)0.;
            return;
        }

        fG0 = (dInGain/ dOutGain);
        fG0 *= AGC_FACTORM1;
    }

    /* gain(n) = AGC_FACTOR gain(n-1) + (1-AGC_FACTOR)fG0 */
    /* pDstSignal(n) = gain(n) pDstSignal(n)                                   */
    ippsGainControl_G729_32f_I((Ipp32f)fG0, AGC_FACTOR, pDstSignal, pSrcDstGain);
    return;
}
