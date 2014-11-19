/*/////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2004-2011 Intel Corporation. All Rights Reserved.
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
// Purpose: G.729 floating-point speech codec: main own header file.
//
*/

#ifndef __OWNG729FP_H__
#define __OWNG729FP_H__

#include <ipps.h>
#include <ippsc.h>
#include "g729fpapi.h"
#include "scratchmem.h"

#define ENC_KEY 0xecd729
#define DEC_KEY 0xdec729

#define G729FP_ENCODER_SCRATCH_MEMORY_SIZE   (2*7168+40)

#define SPEECH_BUFF_LEN 240     /* Total size of speech buffer               */
#define FRM_LEN         80      /* LPC update frame size                     */
#define SUBFR_LEN       40      /* Sub-frame size                            */
#define NUN_SUBFRAMES   FRM_LEN/SUBFR_LEN

#define WINDOW_LEN        240     /* LPC analysis window size                  */
#define LOOK_AHEAD_LEN          40      /* Samples of next frame needed for LPC ana. */

#define LPC_ORDER       10      /* LPC order                                 */
#define LPC_ORDERP1     (LPC_ORDER+1)    /* LPC order+1                               */

#define MOVING_AVER_ORDER          4       /* MA prediction order for LSP                     */
#define N_BITS_1ST_STAGE           7       /* number of bits in first stage for LSP index     */
#define N_ELEM_1ST_STAGE          (1<<N_BITS_1ST_STAGE) /* number of entries in first stage for LSP index  */
#define N_BITS_2ND_STAGE           5       /* number of bits in second stage for LSP index    */
#define N_ELEM_2ND_STAGE          (1<<N_BITS_2ND_STAGE) /* number of entries in second stage for LSP index */

#define LSF_LOW_LIMIT         (Ipp32f)0.005
#define LSF_HI_LIMIT          (Ipp32f)3.135
#define LSF_DIST              (Ipp32f)0.0392

#ifdef CLIPPING_DENORMAL_MODE
   #define DENORMAL_THRESHOLD 1.0E-7

   #define CLIP_DENORMAL(src,dst) if(fabs(src) < DENORMAL_THRESHOLD) dst = 0;\
                                 else dst = src
   #define CLIP_DENORMAL_I(srcdst) if(fabs(srcdst) < DENORMAL_THRESHOLD) srcdst = 0;
#else

   #define CLIP_DENORMAL(src,dst) dst = src
   #define CLIP_DENORMAL_I(srcdst)
#endif

#define LAR_THRESH1   (Ipp32f)-1.74
#define LAR_THRESH2   (Ipp32f)-1.52
#define LAR_THRESH3   (Ipp32f)0.65
#define LAR_THRESH4   (Ipp32f)0.43
#define GAMMA1_TILTED         (Ipp32f)0.98
#define GAMMA2_TILTED_MAX     (Ipp32f)0.7
#define GAMMA2_TILTED_MIN     (Ipp32f)0.4
#define GAMMA1_FLAT           (Ipp32f)0.94
#define GAMMA2_FLAT           (Ipp32f)0.6
#define GAMMA2_TILTED_SCALE   (Ipp32f)-6.0
#define GAMMA2_TILTED_SHIFT   (Ipp32f)1.0

#define PITCH_LAG_MIN   20      /* Minimum pitch lag in samples              */
#define PITCH_LAG_MAX   143     /* Maximum pitch lag in samples              */
#define INTERPOL_LEN    (10+1)  /* Length of filter for interpolation.       */
#define INTER_PITCH_LEN 10      /* Length for pitch interpolation            */
#define INTERPOL4_LEN   4       /* Upsampling ration for pitch search        */
#define UP_SAMPLING     3       /* Resolution of fractional delays           */
#define PITCH_THRESH    0.85f   /* Threshold to favor smaller pitch lags     */
#define GAIN_PIT_MAX    1.2f    /* Maximum adaptive codebook gain            */

/* Constants for fixed codebook search. */
#define TOEPLIZ_MATRIX_SIZE  616 /* size of correlation matrix                            */

