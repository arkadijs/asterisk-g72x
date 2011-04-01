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
// Purpose: G.723.1 speech codec: main own header file.
//
*/

#ifndef __OWNG723_H__
#define __OWNG723_H__

#if defined( _WIN32_WCE)
#pragma warning( disable : 4505 )
#endif

#include "g723ipp.h"
#include "g723api.h"
#include "scratchmem.h"

/* G.723.1 coder global constants */
#define  G723_FRM_LEN                  240
#define  G723_SBFR_LEN            (G723_FRM_LEN/4)
#define  G723_HALFFRM_LEN         (G723_FRM_LEN/2)
#define  G723_LPC_ORDER                 10                  /*order of LPC filters */
#define  G723_LPC_ORDERP1               11                  /*order of LPC filters plus 1*/
#define  SmoothCoeff                (Ipp32s) 0xffffc000     /* -0.25 */
#define  G723_MIN_PITCH                 18
#define  AdCdbkSizeHighRate             85
#define  G723_MAX_PITCH              ((G723_MIN_PITCH)+127)
#define  GRIDSIZE                        2
#define  AdCdbkSizeLowRate             170
#define  N_PULSES                        6
#define  G723_TOEPLIZ_MATRIX_SIZE      416
#define  N_GAINS                        24
#define  N_AUTOCORRS_BLOCKS              3  /* N frames for AutoCorrs averaging     */
#define  N_GAIN_AVER_FRMS                3  /* N frames for gain averaging    */
#define  NUM_PULSE_IN_BLOCK             11 /* N pulses per block             */
#define  INV_NUM_PULSE_IN_BLOCK       2979 /* 1/NUM_PULSE_IN_BLOCK                  */
#define  MAX_GAIN                       50 /* Maximum gain in CNG excitation    */
#define  AUOTOCORRS_BUFF_SIZE        ((N_AUTOCORRS_BLOCKS+1)*(G723_LPC_ORDER+1)) /* size of AutoCorrs array    */
#define  G723_MAX_GAIN               10000  /* Maximum gain for fixed CNG excitation   */

typedef enum {
   G723_Rate63 = 0,
   G723_Rate53
}G723_Rate;

typedef enum {
   G723_UntransmittedFrm = 0,
   G723_ActiveFrm,
   G723_SIDFrm
}G723_FrameType;

typedef  struct   {
   Ipp16s    isBadFrame;
   G723_FrameType     FrameType;
   G723_Rate currRate;
   Ipp32s    lLSPIdx ;
   Ipp16s    PitchLag[2] ;
   Ipp16s    AdCdbkLag[4];
   Ipp16s    AdCdbkGain[4];
   Ipp16s    sAmpIndex[4];
   Ipp16s    sGrid[4];
   Ipp16s    sTrainDirac[4];
   Ipp16s    sAmplitude[4];
   Ipp32s    sPosition[4];
} ParamStream_G723;

typedef  struct   {
   Ipp16s   sDelay;
   Ipp16s   sGain;
   Ipp16s   sScalingGain;
} GainInfo_G723;

void DecoderCNG_G723(G723Decoder_Obj* decoderObj, ParamStream_G723 *CurrentParams, Ipp16s *pExcitation, Ipp16s *pDstLPC);

void  FixedCodebookSearch_G723_16s(G723Encoder_Obj *encoderObj, ParamStream_G723 *Params, Ipp16s *pSrcDst, Ipp16s *ImpResp, Ipp16s Sfc);
void  InterpolationIndex_G723_16s( Ipp16s *pDecodedExc, Ipp16s sPitchLag, Ipp16s *pGain, Ipp16s *pGainSFS, Ipp16s *pDstIdx);
void ErrorUpdate_G723(Ipp32s *pError, Ipp16s openLoopLag, Ipp16s AdCbbkLag, Ipp16s AdCbbkGain, G723_Rate currRate);
void  PostFilter(G723Decoder_Obj* decoderObj, Ipp16s *pSrcDstSignal, Ipp16s *pSrcLPC );
void  LSPInterpolation(const Ipp16s *pSrcLSP, const Ipp16s *pSrcPrevLSP, Ipp16s *pDstLPC);
void    SetParam2Bitstream(G723Encoder_Obj* encoderObj, ParamStream_G723 *Params, Ipp8s *pDstBitStream);
void GetParamFromBitstream( const Ipp8s *pSrcBitStream, ParamStream_G723 *Params);
void   UpdateSineDetector(Ipp16s *SineWaveDetector, Ipp16s *isNotSineWave);
extern Ipp16s   LSFCTbl[G723_LPC_ORDER] ;
extern Ipp16s   PerceptualFltCoeffTbl[2*G723_LPC_ORDER] ;
extern Ipp16s   GainDBLvls[N_GAINS] ;
extern IppSpchBitRate SA_Rate[2];

void EncoderCNG_G723(G723Encoder_Obj* encoderObj, ParamStream_G723 *Params, Ipp16s *pExcitation, Ipp16s *pDstLPC);
void UpdateAutoCorrs_G723(G723Encoder_Obj* encoderObj, const Ipp16s *pSrcAutoCorrs, const Ipp16s *pSrcAutoCorrsSFS);
void DecodeSIDGain_G723_16s (Ipp32s pIndx, Ipp16s *pGain);

#define ComfortNoiseExcitation_G723_16s_Buff_Size (2*NUM_PULSE_IN_BLOCK+2*NUM_PULSE_IN_BLOCK+G723_SBFR_LEN/GRIDSIZE+4+2*G723_SBFR_LEN)*sizeof(Ipp16s)

