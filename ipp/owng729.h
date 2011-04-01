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
// Purpose: G.729/A/B/D/E speech codec: main own header file.
//
*/

#ifndef __OWNG729_H__
#define __OWNG729_H__

#if defined( _WIN32_WCE)
#pragma warning( disable : 4505 )
#endif

#include <ipps.h>
#include <ippsc.h>
#include "g729api.h"
#include "scratchmem.h"

#define ENC_KEY 0xecd729
#define DEC_KEY 0xdec729

#define G729_ENCODER_SCRATCH_MEMORY_SIZE (10240+40)
#define SEC_STAGE_BITS      5
#define FIR_STAGE_BITS      7
#define PARM_DIM            11
#define FIR_STAGE           (1<<FIR_STAGE_BITS)
#define SPEECH_BUF_DIM      240
#define SEC_STAGE           (1<<SEC_STAGE_BITS)
#define LP_LOOK_AHEAD       40
#define LP_WINDOW_DIM       240
#define LP_SUBFRAME_DIM     40
#define LP_FRAME_DIM        80
#define LPC_WINDOW          (speechHistory + SPEECH_BUF_DIM - LP_WINDOW_DIM)
#define PRESENT_SPEECH      (speechHistory + SPEECH_BUF_DIM - LP_FRAME_DIM - LP_LOOK_AHEAD)
#define MIN_PITCH_LAG       20
#define LPF_DIM             10      /*LP filter order */
#define INTERPOLATION_FILTER_DIM   (10+1)
#define MAX_PITCH_LAG       143
#define L_prevExcitat       (MAX_PITCH_LAG+INTERPOLATION_FILTER_DIM)
#define PITCH_SHARP_MIN     3277      /*pitch sharpening min = 0.2 */
#define BWF2                22938     /*bandwitdh = 0.7 */
#define PITCH_SHARP_MAX     13017     /*pitch sharpening max = 0.8 */
#define COEFF1              29491
#define CDBK1_BIT_NUM       3
#define CDBK2_BIT_NUM       4
#define CDBK1_DIM           (1<<CDBK1_BIT_NUM)
#define LSF_MIN             40         /*lsf min = 0.005 */
#define CDBK2_DIM           (1<<CDBK2_BIT_NUM)
#define LSP_MA_ORDER        4          /*MA moving average */
#define BW_EXP_FACT         321        /*bandwidth expansion factor */
#define LSF_MAX             25681      /*lsf max = 3.135 */
#define BWF2_PST            18022      /*weighting factor */
#define PITCH_GAIN_MAX      15564      /*for improving */
#define IMP_RESP_LEN        20
#define BWF1_PST            BWF2       /*weighting factor */
#define CORR_DIM            616
#define RES_FIL_DIM         (MAX_PITCH_LAG + 1 + 8)
#define SID_FRAME_MIN       3
#define SEED_INIT           11111
#define ACF_NOW             2
#define ACF_TOTAL           3
#define GAIN0               28672
#define GAIN1               (-IPP_MIN_16S-GAIN0)
#define GAIN_NUM            2
#define TOTAL_ACF_DIM       (ACF_TOTAL *(LPF_DIM+1))
#define VAD_LPC_DIM         12
#define ACF_DIM             (ACF_NOW *(LPF_DIM+1))
#define SIX_PI              19302

#define BWLPCF_DIM          30         /*backward LP filter order*/
#define SYNTH_BWD_DIM       BWLPCF_DIM + 35
#define BWLPCF1_DIM         (BWLPCF_DIM+1)
#define TBWD_DIM            LP_FRAME_DIM + SYNTH_BWD_DIM
#define N0_98               32113
#define BWF2_PST_E          21300      /*Numerator wgt factor */
#define BWF1_PST_E          BWF2       /*Denominator wgt factor 0.70*/
#define IMP_RESP_LEN_E      32         /*Imp.resp. len */
#define BWF_HARMONIC_E      (1<<13)
#define BWF_HARMONIC        (1<<14)

#define CDBK1_BIT_NUM_6K    3
#define CDBK2_BIT_NUM_6K    3
#define CDBK1_DIM_6K        (1<<CDBK1_BIT_NUM_6K)
#define CDBK2_DIM_6K        (1<<CDBK2_BIT_NUM_6K)
#define CNG_STACK_SIZE      (1<<5)
#define G729_CODECFUN(type,name,arg) extern type name arg

typedef struct _SynthesisFilterState {
    Ipp32s nTaps;
    Ipp16s *buffer;
}SynthesisFilterState;