#define PITCH_SHARPMAX        (Ipp32f)0.7945  /* Maximum value of pitch sharpening */
#define PITCH_SHARPMIN        (Ipp32f)0.2     /* minimum value of pitch sharpening */

/* Constants for taming procedure.*/
#define MAX_GAIN_TIMING      (Ipp32f)0.95     /* Maximum pitch gain if taming is needed */
#define MAX_GAIN_TIMING2     (Ipp32f)0.94     /* Maximum pitch gain if taming is needed */
#define THRESH_ERR           (Ipp32f)60000.   /* Error threshold taming    */
#define INV_SUBFR_LEN (Ipp32f) ((Ipp32f)1./(Ipp32f)SUBFR_LEN) /* =0.025 */

/* Constants for postfilter */
/* Ipp16s term pst parameters :  */
#define GAMMA1_POSTFLT      (Ipp32f)0.7     /* denominator weighting factor           */
#define GAMMA2_POSTFLT      (Ipp32f)0.55    /* numerator  weighting factor            */
#define SHORTTERM_POSTFLT_LEN       20      /* impulse response length                   */
#define GAMMA3_POSTFLT_P    (Ipp32f)0.2     /* tilt weighting factor when k1>0        */
#define GAMMA3_POSTFLT_M    (Ipp32f)0.9     /* tilt weighting factor when k1<0        */

/* long term pst parameters :   */
#define SUBFR_LENP1 (SUBFR_LEN + 1) /* Sub-frame size + 1                        */
#define FRAC_DELAY_RES        8       /* resolution for fractionnal delay          */

#define SHORT_INT_FLT_LEN     4       /* length of Ipp16s interp. subfilters        */
#define LONG_INT_FLT_LEN      16      /* length of long interp. subfilters         */
#define SHORT_INT_FLT_LEN_BY2    (SHORT_INT_FLT_LEN/2)
#define LONG_INT_FLT_LEN_BY2     (LONG_INT_FLT_LEN/2)

#define LTPTHRESHOLD    (Ipp32f)0.5f    /* threshold LT to switch off postfilter */
#define AGC_FACTOR      (Ipp32f)0.9875  /* gain adjustment factor                 */

#define AGC_FACTORM1    ((Ipp32f)1. - AGC_FACTOR)    /* gain adjustment factor                 */

/* Array sizes */
#define RESISDUAL_MEMORY (PITCH_LAG_MAX + 1 + LONG_INT_FLT_LEN_BY2)
#define SIZE_RESISDUAL_MEMORY (RESISDUAL_MEMORY + SUBFR_LEN)
#define SIZE_SEARCH_DEL_MEMORY  ((FRAC_DELAY_RES-1) * SUBFR_LENP1)
#define SIZE_LONG_INT_FLT_MEMORY ((FRAC_DELAY_RES-1) * LONG_INT_FLT_LEN)
#define SIZE_SHORT_INT_FLT_MEMORY ((FRAC_DELAY_RES-1) * SHORT_INT_FLT_LEN)

#define G729D_MODE      0      /* Low  rate  (6400 bit/s)       */
#define G729_BASE       1      /* Full rate  (8000 bit/s)       */
#define G729E_MODE      2      /* High rate (11800 bit/s)       */

/* backward LPC analysis parameters */
#define BWD_LPC_ORDER         30         /* Order of Backward LP filter.              */
#define BWD_LPC_ORDERP1               (BWD_LPC_ORDER+1)  /* Order of Backward LP filter + 1           */
#define NON_RECURSIVE_PART    35
#define BWD_SYNTH_MEM           (BWD_LPC_ORDER + NON_RECURSIVE_PART)
#define BWD_ANALISIS_WND_LEN             (FRM_LEN + BWD_SYNTH_MEM)
#define BWD_GAMMA 0.98f

/* Annex E adaptive Ipp16s term postfilter parameters:*/
#define GAMMA1_POSTFLT_E  0.7f      /* denominator weighting factor */
#define GAMMA2_POSTFLT_E  0.65f     /* numerator  weighting factor */
#define SHORTTERM_POSTFLT_LEN_E   32        /* Lenght of the impulse response*/
#define GAMMA_HARM_POSTFLT_E 0.25f
#define GAMMA_HARM_POSTFLT   0.5f

