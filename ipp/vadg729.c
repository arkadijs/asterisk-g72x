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
// Purpose: G.729 VAD functions.
//
*/

#include "vadg729.h"
#include "aux_fnxs.h"

#define     ZEROcrossBegin  120
#define     VADinitFrame    32
#define     ZEROcrossEnd    200

static __ALIGN32 CONST Ipp16s lbfCorr[VAD_LPC_DIM +1] = { 7869, 7011, 4838, 2299, 321,
    -660, -782, -484, -164, 3, 39, 21, 4};
static __ALIGN32 CONST Ipp16s ifactor[33] = {  IPP_MAX_16S, 16913, 17476, 18079,
    18725, 19418, 20165, 20972, 21845, 22795, 23831, 24966,
    26214, 27594,
    29127, 30840, IPP_MAX_16S, 17476, 18725, 20165, 21845, 23831, 26214, 29127,
    IPP_MAX_16S, 18725, 21845, 26214, IPP_MAX_16S, 21845, IPP_MAX_16S, IPP_MAX_16S, 0
};
static __ALIGN32 CONST Ipp16s ishift[33] = { 15, (7<<1), (7<<1), (7<<1), (7<<1), (7<<1), (7<<1), (7<<1),
    (7<<1), (7<<1), (7<<1), (7<<1), (7<<1), (7<<1), (7<<1), (7<<1), (7<<1), 13, 13, 13, 13, 13, 13, 13, 13, 12, 12,
    12, 12, 11, 11, 10, 15
};

static Ipp32s ownSignChangeRate(Ipp16s *pSrc /* [ZEROcrossEnd-ZEROcrossBegin+1] */) {
    Ipp32s i;
    Ipp32s sum=0;
    for(i=1; i<=ZEROcrossEnd-ZEROcrossBegin; i++)
        sum += (pSrc[i-1] * pSrc[i] < 0);
    return sum;
}


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ownMakeDecision
//  Purpose:
//  Parameters:
//  lowBandE       - energy
//  highBandE      - fullenergy
//  spectrDist     - distortion
//  crossZeroRate  - zero crossing
*/
static Ipp16s ownMakeDecision(Ipp16s lowBandE,Ipp16s highBandE, Ipp16s spectrDist,Ipp16s crossZeroRate) {
    Ipp32s L_tmp;
    /* spectrDist vs crossZeroRate */
    L_tmp = crossZeroRate * (-14680);
    L_tmp += BWF_HARMONIC_E * (-28521);
    L_tmp = L_tmp >> 7;
    if(L_tmp > -spectrDist<<16 )
        return 1;
    L_tmp = crossZeroRate * 19065;
    L_tmp += BWF_HARMONIC_E * (-19446);
    L_tmp = L_tmp>>6;
    if(L_tmp > -spectrDist<<16)
        return 1;
    /* highBandE vs crossZeroRate */
    L_tmp = crossZeroRate * 20480;
    L_tmp += BWF_HARMONIC_E * BWF_HARMONIC;
    L_tmp = L_tmp>>1;
    if(L_tmp < -highBandE<<16)
        return 1;
    L_tmp = crossZeroRate * (-BWF_HARMONIC);
    L_tmp += BWF_HARMONIC_E * 19660;
    L_tmp = L_tmp>>1;
    if(L_tmp < -highBandE<<16)
        return 1;
    L_tmp = highBandE * IPP_MAX_16S;
    if(L_tmp < -1024 * 30802)
        return 1;
    /* highBandE vs spectrDist */
    L_tmp = spectrDist * (-28160);
    L_tmp += 64 * 19988;
    if(L_tmp < -highBandE<<9)
        return 1;
    L_tmp = spectrDist * IPP_MAX_16S;
    if(L_tmp > 32 * 30199)
        return 1;
    /* lowBandE vs crossZeroRate */
    L_tmp = crossZeroRate * (-20480);
    L_tmp += BWF_HARMONIC_E * BWF2;
    L_tmp = L_tmp >> 1;
    if(L_tmp < -highBandE<<16)
        return 1;
    L_tmp = crossZeroRate * 23831;
    L_tmp += (1<<12) * 31576;
    L_tmp = L_tmp>>1;
    if(L_tmp < -highBandE<<16)
        return 1;
    L_tmp = highBandE * IPP_MAX_16S;
    if(L_tmp < -(1<<11) * 17367
        ) return 1;
    /* lowBandE vs spectrDist */
    L_tmp = spectrDist * (-22400);
    L_tmp += (1<<5) * 25395;
    if(L_tmp < -lowBandE<<8)
        return 1;
    /* lowBandE vs highBandE */
    L_tmp = highBandE * (-30427);
    L_tmp += (1<<8) * (-29959);
    if(L_tmp > -lowBandE<<15)
        return 1;
    L_tmp = highBandE * (-23406);
    L_tmp += (1<<9) * 28087;
    if(L_tmp < -lowBandE<<15)
        return 1;
    L_tmp = highBandE * 24576;
    L_tmp += (1<<10) * COEFF1;
    if(L_tmp < -lowBandE<<14)
        return 1;
    return 0;
}