typedef struct _G729Coder_Obj {
    Ipp32s         objSize;
    Ipp32s         key;
    Ipp32u         mode;
    G729Codec_Type codecType;
}G729Coder_Obj;

struct _G729Decoder_Obj {
    G729Coder_Obj objPrm;
    ScratchMem_Obj Mem;
    Ipp16s   resFilBuf1[RES_FIL_DIM+LP_SUBFRAME_DIM];
    Ipp16s   prevLSPfreq[LSP_MA_ORDER][LPF_DIM];
    Ipp16s   LTPostFilt[TBWD_DIM];
    Ipp16s   prevExcitat[LP_FRAME_DIM+L_prevExcitat];
    Ipp16s   prevSubfrLSP[LPF_DIM];
    Ipp16s   prevSubfrLSPquant[LPF_DIM];
    Ipp16s   zeroPostFiltVec1[LPF_DIM+BWLPCF_DIM+2];
    Ipp16s   decPrm[20];            /*analysis data pointer */
    Ipp32s   coderErr[4];           /*memory for improving */
    Ipp16s   pBwdLPC[BWLPCF1_DIM];
    Ipp16s   pBwdLPC2[BWLPCF1_DIM];
    Ipp16s   pPrevBwdLPC[BWLPCF1_DIM];
    Ipp32s   hwState[BWLPCF1_DIM];
    Ipp16s   pPrevFilt[BWLPCF1_DIM]; /* selected previously filter */
    G729Codec_Type codecType;
    Ipp16s   preemphFilt;
    Ipp16s   seed;
    Ipp16s   voiceFlag;
    Ipp16s   gainExact;             /*gain's precision */
    Ipp16s   betaPreFilter;         /*quant adaptive codebook gain from the previous subframe */
    Ipp16s   gainNow;
    Ipp16s   sidGain;
    Ipp16s   seedSavage;
    Ipp16s   CNGvar;
    Ipp16s   SIDflag0;
    Ipp16s   lspSID[LPF_DIM];
    Ipp16s   SIDflag1;
    Ipp16s   CNGidx;                /*CNG cache parameters  */
    Ipp8s    *postProc;             /*High pass post processing filter memory */
    Ipp8s    *synFltw;              /*Synthesis filter memory */
    Ipp8s    *synFltw0;             /*Synthesis filter memory */
    Ipp32s   pstFltMode;            /*post filter: 1- on, 0- off */
    Ipp16s   gains[2];              /*pitch + vcodebook gains */
    Ipp16s   prevFrameDelay;
    Ipp16s   prevMA;                /*previous MA prediction coef.*/
    Ipp16s   prevFrameQuantEn[4];
    Ipp16s   prevVoiceFlag;
    Ipp16s   prevBFI;
    Ipp16s   prevLPmode;
    Ipp16s   interpCoeff2;
    Ipp16s   interpCoeff2_2;
    Ipp16s   valGainAttenuation;
    Ipp16s   BFIcount;
    IppsPhaseDispersion_State_G729D *PhDispMem;
    Ipp16s   pPrevBwdRC[2];
    Ipp16s   BWDFrameCounter;
    Ipp16s   stat_pitch;            /*pitch stationarity */
    Ipp16s   prevFrameDelay2;       /*previous frame delay */
    Ipp16s   pitchStatIntDelay;
    Ipp16s   pitchStatFracDelay;
    Ipp16s   prevPitch;
    Ipp16s   gammaPost1;
    Ipp16s   gammaPost2;
    Ipp16s   gammaHarm;
    Ipp16s   BWDcounter2;
    Ipp16s   FWDcounter2;
};
struct _G729Encoder_Obj {
    G729Coder_Obj       objPrm;
    ScratchMem_Obj      Mem;
    Ipp16s   encSyn[LP_FRAME_DIM];/*encodersynthesisbuffer*/
    Ipp16s   speechHistory[SPEECH_BUF_DIM];
    Ipp16s   prevLSPfreq[LSP_MA_ORDER][LPF_DIM];
    Ipp16s   energySfs[GAIN_NUM];/*energyscalefactors*/
    Ipp16s   pACF[ACF_DIM];
    Ipp16s   ACFsum[TOTAL_ACF_DIM];
    Ipp16s   BWDsynth[TBWD_DIM];
    Ipp16s   energy[GAIN_NUM];
    Ipp16s   prevFrameQuantEn[4];/*quantizedenergyforpreviousframes*/
    Ipp16s   resFilMem0[BWLPCF_DIM];
    Ipp16s   resFilMem[BWLPCF_DIM+LP_SUBFRAME_DIM];
    Ipp16s   quantLspSID[LPF_DIM];
    Ipp16s   prevSubfrLSP[LPF_DIM];
    Ipp16s   prevSubfrLSPquant[LPF_DIM];
    Ipp32s   hwState[BWLPCF1_DIM];
    Ipp16s   ACFsfs[ACF_NOW];
    Ipp16s   ACFsumSfs[ACF_TOTAL];
    Ipp16s   pPrevFilt[BWLPCF1_DIM];
    Ipp16s   pPrevBwdLPC[BWLPCF1_DIM];
    Ipp16s   pBwdLPC2[BWLPCF1_DIM];
    Ipp16s   betaPreFilter;/*quantadaptivecodebookgainfromtheprevioussubframe*/
    Ipp16s   prevWgtSpeech[LP_FRAME_DIM+MAX_PITCH_LAG+1];
    Ipp16s   prevExcitat[LP_FRAME_DIM+L_prevExcitat+2];
    Ipp16s   encPrm[19];
    Ipp16s   zeroPostFiltVec1[LP_SUBFRAME_DIM+BWLPCF_DIM+1];/*zeroextendedimpulseresponse*/
    Ipp16s   prevCoeff[LPF_DIM+1];
    Ipp16s   prevSubfrLPC[LPF_DIM+1];
    Ipp16s   reflC[LPF_DIM+1];
    G729Codec_Type codecType;
    Ipp16s   ACnorm;
    Ipp16s   ACFcounter;
    Ipp16s   gainNow;
    Ipp16s   energyDim;/*energiesnumber*/
    Ipp16s   SIDframeCounter;
    Ipp16s   prevSubfrSmooth;/*perceptualweighting*/
    Ipp16s   sidGain;
    Ipp16s   speechDiff;
    Ipp16s   prevLPmode;
    Ipp16s   *pSynth;
    Ipp16s   pPrevBwdRC[2];
    Ipp16s   prevLAR[2];/*previoussubframelogarearatio*/
    Ipp16s   prevDTXEnergy;
    Ipp16s   seed;
    Ipp16s   CNGidx;/*CNGcacheparameters*/
    Ipp8s    *preProc;/*highpasspreprocessingfiltermemory*/
    Ipp8s    *synFltw;/*synthesisfilter1memory*/
    Ipp8s    *synFltw0;/*synthesisfilter2memory*/
    Ipp8s    *vadMem;/*VADmemory*/
    Ipp32s   mode;/*mode's*/
    Ipp16s   extraTime;/*fixedcodebooksearchextratime*/
    Ipp16s   prevRC[2];
    Ipp32s   coderErr[4];
    Ipp16s   dominantBWDmode;
    Ipp16s   interpCoeff2_2;
    Ipp16s   statGlobal;/*globalstationaritymesure*/
    Ipp16s   pLag[5];
    Ipp16s   pGain[5];
    Ipp16s   BWDFrameCounter;/*consecutivebackwardframesNbre*/
    Ipp16s   val_BWDFrameCounter;/*BWDFrameCounterassociated*/
    Ipp16s   BWDcounter2;
    Ipp16s   FWDcounter2;
};