/* Constants for backward/forward decision*/
#define THRES_ENERGY 40.f /*Low energy frame threshold*/
/* Gains levels */
#define TH1 1.f
#define TH2 2.f
#define TH3 3.f
#define TH4 4.f
#define TH5 4.7f
#define GAP_FACT (Ipp32f)0.000114375f
#define INVERSE_LOG2 (Ipp32f) (1./log10(2.))

/*Constants for gain quantization.*/
#define MEAN_ENER        (Ipp32f)36.0              /* average innovation energy */
#define NCODE1_BITS  3                             /* number of Codebook-bit */
#define NCODE2_BITS  4                             /* number of Codebook-bit */
#define SIZECODEBOOK1    (1<<NCODE1_BITS)          /* Codebook 1 size */
#define SIZECODEBOOK2    (1<<NCODE2_BITS)          /* Codebook 2 size */
#define NUM_CAND1            4                     /* Pre-selecting order for #1 */
#define NUM_CAND2            8                     /* Pre-selecting order for #2 */
#define INV_COEF_BASE   (Ipp32f)-0.032623

/*Constants for gain quantization in Annex D mode*/
#define NCODE1_B_ANNEXD  3                         /* number of Codebook-bit */
#define NCODE2_B_ANNEXD  3                         /* number of Codebook-bit */
#define SIZECODEBOOK1_ANNEXD (1<<NCODE1_B_ANNEXD)  /* Codebook 1 size */
#define SIZECODEBOOK2_ANNEXD (1<<NCODE2_B_ANNEXD)  /* Codebook 2 size */
#define NUM_CAND1_ANNEXD  6                        /* Pre-selecting order for #1 */
#define NUM_CAND2_ANNEXD  6                        /* Pre-selecting order for #2 */
#define INV_COEF_ANNEXD  ((Ipp32f)-0.027599)
#define NUM_TRACK_ACELP          4

/* VAD */
#define     LPC_ORDERP2      12                 /* LPC order plus 2*/
#define     VAD_NOISE         0                 /* Non-active frame*/
#define     VAD_VOICE         1                 /* Active frame*/
#define     END_OF_INIT   32
#define     ZC_START_INDEX      120
#define     ZC_END_INDEX        200

/* DTX constants */
#define ENCODER         1
#define DECODER         0
#define INIT_SEED_VAL       11111
#define N_MIN_SIM_RRAMES      3
#define ITAKURATHRESH1         (Ipp32f)1.1481628/2.
#define ITAKURATHRESH2         (Ipp32f)1.0966466/2.

#define GAIN_INT_FACTOR        (Ipp32f)0.875
#define INV_GAIN_INT_FACTOR    ((Ipp32f)1. - GAIN_INT_FACTOR)

#define MIN_ENER        (Ipp32f)0.1588489319   /*- 8 dB threshold*/

/* CNG constants */
#define NORM_GAUSS      (Ipp32f)3.16227766  /* sqrt(40)xalpha, alpha=0.5 */
#define K_MUL_COEFF              (Ipp32f)3.          /* 4 x (1 - alpha ** 2), alpha=0.5*/
#define CNG_MAX_GAIN           (Ipp32f)5000.

#define GAMMA1_G729A       (Ipp32f)0.75    /* Bandwitdh expansion for W(z)             */
#define  GAMMA_POSTFLT_G729A      (Ipp32f)0.50       /* Harmonic postfilt factor              */
#define  INV_GAMMA_POSTFLT_G729A  ((Ipp32f)1.0/((Ipp32f)1.0+GAMMA_POSTFLT_G729A))
#define  GAMMA2_POSTFLT_G729A    (GAMMA_POSTFLT_G729A/((Ipp32f)1.0+GAMMA_POSTFLT_G729A))
#define  TILT_FLT_FACTOR          (Ipp32f)0.8        /* Factor for tilt compensation filter   */
#define  AGC_FACTOR_G729A     (Ipp32f)0.9        /* Factor for automatic gain control     */
#define  AGC_FACTOR_1M_G729A     ((Ipp32f)1.-AGC_FACTOR_G729A)
#define  PST_IMPRESP_LEN 22   /* size of truncated impulse response of A(z/g1)/A(z/g2) */

