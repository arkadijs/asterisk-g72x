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
// Purpose: G.729 float VAD functions.
//
*/

#include <math.h>
#include "vadg729fp.h"

static __ALIGN32 CONST Ipp32f a[14] = {
   1.750000e-03f, -4.545455e-03f, -2.500000e+01f, 2.000000e+01f,
   0.000000e+00f, 8.800000e+03f, 0.000000e+00f, 2.5e+01f,
   -2.909091e+01f, 0.000000e+00f, 1.400000e+04f, 0.928571f,
   -1.500000e+00f, 0.714285f
};

static __ALIGN32 CONST Ipp32f b[14] = {
   0.00085f, 0.001159091f, -5.0f, -6.0f, -4.7f, -12.2f, 0.0009f,
   -7.0f, -4.8182f, -5.3f, -15.5f, 1.14285f, -9.0f, -2.1428571f
};

static Ipp32s MakeDecision(Ipp32f fLowBandEnergyDiff, Ipp32f fFullBandEnergyDiff,  Ipp32f fSpectralDistortion, Ipp32f fZeroCrossingDiff)
{
    /* The spectral distortion vs zero-crossing difference */
    if (fSpectralDistortion > a[0]*fZeroCrossingDiff+b[0]) {
        return(VAD_VOICE);
    }
    if (fSpectralDistortion > a[1]*fZeroCrossingDiff+b[1]) {
        return(VAD_VOICE);
    }

    /* full-band energy difference vs zero-crossing difference */

    if (fFullBandEnergyDiff < a[2]*fZeroCrossingDiff+b[2]) {
        return(VAD_VOICE);
    }
    if (fFullBandEnergyDiff < a[3]*fZeroCrossingDiff+b[3]) {
        return(VAD_VOICE);
    }
    if (fFullBandEnergyDiff < b[4]) {
        return(VAD_VOICE);
    }

    /*   full-band energy difference vs the spectral distortion */
    if (fFullBandEnergyDiff < a[5]*fSpectralDistortion+b[5]) {
        return(VAD_VOICE);
    }
    if (fSpectralDistortion > b[6]) {
        return(VAD_VOICE);
    }

    /* full-band energy difference vs zero-crossing difference */
    if (fFullBandEnergyDiff < a[7]*fZeroCrossingDiff+b[7]) {
        return(VAD_VOICE);
    }
    if (fFullBandEnergyDiff < a[8]*fZeroCrossingDiff+b[8]) {
        return(VAD_VOICE);
    }
    if (fFullBandEnergyDiff < b[9]) {
        return(VAD_VOICE);
    }

    /* low-band energy difference vs the spectral distortion */
    if (fLowBandEnergyDiff < a[10]*fSpectralDistortion+b[10]) {
        return(VAD_VOICE);
    }

    /* low-band energy difference vs full-band eneggy difference */
    if (fLowBandEnergyDiff > a[11]*fFullBandEnergyDiff+b[11]) {
        return(VAD_VOICE);
    }

    if (fLowBandEnergyDiff < a[12]*fFullBandEnergyDiff+b[12]) {
        return(VAD_VOICE);
    }
    if (fLowBandEnergyDiff < a[13]*fFullBandEnergyDiff+b[13]) {
        return(VAD_VOICE);
    }

    return(VAD_NOISE);
}

void VADInit(Ipp8s *pVADmem)
{
   VADmemory *vadState = (VADmemory *)pVADmem;
   ippsZero_16s((Ipp16s*)vadState,sizeof(VADmemory)>>1) ;

   ippsZero_32f(vadState->MeanLSFVec, LPC_ORDER);
   vadState->fMeanFullBandEnergy = 0.0f;
   vadState->fMeanLowBandEnergy = 0.0f;
   vadState->fMeanEnergy = 0.0f;
   vadState->fMeanZeroCrossing = 0.0f;
   vadState->lSilenceCounter = 0;
   vadState->lUpdateCounter = 0;
   vadState->lSmoothingCounter = 0;
   vadState->lLessEnergyCounter = 0;
   vadState->lFVD = 1;
   vadState->fMinEnergy = IPP_MAXABS_32F;
   return;
}

void VADGetSize(Ipp32s *pDstSize)
{
   *pDstSize = sizeof(VADmemory);
   return;
}

static __ALIGN32 CONST Ipp32f ToplAutoCorrMtr[LPC_ORDERP2+1]={
    0.120089698456645f, 0.21398822343783f, 0.14767692339633f,
    0.07018811903116f, 0.00980856433051f, -0.02015934721195f,
    -0.02388269958005f, -0.01480076155002f, -0.00503292155509f,
    0.00012141366508f, 0.00119354245231f, 0.00065908718613f,
    0.00015015782285f
};

