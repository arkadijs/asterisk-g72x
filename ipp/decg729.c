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
// Purpose: G.729/A/B/D/E speech codec: decoder API functions.
//
*/

#include <stdlib.h>
#include <ippsc.h>
#include "owng729.h"

/* HPF coefficients */

static __ALIGN32 CONST Ipp16s tab1[3] = {
 7699,
 -15398,
 7699};
static __ALIGN32 CONST Ipp16s tab2[3] = {BWF_HARMONIC_E, 15836, -7667};
static __ALIGN32 CONST Ipp16s lspSID_init[LPF_DIM] = {31441,  27566,  21458, 13612,
 4663, -4663, -13612,
 -21458, -27566, -31441};

static __ALIGN32 CONST Ipp16s tab3[32] = {
        1, 3, 8, 6, 18, 16,
        11, 13, 38, 36, 31, 33,
        21, 23, 28, 26, 0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0};

static __ALIGN32 CONST Ipp16s tab4[32] = {
    0, 2, 5, 4, 12, 10, 7, 9, 25,
    24, 20, 22, 14, 15, 19, 17,
    36, 31, 21, 26, 1, 6, 16, 11,
    27, 29, 32, 30, 39, 37, 34, 35};

static Ipp32s DecoderObjSize(void) {
    Ipp32s fltSize;
    Ipp32s objSize = sizeof(G729Decoder_Obj);
    ippsHighPassFilterSize_G729(&fltSize);
    objSize += fltSize; /* provide memory for postprocessing high pass filter with upscaling */
    SynthesisFilterSize_G729(&fltSize);
    objSize += 2 * fltSize;/* provide memory for two synthesis filters */
    ippsPhaseDispersionGetStateSize_G729D_16s(&fltSize);
    objSize += fltSize; /* provide memory for phase dispersion */
    objSize += 4*32;
    return objSize;
}

G729_CODECFUN( APIG729_Status, apiG729Decoder_Alloc,
               (G729Codec_Type codecType, Ipp32s *pCodecSize)) {
    if((codecType != G729_CODEC)&&(codecType != G729A_CODEC)
       &&(codecType != G729D_CODEC)&&(codecType != G729E_CODEC)&&(codecType != G729I_CODEC)) {
        return APIG729_StsBadCodecType;
    }
    *pCodecSize =  DecoderObjSize();
    return APIG729_StsNoErr;
}

G729_CODECFUN( APIG729_Status, apiG729Decoder_Init,
               (G729Decoder_Obj* decoderObj, G729Codec_Type codecType)) {
    Ipp32s i,fltSize;
    Ipp16s abDec[6];

   Ipp8s* oldMemBuff;

    if((codecType != G729_CODEC)&&(codecType != G729A_CODEC)
       &&(codecType != G729D_CODEC)&&(codecType != G729E_CODEC)&&(codecType != G729I_CODEC)) {
        return APIG729_StsBadCodecType;
    }

    oldMemBuff = decoderObj->Mem.base; /* if Reinit */

    ippsZero_16s((Ipp16s*)decoderObj,sizeof(*decoderObj)>>1) ;

    decoderObj->objPrm.objSize = DecoderObjSize();
    decoderObj->objPrm.key = DEC_KEY;
    decoderObj->objPrm.codecType=codecType;

    decoderObj->codecType=codecType;
    decoderObj->synFltw=NULL;
    decoderObj->synFltw0=NULL;
    decoderObj->PhDispMem=NULL;

    decoderObj->postProc = (Ipp8s*)decoderObj + sizeof(G729Decoder_Obj);
    decoderObj->postProc = IPP_ALIGNED_PTR(decoderObj->postProc, 16);
    ippsHighPassFilterSize_G729(&fltSize);
    decoderObj->synFltw = (Ipp8s*)decoderObj->postProc + fltSize;
    decoderObj->synFltw = IPP_ALIGNED_PTR(decoderObj->synFltw, 16);
    SynthesisFilterSize_G729(&fltSize);
    decoderObj->synFltw0 = (Ipp8s*)decoderObj->synFltw + fltSize;
    decoderObj->synFltw0 = IPP_ALIGNED_PTR(decoderObj->synFltw0, 16);
    decoderObj->PhDispMem = (IppsPhaseDispersion_State_G729D*)((char*)decoderObj->synFltw0 + fltSize);
    decoderObj->PhDispMem = IPP_ALIGNED_PTR(decoderObj->PhDispMem, 16);
    abDec[0] = tab2[0];
    abDec[1] = tab2[1];
    abDec[2] = tab2[2];
    abDec[3] = tab1[0];
    abDec[4] = tab1[1];
    abDec[5] = tab1[2];
    for(i=0;i<4;i++) decoderObj->prevFrameQuantEn[i]=-14336;
    ippsHighPassFilterInit_G729(abDec,decoderObj->postProc);
    SynthesisFilterInit_G729(decoderObj->synFltw);
    SynthesisFilterInit_G729(decoderObj->synFltw0);
    ippsPhaseDispersionInit_G729D_16s(decoderObj->PhDispMem);

    /* synthesis speech buffer*/
    ippsZero_16s(decoderObj->LTPostFilt,TBWD_DIM);
    decoderObj->voiceFlag=60;
    ippsZero_16s(decoderObj->prevExcitat, L_prevExcitat);
    decoderObj->betaPreFilter = PITCH_SHARP_MIN;
    decoderObj->prevFrameDelay = 60;
    decoderObj->gains[0] = 0;
    decoderObj->gains[1] = 0;
    for(i=0; i<LSP_MA_ORDER; i++)
        ippsCopy_16s( &resetPrevLSP[0], &decoderObj->prevLSPfreq[i][0], LPF_DIM );
    ippsCopy_16s(presetLSP, decoderObj->prevSubfrLSP, LPF_DIM );
    ippsCopy_16s(resetPrevLSP, decoderObj->prevSubfrLSPquant, LPF_DIM);
    decoderObj->preemphFilt = 0;
    ippsZero_16s(decoderObj->resFilBuf1, MAX_PITCH_LAG+LP_SUBFRAME_DIM);
    ippsZero_16s(decoderObj->zeroPostFiltVec1 + LPF_DIM+1, BWLPCF1_DIM/*IMP_RESP_LEN*/);
    decoderObj->seedSavage = 21845;
    decoderObj->seed = SEED_INIT;
    decoderObj->CNGvar = 3;

    decoderObj->pstFltMode = 1;

    if(decoderObj->codecType == G729_CODEC ) {
        decoderObj->gainExact = BWF_HARMONIC;
    } else if( decoderObj->codecType == G729A_CODEC) {
        decoderObj->gainExact = (1<<12);
        decoderObj->CNGidx = 0;
        decoderObj->SIDflag0 = 0;
        decoderObj->SIDflag1 = 1;
        ippsCopy_16s( lspSID_init, decoderObj->lspSID, LPF_DIM );
    } else {
        decoderObj->prevMA = 0;
        decoderObj->gammaPost1 = BWF1_PST_E;
        decoderObj->gammaPost2 = BWF2_PST_E;
        decoderObj->gammaHarm = BWF_HARMONIC_E;
        decoderObj->BWDcounter2 = 0;
        decoderObj->FWDcounter2 = 0;

        ippsZero_16s(decoderObj->pBwdLPC, BWLPCF1_DIM);
        ippsZero_16s(decoderObj->pBwdLPC2, BWLPCF1_DIM);
        decoderObj->pBwdLPC[0] = (1<<12);
        decoderObj->pBwdLPC2[0] = (1<<12);

        decoderObj->prevVoiceFlag = 0;
        decoderObj->prevBFI = 0;
        decoderObj->prevLPmode = 0;
        decoderObj->interpCoeff2 = 0;
        decoderObj->interpCoeff2_2 = 4506;
        ippsZero_16s(decoderObj->pPrevFilt, BWLPCF1_DIM);
        decoderObj->pPrevFilt[0] = (1<<12);
        decoderObj->prevPitch = 30;
        decoderObj->stat_pitch = 0;
        ippsZero_16s(decoderObj->pPrevBwdLPC, BWLPCF1_DIM);
        decoderObj->pPrevBwdLPC[0]= (1<<12);
        ippsZero_16s(decoderObj->pPrevBwdRC, 2);
        decoderObj->valGainAttenuation = IPP_MAX_16S;
        decoderObj->BFIcount = 0;
        decoderObj->BWDFrameCounter = 0;

        decoderObj->gainExact = BWF_HARMONIC;
        ippsWinHybridGetStateSize_G729E_16s(&fltSize);
        if(fltSize > sizeof(Ipp32s)*BWLPCF1_DIM) {
            return APIG729_StsNotInitialized;
        }
        ippsWinHybridInit_G729E_16s((IppsWinHybridState_G729E_16s*)&decoderObj->hwState);

        decoderObj->SIDflag0 = 0;
        decoderObj->SIDflag1 = 1;
        decoderObj->CNGidx = 0;
        ippsCopy_16s( lspSID_init, decoderObj->lspSID, LPF_DIM );
        decoderObj->sidGain = SIDgain[0];
    }

   apiG729Decoder_InitBuff(decoderObj,oldMemBuff);

    return APIG729_StsNoErr;
}

G729_CODECFUN( APIG729_Status, apiG729Decoder_InitBuff,
               (G729Decoder_Obj* decoderObj, Ipp8s *buff)) {
    if(NULL==decoderObj || NULL==buff)
        return APIG729_StsBadArgErr;

    decoderObj->Mem.base = buff;
    decoderObj->Mem.CurPtr = decoderObj->Mem.base;
    decoderObj->Mem.VecPtr = (Ipp32s *)(decoderObj->Mem.base+G729_ENCODER_SCRATCH_MEMORY_SIZE);

    return APIG729_StsNoErr;
}

G729_CODECFUN( APIG729_Status, apiG729Decoder_Mode,
               (G729Decoder_Obj* decoderObj, Ipp32s mode))
{
   if(NULL==decoderObj)
        return APIG729_StsBadArgErr;
   decoderObj->pstFltMode = mode;
   return APIG729_StsNoErr;
}
static APIG729_Status G729Decode
(G729Decoder_Obj* decoderObj,const Ipp8u* src, Ipp32s frametype, Ipp16s* dst);
static APIG729_Status G729ADecode
(G729Decoder_Obj* decoderObj,const Ipp8u* src, Ipp32s frametype, Ipp16s* dst);
static APIG729_Status G729BaseDecode
(G729Decoder_Obj* decoderObj,const Ipp8u* src, Ipp32s frametype, Ipp16s* dst);

G729_CODECFUN( APIG729_Status, apiG729Decode,
               (G729Decoder_Obj* decoderObj,const Ipp8u* src, Ipp32s frametype, Ipp16s* dst)) {
    if(decoderObj->codecType == G729A_CODEC) {
        if(G729ADecode(decoderObj,src,frametype,dst) != APIG729_StsNoErr) {
            return APIG729_StsErr;
        }
    } else if(decoderObj->codecType == G729_CODEC && frametype != 2 && frametype != 4) {
        if(G729BaseDecode(decoderObj,src,frametype,dst) != APIG729_StsNoErr) {
            return APIG729_StsErr;
        }
    } else {
        if(G729Decode(decoderObj,src,frametype,dst) != APIG729_StsNoErr) {
            return APIG729_StsErr;
        }
    }
    return APIG729_StsNoErr;
}