void ComfortNoiseExcitation_G723_16s (Ipp16s gain, Ipp16s *pPrevExc, Ipp16s *pExc,
                                      Ipp16s *pSeed, Ipp16s *pOlp, Ipp16s *pLags,
                                      Ipp16s *pGains, G723_Rate currRate, Ipp8s *buff, Ipp16s *CasheCounter);
void FixedCodebookGain_G723_16s(const Ipp16s *pSrc1, const Ipp16s *pSrc2,
                                Ipp16s *pGainCoeff, Ipp32s *pGainIdx, Ipp16s *y);
void ExcitationResidual_G723_16s(const Ipp16s *pSrc1, const Ipp16s *pSrc2,  Ipp16s *pSrcDst,G723Encoder_Obj *encoderObj);

void FixedCodebookVector_G723_16s( Ipp32s lDecPos, Ipp32s lAmplitude, Ipp32s mamp, Ipp32s lGrid,
       Ipp32s lGain, Ipp32s lNSbfr, G723_Rate currRate, Ipp16s *pDst, Ipp32s *pLag, Ipp16s *pGain );

void QuantSIDGain_G723_16s(const Ipp16s *pSrc, const Ipp16s *pSrcSfs, Ipp32s len, Ipp32s *pIndx);

void ResidualInterpolation_G723_16s_I
        (Ipp16s *pSrcDst, Ipp16s *pDst,  Ipp32s lag, Ipp16s gain, Ipp16s *pSeed);

struct _G723Encoder_Obj{
   G723_Obj_t          objPrm;  /* must be on top     */

   Ipp16s              ZeroSignal[G723_SBFR_LEN];
   Ipp16s              UnitImpulseSignal[G723_SBFR_LEN];
   Ipp16s              PrevWeightedSignal[G723_MAX_PITCH+3];            /* All pitch operation buffers */
   Ipp16s              FltSignal[G723_MAX_PITCH+G723_SBFR_LEN+3];     /* shift 3 - aligning */
   Ipp16s              PrevExcitation[G723_MAX_PITCH+3];
   Ipp16s              SignalDelayLine[G723_HALFFRM_LEN];/* Required memory for the delay */
   Ipp16s              WeightedFltMem[2*G723_LPC_ORDER];/* Used delay lines */
   Ipp16s              RingWeightedFltMem[2*G723_LPC_ORDER];/* formant perceptual weighting filter delay line */
   Ipp16s              RingSynthFltMem[G723_LPC_ORDER]; /* synthesis filter delay line */
   Ipp16s              PrevOpenLoopLags[2];
   Ipp16s              PrevLSF[G723_LPC_ORDER];/* LSF previous vector */
   Ipp16s              AverEnerCounter;
   Ipp16s              PrevQGain;
   Ipp16s              SIDLSP[G723_LPC_ORDER];
   Ipp16s              CNGSeed;
   Ipp16s              CurrGain;
   Ipp16s              LPCSID[G723_LPC_ORDER];
   Ipp16s              ReflectionCoeffSFS;
   Ipp16s              sSearchTime;                 /* for ippsFixedCodebookSearch_G723_16s function*/
   Ipp16s              prevSidLpc[G723_LPC_ORDERP1];
   Ipp16s              SineWaveDetector;/* Sine wave detector */
   Ipp16s              sSidGain;
   Ipp16s              ReflectionCoeff[G723_LPC_ORDER+1];
   G723_FrameType      PastFrameType;
   Ipp16s              AutoCorrs[AUOTOCORRS_BUFF_SIZE];
   Ipp16s              AutoCorrsSFS[N_AUTOCORRS_BLOCKS+1];
   Ipp16s              GainAverEnergies[N_GAIN_AVER_FRMS];
   Ipp32s              ExcitationError[5];/* Taming procedure errors */
   Ipp32s              HPFltMem[2];/* High pass variables */
   Ipp8s               *vadMem;               /* VAD memory */
   Ipp32s              AdaptEnableFlag;
   Ipp16s              CasheCounter;
   ScratchMem_Obj      Mem;
};

struct _G723Decoder_Obj{
   G723_Obj_t          objPrm;  /* must be on top     */

   Ipp16s              PrevExcitation[G723_MAX_PITCH+3]; /* 3 - shift for aligning */
   Ipp16s              PostFilterMem[2*G723_LPC_ORDER]; /* Fir+Iir delays */
   Ipp16s              PrevLSF[G723_LPC_ORDER];/* LSF previous vector */
   Ipp16s              ErasedFramesCounter;
   Ipp16s              InterpolatedGain;
   Ipp16s              SyntFltIIRMem[G723_LPC_ORDER];/* Used delay lines */
   Ipp16s              InterpolationIdx;
   Ipp16s              ResIntSeed;
   Ipp16s              SIDLSP[G723_LPC_ORDER];
   Ipp16s              ReflectionCoeff;
   Ipp16s              PstFltGain;
   Ipp16s              CurrGain;
   G723_FrameType      PastFrameType;
   Ipp16s              sSidGain;
   Ipp16s              CNGSeed;
   Ipp16s              CasheCounter;
   ScratchMem_Obj      Mem;
};

#define   G723_CODECFUN(type,name,arg)                extern type name arg

#include "aux_fnxs.h"

#endif /* __OWNG723_H__ */