#define CLIP_TO_UPLEVEL(value,maxValue)\
   if(value>maxValue) value=maxValue

#define CLIP_TO_LOWLEVEL(value,minValue)\
   if(value<minValue) value=minValue

void CNGGetSize(Ipp32s *pDstSize);
void CNGInit(Ipp8s *cngMem);
void PSTGetSize(Ipp32s *pDstSize);
void PSTInit(Ipp8s *cngMem);
void MSDGetSize(Ipp32s *pDstSize);
void MSDInit(Ipp8s *msdMem);
void PHDGetSize(Ipp32s *pDstSize);
void PHDInit(Ipp8s *phdMem);

Ipp32s  ExtractBitsG729FP( const Ipp8u **pBits, Ipp32s *nBit, Ipp32s Count );

void ownAutoCorr_G729_32f(Ipp32f *pSrc, Ipp32s len, Ipp32f *pDst, Ipp32f *pExtBuff);
void ownACOS_G729_32f(Ipp32f *pSrc, Ipp32f *pDst, Ipp32s len);
void ownCOS_G729_32f(Ipp32f *pSrc, Ipp32f *pDst, Ipp32s len);
Ipp32f ownAdaptiveCodebookGainCoeff_G729_32f(Ipp32f *pSrcTargetVector, Ipp32f *pSrcFltAdaptivCdbkVec,
               Ipp32f *pDstCorrCoeff, Ipp32s len);

void AdaptiveCodebookGainCoeff_G729_32f( Ipp32f *pSrcTargetVector, Ipp32f *pSrcFltAdaptiveCodebookVector,
                                         Ipp32f *pSrcFltInnovation, Ipp32f *pDstCoeff);

Ipp32s ownAdaptiveCodebookSearch_G729A_32f(Ipp32f *pSrcExc, Ipp32f *pSrcTargetVector, Ipp32f *pSrcImpulseResponse,
  Ipp32s minPitchDelay, Ipp32s maxPitchDelay, Ipp32s nSbfr, Ipp32s *fracPartPitchDelay, Ipp32f *pExtBuff);

void PWGammaFactor_G729(Ipp32f *pGamma1, Ipp32f *pGamma2, Ipp32f *pIntLSF, Ipp32f *CurrLSF,
                   Ipp32f *ReflectCoeff, Ipp32s   *isFlat, Ipp32f *PrevLogAreaRatioCoeff);

void MusicDetection_G729E_32f( G729FPEncoder_Obj *encoderObj, G729Codec_Type codecType, Ipp32f Energy, Ipp32f *ReflectCoeff, Ipp32s *Vad,
                                 Ipp32f EnergydB,Ipp8s *msdMem,Ipp32f *pExtBuff);

void PitchTracking_G729FPE(Ipp32s *T0, Ipp32s *T0_frac, Ipp32s *lPrevPitchPT, Ipp32s *lStatPitchPT, Ipp32s *lStatPitch2PT,  Ipp32s *lStatFracPT);

void OpenLoopPitchSearch_G729_32f(const Ipp32f *pSrc, Ipp32s* bestLag);

void WeightLPCCoeff_G729(Ipp32f *pSrcLPC, Ipp32f valWeightingFactor, Ipp32s len, Ipp32f *pDstWeightedLPC);

Ipp32s TestErrorContribution_G729(Ipp32s valPitchDelay, Ipp32s valFracPitchDelay, Ipp32f *ExcErr);

void UpdateExcErr_G729(Ipp32f valPitchGain, Ipp32s valPitchDelay, Ipp32f *pExcErr);

void isBackwardModeDominant_G729(Ipp32s *isBackwardModeDominant, Ipp32s LPCMode, Ipp32s *pCounterBackward, Ipp32s *pCounterForward);

Ipp32f CalcEnergy_dB_G729(Ipp32f *pSrc, Ipp32s len);