APIG729_Status G729Decode
(G729Decoder_Obj* decoderObj,const Ipp8u* src, Ipp32s frametype, Ipp16s* dst) {

    LOCAL_ALIGN_ARRAY(32, Ipp16s, AzDec, BWLPCF1_DIM*2,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, newLSP,LPF_DIM,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, ACELPcodeVec, LP_SUBFRAME_DIM,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, BWDfiltLPC, 2*BWLPCF1_DIM,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, FWDfiltLPC, 2*LPF_DIM+2,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, BWDrc,BWLPCF_DIM,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp32s,   BWDacf,BWLPCF1_DIM,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp32s,   BWDhighAcf,BWLPCF1_DIM,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, phaseDispExcit,LP_SUBFRAME_DIM,decoderObj);
    LOCAL_ARRAY(Ipp16s, idx,4,decoderObj);
    LOCAL_ARRAY(Ipp16s, delayVal,2,decoderObj);
    LOCAL_ARRAY(Ipp16s, tmp_parm,5,decoderObj);
    Ipp16s prevFrameDelay1=0;  /* 1-st subframe pitch lag*/
    Ipp16s subfrVoiceFlag, *pAz, *pA;
    Ipp8s *synFltw = decoderObj->synFltw;
    Ipp16s *prevExcitat = decoderObj->prevExcitat;
    Ipp16s *excitation = prevExcitat + L_prevExcitat;
    Ipp16s *synth = decoderObj->LTPostFilt+SYNTH_BWD_DIM;
    Ipp16s *prevSubfrLSP = decoderObj->prevSubfrLSP;
    const  Ipp8u *pParm;
    const  Ipp16s *parm;
    Ipp16s *ppAz, temp, badFrameIndicator, badPitch, index, pulseSign, gPl, gC;
    Ipp16s voiceFlag = decoderObj->voiceFlag;
    Ipp16s sidGain = decoderObj->sidGain;
    Ipp16s gainNow = decoderObj->gainNow;
    Ipp16s *lspSID = decoderObj->lspSID;
    Ipp32s i, j, subfrIdx, index2, fType, gpVal=0, LPmode = 0;
    Ipp16s satFilter, statStat, mAq, prevM, dominantBWDmode = 0, L_hSt;
    IppStatus status;

    if(NULL==decoderObj || NULL==src || NULL ==dst)
        return APIG729_StsBadArgErr;
    if((decoderObj->codecType != G729_CODEC)
       &&(decoderObj->codecType != G729D_CODEC)&&(decoderObj->codecType != G729E_CODEC)&&(decoderObj->codecType != G729I_CODEC)) {
        return APIG729_StsBadCodecType;
    }
    if(decoderObj->objPrm.objSize <= 0)
        return APIG729_StsNotInitialized;
    if(DEC_KEY != decoderObj->objPrm.key)
        return APIG729_StsBadCodecType;

    delayVal[0]=delayVal[1]=0;

    pA = AzDec;
    pParm = src;
    if(frametype == -1) {
        decoderObj->decPrm[1] = 0;
        decoderObj->decPrm[0] = 1;
    } else if(frametype == 0) {
        decoderObj->decPrm[1] = 0;
        decoderObj->decPrm[0] = 0;

    } else if(frametype == 1) {
        decoderObj->decPrm[1] = 1;
        decoderObj->decPrm[0] = 0;
        i=0;
        decoderObj->decPrm[1+1] = ExtractBitsG729(&pParm,&i,1);
        decoderObj->decPrm[1+2] = ExtractBitsG729(&pParm,&i,5);
        decoderObj->decPrm[1+3] = ExtractBitsG729(&pParm,&i,4);
        decoderObj->decPrm[1+4] = ExtractBitsG729(&pParm,&i,5);

    } else if(frametype == 2) {
        decoderObj->decPrm[0] = 0;
        decoderObj->decPrm[1] = 2;
        i=0;
        decoderObj->decPrm[2] = ExtractBitsG729(&pParm,&i,1+FIR_STAGE_BITS);
        decoderObj->decPrm[3] = ExtractBitsG729(&pParm,&i,SEC_STAGE_BITS*2);
        decoderObj->decPrm[4] = ExtractBitsG729(&pParm,&i,8);
        decoderObj->decPrm[5] = ExtractBitsG729(&pParm,&i,9);
        decoderObj->decPrm[6] = ExtractBitsG729(&pParm,&i,2);
        decoderObj->decPrm[7] = ExtractBitsG729(&pParm,&i,6);
        decoderObj->decPrm[8] = ExtractBitsG729(&pParm,&i,4);
        decoderObj->decPrm[9] = ExtractBitsG729(&pParm,&i,9);
        decoderObj->decPrm[10] = ExtractBitsG729(&pParm,&i,2);
        decoderObj->decPrm[11] = ExtractBitsG729(&pParm,&i,6);

        decoderObj->codecType = G729D_CODEC;
    } else if(frametype == 3) {
        i=0;
        decoderObj->decPrm[1] = 3;
        decoderObj->decPrm[0] = 0;
        decoderObj->decPrm[1+1] = ExtractBitsG729(&pParm,&i,1+FIR_STAGE_BITS);
        decoderObj->decPrm[1+2] = ExtractBitsG729(&pParm,&i,SEC_STAGE_BITS*2);
        decoderObj->decPrm[1+3] = ExtractBitsG729(&pParm,&i,8);
        decoderObj->decPrm[1+4] = ExtractBitsG729(&pParm,&i,1);
        decoderObj->decPrm[1+5] = ExtractBitsG729(&pParm,&i,13);
        decoderObj->decPrm[1+6] = ExtractBitsG729(&pParm,&i,4);
        decoderObj->decPrm[1+7] = ExtractBitsG729(&pParm,&i,7);
        decoderObj->decPrm[1+8] = ExtractBitsG729(&pParm,&i,5);
        decoderObj->decPrm[1+9] = ExtractBitsG729(&pParm,&i,13);
        decoderObj->decPrm[1+10] = ExtractBitsG729(&pParm,&i,4);
        decoderObj->decPrm[1+11] = ExtractBitsG729(&pParm,&i,7);
        decoderObj->decPrm[1+4] =  (Ipp16s)((equality(decoderObj->decPrm[1+3])+decoderObj->decPrm[1+4]) & 0x00000001); /*  parity error (1) */
        decoderObj->codecType = G729_CODEC;
    } else if(frametype == 4) {
        Ipp16s tmp;
        i=0;
        tmp = ExtractBitsG729(&pParm,&i,2);

        decoderObj->decPrm[0] = 0;
        decoderObj->decPrm[1] = 4;
        if(tmp==0) {
            decoderObj->decPrm[2] = 0;

            decoderObj->decPrm[3] = ExtractBitsG729(&pParm,&i,1+FIR_STAGE_BITS);
            decoderObj->decPrm[4] = ExtractBitsG729(&pParm,&i,SEC_STAGE_BITS*2);
            decoderObj->decPrm[5] = ExtractBitsG729(&pParm,&i,8);
            decoderObj->decPrm[6] = ExtractBitsG729(&pParm,&i,1);
            decoderObj->decPrm[7] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[8] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[9] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[10] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[11] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[12] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[13] = ExtractBitsG729(&pParm,&i,5);
            decoderObj->decPrm[14] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[15] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[16] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[17] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[18] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[19] = ExtractBitsG729(&pParm,&i,7);

            i = decoderObj->decPrm[5]>>1;
            i &= 1;
            decoderObj->decPrm[6] = (Ipp16s)(decoderObj->decPrm[6] + i);
            decoderObj->decPrm[6] = (Ipp16s)((equality(decoderObj->decPrm[5])+decoderObj->decPrm[6]) & 0x00000001);/* parm[6] = parity error (1) */
        } else {
            decoderObj->decPrm[2] = 1; /*LPmode*/

            decoderObj->decPrm[3] = ExtractBitsG729(&pParm,&i,8);
            decoderObj->decPrm[4] = ExtractBitsG729(&pParm,&i,1);
            decoderObj->decPrm[5] = ExtractBitsG729(&pParm,&i,13);
            decoderObj->decPrm[6] = ExtractBitsG729(&pParm,&i,10);
            decoderObj->decPrm[7] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[8] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[9] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[10] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[11] = ExtractBitsG729(&pParm,&i,5);
            decoderObj->decPrm[12] = ExtractBitsG729(&pParm,&i,13);
            decoderObj->decPrm[13] = ExtractBitsG729(&pParm,&i,10);
            decoderObj->decPrm[14] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[15] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[16] = ExtractBitsG729(&pParm,&i,7);
            decoderObj->decPrm[17] = ExtractBitsG729(&pParm,&i,7);

            i = decoderObj->decPrm[3]>>1;
            i &= 1;
            decoderObj->decPrm[4] = (Ipp16s)(decoderObj->decPrm[4] + i);
            decoderObj->decPrm[4] = (Ipp16s)((equality(decoderObj->decPrm[3])+decoderObj->decPrm[4]) & 0x00000001);/* parm[4] = parity error (1) */
        }
        decoderObj->codecType = G729E_CODEC;
    }

    parm = decoderObj->decPrm;

    badFrameIndicator = *parm++;
    fType = *parm;
    if(badFrameIndicator == 1) {
        fType = decoderObj->CNGvar;
        if(fType == 1) fType = 0;
    } else {
        decoderObj->valGainAttenuation = IPP_MAX_16S;
        decoderObj->BFIcount = 0;
    }

    if( fType == 4) {
        if(badFrameIndicator != 0) {
            LPmode = decoderObj->prevLPmode;
        } else {
            LPmode = parm[1];
        }
        if(decoderObj->prevBFI != 0) {
            voiceFlag = decoderObj->prevVoiceFlag;
            decoderObj->voiceFlag = decoderObj->prevVoiceFlag;
        }
        ippsWinHybrid_G729E_16s32s(decoderObj->LTPostFilt, BWDacf,
                                   (IppsWinHybridState_G729E_16s*)&decoderObj->hwState);

        BWDLagWindow(BWDacf, BWDhighAcf);

        if(ippsLevinsonDurbin_G729_32s16s(BWDhighAcf, BWLPCF_DIM, &BWDfiltLPC[BWLPCF1_DIM], BWDrc,&temp) == ippStsOverflow) {
            ippsCopy_16s(decoderObj->pPrevBwdLPC,&BWDfiltLPC[BWLPCF1_DIM],BWLPCF1_DIM);
            BWDrc[0] = decoderObj->pPrevBwdRC[0];
            BWDrc[1] = decoderObj->pPrevBwdRC[1];

        } else {
            ippsCopy_16s(&BWDfiltLPC[BWLPCF1_DIM],decoderObj->pPrevBwdLPC,BWLPCF1_DIM);
            decoderObj->pPrevBwdRC[0] = BWDrc[0];
            decoderObj->pPrevBwdRC[1] = BWDrc[1];
        }

        satFilter = 0;
        for(i=BWLPCF1_DIM; i<2*BWLPCF1_DIM; i++) {
            if(BWDfiltLPC[i] >= IPP_MAX_16S) {
                satFilter = 1;
                break;
            }
        }
        if(satFilter == 1) ippsCopy_16s(decoderObj->pBwdLPC2, &BWDfiltLPC[BWLPCF1_DIM], BWLPCF1_DIM);
        else ippsCopy_16s(&BWDfiltLPC[BWLPCF1_DIM], decoderObj->pBwdLPC2, BWLPCF1_DIM);

        ippsMulPowerC_NR_16s_Sfs(&BWDfiltLPC[BWLPCF1_DIM], N0_98, &BWDfiltLPC[BWLPCF1_DIM], BWLPCF1_DIM, 15);
    }
    ippsMove_16s(&decoderObj->LTPostFilt[LP_FRAME_DIM], &decoderObj->LTPostFilt[0], SYNTH_BWD_DIM);

    if(LPmode == 1) {
        Ipp16s tmp;

        if((decoderObj->interpCoeff2 != 0)) {
            tmp = (Ipp16s)((1<<12) - decoderObj->interpCoeff2);
            ippsInterpolateC_G729_16s_Sfs(BWDfiltLPC + BWLPCF1_DIM, tmp,
                                          decoderObj->pBwdLPC, decoderObj->interpCoeff2, BWDfiltLPC + BWLPCF1_DIM, BWLPCF1_DIM, 12);
        }
    }
    if((badFrameIndicator != 0)&&(decoderObj->prevBFI == 0) && (decoderObj->CNGvar >3))
        ippsCopy_16s(&BWDfiltLPC[BWLPCF1_DIM],decoderObj->pBwdLPC,BWLPCF1_DIM);

    if(fType < 2) {
        if(fType == 1) {
            LOCAL_ALIGN_ARRAY(32, Ipp16s, lsfq,LPF_DIM,decoderObj);
            sidGain = SIDgain[(Ipp32s)parm[4]];
            ippsLSFDecode_G729B_16s(&parm[1],(Ipp16s*)(decoderObj->prevLSPfreq),lsfq);
            ippsLSFToLSP_G729_16s(lsfq,lspSID);
            LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, lsfq,LPF_DIM,decoderObj);
        } else {
            if(decoderObj->CNGvar > 1) {
                QuantSIDGain_G729B_16s(&decoderObj->SIDflag0, &decoderObj->SIDflag1, 0, &temp, &index2);
                sidGain = SIDgain[(Ipp32s)index2];
            }
        }
        if(decoderObj->CNGvar > 1) {
            gainNow = sidGain;
        } else {
            gainNow = (Ipp16s)((gainNow * GAIN0 + BWF_HARMONIC)>>15);
            gainNow = Add_16s(gainNow, (Ipp16s)((sidGain * GAIN1 + BWF_HARMONIC)>>15));
        }

        if(gainNow == 0) {
            ippsZero_16s(excitation,LP_FRAME_DIM);
            gpVal = 0;
            for(subfrIdx = 0;  subfrIdx < LP_FRAME_DIM; subfrIdx += LP_SUBFRAME_DIM) {
                ippsPhaseDispersionUpdate_G729D_16s((Ipp16s)gpVal, gainNow,decoderObj->PhDispMem);
            }
        } else {
            for(i = 0;  i < LP_FRAME_DIM; i += LP_SUBFRAME_DIM) {
                Ipp32s invSq;
                Ipp16s pG2, tmp, tmp2;
                const Ipp16s *excCached;
                LOCAL_ARRAY(Ipp16s, IdxVec, 4, decoderObj);
                LOCAL_ARRAY(Ipp16s, pulseSignVec, 4, decoderObj);
                LOCAL_ALIGN_ARRAY(32, Ipp16s, excg, LP_SUBFRAME_DIM, decoderObj);
                LOCAL_ARRAY(Ipp16s,tempArray,LP_SUBFRAME_DIM, decoderObj);
                RandomCodebookParm_G729B_16s(&decoderObj->seed,IdxVec,pulseSignVec,&pG2,delayVal);
                ippsDecodeAdaptiveVector_G729_16s_I(delayVal,&prevExcitat[i]);
                if(decoderObj->CNGidx > CNG_STACK_SIZE-1) { /* not cached */
                    ippsRandomNoiseExcitation_G729B_16s(&decoderObj->seed,excg,LP_SUBFRAME_DIM);
                    ippsDotProd_16s32s_Sfs(excg,excg,LP_SUBFRAME_DIM,&invSq,0);
                    ippsInvSqrt_32s_I(&invSq,1);
                    excCached=excg;
                } else {
                    decoderObj->seed = cngSeedOut[decoderObj->CNGidx];
                    invSq = cngInvSqrt[decoderObj->CNGidx];
                    excCached=&cngCache[decoderObj->CNGidx][0];
                    decoderObj->CNGidx++;
                }
                NoiseExcitationFactorization_G729B_16s(excCached,invSq,gainNow,excg,LP_SUBFRAME_DIM);
                tmp2 = (Ipp16s)ComfortNoiseExcitation_G729B_16s_I(excg,IdxVec,pulseSignVec,gainNow,pG2,&excitation[i],&tmp,tempArray);
                if(tmp2 < 0) gpVal = 0;
                if(tmp < 0) tmp = -tmp;

                ippsPhaseDispersionUpdate_G729D_16s((Ipp16s)gpVal,tmp,decoderObj->PhDispMem);

                LOCAL_ARRAY_FREE(Ipp16s,tempArray,LP_SUBFRAME_DIM, decoderObj);
                LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, excg, LP_SUBFRAME_DIM, decoderObj);
                LOCAL_ARRAY_FREE(Ipp16s, pulseSignVec, 4, decoderObj);
                LOCAL_ARRAY_FREE(Ipp16s, IdxVec, 4, decoderObj);
            }
        }
        ippsInterpolate_G729_16s(prevSubfrLSP,lspSID,prevSubfrLSP, LPF_DIM );
        ippsLSPToLPC_G729_16s(prevSubfrLSP,&FWDfiltLPC[0]);
        ippsLSPToLPC_G729_16s(lspSID,&FWDfiltLPC[LPF_DIM+1]);
        ippsCopy_16s(lspSID, prevSubfrLSP, LPF_DIM );
        decoderObj->sidGain = sidGain;
        decoderObj->gainNow = gainNow;

        ppAz = FWDfiltLPC;
        for(subfrIdx = 0; subfrIdx < LP_FRAME_DIM; subfrIdx += LP_SUBFRAME_DIM) {
            if(SynthesisFilter_G729_16s(ppAz,&excitation[subfrIdx],&synth[subfrIdx],LP_SUBFRAME_DIM,synFltw,20)==
               ippStsOverflow) {
                /* scale down excitation and redo in case of overflow */
                ippsRShiftC_16s_I(2,prevExcitat,L_prevExcitat+LP_FRAME_DIM);
                SynthesisFilterOvf_G729_16s(ppAz,&excitation[subfrIdx],&synth[subfrIdx],LP_SUBFRAME_DIM,synFltw,20);
            }

            ppAz += LPF_DIM+1;
        }
        decoderObj->betaPreFilter = PITCH_SHARP_MIN;

        prevFrameDelay1 = decoderObj->prevFrameDelay;
        decoderObj->interpCoeff2_2 = 4506;
        decoderObj->BWDFrameCounter = 0;
        decoderObj->stat_pitch = 0;
        ippsCopy_16s(&FWDfiltLPC[LPF_DIM+1], decoderObj->pPrevFilt, LPF_DIM+1);
        ippsZero_16s(&decoderObj->pPrevFilt[LPF_DIM+1], (BWLPCF1_DIM-LPF_DIM-1));

    } else {
        decoderObj->seed = SEED_INIT;
        decoderObj->CNGidx = 0;
        parm++;
        if(decoderObj->codecType == G729E_CODEC) parm++;
        if( LPmode==0 ) {
            LOCAL_ARRAY(Ipp16s, qIndex,4,decoderObj);

            qIndex[0] = (Ipp16s)((parm[0] >> FIR_STAGE_BITS) & 1);
            qIndex[1] = (Ipp16s)(parm[0] & (FIR_STAGE - 1));
            qIndex[2] = (Ipp16s)((parm[1] >> SEC_STAGE_BITS) & (SEC_STAGE - 1));
            qIndex[3] = (Ipp16s)(parm[1] & (SEC_STAGE - 1));
            if(!badFrameIndicator) {
                decoderObj->prevMA = qIndex[0];
                ippsLSFDecode_G729_16s( qIndex, (Ipp16s*)decoderObj->prevLSPfreq, decoderObj->prevSubfrLSPquant);
            } else {
                ippsLSFDecodeErased_G729_16s( decoderObj->prevMA,
                                              (Ipp16s*)decoderObj->prevLSPfreq, decoderObj->prevSubfrLSPquant);
            }

            ippsLSFToLSP_G729_16s(decoderObj->prevSubfrLSPquant, newLSP); /* Convert LSFs to LSPs */
            parm += 2;
            LOCAL_ARRAY_FREE(Ipp16s, qIndex,4,decoderObj);

            if( decoderObj->prevLPmode == 0) {
                ippsInterpolate_G729_16s(newLSP,prevSubfrLSP,prevSubfrLSP, LPF_DIM );

                ippsLSPToLPC_G729_16s(prevSubfrLSP, FWDfiltLPC);            /*  1-st subframe */

                ippsLSPToLPC_G729_16s(newLSP, &FWDfiltLPC[LPF_DIM+1]);      /* 2-nd subframe */
            } else {
                ippsLSPToLPC_G729_16s(newLSP, FWDfiltLPC);                  /* 1-st subframe */
                ippsMove_16s(FWDfiltLPC, &FWDfiltLPC[LPF_DIM+1], LPF_DIM+1);/* 2-nd subframe */
            }

            /* update the next frame LSFs*/
            ippsCopy_16s(newLSP, prevSubfrLSP, LPF_DIM );
            decoderObj->interpCoeff2_2 = 4506;
            mAq = LPF_DIM;
            pA = FWDfiltLPC;
            ippsCopy_16s(&FWDfiltLPC[LPF_DIM+1], decoderObj->pPrevFilt, LPF_DIM+1);
            ippsZero_16s(&decoderObj->pPrevFilt[LPF_DIM+1], (BWLPCF1_DIM-LPF_DIM-1));
        } else {
            Ipp16s tmp;
            decoderObj->interpCoeff2_2 = (Ipp16s)(decoderObj->interpCoeff2_2 - 410);
            if( decoderObj->interpCoeff2_2 < 0) decoderObj->interpCoeff2_2 = 0;
            tmp = (Ipp16s)((1<<12) - decoderObj->interpCoeff2_2);
            ippsInterpolateC_G729_16s_Sfs(BWDfiltLPC + BWLPCF1_DIM, tmp,
                                          decoderObj->pPrevFilt, decoderObj->interpCoeff2_2, BWDfiltLPC + BWLPCF1_DIM, BWLPCF1_DIM, 12);
            ippsInterpolate_G729_16s
            (BWDfiltLPC + BWLPCF1_DIM, decoderObj->pPrevFilt, BWDfiltLPC, BWLPCF1_DIM);
            mAq = BWLPCF_DIM;
            pA = BWDfiltLPC;
            ippsCopy_16s(&BWDfiltLPC[BWLPCF1_DIM], decoderObj->pPrevFilt, BWLPCF1_DIM);
        }

        for(ppAz=pA,subfrIdx = 0; subfrIdx < LP_FRAME_DIM; subfrIdx += LP_SUBFRAME_DIM) {
            Ipp32s pitchIndx;
            pitchIndx = *parm++;
            badPitch = badFrameIndicator;

            if(subfrIdx == 0) {
                if(decoderObj->codecType != G729D_CODEC)
                    badPitch = (Ipp16s)(badFrameIndicator + *parm++);
            }
            DecodeAdaptCodebookDelays(&decoderObj->prevFrameDelay,&decoderObj->prevFrameDelay2,delayVal,subfrIdx,badPitch,pitchIndx,decoderObj->codecType);
            if(subfrIdx == 0)
                prevFrameDelay1 = delayVal[0];         /* if first frame */
            ippsDecodeAdaptiveVector_G729_16s_I(delayVal,&prevExcitat[subfrIdx]);

            /* pitch tracking */
            if( decoderObj->codecType == G729E_CODEC) {
                PitchTracking_G729E(&decoderObj->prevFrameDelay, &decoderObj->prevFrameDelay2, &decoderObj->prevPitch, &decoderObj->stat_pitch,
                                    &decoderObj->pitchStatIntDelay, &decoderObj->pitchStatFracDelay);
            } else {
                Ipp16s sTmpPrevFrameDelay, sTmpPrevFrameDelay2;
                sTmpPrevFrameDelay = decoderObj->prevFrameDelay;
                sTmpPrevFrameDelay2 = decoderObj->prevFrameDelay2;
                PitchTracking_G729E(&sTmpPrevFrameDelay, &sTmpPrevFrameDelay2, &decoderObj->prevPitch, &decoderObj->stat_pitch,
                                    &decoderObj->pitchStatIntDelay, &decoderObj->pitchStatFracDelay);
            }

            statStat = 0;
            if(decoderObj->codecType == G729_CODEC) {
                if(badFrameIndicator != 0) {
                    index = (Ipp16s)(Rand_16s(&decoderObj->seedSavage) & (Ipp16s)0x1fff);     /* 13 bits random */
                    pulseSign = (Ipp16s)(Rand_16s(&decoderObj->seedSavage) & (Ipp16s)15);     /*  4 bits random */
                } else {
                    index = parm[0];
                    pulseSign = parm[1];
                }
                i      = index & 7;
                idx[0] = (Ipp16s)(5 * i);
                index  = (Ipp16s)(index >> 3);
                i      = index & 7;
                idx[1] = (Ipp16s)(5 * i + 1);
                index  = (Ipp16s)(index >> 3);
                i      = index & 7;
                idx[2] = (Ipp16s)(5 * i + 2);
                index  = (Ipp16s)(index >> 3);
                j      = index & 1;
                index  = (Ipp16s)(index >> 1);
                i      = index & 7;
                idx[3] = (Ipp16s)(i * 5 + 3 + j);

                /* decode the signs & build the codeword */
                ippsZero_16s(ACELPcodeVec,LP_SUBFRAME_DIM);
                for(j=0; j<4; j++) {
                    if((pulseSign & 1) != 0) {
                        ACELPcodeVec[idx[j]] = 8191;
                    } else {
                        ACELPcodeVec[idx[j]] = -BWF_HARMONIC_E;
                    }
                    pulseSign = (Ipp16s)(pulseSign >> 1);
                }

                parm += 2;
                decoderObj->BWDFrameCounter = 0;
            } else if(decoderObj->codecType == G729D_CODEC) {
                Ipp16s sTmpIdx;

                if(badFrameIndicator != 0) {
                    index = Rand_16s(&decoderObj->seedSavage);
                    pulseSign = Rand_16s(&decoderObj->seedSavage);
                } else {
                    index = parm[0];
                    pulseSign = parm[1];
                }
                ippsZero_16s(ACELPcodeVec,LP_SUBFRAME_DIM);
                sTmpIdx = tab3[index & 15];
                if((pulseSign & 1) != 0) {
                    ACELPcodeVec[sTmpIdx] += 8191;
                } else {
                    ACELPcodeVec[sTmpIdx] -= BWF_HARMONIC_E;
                }
                index >>= 4;
                pulseSign >>= 1;
                sTmpIdx = tab4[index & 31];

                if((pulseSign & 1) != 0) {
                    ACELPcodeVec[sTmpIdx] += 8191;
                } else {
                    ACELPcodeVec[sTmpIdx] -= BWF_HARMONIC_E;
                }

                parm += 2;
                decoderObj->BWDFrameCounter = 0;
            } else if(decoderObj->codecType == G729E_CODEC) {
                Ipp16s sIdxCounter, trackVal;
                Ipp16s pos1, pos2, pos3, sTmpPulseSign;
                ippsZero_16s(ACELPcodeVec,LP_SUBFRAME_DIM);

                if(badFrameIndicator != 0) {
                    tmp_parm[0] = Rand_16s(&decoderObj->seedSavage);
                    tmp_parm[1] = Rand_16s(&decoderObj->seedSavage);
                    tmp_parm[2] = Rand_16s(&decoderObj->seedSavage);
                    tmp_parm[3] = Rand_16s(&decoderObj->seedSavage);
                    tmp_parm[4] = Rand_16s(&decoderObj->seedSavage);
                } else {
                    ippsCopy_16s(parm, tmp_parm, 5);
                }
                if(LPmode == 0) {

                    pos1 = (Ipp16s)((tmp_parm[0] & 7) * 5);
                    if(((tmp_parm[0]>>3) & 1) == 0)
                        sTmpPulseSign = (1<<12);
                    else sTmpPulseSign = -(1<<12);
                    ACELPcodeVec[pos1] = sTmpPulseSign;
                    pos2 = (Ipp16s)(((tmp_parm[0]>>4) & 7) * 5);
                    if(pos2 > pos1)
                        sTmpPulseSign = (Ipp16s)(-sTmpPulseSign);
                    ACELPcodeVec[pos2] = (Ipp16s)(ACELPcodeVec[pos2] + sTmpPulseSign);
                    pos1 = (Ipp16s)(((tmp_parm[1] & 7) * 5) + 1);
                    if(((tmp_parm[1]>>3) & 1) == 0)
                        sTmpPulseSign = (1<<12);
                    else
                        sTmpPulseSign = -(1<<12);
                    ACELPcodeVec[pos1] = sTmpPulseSign;

                    pos2 = (Ipp16s)((((tmp_parm[1]>>4) & 7) * 5) + 1);
                    if(pos2 > pos1)
                        sTmpPulseSign = (Ipp16s)(-sTmpPulseSign);

                    ACELPcodeVec[pos2] = (Ipp16s)(ACELPcodeVec[pos2] + sTmpPulseSign);

                    pos1 = (Ipp16s)(((tmp_parm[2] & 7) * 5) + 2);
                    if(((tmp_parm[2]>>3) & 1) == 0)
                        sTmpPulseSign = (1<<12);
                    else
                        sTmpPulseSign = -(1<<12);
                    ACELPcodeVec[pos1] = sTmpPulseSign;

                    pos2 = (Ipp16s)((((tmp_parm[2]>>4) & 7) * 5) + 2);
                    if(pos2 > pos1)
                        sTmpPulseSign = (Ipp16s)(-sTmpPulseSign);
                    ACELPcodeVec[pos2] = (Ipp16s)(ACELPcodeVec[pos2] + sTmpPulseSign);
                    pos1 = (Ipp16s)(((tmp_parm[3] & 7) * 5) + 3);
                    if(((tmp_parm[3]>>3) & 1) == 0)
                        sTmpPulseSign = (1<<12);
                    else
                        sTmpPulseSign = -(1<<12);
                    ACELPcodeVec[pos1] = sTmpPulseSign;
                    pos2 = (Ipp16s)((((tmp_parm[3]>>4) & 7) * 5) + 3);
                    if(pos2 > pos1)
                        sTmpPulseSign = (Ipp16s)(-sTmpPulseSign);

                    ACELPcodeVec[pos2] = (Ipp16s)(ACELPcodeVec[pos2] + sTmpPulseSign);

                    pos1 = (Ipp16s)(((tmp_parm[4] & 7) * 5) + 4);
                    if(((tmp_parm[4]>>3) & 1) == 0)
                        sTmpPulseSign = (1<<12);
                    else sTmpPulseSign = -(1<<12);
                    ACELPcodeVec[pos1] = sTmpPulseSign;

                    pos2 = (Ipp16s)((((tmp_parm[4]>>4) & 7) * 5) + 4);
                    if(pos2 > pos1) sTmpPulseSign = (Ipp16s)(-sTmpPulseSign);

                    ACELPcodeVec[pos2] = (Ipp16s)(ACELPcodeVec[pos2] + sTmpPulseSign);
                    decoderObj->BWDFrameCounter = 0;
                } else {
                    trackVal = (Ipp16s)((tmp_parm[0]>>10) & 7);
                    if(trackVal > 4)
                        trackVal = 4;

                    for(sIdxCounter=0; sIdxCounter<2; sIdxCounter++) {
                        pos1 = (Ipp16s)(((tmp_parm[sIdxCounter] & 7) * 5) + trackVal);
                        if(((tmp_parm[sIdxCounter]>>3) & 1) == 0)
                            sTmpPulseSign = (1<<12);
                        else
                            sTmpPulseSign = -(1<<12);
                        ACELPcodeVec[pos1] = sTmpPulseSign;

                        pos2 = (Ipp16s)((((tmp_parm[sIdxCounter]>>4) & 7) * 5) + trackVal);
                        if(pos2 > pos1)
                            sTmpPulseSign = (Ipp16s)(-sTmpPulseSign);

                        ACELPcodeVec[pos2] = (Ipp16s)(ACELPcodeVec[pos2] + sTmpPulseSign);

                        pos3 = (Ipp16s)((((tmp_parm[sIdxCounter]>>7) & 7) * 5) + trackVal);
                        if(pos3 > pos2)
                            sTmpPulseSign = (Ipp16s)(-sTmpPulseSign);

                        ACELPcodeVec[pos3] = (Ipp16s)(ACELPcodeVec[pos3] + sTmpPulseSign);

                        trackVal++;
                        if(trackVal > 4)
                            trackVal = 0;
                    }

                    for(sIdxCounter=2; sIdxCounter<5; sIdxCounter++) {
                        pos1 = (Ipp16s)(((tmp_parm[sIdxCounter] & 7) * 5) + trackVal);
                        if(((tmp_parm[sIdxCounter]>>3) & 1) == 0)
                            sTmpPulseSign = (1<<12);
                        else
                            sTmpPulseSign = -(1<<12);
                        ACELPcodeVec[pos1] = sTmpPulseSign;

                        pos2 = (Ipp16s)((((tmp_parm[sIdxCounter]>>4) & 7) * 5) + trackVal);
                        if(pos2 > pos1)
                            sTmpPulseSign = (Ipp16s)(-sTmpPulseSign);
                        ACELPcodeVec[pos2] = (Ipp16s)(ACELPcodeVec[pos2] + sTmpPulseSign);
                        trackVal++;
                        if(trackVal > 4)
                            trackVal = 0;
                    }
                    decoderObj->BWDFrameCounter++;
                    if(decoderObj->BWDFrameCounter >= 30) {
                        statStat = 1;
                        decoderObj->BWDFrameCounter = 30;
                    }
                }
                parm += 5;
            }

            decoderObj->betaPreFilter = (Ipp16s)(decoderObj->betaPreFilter << 1);
            if(delayVal[0] < LP_SUBFRAME_DIM) {
                ippsHarmonicFilter_16s_I(decoderObj->betaPreFilter,delayVal[0],&ACELPcodeVec[delayVal[0]],LP_SUBFRAME_DIM-delayVal[0]);
            }
            pitchIndx = *parm++;

            if(decoderObj->codecType == G729_CODEC) {
                if(!badFrameIndicator) {
                    LOCAL_ARRAY(Ipp16s, gIngx, 2, decoderObj);
                    ippsDotProd_16s32s_Sfs(ACELPcodeVec, ACELPcodeVec, LP_SUBFRAME_DIM, &i, 0); /* ACELPcodeVec energy */
                    gIngx[0] = (Ipp16s)(pitchIndx >> CDBK2_BIT_NUM) ;
                    gIngx[1] = (Ipp16s)(pitchIndx & (CDBK2_DIM-1));
                    ippsDecodeGain_G729_16s(i, decoderObj->prevFrameQuantEn, gIngx, decoderObj->gains);
                    LOCAL_ARRAY_FREE(Ipp16s, gIngx, 2, decoderObj);
                } else {
                    ippsDecodeGain_G729_16s(0, decoderObj->prevFrameQuantEn, NULL, decoderObj->gains);
                }
            } else {
                Ipp32s energy;

                ippsDotProd_16s32s_Sfs(ACELPcodeVec, ACELPcodeVec, LP_SUBFRAME_DIM, &energy, 0); /* ACELPcodeVec energy */
                if(decoderObj->codecType == G729D_CODEC) {
                    if(badFrameIndicator) {
                        ippsDecodeGain_G729_16s(0, decoderObj->prevFrameQuantEn, NULL, decoderObj->gains);
                    } else {
                        LOCAL_ARRAY(Ipp16s, gIngx, 2, decoderObj);
                        Ipp16s foo = 1;
                        gIngx[0] =  (Ipp16s)(pitchIndx >> CDBK2_BIT_NUM_6K) ;
                        gIngx[1] =  (Ipp16s)(pitchIndx & (CDBK2_DIM_6K-1)) ;
                        ippsDecodeGain_G729I_16s(energy, foo, decoderObj->prevFrameQuantEn, gIngx, decoderObj->gains );
                        LOCAL_ARRAY_FREE(Ipp16s, gIngx, 2, decoderObj);
                    }
                } else { /* G729E_CODEC*/
                    if(!badFrameIndicator) {
                        LOCAL_ARRAY(Ipp16s, gIngx, 2, decoderObj);
                        gIngx[0] = (Ipp16s)(pitchIndx >> CDBK2_BIT_NUM) ;
                        gIngx[1] = (Ipp16s)(pitchIndx & (CDBK2_DIM-1));
                        ippsDecodeGain_G729_16s(energy, decoderObj->prevFrameQuantEn, gIngx, decoderObj->gains);
                        LOCAL_ARRAY_FREE(Ipp16s, gIngx, 2, decoderObj);
                    } else { /* erasure*/
                        Ipp16s oldCodebookGain = decoderObj->gains[1];

                        ippsDecodeGain_G729I_16s(0, decoderObj->valGainAttenuation, decoderObj->prevFrameQuantEn, NULL, decoderObj->gains );
                        if(decoderObj->BFIcount < 2) {
                            decoderObj->gains[0] = (Ipp16s)((statStat)? BWF_HARMONIC : PITCH_GAIN_MAX);
                            decoderObj->gains[1] = oldCodebookGain;
                        } else {
                            if(statStat) {
                                if(decoderObj->BFIcount > 10) decoderObj->valGainAttenuation = (Ipp16s)((decoderObj->valGainAttenuation * 32604)>>15);
                            } else decoderObj->valGainAttenuation = (Ipp16s)((decoderObj->valGainAttenuation * 32112)>>15);
                        }
                    }
                }
            }

            /* update pitch sharpening  with quantized gain pitch */
            decoderObj->betaPreFilter = decoderObj->gains[0];
            if(decoderObj->betaPreFilter > PITCH_SHARP_MAX)
                decoderObj->betaPreFilter = PITCH_SHARP_MAX;
            if(decoderObj->betaPreFilter < PITCH_SHARP_MIN)
                decoderObj->betaPreFilter = PITCH_SHARP_MIN;

            /* synthesis of speech corresponding to excitation*/
            if(badFrameIndicator) {
                decoderObj->BFIcount++;
                if(voiceFlag == 0 ) {
                    gC = decoderObj->gains[1];
                    gPl = 0;
                } else {
                    gC = 0;
                    gPl = decoderObj->gains[0];
                }
            } else {
                gC = decoderObj->gains[1];
                gPl = decoderObj->gains[0];
            }
            ippsInterpolateC_NR_G729_16s_Sfs(&excitation[subfrIdx],gPl,ACELPcodeVec,gC,&excitation[subfrIdx],LP_SUBFRAME_DIM,14);
            if(decoderObj->codecType == G729D_CODEC)
                status = SynthesisFilter_G729_16s_update(ppAz,&excitation[subfrIdx],&synth[subfrIdx],LP_SUBFRAME_DIM,synFltw,BWLPCF_DIM-mAq,0);
            else
                status = SynthesisFilter_G729_16s(ppAz,&excitation[subfrIdx],&synth[subfrIdx],LP_SUBFRAME_DIM,synFltw,BWLPCF_DIM-mAq);
            if(status == ippStsOverflow) {

                ippsRShiftC_16s_I(2,prevExcitat,L_prevExcitat+LP_FRAME_DIM);
                SynthesisFilterOvf_G729_16s(ppAz,&excitation[subfrIdx],&synth[subfrIdx],LP_SUBFRAME_DIM,synFltw,BWLPCF_DIM-mAq);
            }
            if(decoderObj->codecType == G729D_CODEC) {
                ippsPhaseDispersion_G729D_16s(&excitation[subfrIdx], phaseDispExcit, decoderObj->gains[1],
                                              decoderObj->gains[0], ACELPcodeVec, decoderObj->PhDispMem);
                SynthesisFilter_G729_16s(ppAz,phaseDispExcit,&synth[subfrIdx],LP_SUBFRAME_DIM,synFltw,BWLPCF_DIM-mAq);
            } else {
                ippsPhaseDispersionUpdate_G729D_16s(decoderObj->gains[0], decoderObj->gains[1],decoderObj->PhDispMem);
            }
            ppAz += mAq+1;
        }
    }

    if(badFrameIndicator == 0) {
        ippsDotProd_16s32s_Sfs(excitation,excitation,LP_FRAME_DIM,&i,-1);
        decoderObj->SIDflag1 = Exp_32s(i);
        decoderObj->SIDflag0 = (Ipp16s)(((i << decoderObj->SIDflag1)+0x8000)>>16);
        decoderObj->SIDflag1 = (Ipp16s)(16 - decoderObj->SIDflag1);
    }
    decoderObj->CNGvar = (Ipp16s)fType;

    if(enerDB(synth, (Ipp16s)LP_FRAME_DIM) >= BWF_HARMONIC_E) tstDominantBWDmode(&decoderObj->BWDcounter2,&decoderObj->FWDcounter2,&dominantBWDmode, (Ipp16s)LPmode);

    ippsMove_16s(&prevExcitat[LP_FRAME_DIM], &prevExcitat[0], L_prevExcitat);

    if( LPmode == 0) {
        ippsCopy_16s(FWDfiltLPC, AzDec, 2*LPF_DIM+2);
        prevM = LPF_DIM;
    } else {
        ippsCopy_16s(BWDfiltLPC, AzDec, 2*BWLPCF1_DIM);
        prevM = BWLPCF_DIM;
    }

    decoderObj->prevBFI = badFrameIndicator;
    decoderObj->prevLPmode = (Ipp16s)LPmode;
    decoderObj->prevVoiceFlag = voiceFlag;

    if(badFrameIndicator != 0)
        decoderObj->interpCoeff2 = (1<<12);
    else {
        if(LPmode == 0)
            decoderObj->interpCoeff2 = 0;
        else {
            if(dominantBWDmode == 1)
                decoderObj->interpCoeff2 -= 410;
            else decoderObj->interpCoeff2 -= 2048;
                if(decoderObj->interpCoeff2 < 0)
                    decoderObj->interpCoeff2= 0;
        }
    }
    decoderObj->voiceFlag = 0;
    pAz = AzDec;

    if (decoderObj->pstFltMode) {
      if((decoderObj->codecType == G729_CODEC)&&(fType>2)) {
        for(subfrIdx=0; subfrIdx<LP_FRAME_DIM; subfrIdx+=LP_SUBFRAME_DIM) {
            Post_G729(prevFrameDelay1,(Ipp16s)subfrIdx,pAz,&dst[subfrIdx],&subfrVoiceFlag,decoderObj);
            if(subfrVoiceFlag != 0) {
                decoderObj->voiceFlag = subfrVoiceFlag;
            }
            pAz += LPF_DIM+1;
        }
      } else {
        parm = decoderObj->decPrm;
        if(fType<4) {
            L_hSt = IMP_RESP_LEN;
            decoderObj->gammaPost1 = BWF1_PST;
            decoderObj->gammaPost2 = BWF2_PST;
            decoderObj->gammaHarm = BWF_HARMONIC;
        } else {
            L_hSt = IMP_RESP_LEN_E;
            /* reduce postfiltering */
            if((parm[2] == 1) && (dominantBWDmode == 1)) {
                decoderObj->gammaHarm -= 410;
                if(decoderObj->gammaHarm < 0)
                    decoderObj->gammaHarm = 0;
                decoderObj->gammaPost1 -= 1147;
                if(decoderObj->gammaPost1 < 0)
                    decoderObj->gammaPost1 = 0;
                decoderObj->gammaPost2 -= 1065;
                if(decoderObj->gammaPost2 < 0)
                    decoderObj->gammaPost2 = 0;
            } else {
                decoderObj->gammaHarm += 410;
                if(decoderObj->gammaHarm > BWF_HARMONIC_E)
                    decoderObj->gammaHarm = BWF_HARMONIC_E;
                decoderObj->gammaPost1 += 1147;
                if(decoderObj->gammaPost1 > BWF1_PST_E)
                    decoderObj->gammaPost1 = BWF1_PST_E;
                decoderObj->gammaPost2 += 1065;
                if(decoderObj->gammaPost2 > BWF2_PST_E)
                    decoderObj->gammaPost2 = BWF2_PST_E;
            }
        }

        for(i=0; i<LP_FRAME_DIM; i+=LP_SUBFRAME_DIM) {
            Post_G729I(prevFrameDelay1, (Ipp16s)i, pAz, &dst[i],
                       &subfrVoiceFlag, L_hSt, prevM, (Ipp16s)fType, decoderObj);
            if(subfrVoiceFlag != 0)
                decoderObj->voiceFlag = subfrVoiceFlag;
            pAz += prevM+1;
        }
      }
      ippsHighPassFilter_G729_16s_ISfs(dst,LP_FRAME_DIM,13,decoderObj->postProc);
    } else {
      ippsAdd_16s(synth, synth, dst, LP_FRAME_DIM);
    }

    CLEAR_SCRATCH_MEMORY(decoderObj);

    return APIG729_StsNoErr;
}