void VoiceActivityDetect_G729_32f(Ipp32f ReflectCoeff, Ipp32f *pLSF, Ipp32f *pAutoCorr, Ipp32f *pSrc, Ipp32s FrameCounter,
                                  Ipp32s prevDecision, Ipp32s prevPrevDecision, Ipp32s *pVad, Ipp32f *pEnergydB,Ipp8s *pVADmem,Ipp32f *pExtBuff)
{
    Ipp32f *pTmp;
    Ipp32f fSpectralDistortion, fFullBandEnergyDiff, fLowBandEnergyDiff, lNumZeroCrossing, fZeroCrossingDiff;
    Ipp32f fLowBandEnergy;
    Ipp32f fFullBandEnergy;
    Ipp32f zeroNum;
    Ipp32s i;
    static __ALIGN32 CONST Ipp32f vadTable[7][6]={
      /* coeff C_coeff coeffZC C_coeffZC coeffSD C_coeffSD  */
       { 0.75f,  0.25f,   0.8f,   0.2f,  0.6f,  0.4f},
       { 0.75f,  0.25f,   0.8f,   0.2f,  0.6f,  0.4f},
       { 0.95f,  0.05f,  0.92f,  0.08f, 0.65f, 0.35f},
       { 0.97f,  0.03f,  0.94f,  0.06f, 0.70f,  0.3f},
       { 0.99f,  0.01f,  0.96f,  0.04f, 0.75f, 0.25f},
       {0.995f, 0.005f,  0.99f,  0.01f, 0.75f, 0.25f},
       {0.995f, 0.005f, 0.998f, 0.002f, 0.75f, 0.25f},
    };
    const Ipp32f *pVadTable;
    VADmemory *vadState = (VADmemory *)pVADmem;

    pTmp = &pExtBuff[0]; /*10 elements*/

    /* compute the frame energy, full-band energy */
    fFullBandEnergy = 10.0f * (Ipp32f) log10( pAutoCorr[0]/240.0f + IPP_MINABS_32F);
    *pEnergydB = fFullBandEnergy ;

    /* compute the low-band energy (El)*/
    ippsDotProd_32f(pAutoCorr, ToplAutoCorrMtr, LPC_ORDERP2+1, &fLowBandEnergy);
    if (fLowBandEnergy < 0.0f) fLowBandEnergy = 0.0f;
    fLowBandEnergy= 10.0f * (Ipp32f) log10((Ipp32f) (fLowBandEnergy/120.0f + IPP_MINABS_32F));

    /* Normalize line spectral frequences */
    for(i=0; i<LPC_ORDER; i++) pLSF[i] /= (Ipp32f)IPP_2PI;
    /* compute spectral distortion */
    ippsSub_32f(pLSF, vadState->MeanLSFVec, pTmp, LPC_ORDER);
    ippsDotProd_32f(pTmp, pTmp, LPC_ORDER, &fSpectralDistortion);

    /* compute # zero crossing */
    // IPP 5.3 has no ippsZeroCrossing()
    // IPP 7.0 has no ippsSignChangeRate())
    // http://software.intel.com/sites/products/documentation/hpc/ipp/ipps/ipps_ch5/functn_ZeroCrossing.html
    #if IPP_VERSION_MAJOR < 7
        ippsSignChangeRate_32f(&pSrc[ZC_START_INDEX], ZC_END_INDEX+1-ZC_START_INDEX, &zeroNum);
    #else
        ippsZeroCrossing_32f(&pSrc[ZC_START_INDEX], ZC_END_INDEX+1-ZC_START_INDEX, &zeroNum, ippZCR);
    #endif
    lNumZeroCrossing = zeroNum/80;

    /* Initialize and update min energies */
    if( FrameCounter < 129 ) {
        if( fFullBandEnergy < vadState->fMinEnergy ){
            vadState->fMinEnergy = fFullBandEnergy;
            vadState->fPrevMinEnergy = fFullBandEnergy;
        }
        if( (FrameCounter % 8) == 0){
            vadState->MinimumBuff[FrameCounter/8 -1] = vadState->fMinEnergy;
            vadState->fMinEnergy = IPP_MAXABS_32F;
        }
    }
    if( (FrameCounter % 8) == 0){
        ippsMin_32f(vadState->MinimumBuff,15,&vadState->fPrevMinEnergy);
    }

    if( FrameCounter >= 129 ) {
        if( (FrameCounter % 8 ) == 1) {
            vadState->fMinEnergy = vadState->fPrevMinEnergy;
            vadState->fNextMinEnergy = IPP_MAXABS_32F;
        }
        if( fFullBandEnergy < vadState->fMinEnergy )
            vadState->fMinEnergy = fFullBandEnergy;
        if( fFullBandEnergy < vadState->fNextMinEnergy )
            vadState->fNextMinEnergy = fFullBandEnergy;
        if( (FrameCounter % 8) == 0){
            for ( i =0; i< 15; i++)
                vadState->MinimumBuff[i] = vadState->MinimumBuff[i+1];
            vadState->MinimumBuff[15]  = vadState->fNextMinEnergy;
            ippsMin_32f(vadState->MinimumBuff,16,&vadState->fPrevMinEnergy);

        }
    }

    if (FrameCounter <= END_OF_INIT){
        if( fFullBandEnergy < 21.0f){
            vadState->lLessEnergyCounter++;
            *pVad = VAD_NOISE;
        }
        else{
            *pVad = VAD_VOICE;
            vadState->fMeanEnergy = (vadState->fMeanEnergy*( (Ipp32f)(FrameCounter-vadState->lLessEnergyCounter -1)) +
                fFullBandEnergy)/(Ipp32f) (FrameCounter-vadState->lLessEnergyCounter);
            vadState->fMeanZeroCrossing = (vadState->fMeanZeroCrossing*( (Ipp32f)(FrameCounter-vadState->lLessEnergyCounter -1)) +
                lNumZeroCrossing)/(Ipp32f) (FrameCounter-vadState->lLessEnergyCounter);
            ippsInterpolateC_G729_32f(vadState->MeanLSFVec, (Ipp32f)(FrameCounter-vadState->lLessEnergyCounter -1),
                                             pLSF, 1.0f, vadState->MeanLSFVec, LPC_ORDER);
            ippsMulC_32f_I(1.0f/(Ipp32f) (FrameCounter-vadState->lLessEnergyCounter ), vadState->MeanLSFVec, LPC_ORDER);
        }
    }

    if (FrameCounter >= END_OF_INIT ){
        if (FrameCounter == END_OF_INIT ){
            vadState->fMeanFullBandEnergy = vadState->fMeanEnergy - 10.0f;
            vadState->fMeanLowBandEnergy = vadState->fMeanEnergy - 12.0f;
        }

        fFullBandEnergyDiff = vadState->fMeanFullBandEnergy - fFullBandEnergy;
        fLowBandEnergyDiff = vadState->fMeanLowBandEnergy - fLowBandEnergy;
        fZeroCrossingDiff = vadState->fMeanZeroCrossing - lNumZeroCrossing;

        if( fFullBandEnergy < 21.0f ){
            *pVad = VAD_NOISE;
        }
        else{
            *pVad =MakeDecision(fLowBandEnergyDiff, fFullBandEnergyDiff, fSpectralDistortion, fZeroCrossingDiff );
        }

        vadState->lVADFlag =VAD_NOISE;
        if( (prevDecision == VAD_VOICE) && (*pVad == VAD_NOISE) &&
            (fFullBandEnergy > vadState->fMeanFullBandEnergy + 2.0f) && ( fFullBandEnergy > 21.0f)){
            *pVad = VAD_VOICE;
            vadState->lVADFlag=VAD_VOICE;
        }

        if((vadState->lFVD == 1) ){
            if( (prevPrevDecision == VAD_VOICE) && (prevDecision == VAD_VOICE) &&
                (*pVad == VAD_NOISE) && (fabs(vadState->fPrevEnergy - fFullBandEnergy)<= 3.0f)){
                vadState->lSmoothingCounter++;
                *pVad = VAD_VOICE;
                vadState->lVADFlag=VAD_VOICE;
                if(vadState->lSmoothingCounter <=4)
                    vadState->lFVD =1;
                else{
                    vadState->lFVD =0;
                    vadState->lSmoothingCounter=0;
                }
            }
        }
        else
            vadState->lFVD =1;

        if(*pVad == VAD_NOISE)
            vadState->lSilenceCounter++;

        if((*pVad == VAD_VOICE) && (vadState->lSilenceCounter > 10) &&
            ((fFullBandEnergy - vadState->fPrevEnergy) <= 3.0f)){
            *pVad = VAD_NOISE;
            vadState->lSilenceCounter=0;
        }


        if(*pVad == VAD_VOICE)
            vadState->lSilenceCounter=0;

        if ((fFullBandEnergy < vadState->fMeanFullBandEnergy + 3.0f) && ( FrameCounter >128)
             &&( !vadState->lVADFlag) && (ReflectCoeff < 0.6f) )
             *pVad = VAD_NOISE;

        if ((fFullBandEnergy < vadState->fMeanFullBandEnergy + 3.0f) && (ReflectCoeff < 0.75f) && ( fSpectralDistortion < 0.002532959f)){
            vadState->lUpdateCounter++;
            i = vadState->lUpdateCounter/10;
            if(i>6) i=6;
            pVadTable = vadTable[i];
            ippsInterpolateC_G729_32f(vadState->MeanLSFVec, pVadTable[4],
                                             pLSF, (pVadTable[5]), vadState->MeanLSFVec, LPC_ORDER);
            vadState->fMeanFullBandEnergy = pVadTable[0]*vadState->fMeanFullBandEnergy+(pVadTable[1])*fFullBandEnergy;
            vadState->fMeanLowBandEnergy = pVadTable[0]*vadState->fMeanLowBandEnergy+(pVadTable[1])*fLowBandEnergy;
            vadState->fMeanZeroCrossing = pVadTable[2]*vadState->fMeanZeroCrossing+(pVadTable[3])*lNumZeroCrossing;
        }

        if((FrameCounter > 128) && ( (  vadState->fMeanFullBandEnergy < vadState->fMinEnergy )
            && ( fSpectralDistortion < 0.002532959f)) || ( vadState->fMeanFullBandEnergy > vadState->fMinEnergy + 10.0f )){
            vadState->fMeanFullBandEnergy = vadState->fMinEnergy;
            vadState->lUpdateCounter = 0;
        }
    }

    vadState->fPrevEnergy = fFullBandEnergy;
    return;
}