void InterpolatedBackwardFilter_G729(Ipp32f *pSrcDstLPCBackwardFlt, Ipp32f *pSrcPrevFilter, Ipp32f *pSrcDstIntCoeff);

void PhaseDispersionUpdate_G729D(Ipp32f valPitchGain, Ipp32f valCodebookGain, Ipp8s *phdMem);
void PhaseDispersion_G729D(Ipp32f *pSrcExcSignal, Ipp32f *pDstFltExcSignal, Ipp32f valCodebookGain,
                           Ipp32f valPitchGain, Ipp32f *pSrcDstInnovation, Ipp8s *phdMem,Ipp8s *pExtBuff);

void SetLPCMode_G729FPE(G729FPEncoder_Obj* encoderObj, Ipp32f *pSrcSignal, Ipp32f *pSrcForwardLPCFilter,
                        Ipp32f *pSrcBackwardLPCFilter, Ipp32s *pDstLPCMode, Ipp32f *pSrcLSP,Ipp32f *pExtBuff);

Ipp32s AdaptiveCodebookSearch_G729_32f(Ipp32f *pSrcExc, Ipp32f *pSrcTargetVector, Ipp32f *pSrcImpulseResponse, Ipp32s len,
                                       Ipp32s minLag, Ipp32s maxLag, Ipp32s valSubframeNum, Ipp32s *pDstFracPitch, G729Codec_Type codecType,Ipp32f *pExtBuff);

void ComfortNoiseExcitation_G729(Ipp32f fCurrGain, Ipp32f *exc, Ipp16s *sCNGSeed, Ipp32s flag_cod, Ipp32f *ExcitationError, Ipp8s *phdMem, Ipp8s *pExtBuff);

void QuantSIDGain_G729B(Ipp32f *ener, Ipp32s lNumSavedEnergies, Ipp32f *enerq, Ipp32s *idx);

Ipp32s GainQuant_G729(Ipp32f *FixedCodebookExc, Ipp32f *pGainCoeff, Ipp32s lSbfrLen, Ipp32f *gain_pit, Ipp32f *fCodeGain,
                      Ipp32s tamingflag, Ipp32f *PastQuantEnergy, G729Codec_Type codecType,Ipp8s *pExtBuff);
void DecodeGain_G729(Ipp32s index, Ipp32f *code, Ipp32s l_subfr, Ipp32f *pitchGain, Ipp32f *codeGain, Ipp32s rate, Ipp32f *PastQuantEnergy);

void Post_G729E(G729FPDecoder_Obj *decoderObj, Ipp32s pitchDelay, Ipp32f *pSignal, Ipp32f *pLPC, Ipp32f *pDstFltSignal, Ipp32s *pVoicing,
                Ipp32s len, Ipp32s lenLPC, Ipp32s Vad);

typedef struct _G729CCoder_Obj {
   Ipp32s         objSize;
   Ipp32s         key;
   Ipp32u         mode;          /* coder mode's */
   G729Codec_Type codecType;
}G729CCoder_Obj;

#define SUMAUTOCORRS_NUM       3
#define SUMAUTOCORRS_SIZE      (SUMAUTOCORRS_NUM * LPC_ORDERP1)
#define CURRAUTOCORRS_NUM      2
#define AUTOCORRS_SIZE         (CURRAUTOCORRS_NUM * LPC_ORDERP1)
#define GAINS_NUM              2

extern CONST Ipp32f InitLSP[LPC_ORDER];
extern CONST Ipp32f InitFrequences[LPC_ORDER];
extern CONST Ipp32f lagBwd[BWD_LPC_ORDER];
extern CONST Ipp32f SIDGainTbl[32];

typedef struct _CNGmemory{
   Ipp32f AutoCorrs[AUTOCORRS_SIZE];
   Ipp32f SumAutoCorrs[SUMAUTOCORRS_SIZE];
   Ipp32f Energies[GAINS_NUM];
   Ipp32s lAutoCorrsCounter;
   Ipp32f fCurrGain;
   Ipp32s lFltChangeFlag;
   Ipp32f SIDQuantLSP[LPC_ORDER];
   Ipp32f ReflectCoeffs[LPC_ORDERP1];
   Ipp32s lNumSavedEnergies;
   Ipp32f fSIDGain;
   Ipp32f fPrevEnergy;
   Ipp32s lFrameCounter0;
}CNGmemory;