void Post_G729Base(
                  Ipp16s delayVal,          /* pitch delayVal given by coder */
                  Ipp16s subfrIdx,
                  const Ipp16s *srcLPC,     /* LPC coefficients for current subframe */
                  Ipp16s *dstSignal,        /* postfiltered output */
                  Ipp16s *voiceFlag,        /* voiceFlag decision 0 = uv,  > 0 delayVal */
                  Ipp16s fType,
                  G729Decoder_Obj *decoderObj
                  ) {
    Ipp16s bwf1 = decoderObj->gammaPost1;
    Ipp16s bwf2 = decoderObj->gammaPost2;
    Ipp16s gamma_harm = decoderObj->gammaHarm;
    LOCAL_ARRAY(Ipp32s,irACF,2, decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s,y, IMP_RESP_LEN_E, decoderObj);
    LOCAL_ALIGN_ARRAY(32,Ipp16s, LTPsignal, LP_SUBFRAME_DIM+1, decoderObj);
    LOCAL_ALIGN_ARRAY(32,Ipp16s, LPCdenom, LPF_DIM+1, decoderObj);  /* denominator srcLPC */
    LOCAL_ALIGN_ARRAY(32,Ipp16s, LPCnum, IMP_RESP_LEN_E, decoderObj); /* numerator srcLPC */
    Ipp16s tmp, g0Val, temp, ACFval0, ACFval1;
    Ipp16s *iirdl = ((SynthesisFilterState*)decoderObj->synFltw0)->buffer;
    Ipp32s   L_g0Val, normVal = 0, status = 0;
    const Ipp16s *signal_ptr = &decoderObj->LTPostFilt[SYNTH_BWD_DIM+subfrIdx];
    irACF[0] = irACF[1] = 0;
    ippsZero_16s(LPCnum, IMP_RESP_LEN_E);

    ippsMulPowerC_NR_16s_Sfs(srcLPC,bwf1, LPCdenom,LPF_DIM+1,15);
    ippsMulPowerC_NR_16s_Sfs(srcLPC,bwf2, LPCnum,LPF_DIM+1,15);

    ippsResidualFilter_G729_16s((Ipp16s *)signal_ptr, LPCnum, &decoderObj->resFilBuf1[RES_FIL_DIM]);

    if(fType > 1)
        ippsLongTermPostFilter_G729_16s(gamma_harm,delayVal, &decoderObj->resFilBuf1[RES_FIL_DIM],
                                        LTPsignal + 1, voiceFlag);
    else {
        *voiceFlag = 0;
        ippsCopy_16s(&decoderObj->resFilBuf1[RES_FIL_DIM], LTPsignal + 1, LP_SUBFRAME_DIM);
    }
    LTPsignal[0] = decoderObj->preemphFilt;
    ippsSynthesisFilter_NR_16s_Sfs(LPCdenom, LPCnum,y,IMP_RESP_LEN, 12, &decoderObj->zeroPostFiltVec1[LPF_DIM+1]);

    status = ippsAutoCorr_NormE_16s32s(y,IMP_RESP_LEN,irACF,2,&normVal);
    ACFval0 = (Ipp16s)(irACF[0]>>16);
    ACFval1 = (Ipp16s)(irACF[1]>>16);
    if( ACFval0 < Abs_16s(ACFval1) || status) {
        tmp = 0;
    } else {
        tmp = (Ipp16s)((Abs_16s(ACFval1)<<15)/ACFval0);
        if(ACFval1 > 0) {
            tmp = (Ipp16s)(-tmp);
        }
    }

    ippsAbs_16s_I(y,IMP_RESP_LEN);
    ippsSum_16s32s_Sfs(y,IMP_RESP_LEN,&L_g0Val,0);
    g0Val = (Ipp16s)(ShiftL_32s(L_g0Val, 14)>>16);

    if(g0Val > 1024) {
        temp = (Ipp16s)((1024<<15)/g0Val);
        ippsMulC_NR_16s_ISfs(temp,LTPsignal + 1,LP_SUBFRAME_DIM,15);
    }
    ippsSynthesisFilter_NR_16s_ISfs(LPCdenom, LTPsignal + 1, LP_SUBFRAME_DIM, 12, &iirdl[BWLPCF_DIM-LPF_DIM]);
    decoderObj->preemphFilt = LTPsignal[LP_SUBFRAME_DIM];
    ippsCopy_16s(&LTPsignal[LP_SUBFRAME_DIM-LPF_DIM+1], &iirdl[BWLPCF_DIM-LPF_DIM], LPF_DIM );
    ippsTiltCompensation_G729E_16s(tmp,LTPsignal, dstSignal);
    ippsGainControl_G729_16s_I(signal_ptr, dstSignal, &decoderObj->gainExact);
    ippsMove_16s(&decoderObj->resFilBuf1[LP_SUBFRAME_DIM], &decoderObj->resFilBuf1[0], RES_FIL_DIM);

    LOCAL_ALIGN_ARRAY_FREE(32,Ipp16s, LPCnum, IMP_RESP_LEN_E, decoderObj); /* numerator srcLPC  */
    LOCAL_ALIGN_ARRAY_FREE(32,Ipp16s, LPCdenom, LPF_DIM+1, decoderObj);  /* denominator srcLPC  */
    LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, LTPsignal,LP_SUBFRAME_DIM+1,decoderObj);
    LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s,y, IMP_RESP_LEN_E,decoderObj);
    LOCAL_ARRAY_FREE(Ipp32s, irACF, 2,decoderObj);
    return;
}