Ipp16s ExtractBitsG729(const Ipp8u **pSrc, Ipp32s *len, Ipp32s Count);
void NoiseExcitationFactorization_G729B_16s(const Ipp16s *pSrc,Ipp32s val1,
                                            Ipp16s val2, Ipp16s *pDst, Ipp32s len);
Ipp32s ComfortNoiseExcitation_G729B_16s_I(const Ipp16s *pSrc, const Ipp16s *pPos,
                                       const Ipp16s *pSign, Ipp16s val, Ipp16s t,
                                       Ipp16s *pSrcDst, Ipp16s *t2, Ipp16s *Sfs);
void RandomCodebookParm_G729B_16s(Ipp16s *pSrc1, Ipp16s *pSrc2, Ipp16s *pSrc3,
                                  Ipp16s *pSrc4, Ipp16s *n);
void QuantSIDGain_G729B_16s(const Ipp16s *pSrc, const Ipp16s *pSrcSfs,
                            Ipp32s len, Ipp16s *p, Ipp32s *pIdx);
void Sum_G729_16s_Sfs(const Ipp16s *pSrc, const Ipp16s *pSrcSfs,
                      Ipp16s *pDst, Ipp16s *pDstSfs, Ipp32s len, Ipp32s*pSumMem);
void VADMusicDetection( G729Codec_Type codecType, Ipp32s Val, Ipp16s expVal, Ipp16s *rc,
                        Ipp16s *lags, Ipp16s *pgains, Ipp16s stat_flg,
                        Ipp16s *Vad, Ipp8s*pVADmem);