typedef struct _PSTmemory{
   Ipp32f STPNumCoeff[SHORTTERM_POSTFLT_LEN_E];  /* s.t. numerator coeff.        */
   Ipp32f STPMemory[BWD_LPC_ORDER];              /* s.t. postfilter memory       */
   Ipp32f ZeroMemory[BWD_LPC_ORDER];             /* null memory to compute h_st  */
   Ipp32f ResidualMemory[SIZE_RESISDUAL_MEMORY]; /* A(gamma2) residual           */
   Ipp32f gainPrec;
}PSTmemory;

typedef struct _MusDetectMemory{
   Ipp32s lMusicCounter;
   Ipp32f fMusicCounter;
   Ipp32s lZeroMusicCounter;
   Ipp32f fMeanPitchGain;
   Ipp32s lPFlagCounter;
   Ipp32f fMeanPFlagCounter;
   Ipp32s lConscPFlagCounter;
   Ipp32s lRCCounter;
   Ipp32f MeanRC[10];
   Ipp32f fMeanFullBandEnergy;
}MusDetectMemory;

typedef struct _PHDmemory{
   Ipp32s prevDispState;
   Ipp32f gainMem[6];
   Ipp32f prevCbGain;
   Ipp32s onset;
}PHDmemory;

struct _G729FPEncoder_Obj{
   G729CCoder_Obj      objPrm;
   ScratchMem_Obj      Mem;
   Ipp32f OldSpeechBuffer[SPEECH_BUFF_LEN];
   Ipp32f fBetaPreFilter;
   Ipp32f OldWeightedSpeechBuffer[FRM_LEN+PITCH_LAG_MAX];
   Ipp32f OldExcitationBuffer[FRM_LEN+PITCH_LAG_MAX+INTERPOL_LEN];
   Ipp32f WeightedFilterMemory[BWD_LPC_ORDER];
   Ipp32f FltMem[BWD_LPC_ORDER];
   Ipp32f OldLSP[LPC_ORDER];
   Ipp32f OldQuantLSP[LPC_ORDER];
   Ipp32f ExcitationError[4];
   IppsIIRState_32f *iirstate;
   Ipp32f PastQuantEnergy[4];
   Ipp32f PrevFreq[MOVING_AVER_ORDER][LPC_ORDER];    /* previous LSP vector       */
   /* Last forkward A(z) for case of unstable filter */
   Ipp32f OldForwardLPC[LPC_ORDERP1];
   Ipp32f OldForwardRC[2];
   Ipp16s sFrameCounter; /* frame counter for VAD*/
   /* DTX variables */
   Ipp32s prevVADDec;
   Ipp32s prevPrevVADDec;
   Ipp16s sCNGSeed;
   Ipp8s *vadMem;
   Ipp8s *cngMem;
   Ipp8s *msdMem;
   /* G729CA_CODEC*/
   Ipp32f ZeroMemory[LPC_ORDER];
   /* Not G.729A */
   Ipp32f SynFltMemory[BWD_LPC_ORDER];
   Ipp32f ErrFltMemory[BWD_LPC_ORDER+SUBFR_LEN];
   Ipp32f UnitImpulse[SUBFR_LEN+BWD_LPC_ORDERP1];
   /* for G.729E */
   /* for the backward analysis */
   Ipp32f PrevFlt[BWD_LPC_ORDERP1]; /* Previous selected filter */
   Ipp32f SynthBuffer[BWD_ANALISIS_WND_LEN];
   Ipp32s prevLPCMode;
   Ipp32f BackwardLPCMemory[BWD_LPC_ORDERP1];
   Ipp32s isBWDDominant;
   Ipp32f fInterpolationCoeff;
   Ipp16s sGlobalStatInd;  /* Mesure of global stationnarity */
   Ipp16s sBWDStatInd;       /* Num of consecutive backward frames */
   Ipp16s sValBWDStatInd;   /* Value associated with stat_bwd */
   /* Last backward A(z) for case of unstable filter */
   Ipp32f OldBackwardLPC[BWD_LPC_ORDERP1];
   Ipp32f OldBackwardRC[2];
   Ipp32s LagBuffer[5];
   Ipp32f PitchGainBuffer[5];
   Ipp32s  sBWDFrmCounter;
   Ipp32s sFWDFrmCounter;
   Ipp32s isSmooth;
   Ipp32f LogAreaRatioCoeff[2];
   Ipp32s sSearchTimes;
   IppsWinHybridState_G729E_32f *pHWState;
};