void Post_G729( Ipp16s delayVal, Ipp16s subfrIdx, const Ipp16s *srcLPC, Ipp16s *dstSignal,
              Ipp16s *voiceFlag, G729Decoder_Obj *decoderObj) {
    Ipp16s *gainExact = &decoderObj->gainExact;
    Ipp16s *iirdl = ((SynthesisFilterState*)decoderObj->synFltw0)->buffer;
    LOCAL_ALIGN_ARRAY(32, Ipp16s,y, IMP_RESP_LEN, decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, LTPsignalBuf, LP_SUBFRAME_DIM+1+LPF_DIM, decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, LPCnum, LPF_DIM+1, decoderObj);
    Ipp16s tmp;
    const Ipp16s *res = &decoderObj->LTPostFilt[SYNTH_BWD_DIM+subfrIdx];
    Ipp16s *LTPsignal = LTPsignalBuf+LPF_DIM+1;

    ippsMul_NR_16s_Sfs(g729gammaFac2_pst,srcLPC, LPCnum,LPF_DIM+1,15);
    ippsResidualFilter_G729_16s(res, LPCnum, decoderObj->resFilBuf1 + RES_FIL_DIM);
    ippsLongTermPostFilter_G729_16s(BWF_HARMONIC,delayVal, decoderObj->resFilBuf1 + RES_FIL_DIM, LTPsignal+1, &tmp);
    *voiceFlag = (Ipp16s)(tmp != 0);

    ippsMove_16s(&decoderObj->resFilBuf1[LP_SUBFRAME_DIM], &decoderObj->resFilBuf1[0], RES_FIL_DIM);
    ippsCopy_16s(iirdl+20,LTPsignal+1-LPF_DIM, LPF_DIM );
    ippsShortTermPostFilter_G729_16s(srcLPC, LTPsignal+1,LTPsignal+1,y);
    ippsCopy_16s((LTPsignal+1+LP_SUBFRAME_DIM-BWLPCF_DIM), iirdl, BWLPCF_DIM);

    LTPsignal[0] = decoderObj->preemphFilt;
    decoderObj->preemphFilt = LTPsignal[LP_SUBFRAME_DIM];

    ippsTiltCompensation_G729_16s(y, LTPsignal+1);
    ippsCopy_16s(LTPsignal+1, dstSignal,LP_SUBFRAME_DIM);
    ippsGainControl_G729_16s_I(res, dstSignal, gainExact);

    LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, LPCnum, LPF_DIM+1,decoderObj);
    LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, LTPsignalBuf,LP_SUBFRAME_DIM+1+LPF_DIM,decoderObj);
    LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s,y, IMP_RESP_LEN,decoderObj);

    return;
}