IppStatus SynthesisFilter_G729_16s (const Ipp16s *pLPC, const Ipp16s *pSrc,
                                    Ipp16s *pDst, Ipp32s len, Ipp8s *pMemUpdated, Ipp32s HistLen);
IppStatus SynthesisFilter_G729_16s_update (const Ipp16s *pLPC, const Ipp16s *pSrc,
                                           Ipp16s *pDst, Ipp32s len, Ipp8s *pMemUpdated, Ipp32s hLen,
                                           Ipp32s update);
void SynthesisFilterOvf_G729_16s_I(const Ipp16s *pLPC, Ipp16s *pSrcDst,
                                   Ipp32s len, Ipp8s *pMemUpdated, Ipp32s HistLen);
void SynthesisFilterOvf_G729_16s (const Ipp16s *pLPC, const Ipp16s *pSrc,
                                  Ipp16s *pDst, Ipp32s len, Ipp8s *pMemUpdated, Ipp32s HistLen);
void SynthesisFilterInit_G729 (Ipp8s *pMemUpdated);
void SynthesisFilterSize_G729 (Ipp32s *pSize);
void CodewordImpConv_G729(Ipp32s index, const Ipp16s *pSrc1,const Ipp16s *pSrc2,Ipp16s *pDst);
void _ippsRCToLAR_G729_16s (const Ipp16s*pSrc, Ipp16s*pDst, Ipp32s len);
void _ippsPWGammaFactor_G729_16s (const Ipp16s*pLAR, const Ipp16s*pLSF,
                                   Ipp16s *flat, Ipp16s*pGamma1, Ipp16s*pGamma2, Ipp16s *pMem );
void CNG_encoder(Ipp16s *exc, Ipp16s *prevSubfrLSPquant, Ipp16s *Aq, Ipp16s *ana, G729Encoder_Obj *encoderObj);
void CNG_Update(Ipp16s *pSrc, Ipp16s val, Ipp16s vad, G729Encoder_Obj *encoderObj);
void Post_G729(Ipp16s idx, Ipp16s id, const Ipp16s *LPC, Ipp16s *pDst, Ipp16s *voiceFlag, G729Decoder_Obj *decoderObj);
void Post_G729AB(Ipp16s idx, Ipp16s id, const Ipp16s *LPC, Ipp16s *pDst, Ipp16s vad, G729Decoder_Obj *decoderObj);
void Post_G729I(Ipp16s idx, Ipp16s id, const Ipp16s *LPC, Ipp16s *pDst, Ipp16s *val, Ipp16s val1, Ipp16s val2,
                Ipp16s fType, G729Decoder_Obj *decoderObj);
void Post_G729Base(Ipp16s idx, Ipp16s id, const Ipp16s *LPC, Ipp16s *pDst, Ipp16s *voiceFlag, Ipp16s ftyp, G729Decoder_Obj *decoderObj);
void updateExcErr_G729(Ipp16s x, Ipp32s y, Ipp32s *err);
Ipp16s calcErr_G729(Ipp32s val, Ipp32s *pSrc);
void BWDLagWindow(Ipp32s *pSrc, Ipp32s *pDst);
void SetLPCMode_G729E(Ipp16s *signal_ptr, Ipp16s *aFwd, Ipp16s *pBwdLPC,
                      Ipp16s *lpMode, Ipp16s *lspnew, Ipp16s *lspold,
                      G729Encoder_Obj *encoderObj);
void PitchTracking_G729E(Ipp16s *val1, Ipp16s *val2, Ipp16s *prevPitch,
                         Ipp16s *pitchStat, Ipp16s *pitchStatIntDelay,  Ipp16s *pitchStatFracDelay);
Ipp16s enerDB(Ipp16s *synth, Ipp16s L);
void tstDominantBWDmode(Ipp16s *BWDcounter2,Ipp16s *FWDcounter2,Ipp16s *highStat, Ipp16s mode);
void Init_CNG_encoder(G729Encoder_Obj *encoderObj);
void Log2_G729(Ipp32s val, Ipp16s *pDst1, Ipp16s *pDst2);