struct _G729FPDecoder_Obj{
   G729CCoder_Obj       objPrm;
   ScratchMem_Obj      Mem;
   Ipp32f OldExcitationBuffer[FRM_LEN+PITCH_LAG_MAX+INTERPOL_LEN];
   Ipp32f fBetaPreFilter;   /* pitch sharpening of previous frame */
   Ipp32s   prevPitchDelay; /* integer delay of previous frame    */
   Ipp32f fCodeGain;        /* Code gain                          */
   Ipp32f fPitchGain;       /* Pitch gain                         */
   Ipp32f OldLSP[LPC_ORDER];
   IppsIIRState_32f *iirstate;
   Ipp32f PastQuantEnergy[4];
   Ipp32f PrevFreq[MOVING_AVER_ORDER][LPC_ORDER];    /* previous LSP vector       */
   Ipp32s prevMA;                  /* previous MA prediction coef.*/
   Ipp32f prevLSF[LPC_ORDER];            /* previous LSF vector         */
   /* for G.729B */
   Ipp16s sFESeed;
   /* CNG variables */
   Ipp32s  prevFrameType;
   Ipp32s  prevSID[4];/*prev SID need for erased only*/
   Ipp16s sCNGSeed;
   Ipp32f SID;
   Ipp32f fCurrGain;
   Ipp32f SIDLSP[LPC_ORDER];
   Ipp32f fSIDGain;
   Ipp32f SynFltMemory[BWD_LPC_ORDER];        /* Synthesis filter's memory          */
   Ipp8s *phdMem;
   /* for G.729A */
   Ipp32f PstFltMemoryA[LPC_ORDER];
   Ipp32f fPastGain;
   Ipp32f ResidualBufferA[PITCH_LAG_MAX+SUBFR_LEN]; /* inverse filtered synthesis (with A(z/GAMMA2_POSTFLT))   */
   Ipp32f *ResidualMemory;
   Ipp32f PstSynMemoryA[LPC_ORDER];   /* memory of filter 1/A(z/GAMMA1_POSTFLT) */
   Ipp32f fPreemphMemoryA;
   /* Not G.729A */
   Ipp32f SynthBuffer[BWD_ANALISIS_WND_LEN];  /* Synthesis                   */
   Ipp32s   prevFracPitchDelay;    /* integer delay of previous frame    */
   /* for the backward analysis */
   Ipp32f BackwardUnqLPC[BWD_LPC_ORDERP1];
   Ipp32f BackwardLPCMemory[BWD_LPC_ORDERP1];
   Ipp32s   lPrevVoicing;
   Ipp32s   lPrevBFI;
   Ipp32s   prevLPCMode;
   Ipp32f fFEInterpolationCoeff;
   Ipp32f fInterpolationCoeff;
   Ipp32f PrevFlt[BWD_LPC_ORDERP1]; /* Previous selected filter */
   Ipp32s   lPrevPitchPT;
   Ipp32s   lStatPitchPT;
   Ipp32s   lStatPitch2PT;
   Ipp32s   lStatFracPT;
   /* Last backward A(z) for case of unstable filter */
   Ipp32f OldBackwardLPC[BWD_LPC_ORDERP1];
   Ipp32f OldBackwardRC[2];
   Ipp32f  fPitchGainMemory;
   Ipp32f  fCodeGainMemory;
   Ipp32f  fGainMuting;
   Ipp32s  lBFICounter;
   Ipp32s  sBWDStatInd;
   Ipp32s  lVoicing; /* voicing from previous frame */
   Ipp32f g1PST;
   Ipp32f g2PST;
   Ipp32f gHarmPST;
   Ipp32s  sBWDFrmCounter;
   Ipp32s sFWDFrmCounter;
   Ipp8s *pstMem;
   IppsWinHybridState_G729E_32f *pHWState;
};