void Post_G729AB(Ipp16s delayVal, Ipp16s subfrIdx, const Ipp16s *srcLPC, Ipp16s *syn_pst,
                Ipp16s ftype, G729Decoder_Obj *decoderObj) {
    Ipp16s *iirdl = ((SynthesisFilterState*)decoderObj->synFltw0)->buffer;
    Ipp16s *preemphFilt = &decoderObj->preemphFilt;
    LOCAL_ALIGN_ARRAY(32, Ipp16s,sndLPC,2*(LPF_DIM+1), decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, prevResidual, LP_SUBFRAME_DIM+8, decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, prevBuf, LP_SUBFRAME_DIM+LPF_DIM, decoderObj);
    Ipp16s *prevResidual2 = prevResidual+8;
    Ipp16s *pst = prevBuf+LPF_DIM;

    ippsMul_NR_16s_Sfs(g729gammaFac2_pst,srcLPC, decoderObj->zeroPostFiltVec1, LPF_DIM+1,15);
    ippsMul_NR_16s_Sfs(g729gammaFac2_pst,srcLPC, sndLPC, LPF_DIM+1,15);
    ippsMul_NR_16s_Sfs(g729gammaFac1_pst,srcLPC, sndLPC+LPF_DIM+1,LPF_DIM+1,15);

    ippsLongTermPostFilter_G729A_16s(delayVal,&decoderObj->LTPostFilt[LPF_DIM+subfrIdx],
                                     sndLPC,decoderObj->resFilBuf1-LPF_DIM-1,prevResidual2);
    ippsMove_16s(&decoderObj->resFilBuf1[LP_SUBFRAME_DIM], &decoderObj->resFilBuf1[0], MAX_PITCH_LAG);
    if(3 != ftype)
        ippsCopy_16s(decoderObj->resFilBuf1 + MAX_PITCH_LAG,prevResidual2,LP_SUBFRAME_DIM);
    prevResidual2[-1] = *preemphFilt;
    *preemphFilt = prevResidual2[LP_SUBFRAME_DIM-1];

    ippsTiltCompensation_G729A_16s(sndLPC,prevResidual2);

    ippsCopy_16s(iirdl,pst-LPF_DIM, LPF_DIM );
    ippsSynthesisFilter_NR_16s_Sfs(sndLPC+LPF_DIM+1,prevResidual2,pst,
                                        LP_SUBFRAME_DIM, 12, pst-LPF_DIM);
    ippsCopy_16s((pst+LP_SUBFRAME_DIM-LPF_DIM), iirdl, LPF_DIM );
    ippsCopy_16s(pst,syn_pst,LP_SUBFRAME_DIM);

    ippsGainControl_G729A_16s_I(&decoderObj->LTPostFilt[LPF_DIM+subfrIdx], syn_pst, &decoderObj->gainExact);

    LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, prevBuf,LP_SUBFRAME_DIM+LPF_DIM,decoderObj);
    LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, prevResidual, LP_SUBFRAME_DIM+8,decoderObj);
    LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, sndLPC, 2*(LPF_DIM+1),decoderObj);
}