void VoiceActivityDetectInit_G729(Ipp8s* pVADmem) {
    VADmemory_Obj* vadMem =  (VADmemory_Obj*)pVADmem;
    ippsZero_16s(vadMem->LSFMean, LPF_DIM);
    ippsZero_16s(vadMem->minBuf, 16);
    vadMem->SEMean = 0;
    vadMem->SLEMean = 0;
    vadMem->EMean = 0;
    vadMem->SZCMean = 0;
    vadMem->SILcounter = 0;
    vadMem->updateCounter = 0;
    vadMem->extCounter = 0;
    vadMem->lessCounter = 0;
    vadMem->frameCounter = 0;
    vadMem->VADflag = 1;
    vadMem->minVAD = IPP_MAX_16S;
    vadMem->VADPrev  = 1;
    vadMem->VADPPrev = 1;
    vadMem->minPrev = IPP_MAX_16S;
    vadMem->minNext = IPP_MAX_16S;
    vadMem->VADPrevEnergy = IPP_MAX_16S;
    vadMem->VADflag2 = 0;
    ippsZero_16s(vadMem->musicRC, 10);
    vadMem->musicCounter=0;
    vadMem->musicMCounter=0;
    vadMem->conscCounter=0;
    vadMem->MeanPgain =BWF_HARMONIC_E;
    vadMem->count_pflag=0;
    vadMem->Mcount_pflag=0;
    vadMem->conscCounterFlagP=0;
    vadMem->conscCounterFlagR=0;
    vadMem->musicSEMean =0;
}

void VoiceActivityDetectSize_G729(Ipp32s* pVADsize) {
    //pVADsize[0] = (sizeof(VADmemory)+7)&(~7);
pVADsize[0] = (sizeof(VADmemory_Obj)+32);
}

static __ALIGN32 CONST Ipp16s vadTable[6][6]={
    {24576, BWF_HARMONIC_E,26214, 6554,19661,PITCH_SHARP_MAX},
    {31130, 1638,30147, 2621,21299,11469},
    {31785,  983,30802, 1966,BWF2, 9830},
    {32440,  328,31457, 1311,24576, BWF_HARMONIC_E},
    {32604,  164,32440,  328,24576, BWF_HARMONIC_E},
    {32604,  164,32702,   66,24576, BWF_HARMONIC_E},
};

