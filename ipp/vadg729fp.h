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
// Purpose: USC VAD G729fp header file.
//
*/
#ifndef __VADG729FP_H__
#define __VADG729FP_H__

#include <ipps.h>
#include <ippsr.h>
#include "owng729fp.h"

struct _G729FPVADmemory_Obj{
   Ipp32f MeanLSFVec[LPC_ORDER];
   Ipp32f MinimumBuff[16];
   Ipp32f fMeanEnergy;
   Ipp32f fMeanFullBandEnergy;
   Ipp32f fMeanLowBandEnergy;
   Ipp32f fMeanZeroCrossing;
   Ipp32f fPrevMinEnergy;
   Ipp32f fNextMinEnergy;
   Ipp32f fMinEnergy;
   Ipp32f fPrevEnergy;
   Ipp32s lVADFlag;
   Ipp32s lSilenceCounter;
   Ipp32s lUpdateCounter;
   Ipp32s lSmoothingCounter;
   Ipp32s lFVD;
   Ipp32s lLessEnergyCounter;
/*---------------------------*/
   G729CCoder_Obj      objPrm;
   ScratchMem_Obj      Mem;
   Ipp32f OldSpeechBuffer[SPEECH_BUFF_LEN];
   Ipp16s sFrameCounter; /* frame counter for VAD*/
   Ipp32s prevVADDec;
   Ipp32s prevPrevVADDec;
   Ipp32f OldLSP[LPC_ORDER];
   Ipp32f OldForwardLPC[LPC_ORDERP1];
   Ipp32f OldForwardRC[2];
};

void VADInit(Ipp8s *vadMem);
void VADGetSize(Ipp32s *pDstSize);
void VoiceActivityDetect_G729_32f(Ipp32f ReflectCoeff, Ipp32f *pLSF, Ipp32f *pAutoCorr, Ipp32f *pSrc, Ipp32s FrameCounter,
                                  Ipp32s prevDecision, Ipp32s prevPrevDecision, Ipp32s *pVad, Ipp32f *pEnergydB,Ipp8s *pVADmem,Ipp32f *pExtBuff);

#endif /*__VADG729FP_H__*/