void Post_G729I( Ipp16s delayVal, Ipp16s subfrIdx, const Ipp16s *srcLPC, Ipp16s *dstSignal,
               Ipp16s *voiceFlag, Ipp16s L_hSt, Ipp16s prevM, Ipp16s fType,
               G729Decoder_Obj *decoderObj){
    Ipp16s bwf1 = decoderObj->gammaPost1;
    Ipp16s bwf2 = decoderObj->gammaPost2;
    Ipp16s gamma_harm = decoderObj->gammaHarm;
    LOCAL_ARRAY(Ipp32s,irACF,2, decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s,y, IMP_RESP_LEN_E, decoderObj);
    LOCAL_ALIGN_ARRAY(32,Ipp16s, LTPsignal, LP_SUBFRAME_DIM+1, decoderObj);
    LOCAL_ALIGN_ARRAY(32,Ipp16s, LPCdenom, BWLPCF1_DIM, decoderObj);
    LOCAL_ALIGN_ARRAY(32,Ipp16s, LPCnum, IMP_RESP_LEN_E, decoderObj);
    Ipp16s tmp, g0Val, temp, ACFval0, ACFval1;
    Ipp16s *iirdl = ((SynthesisFilterState*)decoderObj->synFltw0)->buffer;
    Ipp32s L_g0Val, normVal = 0, status = 0;
    const Ipp16s *signal_ptr = &decoderObj->LTPostFilt[SYNTH_BWD_DIM+subfrIdx];
    irACF[0] = irACF[1] = 0;
    ippsZero_16s(LPCnum, IMP_RESP_LEN_E);

    ippsMulPowerC_NR_16s_Sfs(srcLPC,bwf1, LPCdenom,prevM+1,15);
    ippsMulPowerC_NR_16s_Sfs(srcLPC,bwf2, LPCnum,prevM+1,15);
    ippsResidualFilter_G729E_16s(LPCnum, prevM,(Ipp16s *)signal_ptr, &decoderObj->resFilBuf1[RES_FIL_DIM], LP_SUBFRAME_DIM);
    if(fType > 1)
        ippsLongTermPostFilter_G729_16s(gamma_harm,delayVal, &decoderObj->resFilBuf1[RES_FIL_DIM],
                                        LTPsignal + 1, voiceFlag);
    else {
        *voiceFlag = 0;
        ippsCopy_16s(&decoderObj->resFilBuf1[RES_FIL_DIM], LTPsignal + 1, LP_SUBFRAME_DIM);
    }

    LTPsignal[0] = decoderObj->preemphFilt;
    ippsSynthesisFilter_G729E_16s(LPCdenom, prevM,LPCnum,y, L_hSt, &decoderObj->zeroPostFiltVec1[LPF_DIM+1]);
    status = ippsAutoCorr_NormE_16s32s(y,L_hSt,irACF,2,&normVal);
    ACFval0   = (Ipp16s)(irACF[0]>>16);
    ACFval1  = (Ipp16s)(irACF[1]>>16);
    if( ACFval0 < Abs_16s(ACFval1) || status) {
        tmp = 0;
    } else {
        tmp = (Ipp16s)((Abs_16s(ACFval1)<<15)/ACFval0);
        if(ACFval1 > 0) {
            tmp = (Ipp16s)(-tmp);
        }
    }
    ippsAbs_16s_I(y,L_hSt);
    ippsSum_16s32s_Sfs(y,L_hSt,&L_g0Val,0);
    g0Val = (Ipp16s)(ShiftL_32s(L_g0Val, 14)>>16);
    if(g0Val > 1024) {
        temp = (Ipp16s)((1024<<15)/g0Val);
        ippsMulC_NR_16s_ISfs(temp,LTPsignal + 1,LP_SUBFRAME_DIM,15);
    }
    ippsSynthesisFilter_G729E_16s_I(LPCdenom,prevM,LTPsignal + 1, LP_SUBFRAME_DIM,&iirdl[BWLPCF_DIM-prevM]);
    decoderObj->preemphFilt = LTPsignal[LP_SUBFRAME_DIM];
    ippsCopy_16s(&LTPsignal[LP_SUBFRAME_DIM-BWLPCF_DIM+1], iirdl, BWLPCF_DIM);

    ippsTiltCompensation_G729E_16s(tmp,LTPsignal, dstSignal);

    ippsGainControl_G729_16s_I(signal_ptr, dstSignal, &decoderObj->gainExact);

    ippsMove_16s(&decoderObj->resFilBuf1[LP_SUBFRAME_DIM], &decoderObj->resFilBuf1[0], RES_FIL_DIM);

    LOCAL_ALIGN_ARRAY_FREE(32,Ipp16s, LPCnum, IMP_RESP_LEN_E, decoderObj);
    LOCAL_ALIGN_ARRAY_FREE(32,Ipp16s, LPCdenom, BWLPCF1_DIM, decoderObj);
    LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, LTPsignal,LP_SUBFRAME_DIM+1,decoderObj);
    LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s,y, IMP_RESP_LEN_E,decoderObj);
    LOCAL_ARRAY_FREE(Ipp32s, irACF, 2,decoderObj);
    return;
}

APIG729_Status G729ADecode
(G729Decoder_Obj* decoderObj,const Ipp8u* src, Ipp32s frametype, Ipp16s* dst) {

    LOCAL_ALIGN_ARRAY(32, Ipp16s, AzDec, (LPF_DIM+1)*2,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, newLSP,LPF_DIM,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, ACELPcodeVec, LP_SUBFRAME_DIM,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, FWDfiltLPC, 2*LPF_DIM+2,decoderObj);
    LOCAL_ARRAY(Ipp16s, prevFrameDelay,2,decoderObj);
    LOCAL_ARRAY(Ipp16s, idx,4,decoderObj);
    LOCAL_ARRAY(Ipp16s, delayVal,2,decoderObj);
    Ipp16s *pAz, *pA, *ppAz, temp;
    Ipp8s *synFltw = decoderObj->synFltw;
    Ipp16s *prevExcitat = decoderObj->prevExcitat;
    Ipp16s *excitation = prevExcitat + L_prevExcitat;
    Ipp16s *synth = decoderObj->LTPostFilt+LPF_DIM;
    Ipp16s *prevSubfrLSP = decoderObj->prevSubfrLSP;
    const Ipp8u *pParm;
    const Ipp16s *parm;
    Ipp16s sidGain = decoderObj->sidGain;
    Ipp16s gainNow = decoderObj->gainNow;
    Ipp16s *lspSID = decoderObj->lspSID;
    Ipp32s   i, j, subfrIdx, index2, fType;
    Ipp16s badFrameIndicator, badPitch, index, pulseSign;

    if(NULL==decoderObj || NULL==src || NULL ==dst)
        return APIG729_StsBadArgErr;
    if(decoderObj->objPrm.objSize <= 0)
        return APIG729_StsNotInitialized;
    if(DEC_KEY != decoderObj->objPrm.key)
        return APIG729_StsBadCodecType;

    delayVal[0]=delayVal[1]=0;
    pA = AzDec;
    pParm = src;

    if(frametype == -1) {
        decoderObj->decPrm[1] = 0;
        decoderObj->decPrm[0] = 1;
    } else if(frametype == 0) {
        decoderObj->decPrm[1] = 0;
        decoderObj->decPrm[0] = 0;
    } else if(frametype == 1 || frametype == 5) {
        decoderObj->decPrm[1] = 1;
        decoderObj->decPrm[0] = 0;
        i=0;
        decoderObj->decPrm[1+1] = ExtractBitsG729(&pParm,&i,1);
        decoderObj->decPrm[1+2] = ExtractBitsG729(&pParm,&i,5);
        decoderObj->decPrm[1+3] = ExtractBitsG729(&pParm,&i,4);
        decoderObj->decPrm[1+4] = ExtractBitsG729(&pParm,&i,5);
    } else if(frametype == 3) {
        i=0;
        decoderObj->decPrm[1] = 3;
        decoderObj->decPrm[0] = 0;
        decoderObj->decPrm[1+1] = ExtractBitsG729(&pParm,&i,1+FIR_STAGE_BITS);
        decoderObj->decPrm[1+2] = ExtractBitsG729(&pParm,&i,SEC_STAGE_BITS*2);
        decoderObj->decPrm[1+3] = ExtractBitsG729(&pParm,&i,8);
        decoderObj->decPrm[1+4] = ExtractBitsG729(&pParm,&i,1);
        decoderObj->decPrm[1+5] = ExtractBitsG729(&pParm,&i,13);
        decoderObj->decPrm[1+6] = ExtractBitsG729(&pParm,&i,4);
        decoderObj->decPrm[1+7] = ExtractBitsG729(&pParm,&i,7);
        decoderObj->decPrm[1+8] = ExtractBitsG729(&pParm,&i,5);
        decoderObj->decPrm[1+9] = ExtractBitsG729(&pParm,&i,13);
        decoderObj->decPrm[1+10] = ExtractBitsG729(&pParm,&i,4);
        decoderObj->decPrm[1+11] = ExtractBitsG729(&pParm,&i,7);
        decoderObj->decPrm[1+4] = (Ipp16s)((equality(decoderObj->decPrm[1+3])+decoderObj->decPrm[1+4]) & 0x00000001); /*  parity error (1) */
    }

    parm = decoderObj->decPrm;

    badFrameIndicator = *parm++;
    fType = *parm;
    if(badFrameIndicator == 1) {
        fType = decoderObj->CNGvar;
        if(fType == 1) fType = 0;
    }

    ippsMove_16s(&decoderObj->LTPostFilt[LP_FRAME_DIM], &decoderObj->LTPostFilt[0], LPF_DIM );

    if(fType < 2) {
        if(fType == 1) {
            LOCAL_ALIGN_ARRAY(32, Ipp16s, lsfq,LPF_DIM,decoderObj);
            sidGain = SIDgain[(Ipp32s)parm[4]];
            ippsLSFDecode_G729B_16s(&parm[1],(Ipp16s*)(decoderObj->prevLSPfreq),lsfq);
            ippsLSFToLSP_G729_16s(lsfq,lspSID);
            LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, lsfq,LPF_DIM,decoderObj);
        } else {
            if(decoderObj->CNGvar > 1) {
                QuantSIDGain_G729B_16s(&decoderObj->SIDflag0, &decoderObj->SIDflag1, 0, &temp, &index2);
                sidGain = SIDgain[(Ipp32s)index2];
            }
        }
        if(decoderObj->CNGvar > 1 || frametype == 5) {
            gainNow = sidGain;
        } else {
            gainNow = (Ipp16s)((gainNow * GAIN0 + BWF_HARMONIC)>>15);
            gainNow = Add_16s(gainNow, (Ipp16s)((sidGain * GAIN1 + BWF_HARMONIC)>>15));
        }

        if(gainNow == 0) ippsZero_16s(excitation,LP_FRAME_DIM);
        else {
            for(i = 0;  i < LP_FRAME_DIM; i += LP_SUBFRAME_DIM) {
                Ipp32s invSq;
                Ipp16s pG2;
                Ipp16s g;
                const Ipp16s *excCached;
                LOCAL_ARRAY(Ipp16s, tmpIdxVec, 4, decoderObj);
                LOCAL_ARRAY(Ipp16s, pulsesSigns, 4, decoderObj);
                LOCAL_ALIGN_ARRAY(32, Ipp16s, excg, LP_SUBFRAME_DIM, decoderObj);
                LOCAL_ARRAY(Ipp16s,tempArray,LP_SUBFRAME_DIM, decoderObj);

                RandomCodebookParm_G729B_16s(&decoderObj->seed,tmpIdxVec,pulsesSigns,&pG2,delayVal);
                ippsDecodeAdaptiveVector_G729_16s_I(delayVal,&prevExcitat[i]);
                if(decoderObj->CNGidx > CNG_STACK_SIZE-1) { /* not cached */
                    ippsRandomNoiseExcitation_G729B_16s(&decoderObj->seed,excg,LP_SUBFRAME_DIM);
                    ippsDotProd_16s32s_Sfs(excg,excg,LP_SUBFRAME_DIM,&invSq,0);
                    ippsInvSqrt_32s_I(&invSq,1);  /* Q30 */
                    excCached=excg;
                } else {
                    decoderObj->seed = cngSeedOut[decoderObj->CNGidx];
                    invSq = cngInvSqrt[decoderObj->CNGidx];
                    excCached=&cngCache[decoderObj->CNGidx][0];
                    decoderObj->CNGidx++;
                }
                NoiseExcitationFactorization_G729B_16s(excCached,invSq,gainNow,excg,LP_SUBFRAME_DIM);
                ComfortNoiseExcitation_G729B_16s_I(excg,tmpIdxVec,pulsesSigns,gainNow,pG2,&excitation[i],&g,tempArray);

                LOCAL_ARRAY_FREE(Ipp16s,tempArray,LP_SUBFRAME_DIM, decoderObj);
                LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, excg, LP_SUBFRAME_DIM, decoderObj);
                LOCAL_ARRAY_FREE(Ipp16s, pulsesSigns, 4, decoderObj);
                LOCAL_ARRAY_FREE(Ipp16s, tmpIdxVec, 4, decoderObj);
            }
        }
        ippsInterpolate_G729_16s(prevSubfrLSP,lspSID,prevSubfrLSP, LPF_DIM );
        ippsLSPToLPC_G729_16s(prevSubfrLSP,&FWDfiltLPC[0]);
        ippsLSPToLPC_G729_16s(lspSID,&FWDfiltLPC[LPF_DIM+1]);
        ippsCopy_16s(lspSID, prevSubfrLSP, LPF_DIM );
        decoderObj->sidGain = sidGain;
        decoderObj->gainNow = gainNow;

        ppAz = FWDfiltLPC;
        for(subfrIdx = 0; subfrIdx < LP_FRAME_DIM; subfrIdx += LP_SUBFRAME_DIM) {
            if(ippsSynthesisFilter_NR_16s_Sfs(ppAz,&excitation[subfrIdx],&synth[subfrIdx], LP_SUBFRAME_DIM, 12,
                                              ((SynthesisFilterState*)synFltw)->buffer)==ippStsOverflow) {
                ippsRShiftC_16s_I(2,prevExcitat,L_prevExcitat+LP_FRAME_DIM);
                ippsSynthesisFilter_NR_16s_Sfs(ppAz,&excitation[subfrIdx],&synth[subfrIdx],LP_SUBFRAME_DIM,12,
                                               ((SynthesisFilterState*)synFltw)->buffer);
            }
            ippsCopy_16s((&synth[subfrIdx]+LP_SUBFRAME_DIM-LPF_DIM), ((SynthesisFilterState*)synFltw)->buffer, LPF_DIM );

            ppAz += LPF_DIM+1;
            prevFrameDelay[subfrIdx/LP_SUBFRAME_DIM] = decoderObj->prevFrameDelay;
        }
        decoderObj->betaPreFilter = PITCH_SHARP_MIN;

    } else {
        LOCAL_ARRAY(Ipp16s, qIndex,4,decoderObj);
        decoderObj->seed = SEED_INIT;
        decoderObj->CNGidx = 0;
        parm++;

        qIndex[0] = (Ipp16s)((parm[0] >> FIR_STAGE_BITS) & 1);
        qIndex[1] = (Ipp16s)(parm[0] & (FIR_STAGE - 1));
        qIndex[2] = (Ipp16s)((parm[1] >> SEC_STAGE_BITS) & (SEC_STAGE - 1));
        qIndex[3] = (Ipp16s)(parm[1] & (SEC_STAGE - 1));
        if(!badFrameIndicator) {
            decoderObj->prevMA = qIndex[0];
            ippsLSFDecode_G729_16s( qIndex, (Ipp16s*)decoderObj->prevLSPfreq, decoderObj->prevSubfrLSPquant);
        } else {
            ippsLSFDecodeErased_G729_16s( decoderObj->prevMA,
                                          (Ipp16s*)decoderObj->prevLSPfreq, decoderObj->prevSubfrLSPquant);
        }

        ippsLSFToLSP_G729_16s(decoderObj->prevSubfrLSPquant, newLSP); /* Convert LSFs to LSPs */
        parm += 2;
        LOCAL_ARRAY_FREE(Ipp16s, qIndex,4,decoderObj);
        ippsInterpolate_G729_16s(newLSP,prevSubfrLSP,prevSubfrLSP, LPF_DIM );

        ippsLSPToLPC_G729_16s(prevSubfrLSP, FWDfiltLPC);          /* 1-st subframe */

        ippsLSPToLPC_G729_16s(newLSP, &FWDfiltLPC[LPF_DIM+1]);    /*  2-nd subframe*/

        ippsCopy_16s(newLSP, prevSubfrLSP, LPF_DIM );
        pA = FWDfiltLPC; ppAz = pA;

        for(subfrIdx=0; subfrIdx < LP_FRAME_DIM; subfrIdx+=LP_SUBFRAME_DIM) {
            Ipp32s pitchIndx;
            badPitch = badFrameIndicator;
            pitchIndx = *parm++;
            if(subfrIdx == 0) {
                i = (Ipp16s)(*parm++);
                badPitch = (Ipp16s)(badFrameIndicator + i);
            }
            DecodeAdaptCodebookDelays(&decoderObj->prevFrameDelay,&decoderObj->prevFrameDelay2,delayVal,subfrIdx,badPitch,pitchIndx,decoderObj->codecType);
            prevFrameDelay[subfrIdx/LP_SUBFRAME_DIM] = delayVal[0];
            ippsDecodeAdaptiveVector_G729_16s_I(delayVal,&prevExcitat[subfrIdx]);

            if(badFrameIndicator != 0) {
                index = (Ipp16s)(Rand_16s(&decoderObj->seedSavage) & (Ipp16s)0x1fff);
                pulseSign = (Ipp16s)(Rand_16s(&decoderObj->seedSavage) & (Ipp16s)15);
            } else {
                index = parm[0];
                pulseSign = parm[1];
            }

            i      = index & 7;
            idx[0] = (Ipp16s)(5 * i);

            index  = (Ipp16s)(index >> 3);
            i      = index & 7;
            idx[1] = (Ipp16s)(5 * i + 1);

            index  = (Ipp16s)(index >> 3);
            i      = index & 7;
            idx[2] = (Ipp16s)(5 * i + 2);

            index  = (Ipp16s)(index >> 3);
            j      = index & 1;
            index  = (Ipp16s)(index >> 1);
            i      = index & 7;
            idx[3] = (Ipp16s)(i * 5 + 3 + j);

            ippsZero_16s(ACELPcodeVec,LP_SUBFRAME_DIM);

            for(j=0; j<4; j++) {
                if((pulseSign & 1) != 0) {
                    ACELPcodeVec[idx[j]] = 8191;
                } else {
                    ACELPcodeVec[idx[j]] = -BWF_HARMONIC_E;
                }
                pulseSign = (Ipp16s)(pulseSign >> 1);
            }
            parm += 2;

            decoderObj->betaPreFilter = (Ipp16s)(decoderObj->betaPreFilter << 1);
            if(delayVal[0] < LP_SUBFRAME_DIM) {
                ippsHarmonicFilter_16s_I(decoderObj->betaPreFilter,delayVal[0],&ACELPcodeVec[delayVal[0]],LP_SUBFRAME_DIM-delayVal[0]);
            }

            pitchIndx = *parm++;
            if(!badFrameIndicator) {
                LOCAL_ARRAY(Ipp16s, gIngx, 2, decoderObj);
                ippsDotProd_16s32s_Sfs(ACELPcodeVec, ACELPcodeVec, LP_SUBFRAME_DIM, &i, 0);
                gIngx[0] = (Ipp16s)(pitchIndx >> CDBK2_BIT_NUM) ;
                gIngx[1] = (Ipp16s)(pitchIndx & (CDBK2_DIM-1));
                ippsDecodeGain_G729_16s(i, decoderObj->prevFrameQuantEn, gIngx, decoderObj->gains);
                LOCAL_ARRAY_FREE(Ipp16s, gIngx, 2, decoderObj);
            } else {
                ippsDecodeGain_G729_16s(0, decoderObj->prevFrameQuantEn, NULL, decoderObj->gains);
            }
            decoderObj->betaPreFilter = decoderObj->gains[0];
            if(decoderObj->betaPreFilter > PITCH_SHARP_MAX) decoderObj->betaPreFilter = PITCH_SHARP_MAX;
            if(decoderObj->betaPreFilter < PITCH_SHARP_MIN) decoderObj->betaPreFilter = PITCH_SHARP_MIN;
            ippsInterpolateC_NR_G729_16s_Sfs(
                                            &excitation[subfrIdx],decoderObj->gains[0],ACELPcodeVec,decoderObj->gains[1],&excitation[subfrIdx],LP_SUBFRAME_DIM,14);
            if(ippsSynthesisFilter_NR_16s_Sfs(ppAz,&excitation[subfrIdx],&synth[subfrIdx], LP_SUBFRAME_DIM, 12,
                                              ((SynthesisFilterState*)synFltw)->buffer)==ippStsOverflow) {

                ippsRShiftC_16s_I(2,prevExcitat,L_prevExcitat+LP_FRAME_DIM);
                ippsSynthesisFilter_NR_16s_Sfs(ppAz,&excitation[subfrIdx],&synth[subfrIdx],LP_SUBFRAME_DIM,12,
                                               ((SynthesisFilterState*)synFltw)->buffer);
            }
            ippsCopy_16s((&synth[subfrIdx]+LP_SUBFRAME_DIM-LPF_DIM), ((SynthesisFilterState*)synFltw)->buffer, LPF_DIM );
            ppAz += LPF_DIM+1;
        }
    }

    if(badFrameIndicator == 0) {
        ippsDotProd_16s32s_Sfs(excitation,excitation,LP_FRAME_DIM,&i,-1);
        decoderObj->SIDflag1 = Exp_32s(i);
        decoderObj->SIDflag0 = (Ipp16s)(((i << decoderObj->SIDflag1)+0x8000)>>16);
        decoderObj->SIDflag1 = (Ipp16s)(16 - decoderObj->SIDflag1);
    }
    decoderObj->CNGvar = (Ipp16s)(fType);
    ippsMove_16s(&prevExcitat[LP_FRAME_DIM], &prevExcitat[0], L_prevExcitat);
    if (decoderObj->pstFltMode) {
      pAz = FWDfiltLPC;
      for(i=0, subfrIdx = 0; subfrIdx < LP_FRAME_DIM; subfrIdx += LP_SUBFRAME_DIM,i++) {
        Post_G729AB(prevFrameDelay[i],(Ipp16s)subfrIdx,pAz,&dst[subfrIdx],(Ipp16s)fType,decoderObj);
        pAz += LPF_DIM+1;
      }
      ippsHighPassFilter_G729_16s_ISfs(dst,LP_FRAME_DIM,13,decoderObj->postProc);
    } else {
      ippsAdd_16s(synth, synth, dst, LP_FRAME_DIM);
    }
    CLEAR_SCRATCH_MEMORY(decoderObj);
    return APIG729_StsNoErr;
}