void VoiceActivityDetect_G729(Ipp16s *pSrc,Ipp16s *pLSF,Ipp32s *pAutoCorr,
                              Ipp16s autoExp, Ipp16s pRc, Ipp16s *pVad, Ipp8s *pVADmem,Ipp16s *pTmp) {
    Ipp32s L_tmp;
    VADmemory_Obj *vadMem =  (VADmemory_Obj*)pVADmem;
    const Ipp16s *pVadTable;

    Ipp16s frameCounter, VADPrev, VADPPrev, *LSFMean, *min_buf;
    Ipp16s vadmin,SEMean,minPrev=IPP_MAX_16S,minNext,lessCounter,updateCounter;
    Ipp16s SZCMean,SLEMean,EMean,prev_vadEner,SILcounter,v_flag,cnt_ext,cur_flag;
    Ipp16s i, j, exp, fractal;
    Ipp16s energy, energyLow, spectrDist, ZC, highBandE, lowBandE, crossZeroRate;
    Ipp32s   r0;

    r0 = pAutoCorr[0];
    for(i=0; i<= VAD_LPC_DIM ; i++)
        pTmp[i]=(Ipp16s)(pAutoCorr[i]>>16);

    VADPrev = vadMem->VADPrev;
    VADPPrev = vadMem->VADPPrev;
    LSFMean = vadMem->LSFMean;
    min_buf = vadMem->minBuf;
    vadmin = vadMem->minVAD;
    SEMean = vadMem->SEMean;
    minPrev = vadMem->minPrev;
    minNext = vadMem->minNext;
    lessCounter = vadMem->lessCounter;
    updateCounter = vadMem->updateCounter;
    SZCMean = vadMem->SZCMean;
    SLEMean = vadMem->SLEMean;
    EMean = vadMem->EMean;
    prev_vadEner = vadMem->VADPrevEnergy;
    SILcounter = vadMem->SILcounter;
    v_flag = vadMem->VADflag2;
    cnt_ext = vadMem->extCounter;
    cur_flag = vadMem->VADflag;
    frameCounter = vadMem->frameCounter;
    if(frameCounter == IPP_MAX_16S) frameCounter = 256;
    else frameCounter++;

    /* get frame energy */
    Log2_G729(r0, &exp, &fractal);
    L_tmp = (autoExp+exp)*9864 + ((fractal*9864)>>15);
    energy = (Ipp16s)(L_tmp>>4);
    energy -= 6108;

    /* get low band energy */
    ippsDotProd_16s32s_Sfs(pTmp+1,lbfCorr+1,VAD_LPC_DIM ,&L_tmp,0);
    L_tmp = 4 * L_tmp;
    L_tmp = Add_32s(L_tmp, 2 * pTmp[0] * lbfCorr[0]);
    Log2_G729(L_tmp, &exp, &fractal);
    L_tmp = (autoExp+exp)*9864 + ((fractal*9864)>>15);
    energyLow = (Ipp16s)(L_tmp>>4);
    energyLow -= 6108;

    /* calculate spectrDist */
    for(i=0, L_tmp=0; i<LPF_DIM; i++) {
        j = (Ipp16s)(pLSF[i] - LSFMean[i]);
        L_tmp += j * j;
    }
    spectrDist = (Ipp16s)(L_tmp >> 15);

    /* compute # zero crossing */
   ZC=(Ipp16s)(ownSignChangeRate(pSrc+ZEROcrossBegin)*410);


    if(frameCounter < 129) {
        if(energy < vadmin) {
            vadmin = energy;
            minPrev = energy;
        }

        if(0 == (frameCounter & 7)) {
            i = (Ipp16s)((frameCounter>>3) - 1);
            min_buf[i] = vadmin;
            vadmin = IPP_MAX_16S;
        }
    }

    if(0 == (frameCounter & 7)) {
        ippsMin_16s(min_buf,16,&minPrev);
    }

    if(frameCounter >= 129) {
        if((frameCounter & 7) == 1) {
            vadmin = minPrev;
            minNext = IPP_MAX_16S;
        }
        if(energy < vadmin)
            vadmin = energy;
        if(energy < minNext)
            minNext = energy;

        if(!(frameCounter & 7)) {
            for(i=0; i<15; i++)
                min_buf[i] = min_buf[i+1];
            min_buf[15] = minNext;
            ippsMin_16s(min_buf,16,&minPrev);
        }

    }

    if(frameCounter <= VADinitFrame) {
        if(energy < 3072) {
            pVad[0] = 0;
            lessCounter++;
        } else {
            pVad[0] = 1;
            EMean = (Ipp16s)(EMean + (energy>>5));
            SZCMean = (Ipp16s)(SZCMean + (ZC>>5));
            for(i=0; i<LPF_DIM; i++) {
                LSFMean[i] = (Ipp16s)(LSFMean[i] + (pLSF[i] >> 5));
            }
        }
    }

    if(frameCounter >= VADinitFrame) {
        if(VADinitFrame==frameCounter) {
            L_tmp = EMean * ifactor[lessCounter];
            EMean = (Ipp16s)(L_tmp>>ishift[lessCounter]);

            L_tmp = SZCMean * ifactor[lessCounter];
            SZCMean = (Ipp16s)(L_tmp >> ishift[lessCounter]);

            for(i=0; i<LPF_DIM; i++) {
                L_tmp = LSFMean[i] * ifactor[lessCounter];
                LSFMean[i] = (Ipp16s)(L_tmp >> ishift[lessCounter]);
            }
            SEMean = (Ipp16s)(EMean - 2048);
            SLEMean = (Ipp16s)(EMean - 2458);
        }

        highBandE = (Ipp16s)(SEMean - energy);
        lowBandE = (Ipp16s)(SLEMean - energyLow);
        crossZeroRate = (Ipp16s)(SZCMean - ZC);

        if(energy < 3072)
            pVad[0] = 0;
        else
            pVad[0] = ownMakeDecision(lowBandE, highBandE, spectrDist, crossZeroRate);

        v_flag = 0;
        if((VADPrev==1) && (pVad[0]==0) && (highBandE < -410) && (energy > 3072)) {
            pVad[0] = 1;
            v_flag = 1;
        }

        if(cur_flag == 1) {
            if((VADPPrev == 1) && (VADPrev == 1) && (pVad[0] == 0) &&
               Abs_16s((Ipp16s)(prev_vadEner - energy)) <= 614) {
                cnt_ext++;
                pVad[0] = 1;
                v_flag = 1;
                if(cnt_ext <= 4)
                    cur_flag = 1;
                else {
                    cnt_ext = cur_flag = 0;
                }
            }
        } else
            cur_flag=1;

        if(pVad[0] == 0)
            SILcounter++;

        if((pVad[0] == 1) && (SILcounter > 10)
           && (energy - prev_vadEner <= 614)) {
            pVad[0] = 0;
            SILcounter=0;
        }

        if(pVad[0] == 1)
            SILcounter=0;

        if(((energy - 614) < SEMean) && (frameCounter > 128) && (!v_flag) && (pRc < 19661))
            pVad[0] = 0;

        if(((energy - 614) < SEMean) && (pRc < 24576) && (spectrDist < 83)) {
            updateCounter++;
            if(updateCounter < 20)
                pVadTable = vadTable[0];
            else
                if(updateCounter < 30)
                    pVadTable = vadTable[1];
                else
                    if(updateCounter < 40)
                        pVadTable = vadTable[2];
                    else
                        if(updateCounter < 50)
                            pVadTable = vadTable[3];
                        else
                            if(updateCounter < 60)
                                pVadTable = vadTable[4];
                            else
                                pVadTable = vadTable[5];
            /* update means */
            L_tmp = pVadTable[0] * SEMean + pVadTable[1] * energy;
            SEMean = (Ipp16s)(L_tmp >> 15);

            L_tmp = pVadTable[0] * SLEMean + pVadTable[1] * energyLow;
            SLEMean = (Ipp16s)(L_tmp>>15);

            L_tmp = pVadTable[2] * SZCMean + pVadTable[3] * ZC;
            SZCMean = (Ipp16s)(L_tmp>>15);

            for(i=0; i<LPF_DIM; i++) {
                L_tmp = pVadTable[4] * LSFMean[i] + pVadTable[5] * pLSF[i];
                LSFMean[i] = (Ipp16s)(L_tmp>>15);
            }
        }
        if(frameCounter > 128 && ((SEMean < vadmin &&
           spectrDist < 83) || (SEMean -vadmin) > 2048)) {
            SEMean = vadmin;
            updateCounter = 0;
        }
    }
    vadMem->VADPrevEnergy = energy;
    vadMem->minVAD = vadmin;
    vadMem->SEMean = SEMean;
    vadMem->minPrev = minPrev;
    vadMem->minNext = minNext;
    vadMem->lessCounter = lessCounter;
    vadMem->updateCounter = updateCounter;
    vadMem->SZCMean = SZCMean;
    vadMem->SLEMean = SLEMean;
    vadMem->EMean = EMean;
    vadMem->SILcounter = SILcounter;
    vadMem->VADflag2 = v_flag;
    vadMem->extCounter = cnt_ext;
    vadMem->VADflag = cur_flag;
    vadMem->frameCounter = frameCounter;
}