#define   G729_CODECFUN(type,name,arg)                extern type name arg

__INLINE Ipp32s Parity( Ipp32s val){
  Ipp32s temp, nBits, bit;
  Ipp32s sum;

  temp = val >> 1;
  sum = 1;
  for (nBits = 0; nBits <= 5; nBits++) {
    temp = temp >> 1;
    bit = temp & 0x00000001;
    sum += bit;
  }
  sum = sum & 0x00000001;

  return sum;
}

static __ALIGN32 CONST Ipp16s modtab[]={1, 2, 0, 1, 2, 0, 1, 2};

__INLINE void DecodeAdaptCodebookDelays(Ipp32s *prevPitchDelay, Ipp32s *prevFracPitchDelay,Ipp32s *delayLine,
                                         Ipp32s NSbfr, Ipp32s badPitch,Ipp32s pitchIndx,G729Codec_Type type){
   Ipp16s minPitchDelay, maxPitchDelay;

   if(badPitch == 0){

      if (NSbfr == 0)                  /* if 1st subframe */
      {
         if (pitchIndx < 197)
         {
            delayLine[0] = (pitchIndx+2)/3 + 19;
            delayLine[1] = pitchIndx - delayLine[0] * 3 + 58;
         }
         else
         {
            delayLine[0] = pitchIndx - 112;
            delayLine[1] = 0;
         }

      } else  {/* second subframe */
         /* find minPitchDelay and maxPitchDelay for 2nd subframe */
         minPitchDelay = (Ipp16s)(delayLine[0] - 5);
         CLIP_TO_LOWLEVEL(minPitchDelay,PITCH_LAG_MIN);

         maxPitchDelay = (Ipp16s)(minPitchDelay + 9);
         if (maxPitchDelay > PITCH_LAG_MAX)
         {
            maxPitchDelay = PITCH_LAG_MAX;
            minPitchDelay = (Ipp16s)(maxPitchDelay - 9);
         }
         if (type == G729D_MODE) {
            pitchIndx = pitchIndx & 15;
            if (pitchIndx <= 3) {
               delayLine[0] = minPitchDelay + pitchIndx;
               delayLine[1] = 0;
            }
            else if (pitchIndx < 12) {
               /* *T0_frac = index % 3; */
               delayLine[1] = modtab[pitchIndx - 4];
               delayLine[0] = (pitchIndx - delayLine[1])/3 + minPitchDelay + 2;

               if (delayLine[1] == 2) {
                  delayLine[1] = -1;
                  delayLine[0] += 1;
               }
            }
            else {
                delayLine[0] = minPitchDelay + pitchIndx - 6;
                delayLine[1] = 0;
            }

         }
         else {
            delayLine[0] = minPitchDelay + (pitchIndx + 2)/3 - 1;
            delayLine[1] = pitchIndx - 2 - 3 * ((pitchIndx + 2)/3 - 1);
         }
      }
      *prevPitchDelay = delayLine[0];
      *prevFracPitchDelay = delayLine[1];
   }else {                     /* Bad frame, or parity error */
      delayLine[0]  =  *prevPitchDelay;
      if (type == G729E_MODE) {
         delayLine[1] = *prevFracPitchDelay;
      }
      else {
         delayLine[1] = 0;
         *prevPitchDelay += 1;
         CLIP_TO_UPLEVEL(*prevPitchDelay,PITCH_LAG_MAX);
      }
   }
}

void CodewordImpConv_G729_32f(Ipp32s index, const Ipp32f *pSrc1,const Ipp32f *pSrc2,Ipp32f *pDst);

#define isVarZero(var) (fabs(var) < IPP_MINABS_32F)

#endif /* __OWNG729FP_H__ */