APIG729_Status G729BaseDecode
(G729Decoder_Obj* decoderObj,const Ipp8u* src, Ipp32s frametype, Ipp16s* dst) {
    LOCAL_ALIGN_ARRAY(32, Ipp16s, AzDec, (LPF_DIM+1)*2,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, newLSP,LPF_DIM,decoderObj);
    LOCAL_ALIGN_ARRAY(32, Ipp16s, ACELPcodeVec, LP_SUBFRAME_DIM,decoderObj);
    LOCAL_ARRAY(Ipp16s, idx,4,decoderObj);
    LOCAL_ARRAY(Ipp16s, delayVal,2,decoderObj);
    Ipp16s prevFrameDelay1=0, subfrVoiceFlag, *ppAz;
    Ipp8s *synFltw = decoderObj->synFltw;
    Ipp16s *prevExcitat = decoderObj->prevExcitat;
    Ipp16s *excitation = prevExcitat + L_prevExcitat;
    Ipp16s *synth = decoderObj->LTPostFilt+SYNTH_BWD_DIM;
    Ipp16s *prevSubfrLSP = decoderObj->prevSubfrLSP;
    const Ipp8u *pParm;
    const Ipp16s *parm;
    Ipp16s voiceFlag = decoderObj->voiceFlag;
    Ipp16s sidGain = decoderObj->sidGain;
    Ipp16s gainNow = decoderObj->gainNow;
    Ipp16s *lspSID = decoderObj->lspSID;
    Ipp16s temp, badFrameIndicator, badPitch, index, pulseSign, gPl, gC, fType;
    Ipp32s   i, j, subfrIdx, index2;
    IppStatus status;

    if(NULL==decoderObj || NULL==src || NULL ==dst)
        return APIG729_StsBadArgErr;
    if(decoderObj->objPrm.objSize <= 0)
        return APIG729_StsNotInitialized;
    if(DEC_KEY != decoderObj->objPrm.key)
        return APIG729_StsBadCodecType;

    delayVal[0]=delayVal[1]=0;
    pParm = src;

    if(frametype == -1) {
        decoderObj->decPrm[1] = 0;
        decoderObj->decPrm[0] = 1;
    } else if(frametype == 0) {
        decoderObj->decPrm[1] = 0;
        decoderObj->decPrm[0] = 0;

    } else if(frametype == 1) {
        decoderObj->decPrm[1] = 1;
        decoderObj->decPrm[0] = 0;
        i=0;
        decoderObj->decPrm[1+1] = ExtractBitsG729(&pParm,&i,1);
        decoderObj->decPrm[1+2] = ExtractBitsG729(&pParm,&i,5);
        decoderObj->decPrm[1+3] = ExtractBitsG729(&pParm,&i,4);
        decoderObj->decPrm[1+4] = ExtractBitsG729(&pParm,&i,5);
    } else if(frametype == 3) {
        i=0;
        decoderObj->decPrm[1] = 3;
        decoderObj->decPrm[0] = 0;
        decoderObj->decPrm[1+1] = ExtractBitsG729(&pParm,&i,1+FIR_STAGE_BITS);
        decoderObj->decPrm[1+2] = ExtractBitsG729(&pParm,&i,SEC_STAGE_BITS*2);
        decoderObj->decPrm[1+3] = ExtractBitsG729(&pParm,&i,8);
        decoderObj->decPrm[1+4] = ExtractBitsG729(&pParm,&i,1);
        decoderObj->decPrm[1+5] = ExtractBitsG729(&pParm,&i,13);
        decoderObj->decPrm[1+6] = ExtractBitsG729(&pParm,&i,4);
        decoderObj->decPrm[1+7] = ExtractBitsG729(&pParm,&i,7);
        decoderObj->decPrm[1+8] = ExtractBitsG729(&pParm,&i,5);
        decoderObj->decPrm[1+9] = ExtractBitsG729(&pParm,&i,13);
        decoderObj->decPrm[1+10] = ExtractBitsG729(&pParm,&i,4);
        decoderObj->decPrm[1+11] = ExtractBitsG729(&pParm,&i,7);
        decoderObj->decPrm[1+4] = (Ipp16s)((equality(decoderObj->decPrm[1+3])+decoderObj->decPrm[1+4]) & 0x00000001); /*  parity error (1) */
        decoderObj->codecType = G729_CODEC;
    }
    parm = decoderObj->decPrm;

    badFrameIndicator = *parm++;
    fType = *parm;
    if(badFrameIndicator == 1) {
        fType = decoderObj->CNGvar;
        if(fType == 1)
            fType = 0;
    } else {
        decoderObj->valGainAttenuation = IPP_MAX_16S;
        decoderObj->BFIcount = 0;
    }

    ippsMove_16s(&decoderObj->LTPostFilt[LP_FRAME_DIM+SYNTH_BWD_DIM-LPF_DIM], &decoderObj->LTPostFilt[SYNTH_BWD_DIM-LPF_DIM], LPF_DIM );

    if(fType < 2) {
        if(fType == 1) {
            LOCAL_ALIGN_ARRAY(32, Ipp16s, lsfq,LPF_DIM,decoderObj);
            sidGain = SIDgain[(Ipp32s)parm[4]];
            ippsLSFDecode_G729B_16s(&parm[1],(Ipp16s*)(decoderObj->prevLSPfreq),lsfq);
            ippsLSFToLSP_G729_16s(lsfq,lspSID);
            LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, lsfq,LPF_DIM,decoderObj);
        } else {
            if(decoderObj->CNGvar > 1) {
                QuantSIDGain_G729B_16s(&decoderObj->SIDflag0, &decoderObj->SIDflag1, 0, &temp, &index2);
                sidGain = SIDgain[(Ipp32s)index2];
            }
        }
        if(decoderObj->CNGvar > 1) {
            gainNow = sidGain;
        } else {
            gainNow = (Ipp16s)((gainNow * GAIN0 + BWF_HARMONIC)>>15);
            gainNow = Add_16s(gainNow, (Ipp16s)((sidGain * GAIN1 + BWF_HARMONIC)>>15));
        }

        if(gainNow == 0) {
            ippsZero_16s(excitation,LP_FRAME_DIM);
        } else {
            for(i = 0;  i < LP_FRAME_DIM; i += LP_SUBFRAME_DIM) {
                Ipp32s invSq;
                Ipp16s pG2, tmp;
                const Ipp16s *excCached;
                LOCAL_ARRAY(Ipp16s, IdxsVec, 4, decoderObj);
                LOCAL_ARRAY(Ipp16s, pulsesSignsVec, 4, decoderObj);
                LOCAL_ALIGN_ARRAY(32, Ipp16s, excg, LP_SUBFRAME_DIM, decoderObj);
                LOCAL_ARRAY(Ipp16s,tempArray,LP_SUBFRAME_DIM, decoderObj);
                RandomCodebookParm_G729B_16s(&decoderObj->seed,IdxsVec,pulsesSignsVec,&pG2,delayVal);
                ippsDecodeAdaptiveVector_G729_16s_I(delayVal,&prevExcitat[i]);
                if(decoderObj->CNGidx > CNG_STACK_SIZE-1) { /* not cached */
                    ippsRandomNoiseExcitation_G729B_16s(&decoderObj->seed,excg,LP_SUBFRAME_DIM);
                    ippsDotProd_16s32s_Sfs(excg,excg,LP_SUBFRAME_DIM,&invSq,0);
                    ippsInvSqrt_32s_I(&invSq,1);
                    excCached=excg;
                } else {
                    decoderObj->seed = cngSeedOut[decoderObj->CNGidx];
                    invSq = cngInvSqrt[decoderObj->CNGidx];
                    excCached=&cngCache[decoderObj->CNGidx][0];
                    decoderObj->CNGidx++;
                }
                NoiseExcitationFactorization_G729B_16s(excCached,invSq,gainNow,excg,LP_SUBFRAME_DIM);
                ComfortNoiseExcitation_G729B_16s_I(excg,IdxsVec,pulsesSignsVec,gainNow,pG2,&excitation[i],&tmp,tempArray);

                LOCAL_ARRAY_FREE(Ipp16s,tempArray,LP_SUBFRAME_DIM, decoderObj);
                LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, excg, LP_SUBFRAME_DIM, decoderObj);
                LOCAL_ARRAY_FREE(Ipp16s, pulsesSignsVec, 4, decoderObj);
                LOCAL_ARRAY_FREE(Ipp16s, IdxsVec, 4, decoderObj);
            }
        }
        ippsInterpolate_G729_16s(prevSubfrLSP,lspSID,prevSubfrLSP, LPF_DIM );
        ippsLSPToLPC_G729_16s(prevSubfrLSP,&AzDec[0]);
        ippsLSPToLPC_G729_16s(lspSID,&AzDec[LPF_DIM+1]);
        ippsCopy_16s(lspSID, prevSubfrLSP, LPF_DIM );
        decoderObj->sidGain = sidGain;
        decoderObj->gainNow = gainNow;

        ppAz = AzDec;
        for(subfrIdx = 0; subfrIdx < LP_FRAME_DIM; subfrIdx += LP_SUBFRAME_DIM) {
            if(ippsSynthesisFilter_NR_16s_Sfs(ppAz,&excitation[subfrIdx],&synth[subfrIdx],LP_SUBFRAME_DIM,12,
                                              ((SynthesisFilterState*)synFltw)->buffer+BWLPCF_DIM-LPF_DIM)==ippStsOverflow) {
                ippsRShiftC_16s_I(2,prevExcitat,L_prevExcitat+LP_FRAME_DIM);
                SynthesisFilterOvf_G729_16s(ppAz,&excitation[subfrIdx],&synth[subfrIdx],LP_SUBFRAME_DIM,synFltw,20);
            } else
                ippsCopy_16s((&synth[subfrIdx]+LP_SUBFRAME_DIM-LPF_DIM), ((SynthesisFilterState*)synFltw)->buffer+BWLPCF_DIM-LPF_DIM, LPF_DIM );

            ppAz += LPF_DIM+1;
        }
        decoderObj->betaPreFilter = PITCH_SHARP_MIN;
    } else {
        decoderObj->seed = SEED_INIT;
        decoderObj->CNGidx = 0;
        parm++;
        {
            LOCAL_ARRAY(Ipp16s, qIndex,4,decoderObj);

            qIndex[0] = (Ipp16s)((parm[0] >> FIR_STAGE_BITS) & 1);
            qIndex[1] = (Ipp16s)(parm[0] & (FIR_STAGE - 1));
            qIndex[2] = (Ipp16s)((parm[1] >> SEC_STAGE_BITS) & (SEC_STAGE - 1));
            qIndex[3] = (Ipp16s)(parm[1] & (SEC_STAGE - 1));
            if(!badFrameIndicator) {
                decoderObj->prevMA = qIndex[0];
                ippsLSFDecode_G729_16s( qIndex, (Ipp16s*)decoderObj->prevLSPfreq, decoderObj->prevSubfrLSPquant);
            } else {
                ippsLSFDecodeErased_G729_16s( decoderObj->prevMA,
                                              (Ipp16s*)decoderObj->prevLSPfreq, decoderObj->prevSubfrLSPquant);
            }

            ippsLSFToLSP_G729_16s(decoderObj->prevSubfrLSPquant, newLSP);
            parm += 2;
            LOCAL_ARRAY_FREE(Ipp16s, qIndex,4,decoderObj);
            ippsInterpolate_G729_16s(newLSP,prevSubfrLSP,prevSubfrLSP, LPF_DIM );
            ippsLSPToLPC_G729_16s(prevSubfrLSP, AzDec);          /* 1-st subframe */
            ippsLSPToLPC_G729_16s(newLSP, &AzDec[LPF_DIM+1]);    /* 2-nd one */
            ippsCopy_16s(newLSP, prevSubfrLSP, LPF_DIM );
            decoderObj->interpCoeff2_2 = 4506;
        }
        ppAz = AzDec;

        for(subfrIdx = 0; subfrIdx < LP_FRAME_DIM; subfrIdx += LP_SUBFRAME_DIM) {
            Ipp32s pitchIndx;
            badPitch = badFrameIndicator;
            pitchIndx = *parm++;
            if(subfrIdx == 0) {
                i = *parm++;
                badPitch = (Ipp16s)(badFrameIndicator + i);
            }
            DecodeAdaptCodebookDelays(&decoderObj->prevFrameDelay,&decoderObj->prevFrameDelay2,delayVal,subfrIdx,badPitch,pitchIndx,decoderObj->codecType);
            if(subfrIdx == 0)
                prevFrameDelay1 = delayVal[0];

            ippsDecodeAdaptiveVector_G729_16s_I(delayVal,&prevExcitat[subfrIdx]);

            if(badFrameIndicator != 0) {
                index = (Ipp16s)(Rand_16s(&decoderObj->seedSavage) & (Ipp16s)0x1fff);
                pulseSign = (Ipp16s)(Rand_16s(&decoderObj->seedSavage) & (Ipp16s)15);
            } else {
                index = parm[0]; pulseSign = parm[1];
            }

            i      = index & 7;
            idx[0] = (Ipp16s)(5 * i);

            index  = (Ipp16s)(index >> 3);
            i      = index & 7;
            idx[1] = (Ipp16s)(5 * i + 1);

            index  = (Ipp16s)(index >> 3);
            i      = index & 7;
            idx[2] = (Ipp16s)(5 * i + 2);

            index  = (Ipp16s)(index >> 3);
            j      = index & 1;
            index  = (Ipp16s)(index >> 1);
            i      = index & 7;
            idx[3] = (Ipp16s)(i * 5 + 3 + j);

            ippsZero_16s(ACELPcodeVec,LP_SUBFRAME_DIM);
            for(j=0; j<4; j++) {
                if((pulseSign & 1) != 0) {
                    ACELPcodeVec[idx[j]] = 8191;
                } else {
                    ACELPcodeVec[idx[j]] = -BWF_HARMONIC_E;
                }
                pulseSign = (Ipp16s)(pulseSign >> 1);
            }

            parm += 2;
            decoderObj->betaPreFilter = (Ipp16s)(decoderObj->betaPreFilter << 1);
            if(delayVal[0] < LP_SUBFRAME_DIM) {
                ippsHarmonicFilter_16s_I(decoderObj->betaPreFilter,delayVal[0],&ACELPcodeVec[delayVal[0]],LP_SUBFRAME_DIM-delayVal[0]);
            }

            pitchIndx = *parm++;
            if(!badFrameIndicator) {
                LOCAL_ARRAY(Ipp16s, gIngx, 2, decoderObj);
                ippsDotProd_16s32s_Sfs(ACELPcodeVec, ACELPcodeVec, LP_SUBFRAME_DIM, &i, 0); /* ACELPcodeVec energy */
                gIngx[0] = (Ipp16s)(pitchIndx >> CDBK2_BIT_NUM) ;
                gIngx[1] = (Ipp16s)(pitchIndx & (CDBK2_DIM-1));
                ippsDecodeGain_G729_16s(i, decoderObj->prevFrameQuantEn, gIngx, decoderObj->gains);
                LOCAL_ARRAY_FREE(Ipp16s, gIngx, 2, decoderObj);
            } else {
                ippsDecodeGain_G729_16s(0, decoderObj->prevFrameQuantEn, NULL, decoderObj->gains);
            }
            /* update the pitch sharpening using the quantized gain pitch */
            decoderObj->betaPreFilter = decoderObj->gains[0];
            if(decoderObj->betaPreFilter > PITCH_SHARP_MAX)
                decoderObj->betaPreFilter = PITCH_SHARP_MAX;
            if(decoderObj->betaPreFilter < PITCH_SHARP_MIN)
                decoderObj->betaPreFilter = PITCH_SHARP_MIN;

            if(badFrameIndicator) {
                decoderObj->BFIcount++;
                if(voiceFlag == 0 ) {
                    gC = decoderObj->gains[1];
                    gPl = 0;
                } else {
                    gC = 0;
                    gPl = decoderObj->gains[0];
                }
            } else {
                gC = decoderObj->gains[1];
                gPl = decoderObj->gains[0];
            }
            ippsInterpolateC_NR_G729_16s_Sfs(&excitation[subfrIdx],gPl,ACELPcodeVec,gC,&excitation[subfrIdx],LP_SUBFRAME_DIM,14);
            status = ippsSynthesisFilter_NR_16s_Sfs(ppAz,&excitation[subfrIdx],&synth[subfrIdx], LP_SUBFRAME_DIM, 12,
                                                    ((SynthesisFilterState*)synFltw)->buffer+BWLPCF_DIM-LPF_DIM);
            if(status == ippStsOverflow) {
                ippsRShiftC_16s_I(2,prevExcitat,L_prevExcitat+LP_FRAME_DIM);
                SynthesisFilterOvf_G729_16s(ppAz,&excitation[subfrIdx],&synth[subfrIdx],LP_SUBFRAME_DIM,synFltw,BWLPCF_DIM-LPF_DIM);
            } else
                ippsCopy_16s((&synth[subfrIdx]+LP_SUBFRAME_DIM-LPF_DIM), ((SynthesisFilterState*)synFltw)->buffer+BWLPCF_DIM-LPF_DIM, LPF_DIM );
            ppAz += LPF_DIM+1;
        }
    }
    if(badFrameIndicator == 0) {
        ippsDotProd_16s32s_Sfs(excitation,excitation,LP_FRAME_DIM,&i,-1);
        decoderObj->SIDflag1 = Exp_32s(i);
        decoderObj->SIDflag0 = (Ipp16s)(((i << decoderObj->SIDflag1)+0x8000)>>16);
        decoderObj->SIDflag1 = (Ipp16s)(16 - decoderObj->SIDflag1);
    }
    decoderObj->CNGvar = fType;
    ippsMove_16s(&prevExcitat[LP_FRAME_DIM], &prevExcitat[0], L_prevExcitat);
    decoderObj->voiceFlag = 0;
    decoderObj->gammaPost1 = BWF1_PST;
    decoderObj->gammaPost2 = BWF2_PST;
    decoderObj->gammaHarm = BWF_HARMONIC;
    if (decoderObj->pstFltMode) {
      for(subfrIdx=0; subfrIdx<LP_FRAME_DIM; subfrIdx+=LP_SUBFRAME_DIM) {
          Post_G729Base(prevFrameDelay1, (Ipp16s)subfrIdx, AzDec, &dst[subfrIdx],
                        &subfrVoiceFlag, fType, decoderObj);
          if(subfrVoiceFlag != 0) decoderObj->voiceFlag = subfrVoiceFlag;
          AzDec += LPF_DIM+1;
      }
      ippsHighPassFilter_G729_16s_ISfs(dst,LP_FRAME_DIM,13,decoderObj->postProc);
    } else {
      ippsAdd_16s(synth, synth, dst, LP_FRAME_DIM);
    }
    CLEAR_SCRATCH_MEMORY(decoderObj);
    return APIG729_StsNoErr;
}
