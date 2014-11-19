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
// Purpose: USC VAD G729 header file.
//
*/
#ifndef __VADG729_H__
#define __VADG729_H__

#include <ipps.h>
#include "owng729.h"

struct _VADmemory {
/**/    G729Coder_Obj       objPrm;
/**/    ScratchMem_Obj      Mem;
/**/    Ipp16s   speechHistory[SPEECH_BUF_DIM];
/**/    Ipp16s   BWDsynth[TBWD_DIM];
/**/    Ipp16s   encPrm[19];
/**/    Ipp16s   prevSubfrLPC[LPF_DIM+1];
/**/    Ipp8s    *preProc;/*highpasspreprocessingfiltermemory*/

    Ipp16s   LSFMean[LPF_DIM];
    Ipp16s   minBuf[16];
    Ipp16s   musicRC[10];
    Ipp16s   VADPrev;
    Ipp16s   VADPPrev;
    Ipp16s   minPrev;
    Ipp16s   minNext;
    Ipp16s   minVAD;
    Ipp16s   EMean;
    Ipp16s   SEMean;
    Ipp16s   SLEMean;
    Ipp16s   SZCMean;
    Ipp16s   VADPrevEnergy;
    Ipp16s   SILcounter;
    Ipp16s   updateCounter;
    Ipp16s   extCounter;
    Ipp16s   VADflag;
    Ipp16s   VADflag2;
    Ipp16s   lessCounter;
    Ipp16s   frameCounter;
    Ipp16s   musicCounter;
    Ipp16s   musicSEMean;
    Ipp16s   musicMCounter;
    Ipp16s   conscCounter;
    Ipp16s   conscCounterFlagP;
    Ipp16s   MeanPgain;
    Ipp16s   count_pflag;
    Ipp16s   conscCounterFlagR;
    Ipp16s   Mcount_pflag;
};

//void SynthesisFilterSize_G729 (Ipp32s *pSize);
void VoiceActivityDetectSize_G729(Ipp32s *pSrc);
void VoiceActivityDetectInit_G729(Ipp8s *pSrc);
void VoiceActivityDetect_G729(Ipp16s *pSrc,Ipp16s *pLSF,Ipp32s *pAutoCorr,
                              Ipp16s autoExp, Ipp16s rc, Ipp16s *pVad, Ipp8s*pVADmem,Ipp16s *pTmp);

#endif /*__VADG729_H__*/
