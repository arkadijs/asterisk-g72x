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
// Purpose: USC VAD G723 header file.
//
*/
#ifndef __VADG723_H__
#define __VADG723_H__

#include <ipps.h>
#include "owng723.h"

struct _G723VADmemory {
   Ipp32s  PrevEnergy;
   Ipp32s  NoiseLevel;
   Ipp16s  HangoverCounter;
   Ipp16s  VADCounter;
   Ipp32s  AdaptEnableFlag;
   Ipp16s  OpenLoopDelay[4];
/*--------------*/
   G723_Obj_t          objPrm;  /* must be on top     */
   Ipp16s              PrevOpenLoopLags[2];
   Ipp16s              prevSidLpc[G723_LPC_ORDERP1];
   Ipp16s              SineWaveDetector;/* Sine wave detector */
   G723_FrameType      PastFrameType;
   Ipp32s              HPFltMem[2];/* High pass variables */
   ScratchMem_Obj      Mem;
};

void VoiceActivityDetectSize_G723(Ipp32s* pVADsize);
void VoiceActivityDetectInit_G723(Ipp8s* pVADmem);
void VoiceActivityDetect_G723(const Ipp16s *pSrc, const Ipp16s *pNoiseLPC,
         const Ipp16s *pOpenLoopDelay, Ipp32s SineWaveDetector, Ipp32s *pVADDecision, Ipp32s *pAdaptEnableFlag, Ipp8s* pVADmem, Ipp16s *AlignBuff);

#endif /*__VADG723_H__*/