extern CONST Ipp16s gammaFac1[2*(LPF_DIM+1)];
extern CONST Ipp16s g729gammaFac1_pst[LPF_DIM+1];
extern CONST Ipp16s g729gammaFac2_pst[LPF_DIM+1];
extern CONST Ipp16s cngSeedOut[CNG_STACK_SIZE];
extern CONST Ipp16s cngCache[CNG_STACK_SIZE][LP_SUBFRAME_DIM];
extern CONST Ipp16s LUT1[CDBK1_DIM];
extern CONST Ipp16s presetLSP[LPF_DIM];
extern CONST Ipp32s cngInvSqrt[CNG_STACK_SIZE];
extern CONST Ipp16s resetPrevLSP[LPF_DIM];
extern CONST Ipp16s LUT2[CDBK2_DIM];
extern CONST Ipp16s presetOldA[LPF_DIM+1];
extern CONST Ipp16s areas[L_prevExcitat-1+3];
extern CONST Ipp16s SIDgain[32];

#include "aux_fnxs.h"

__INLINE Ipp32s equality( Ipp32s val) {
    Ipp32s temp, i, bit;
    Ipp32s sum;
    sum = 1;
    temp = val >> 1;
    for(i = 0; i <6; i++) {
        temp >>= 1;
        bit = temp & 1;
        sum += bit;
    }
    sum &= 1;
    return sum;
}

static __ALIGN32 CONST Ipp16s table0[8]={
    1*1, 2*1, 0*1,
    1*1, 2*1, 0*1,
    1*1, 2*1};
__INLINE void DecodeAdaptCodebookDelays(Ipp16s *prevFrameDelay, Ipp16s *prevFrameDelay2,Ipp16s *delay,
                                        Ipp32s id, Ipp32s bad_pitch,Ipp32s pitchIndx,G729Codec_Type type) {
    Ipp16s minPitchSearchDelay, maxPitchSearchDelay;

    if(bad_pitch == 0) {
        if(id == 0) {                  /*if 1-st subframe */
            if(pitchIndx < 197) {
                delay[0] = (Ipp16s)((pitchIndx+2)/3 + 19);
                delay[1] = (Ipp16s)(pitchIndx - delay[0] * 3 + 58);
            } else {
                delay[0] = (Ipp16s)(pitchIndx - 112);
                delay[1] = 0;
            }

        } else {
            /*find minPitchSearchDelay and maxPitchSearchDelay for 2-nd subframe */
            minPitchSearchDelay = (Ipp16s)(delay[0] - 5);
            if(minPitchSearchDelay < MIN_PITCH_LAG)
                minPitchSearchDelay = MIN_PITCH_LAG;

            maxPitchSearchDelay = (Ipp16s)(minPitchSearchDelay + 9);
            if(maxPitchSearchDelay > MAX_PITCH_LAG) {
                maxPitchSearchDelay = MAX_PITCH_LAG;
                minPitchSearchDelay = MAX_PITCH_LAG - 9;
            }
            if(type == G729D_CODEC /* i.e. 6.4 kbps */) {
                pitchIndx = pitchIndx & 15;
                if(pitchIndx <= 3) {
                    delay[0] = (Ipp16s)(minPitchSearchDelay + pitchIndx);
                    delay[1] = 0;
                } else if(pitchIndx < 12) {
                    delay[1] = table0[pitchIndx - 4];
                    delay[0] = (Ipp16s)((pitchIndx - delay[1])/3 + 2 + minPitchSearchDelay);

                    if(delay[1] == 2) {
                        delay[1] = -1;
                        delay[0]++;
                    }
                } else {
                    delay[0] = (Ipp16s)(minPitchSearchDelay + pitchIndx - 6);
                    delay[1] = 0;
                }
            } else {
                delay[0] = (Ipp16s)(minPitchSearchDelay + (pitchIndx + 2)/3 - 1);
                delay[1] = (Ipp16s)(pitchIndx - 2 - 3 *((pitchIndx + 2)/3 - 1));
            }
        }
        *prevFrameDelay = delay[0];
        *prevFrameDelay2 = delay[1];
    } else {                     /* non-equal or bad frame*/
        delay[0]  =  *prevFrameDelay;
        if(type == G729E_CODEC) {
            delay[1] = *prevFrameDelay2;
        } else {
            delay[1] = 0;
            *prevFrameDelay += 1;
            if(*prevFrameDelay > MAX_PITCH_LAG) {
                *prevFrameDelay = MAX_PITCH_LAG;
            }
        }
    }
}

#endif /* __OWNG729_H__ */
