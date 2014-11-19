/*/////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2012 Intel Corporation. All Rights Reserved.
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
// Purpose: G.729/A/B/D/E speech codec: Miscellaneous.
//
*/

#include <ipps.h>
#include "owng729.h"
#include "vadg729.h"
#include <ippdefs.h>

#if defined(__ICL ) && defined(_IPP_A6)
    #include <xmmintrin.h>
#endif

#define     ZEROcrossBegin  120
#define     VADinitFrame    32
#define     ZEROcrossEnd    200
static __ALIGN32 CONST Ipp16s t[GAIN_NUM+1] = {820,26<<1,26};
static __ALIGN32 CONST Ipp16s t1[GAIN_NUM+1] = {0,0,1};
static __ALIGN32 CONST Ipp16s energyLogTable[17] = { 2000+466, 2000+520, 2000+571, 2000+619, 2000+665, 2000+708, 2000+749,
    2789, 2000+827, 2000+863, 2000+898, 2000+931, 2000+964, 2000+995, 3000+25, 3000+54, 3000+83};
static __ALIGN32 CONST Ipp32s wlag_bwd[BWLPCF_DIM]={
    2100000000+47252352,   2100000000+47202688,   2100000000+47120000,   2100000000+47004032,   2100000000+46855040,
    2100000000+46673024,   2100000000+46457728,   2100000000+46209536,   2100000000+45928192,   2100000000+45613952,
    2100000000+45266432,   2100000000+44886016,   2100000000+44472704,   2100000000+44026240,   2100000000+43546880,
    2100000000+43034624,   2100000000+42489344,   2100000000+41911296,   2100000000+41300224,   2100000000+40656512,
    2100000000+39979776,   2100000000+39270400,   2100000000+38528128,   2100000000+37753344,   2100000000+36945920,
    2100000000+36105600,   2100000000+35232896,   2100000000+34327552,   2100000000+33389696,   2100000000+32419328
};
__ALIGN32 CONST Ipp16s presetOldA[LPF_DIM+1]={ (1<<12), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

__ALIGN32 CONST Ipp16s presetLSP[LPF_DIM]={ 3*10000, 26*1000,21*1000, 15*1000, 8*1000, 0*1000,
    -8*1000,-15*1000,-21*1000,-26*1000};

__ALIGN32 CONST Ipp16s resetPrevLSP[LPF_DIM] = { 2000+339, 4000+679,7000+18, 9000+358, 11000+698,
    14000+37, 16000+377,18000+717, 21000+56, 23000+396};
__ALIGN32 CONST Ipp16s SIDgain[32] = {
    (1<<1), (1<<2)+1,  (1<<3), (1<<3)+5,
    (1<<4)+(1<<2), (1<<5), 5*10, (1<<6),
    8*10,  100+1,  100+27,  100+60,
    200+01,  200+53,  300+18,  400+1,
    500+5,  600+35,  800, 1000+7,
    1000+268, 1000+596, 2000+10, 2000+530,
    3000+185, 4000+9, 5000+48, 6000+355,
    8*1000,10000+71,12000+679,15000+962
};
__ALIGN32 CONST Ipp16s areas[L_prevExcitat-1+3] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0
};
__ALIGN32 CONST Ipp16s LUT1[CDBK1_DIM]={
        5,
        1,
        4,
        7,
        3,
        0,
        6,
        2
};
__ALIGN32 CONST Ipp16s LUT2[CDBK2_DIM]={
        4,
        6,
        0,
        2,
        12,
        14,
        8,
        10,
        15,
        11,
        9,
        13,
        7,
        3,
        1,
        5
};

static __ALIGN32 CONST Ipp16s logTable[33]={
    0,1455,2866,4236,5568,6863,
8124,9352,10549,11716,12855,13967,
15054,16117,17156,18172,19167,20142,
21097,22033,22951,23852,24735,25603,
26455,27291,28113,28922,29716,30497,
31266,32023, IPP_MAX_16S};

__ALIGN32 CONST Ipp16s cngCache[CNG_STACK_SIZE][LP_SUBFRAME_DIM]={
    {/* 0 */
86,-609,391,173,-80,-718,-47,559,235,676,
-4,-110,5,-521,-506,-300,256,-727,-527,-9,
-38,60,-69,-263,661,-722,-154,1500,303,-511,
-272,-866,938,179,91,833,4,-698,933,-573*1
    },
    {/* 1 */
225,547,-292,-470,302,264,728,-578,-294,332,
1589,-845,487,241,241,775,83,-523,-242,188,
-992,602,650,-48,332,542,-153,581,-39,323,
931,25,-59,-569,-192,-178,-238,-596,60,482*1
},
{/*2*/
-57,-821,254,-1,365,-278,21,-371,-12,-536,
11,1020,-166,455,-798,74,-95,133,152,-647,
199,-476,-209,-633,-306,-350,-859,118,436,-511,
-260,68,379,63,50,755,33,-674,585,-383*1
},
{/*3*/
1140,589,364,-527,-516,-83,-220,1154,-27,878,
-707,-1166,557,-626,437,-317,-810,15,654,115,
-34,-273,-570,-380,-185,-464,863,-268,271,464,
-169,-571,312,-47,-592,-779,-64,49,616,646*1
},
{/*4*/
259,-1109,-623,852,406,757,-493,911,1032,-480,
630,937,-421,-214,184,-92,-883,-491,-293,382,
-864,735,221,-1223,-878,394,192,723,101,-1004,
130,-434,535,127,549,-88,-86,201,935,739*1
},
{/*5*/
-937,-304,-546,-350,572,460,626,-690,-128,-471,
103,347,549,-539,443,199,41,1282,-400,404,
-114,-642,644,-576,1106,-677,505,333,119,151,
-307,-455,-517,-204,-254,-378,-800,-208,150,-462*1
},
{/*6*/
-497,770,-829,-271,-722,281,1105,-392,-212,15,
192,-290,523,486,-1010,37,-55,-356,1085,75,
-408,51,331,338,486,169,1338,312,-443,512,
-1016,-1,387,-970,-73,-91,-94,-691,-443,44*1
},
{/*7*/
993,-320,-251,208,577,376,150,-71,-254,657,
-890,-255,-478,9,215,-342,-93,482,391,-334,
387,-486,152,-229,452,179,8,-29,100,-1109,
-39,780,-155,-252,9,-364,709,188,-358,-387*1
},
{/*8*/
-49,-395,248,506,25,-9,-461,-148,68,-166,
-179,-BW_EXP_FACT ,-434,156,72,1009,-455,-65,-1246,772,
-509,-319,477,-1034,942,420,-393,201,-200,102,
242,-133,-1885,777,-692,-498,-531,-117,-121,128*1
},
{/*9*/
-828,402,-490,-144,1216,806,-62,-588,-483,539,
-305,345,217,111,315,-419,-266,-476,-760,194,
-910,-200,52,646,-177,429,705,-597,-629,-127,
685,47,-216,182,-518,20,-476,-182,-858,344*1
},
{/*10*/
-250,903,-395,-654,-483,-491,-260,-400,-493,-636,
-414,79,-278,-46,1193,781,160,-255,-561,172,
311,-753,470,-214,173,1022,-324,-375,772,-52,
131,711,1082,635,297,486,81,521,688,484*1
},
{/*11*/
-492,537,420,213,-1076,170,-115,-362,485,-102,
469,-330,605,235,126,-201,-202,667,-122,-1001,
-914,-340,-273,-168,-505,284,-329,-264,-1,1005,
-288,761,-424,-227,360,-679,272,-338,-431,26*1
},
{/*12*/
345,156,94,317,-549,-809,208,-1432,574,-783,
803,-654,640,748,341,1117,-349,-314,360,807,
165,-383,-1190,-558,648,-485,-211,604,-950,-106,
223,-313,493,-272,-910,273,-145,43,-28,316*1
},
{/*13*/
838,-630,164,-588,-38,566,-24,-495,-49,67,
653,-563,291,431,145,234,984,636,-10,358,
492,-856,-326,835,-159,53,-289,-897,-459,-223,
-925,-228,-406,-146,-696,-232,-2,-230,396,117*1
},
{/*14*/
75,505,-74,-735,473,-128,-588,23,584,486,
-365,494,406,-211,81,-348,-571,339,240,571,
723,602,-402,-847,-830,-448,205,1032,-112,263,
-1013,62,-193,685,554,-683,973,-1228,808,843*1
},
{/*15*/
301,-392,-151,1056,702,-671,-470,-199,174,-343,
-694,-847,-769,82,-821,138,-593,602,683,-318,
192,195,748,-677,48,395,-116,-429,-511,-332,
-371,-84,-463,-452,493,356,-830,1061,-594,381*1
},
{/*16*/
64,-830,609,447,379,-460,1164,-222,149,-633,
-362,100,-624,-327,129,391,106,-52,75,-375,
-219,190,1014,-148,447,-623,-128,1070,570,67,
747,-815,-364,-298,-459,-1197,207,328,352,-74*1
},
{/*17*/
-258,-552,-344,654,-342,540,517,-125,439,-63,
-319,-41,-477,-315,222,-628,-528,810,-422,-352,
-228,-274,311,277,425,-493,-141,745,-107,151,
-240,30,226,636,524,690,-114,-64,105,167*1
},
{/*18*/
381,506,-475,-1123,4,-264,539,780,362,215,
242,346,433,-619,-344,137,215,-204,-705,-361,
-290,-1102,-331,-634,964,-239,268,-172,-629,335,
-446,515,-463,622,89,-111,-584,620,335,0*1
},
{/*19*/
-692,-1027,628,210,309,446,-371,450,-643,478,
261,-237,550,607,-1058,707,302,319,-234,-300,
-361,128,-337,-188,-417,32,679,-491,136,545,
256,837,272,129,-72,726,-519,-190,-303,-314*1
},
{/*20*/
-1242,-123,-605,31,408,-336,-1018,57,491,442,
-440,-460,29,165,107,14,48,-145,621,-55,
34,26,80,-157,-11,-348,-494,222,-89,-241,
-75,-456,-198,-167,311,-141,-340,-124,154,141*1
},
{/*21*/
780,-100,322,286,592,505,-201,-213,-779,437,
139,-873,-263,-302,-192,356,94,846,-172,-112,
-222,811,201,285,327,-409,-99,9,203,-1276,
-46,86,944,768,870,2,-12,605,-421,271*1
},
{/*22*/
-962,-727,353,-376,-452,-993,-48,238,-229,502,
286,53,217,684,-179,-421,1400,67,-421,-158,
-266,697,74,840,341,528,-232,15,-365,-442,
-313,-587,690,-163,341,60,-80,-173,196,-94*1
},
{/*23*/
145,200,230,265,338,-543,726,81,1138,-167,
295,-518,-13,-206,-553,2,-557,-1174,-792,-405,
-492,515,88,-1229,-332,763,-983,92,949,-428,
89,484,-235,12,234,461,214,549,-38,533*1
},
{/*24*/
-343,-121,-317,252,724,234,-35,591,734,556,
-296,385,-311,335,434,148,-365,81,-403,390,
-450,308,-1273,89,-568,-10,898,267,307,-359,
-36,-98,637,-229,1047,-496,424,-130,561,-413*1
},
{/*25*/
144,-10,402,-892,-531,235,-353,-471,169,-705,
267,-211,-314,244,216,-110,66,8,1028,-170,
-225,125,145,-389,-677,-431,-899,255,760,-122,
-54,740,-12,1050,630,40,-943,-497,131,-307*1
},
{/*26*/
557,805,-1268,-640,-477,660,-910,351,762,738,
-839,29,-336,17,-33,-68,839,-993,998,-966,
189,-242,-308,409,-236,220,147,471,74,-117,
-199,246,97,-231,-832,756,343,-1144,294,467*1
},
{/*27*/
284,762,-803,-282,309,-22,293,262,429,827,
462,-121,-890,-1301,214,103,-1090,252,-961,424,
-681,-148,7,328,335,-451,-464,-183,-600,365,
184,-87,-416,254,417,106,-135,-786,233,-118*1
},
{/*28*/
-149,359,96,-264,974,-639,179,2,16,380,
229,237,563,342,-776,-73,53,-239,-278,98,
24,171,701,-276,-38,-474,-401,343,-131,-128,
-1022,-95,255,-837,-140,-52,610,-555,199,-42*1
},
{/*29*/
682,6,184,-31,671,19,-675,413,-525,-129,
353,-326,-856,-436,198,822,188,-392,394,787,
49,517,-593,591,-251,-783,307,236,826,-194,
536,-280,206,746,-932,69,451,-521,1,257*1
},
{/*30*/
233,912,-316,-473,856,-10,419,510,-346,-707,
-159,-1356,-299,355,0,75,-541,106,-130,193,
-46,-432,-547,24,160,-233,283,590,-433,-835,
-1222,869,733,836,568,347,-436,-342,535,-462*1
},
{/*31*/
-754,1201,-390,137,254,505,-102,-511,-689,418,
-229,474,-1,938,251,-494,-752,19,-196,171,
-384,-805,-36,419,81,-508,221,252,-383,-116,
61,-332,274,373,510,-819,-510,-67,542,325*1
},
};
__ALIGN32 CONST Ipp16s cngSeedOut[CNG_STACK_SIZE]={
    -26766,    5273,  -20980,   12987,   20566,    2637,  -17200,  -25649,
    24058,  -31807,   27220,    6819,  -31650,  -27403,   23192,    9015,
    18306,    4585,   -3684,   -5749,   -5274,  -29027,   21344,   24991,
    29706,   24337,   25572,   16243,  -23186,  -26811,  -14552,   -2297
};
__ALIGN32 CONST Ipp32s cngInvSqrt[CNG_STACK_SIZE]={
    320863, 323781, 388729, 303149, 269026, 324406, 304813, 391279,
    300993, 335038, 312683, 356481, 286752, 349341, 292467, 315662,
    323859, 418904, 340163, 350720, 452932, 342818, 349410, 315175,
    357434, 352913, 286583, 332201, 413044, 346627, 309243, 358885
};
__ALIGN32 CONST Ipp16s gammaFac1[2*(LPF_DIM+1)]={
IPP_MAX_16S,24000+576,18000+432,13000+824,10000+368,7000+776,5000+832,4000+374,3000+281,2000+461,1000+846,
IPP_MAX_16S,24000+576,18000+432,13000+824,10000+368,7000+776,5000+832,4000+374,3000+281,2000+461,1000+846
};
__ALIGN32 CONST Ipp16s g729gammaFac2_pst[LPF_DIM+1] = {
    IPP_MAX_16S,  BWF2_PST,  9000+912,  5000+451,  2000+998, 1000+649,  900+7,  400+99,  200+74,  100+51,   83
};
__ALIGN32 CONST Ipp16s g729gammaFac1_pst[LPF_DIM+1] = {
    IPP_MAX_16S,  BWF2, 16000+57, 11200+40,  7800+68,  5500+8, 3800+56, 2600+99, 1800+89, 1300+22,  900+25
};

static __ALIGN32 CONST Ipp16s gStats[6] ={400<<4,400<<3,400*6,400<<2,400<<1,400};
static __ALIGN32 CONST Ipp16s gThrs[6]  ={955,819,614,410,205,0};
static __ALIGN32 CONST Ipp16s gStats1[6]={400<<4,400<<3,400<<2,400<<1,400};

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:        updateExcErr_G729
//  Purpose:     update excitation error
//  Parameters:
//  pSrcDst - coder error
*/
void updateExcErr_G729( Ipp16s val, Ipp32s indx, Ipp32s *pSrcDst) {
    Ipp32s   i, area1, area2, L_tmp, L_tmp1=0, tmp = 0;
    Ipp16s high, low;
    L_tmp = -1;
    if(indx < LP_SUBFRAME_DIM) {
        high   = (Ipp16s)(pSrcDst[0] >> 16);
        low    = (Ipp16s)((pSrcDst[0]>>1) & IPP_MAX_16S);
        tmp    = high*val + ((low*val)>>15);
        tmp    = (tmp << 2) + (Ipp32s)BWF_HARMONIC;
        L_tmp  = IPP_MAX(L_tmp,tmp);
        high   = (Ipp16s)(tmp >> 16);
        low    = (Ipp16s)((tmp>>1) & IPP_MAX_16S);
        L_tmp1 = high*val + ((low*val)>>15);
        L_tmp1 = ShiftL_32s(L_tmp1, 2);
        L_tmp1 = Add_32s(L_tmp1, (Ipp32s)BWF_HARMONIC);
        L_tmp  = IPP_MAX(L_tmp,L_tmp1);
    } else {
        area1 = areas[indx-LP_SUBFRAME_DIM];
        area2 = areas[indx-1];
        for(i = area1; i <= area2; i++) {
            high   = (Ipp16s)(pSrcDst[i] >> 16);
            low    = (Ipp16s)((pSrcDst[i]>>1) & IPP_MAX_16S);
            L_tmp1 = high*val + ((low*val)>>15);
            L_tmp1 = (L_tmp1 << 2) + (Ipp32s)BWF_HARMONIC;
            L_tmp  = IPP_MAX(L_tmp,L_tmp1);
        }
    }
    for(i=3; i>=1; i--) {
        pSrcDst[i] = pSrcDst[i-1];
    }
    pSrcDst[0] = L_tmp;
    return;
}
Ipp16s calcErr_G729(Ipp32s val, Ipp32s *pSrc) {
    Ipp16s i, area1;
    area1 = areas[IPP_MAX(0, val - LP_SUBFRAME_DIM - 10)];
    for(i=areas[val + 10 - 2]; i>=area1; i--) {
        if(pSrc[i] > 983040000)
            return 1; /* Error threshold improving BWF_HARMONIC. * 60000.   */
    }
    return 0;
}
/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:    Log2_G729
//  Purpose: update excitation error
//  Parameters:
//   pDst1   - log2 integer part.    0<=val<=30
//   pDst2   - log2 fractional part. 0<=val<1
//   pSrcDst - coder error
*/
void Log2_G729(Ipp32s val, Ipp16s *pDst1, Ipp16s *pDst2) {
    Ipp16s order, i, j;
    Ipp32s   L_tmp;
    if( val <= 0 ) {
        pDst1[0] = pDst2[0] = 0;
        return;
    }
    order    = Exp_32s(val);
    val    <<= order;
    pDst1[0] = (Ipp16s)(30 - order);
    i        = (Ipp16s)(val>>25);
    j        = (Ipp16s)((val >> 10) & IPP_MAX_16S);
    i       -= 32;
    L_tmp    = logTable[i]<<15;
    L_tmp   -= (logTable[i] - logTable[i+1]) * j;
    pDst2[0] = (Ipp16s)(L_tmp >> 15);
    return;
}
void NoiseExcitationFactorization_G729B_16s(const Ipp16s *pSrc,
                                            Ipp32s val1, Ipp16s val2, Ipp16s *pDst, Ipp32s len) {
    Ipp32s   order, L_tmp;
    Ipp16s high, low, tmp1;

    high = (Ipp16s)(val1>>16);
    low = (Ipp16s)((val1>>1)&IPP_MAX_16S);
    tmp1 = (Ipp16s)(((val2 * 19043)+BWF_HARMONIC)>>15);
    tmp1 = (Ipp16s)(tmp1 + val2);
    L_tmp = high*tmp1 + ((low*tmp1)>>15);
    order = Norm_32s_I(&L_tmp);
    tmp1 = (Ipp16s)(L_tmp>>16);
    order -= 15;
    ippsMulC_NR_16s_Sfs(pSrc,tmp1,pDst,len,15);
    if(order<0) {
        ippsLShiftC_16s(pDst,-order,pDst,len);
    } else if(order>0) {
        Ipp16s const1=(Ipp16s)(1<<(order-1));
        ippsAddC_16s_Sfs(pDst,const1,pDst,len,0);
        /* ippsRShiftC_NR_16s_I() */
        ippsRShiftC_16s(pDst,order,pDst,len);
    }
}

Ipp32s ComfortNoiseExcitation_G729B_16s_I(const Ipp16s *pSrc, const Ipp16s *pPos,
                                       const Ipp16s *pSign, Ipp16s val, Ipp16s val1,
                                       Ipp16s *pSrcDst, Ipp16s *pSrcDst1, Ipp16s *shiftDst){
    Ipp16s i, j, tmp1, tmp2, max, high, low, tmp3;
    Ipp16s x1, x2, x3 = (Ipp16s)(val1<<1);
    Ipp16s sVal0, sVal1, sVal2, sVal3;
    Ipp32s   L_tmp, L_tmp1, L_tmp2, order, deltaSign = 1;

    ippsMulC_NR_16s_ISfs(x3,pSrcDst,LP_SUBFRAME_DIM,15);
    ippsAdd_16s_Sfs(pSrc,pSrcDst,pSrcDst,LP_SUBFRAME_DIM,0);
    ippsMax_16s(pSrcDst,LP_SUBFRAME_DIM,&tmp1);
    ippsMin_16s(pSrcDst,LP_SUBFRAME_DIM,&tmp2);
    if(tmp2==IPP_MIN_16S)
        tmp2++;
    if(tmp2<0)
        tmp2 = (Ipp16s)(-tmp2);
    max = (Ipp16s)(IPP_MAX(tmp1,tmp2));
    if(max == 0)
        order = 0;
    else {
        order = 3 - Exp_16s(max);
        if(order < 0)
            order = 0;
    }
    ippsRShiftC_16s(pSrcDst,order,shiftDst, LP_SUBFRAME_DIM);
    ippsDotProd_16s32s_Sfs(shiftDst,shiftDst, LP_SUBFRAME_DIM, &L_tmp1, 0);
    L_tmp1 <<= 1;

    tmp3 = 0;
    for(i=0; i<4; i++) {
        j = pPos[i];
        if(pSign[i] == 0) {
            tmp3 = (Ipp16s)(tmp3 - (pSrcDst[j] >> order));
        } else {
            tmp3 = (Ipp16s)(tmp3 + (pSrcDst[j] >> order));
        }
    }

    L_tmp   = val * LP_SUBFRAME_DIM;
    L_tmp >>= 5;
    L_tmp2  = val * L_tmp;
    tmp1    = (Ipp16s)(order<<1);
    L_tmp   = L_tmp2 >> tmp1;
    L_tmp  -= L_tmp1;
    tmp3  >>= 1;
    L_tmp  += (tmp3 * tmp3)<<1;
    order++;
    if(L_tmp < 0) {
        sVal0 = Abs_16s(pSrc[pPos[0]]); sVal1 = Abs_16s(pSrc[pPos[1]]);
        sVal2 = Abs_16s(pSrc[pPos[2]]); sVal3 = Abs_16s(pSrc[pPos[3]]);
        ippsCopy_16s(pSrc, pSrcDst, LP_SUBFRAME_DIM);
        tmp1 = (Ipp16s)(sVal0 | sVal1);
        tmp2 = (Ipp16s)(sVal2 | sVal3);
        tmp1 = (Ipp16s)(tmp1 | tmp2);
        order = ((tmp1 & BWF_HARMONIC) == 0) ? 1 : 2;

        for(tmp3=i=0; i<4; i++) {
            tmp1 = (Ipp16s)(pSrc[pPos[i]] >> order);
            if(pSign[i] == 0) {
                tmp3 = (Ipp16s)(tmp3 - tmp1);
            } else {
                tmp3 = (Ipp16s)(tmp3 + tmp1);
            }
        }
        high   = (Ipp16s)(L_tmp2>>15);
        low    = (Ipp16s)(L_tmp2 & IPP_MAX_16S);
        L_tmp  = high * 24576 + ((low*24576)>>15);
        tmp1   = (Ipp16s)(2 * order - 2);
        L_tmp  = L_tmp >> tmp1;
        L_tmp += 2 * tmp3 * tmp3;
        deltaSign = -1;
    }

    tmp2 = (Ipp16s)ownSqrt_32s(L_tmp>>1);
    x1   = (Ipp16s)(tmp2 - tmp3);
    x2   = (Ipp16s)(-(tmp3 + tmp2));
    sVal1 = Abs_16s(x1);
    sVal2 = Abs_16s(x2);
    if(sVal2 < sVal1)
        x1 = x2;
    tmp1 = (Ipp16s)(2 - order);
    if(tmp1>=0)
        pSrcDst1[0] = ShiftR_NR_16s(x1, tmp1);
    else
        pSrcDst1[0] = ShiftL_16s((Ipp16s)x1, (Ipp16u)(-tmp1));
    if(pSrcDst1[0] > 5000)
        pSrcDst1[0] = 5000;
    else
        if(pSrcDst1[0] < -5000)
            pSrcDst1[0] = -5000;

    for(i=0; i<4; i++) {
        j = pPos[i];
        if(pSign[i] != 0) {
            pSrcDst[j] = (Ipp16s)(pSrcDst[j] + pSrcDst1[0]);
        } else {
            pSrcDst[j] = (Ipp16s)(pSrcDst[j] - pSrcDst1[0]);
        }
    }
    return deltaSign;
}
void RandomCodebookParm_G729B_16s(Ipp16s *pSeed, Ipp16s *pPos, Ipp16s *pSign,
                                  Ipp16s *pGain, Ipp16s* delay){
    Ipp16s tmp1, tmp2;
    Ipp16s fractal;

    /* generate random adaptive codebook & fixed codebook parameters */
    tmp1 = Rand_16s(pSeed);
    fractal = (Ipp16s)((tmp1 & 3) - 1);
    if(fractal == 2)
        fractal = 0;

    tmp1   >>= 2;
    delay[0] = (Ipp16s)((tmp1 & 0x003f) + 40);

    tmp1  >>= 6;
    tmp2    = (Ipp16s)(tmp1 & 7);
    pPos[0] = (Ipp16s)(5 * tmp2);

    tmp1   >>= 3;
    pSign[0] = (Ipp16s)(tmp1 & 1);

    tmp1  >>= 1;
    tmp2    = (Ipp16s)(tmp1 & 7);
    pPos[1] = (Ipp16s)(5 * tmp2 + 1);

    tmp1   >>= 3;
    pSign[1] = (Ipp16s)(tmp1 & 1);
    tmp1     = Rand_16s(pSeed);
    tmp2     = (Ipp16s)(tmp1 & 7);
    pPos[2]  = (Ipp16s)(5 * tmp2 + 2);

    tmp1   >>= 3;
    pSign[2] = (Ipp16s)(tmp1 & 1);

    tmp1  >>= 1;
    tmp2    = (Ipp16s)(tmp1 & 0x000f);
    pPos[3] = (Ipp16s)((tmp2 & 1) + 3);
    tmp2    = (Ipp16s)((tmp2>>1) & 7);
    pPos[3] = (Ipp16s)(pPos[3] + 5 * tmp2);

    tmp1   >>= 4;
    pSign[3] = (Ipp16s)(tmp1 & 1);
    delay[1] = fractal;
    *pGain   = (Ipp16s)(Rand_16s(pSeed) & 0x1fff);
}

static Ipp16s ownQuantizeEnergy(Ipp32s val, Ipp16s order, Ipp16s *pDst) {
    Ipp16s exp, fractal, tmp, tmp1, index;
    Log2_G729(val, &exp, &fractal);
    tmp1 = (Ipp16s)(exp - order);
    tmp = (Ipp16s)(tmp1 << 10);
    tmp = (Ipp16s)(tmp + ((fractal + 0x10)>>5));
    if(tmp <= -2721) {/* 8 dB */
        pDst[0] = -12;
        return 0;
    }
    if(tmp > 22111) { /* 65 dB */
        pDst[0] = 66;
        return 31;
    }
    if(tmp <= 4762) { /* 14 dB */
        tmp += 3401;
        index = (Ipp16s)((tmp * 24)>>15);
        if(index < 1)
            index = 1;
        pDst[0] = (Ipp16s)((index<<2) - 8);
        return index;
    }
    tmp -= 340;
    index = (Ipp16s)(((tmp*193)>>17) - 1);
    if(index < 6)
        index = 6;
    pDst[0] = (Ipp16s)((index<<1) + 4);
    return(index);
}
/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:    QuantSIDGain_G729B_16s
//  Purpose:
//  Parameters:
//   pSrc      array of energies
//   pSrcSfs   corresponding scaling factors
//   len       number of energies
//   pDst      decoded energies in dB
//   pIndx     SID gain quantization index
*/
void QuantSIDGain_G729B_16s (const Ipp16s *pSrc, const Ipp16s *pSrcSfs,
                             Ipp32s len, Ipp16s *pDst, Ipp32s *pDstIndx){
    Ipp16s i,order,temp,high,low;
    Ipp32s   val;
    if(len == 0) {
        val = *pSrc;

        if(pSrcSfs[0]<0)
            val >>= - pSrcSfs[0];
        else
            val <<=  pSrcSfs[0];
        order  = 0;
        high = (Ipp16s)(val >> 16);
        low  = (Ipp16s)((val>>1)&IPP_MAX_16S);
        val  = high *  t[0] + ((low * t[0])>>15);
    } else {
        ippsMin_16s(pSrcSfs,len,&order);
        order = (Ipp16s)(order + 16 - t1[len]);
        val = 0;
        for(i=0; i<len; i++) {
            temp = (Ipp16s)(order - pSrcSfs[i]);
            val += (pSrc[i]<<temp);
        }
        high = (Ipp16s)(val >> 16);
        low  = (Ipp16s)((val>>1)&IPP_MAX_16S);
        val  = high *  t[i] + (((low * t[i] >> 1) & 0xffff8000)>>14);
    }

    *pDstIndx = ownQuantizeEnergy(val, order, pDst);

}
void Sum_G729_16s_Sfs(const Ipp16s *pSrc, const Ipp16s *pSrcSfs,
                      Ipp16s *pDst, Ipp16s *pDSfs, Ipp32s len, Ipp32s* pSum) {

    Ipp16s sfs, order;
    Ipp16s i, j;
    sfs = pSrcSfs[0];
    for(i=1; i<len; i++) {
        if(pSrcSfs[i] < sfs)
            sfs = pSrcSfs[i];
    }
    for(j=0; j<LPF_DIM+1; j++) {
        pSum[j] = 0;
    }
    sfs += 14;
    for(i=0; i<len; i++) {
        order = (Ipp16s)(sfs - pSrcSfs[i]);
        if(order<0) {
            for(j=0; j<LPF_DIM+1; j++) {
                pSum[j] += pSrc[(LPF_DIM+1)*i+j] >> -order;
            }
        } else {
            for(j=0; j<LPF_DIM+1; j++) {
                pSum[j] += pSrc[(LPF_DIM+1)*i+j] << order;
            }
        }
    }
    order = Norm_32s_I(&pSum[0]);
    pDst[0] = (Ipp16s)(pSum[0]>>16);
    for(i=1; i<=LPF_DIM; i++) {
        pDst[i] = (Ipp16s)((pSum[i]<<order)>>16);
    }
    order -= 16;
    *pDSfs = (Ipp16s)(sfs + order);

}

void VADMusicDetection(G729Codec_Type codecType,Ipp32s autoCorr,Ipp16s expAutoCorr,Ipp16s *pRc,
                       Ipp16s *lags, Ipp16s *pgains,Ipp16s stat_flg,Ipp16s *Vad,Ipp8s* pVADmem) {
    Ipp16s i, j, exp, fractal, tmp, tmp1, tmp2, tmp3, flag1, flag2, flag,
            pderr=IPP_MAX_16S, logE , spectrDist, threshold;
    Ipp32s   L_tmp;
    VADmemory_Obj *vadMem = (VADmemory_Obj*)pVADmem;
    for(i=0; i<4; i++) {
        j = (Ipp16s)((pRc[i] * pRc[i])>>15);
        j = (Ipp16s)(IPP_MAX_16S - j);
        pderr = (Ipp16s)((pderr * j)>>15);
    }
    L_tmp = 2*(autoCorr>>16)*pderr + 2*((((autoCorr>>1)&0x7fff)*pderr)>>15);
    Log2_G729(L_tmp, &exp, &fractal);
    L_tmp = 9864*exp + ((fractal*9864)>>15);
    i = (Ipp16s)(expAutoCorr - 2);
    L_tmp += 9864 * i;
    L_tmp = ShiftL_32s(L_tmp,12);
    logE = (Ipp16s)((L_tmp>>16) - 4875);
    L_tmp = 0;
    for(i=0; i<10; i++) {
        j = (Ipp16s)(vadMem->musicRC[i] - pRc[i]);
        L_tmp += j * j;
    }
    if(L_tmp > IPP_MAX_32S/2)
        spectrDist = IPP_MAX_32S >> 16;
    else
        spectrDist = (Ipp16s)(L_tmp>>15);
    if( *Vad == 0 ) {
        for(i=0; i<10; i++) {
            L_tmp = COEFF1 * vadMem->musicRC[i];
            L_tmp += PITCH_SHARP_MIN * pRc[i];
            vadMem->musicRC[i] = (Ipp16s)(L_tmp>>15);
        }
        L_tmp = COEFF1 * vadMem->musicSEMean;
        L_tmp += PITCH_SHARP_MIN * logE;
        vadMem->musicSEMean = (Ipp16s)(L_tmp>>15);
    }
    /* flag determination */
    tmp3=0; L_tmp=0;
    for(i=0; i<5; i++) {
        L_tmp += pgains[i] * 6554;
        tmp3 = (Ipp16s)(tmp3 + lags[i]);
    }
    L_tmp = (L_tmp>>15) * 6554;
    L_tmp += vadMem->MeanPgain * 26214;
    vadMem->MeanPgain = (Ipp16s)(L_tmp>>15);
    L_tmp = 0;
    for(i=0; i<5; i++) {
        j = (Ipp16s)(lags[i] << 2);
        j = (Ipp16s)(j + lags[i]);
        j = (Ipp16s)(j - tmp3);
        L_tmp += j * j;
    }
    if(L_tmp > 256)
        tmp2 = IPP_MAX_32S>>16;
    else
        tmp2 = (Ipp16s)(L_tmp<<7);
    if( codecType == G729D_CODEC)
        threshold = 11960;
    else
        threshold = 10322;
    if(vadMem->MeanPgain > threshold)
        flag2 =1;
    else
        flag2 =0;
    if( (tmp2 < 21632)  && (vadMem->MeanPgain > 7373))
        flag1 =1;
    else
        flag1 =0;
    flag= (Ipp16s)(((vadMem->VADPrev & (flag1 | flag2))| (flag2)));
    if( (pRc[1] <= 14746) && (pRc[1] >= 0) && (vadMem->MeanPgain < BWF_HARMONIC_E))
        vadMem->conscCounterFlagR++;
    else
        vadMem->conscCounterFlagR =0;
    if( (stat_flg == 1) && (*Vad == 1))
        vadMem->musicCounter += 256;
    if( (vadMem->frameCounter & 0x003f) == 0) {
        if( vadMem->frameCounter == 64)
            vadMem->musicMCounter = vadMem->musicCounter;
        else {
            L_tmp = COEFF1 * vadMem->musicMCounter;
            L_tmp += PITCH_SHARP_MIN * vadMem->musicCounter;
            vadMem->musicMCounter = (Ipp16s)(L_tmp>>15);
        }
    }
    if( vadMem->musicCounter == 0)
        vadMem->conscCounter++;
    else
        vadMem->conscCounter = 0;
    if(  ((vadMem->conscCounter > 500) || (vadMem->conscCounterFlagR > 150))) vadMem->musicMCounter = 0;

    if( (vadMem->frameCounter & 0x003f) == 0) {
        vadMem->musicCounter = 0;
    }

    if( flag== 1 )
        vadMem->count_pflag += 256;

    if( (vadMem->frameCounter & 0x003f) == 0) {
        if( vadMem->frameCounter == 64)
            vadMem->Mcount_pflag = vadMem->count_pflag;
        else {
            if(vadMem->count_pflag > 6400) {
                tmp1 = N0_98;
                tmp = 655;
            } else
                if(vadMem->count_pflag > 5120) {
                tmp1 = 31130;
                tmp = 1638;
            } else {
                tmp1 = COEFF1;
                tmp = PITCH_SHARP_MIN;
            }
            L_tmp = tmp1 * vadMem->Mcount_pflag;
            L_tmp += tmp * vadMem->count_pflag;
            vadMem->Mcount_pflag = (Ipp16s)(L_tmp>>15);
        }
    }
    if( vadMem->count_pflag == 0)
        vadMem->conscCounterFlagP++;
    else
        vadMem->conscCounterFlagP = 0;

    if(((vadMem->conscCounterFlagP > 100) || (vadMem->conscCounterFlagR > 150))) vadMem->Mcount_pflag = 0;


    if( (vadMem->frameCounter & 0x003f) == 0)
        vadMem->count_pflag = 0;

    if(codecType == G729E_CODEC) {
        if( (spectrDist > 4915) && (logE > (vadMem->musicSEMean + 819)) && (vadMem->VADPrevEnergy > 10240) )
            *Vad =1;
        else if( ((spectrDist > 12452) || (logE > (vadMem->musicSEMean + 819))) &&
                 (vadMem->VADPrevEnergy > 10240) )
            *Vad =1;
        else if( ( (vadMem->Mcount_pflag > 2560) || (vadMem->musicMCounter > 280) || (vadMem->frameCounter < 64))
                 && (vadMem->VADPrevEnergy > 1433))
            *Vad =1;
    }
    return;
}
void SynthesisFilterOvf_G729_16s_I(const Ipp16s * pLPC, Ipp16s * pSrcDst,
                                   Ipp32s len, Ipp8s * pMemUpdated,Ipp32s histLen) {
    Ipp32s nTaps;
    SynthesisFilterState *SynFltSt;

    SynFltSt=(SynthesisFilterState*)pMemUpdated;
    nTaps = SynFltSt->nTaps;
    if(!histLen)
        ippsSynthesisFilter_G729E_16s_I(pLPC,30, pSrcDst, len, SynFltSt->buffer);
    else
        ippsSynthesisFilterLow_NR_16s_ISfs(pLPC,pSrcDst, len, 12, SynFltSt->buffer+histLen);
    ippsCopy_16s((pSrcDst+len-nTaps), SynFltSt->buffer, nTaps);
}

IppStatus SynthesisFilter_G729_16s (const Ipp16s * pLPC, const Ipp16s * pSrc,
                                    Ipp16s * pDst, Ipp32s len, Ipp8s * pMemUpdated,Ipp32s histLen) {
    Ipp32s nTaps;
    SynthesisFilterState *SynFltSt;
    IppStatus sts=ippStsNoErr;
    SynFltSt=(SynthesisFilterState*)pMemUpdated;
    nTaps = SynFltSt->nTaps;
    if(histLen==0)
        ippsSynthesisFilter_G729E_16s(pLPC, 30,pSrc, pDst, len, SynFltSt->buffer);
    else
        sts = ippsSynthesisFilter_NR_16s_Sfs(pLPC,pSrc,pDst, len, 12, SynFltSt->buffer+histLen);

    if(sts != ippStsOverflow)
        ippsCopy_16s((pDst+len-nTaps), SynFltSt->buffer, nTaps);
    return sts;
}

IppStatus SynthesisFilter_G729_16s_update (const Ipp16s * pLPC, const Ipp16s * pSrc,
                                           Ipp16s * pDst, Ipp32s len, Ipp8s * pMemUpdated,Ipp32s histLen, Ipp32s update) {
    Ipp32s nTaps;
    SynthesisFilterState *SynFltSt;
    IppStatus sts=ippStsNoErr;

    SynFltSt=(SynthesisFilterState*)pMemUpdated;
    nTaps = SynFltSt->nTaps;
    if(histLen==0) ippsSynthesisFilter_G729E_16s(pLPC, 30,pSrc, pDst, len, SynFltSt->buffer);
    else
        sts = ippsSynthesisFilter_NR_16s_Sfs(pLPC,pSrc,pDst, len, 12, SynFltSt->buffer+histLen);
    if((sts != ippStsOverflow)&&(update==1))
        ippsCopy_16s((pDst+len-nTaps), SynFltSt->buffer, nTaps);
    return sts;
}

void SynthesisFilterOvf_G729_16s (const Ipp16s * pLPC, const Ipp16s * pSrc,
                                  Ipp16s * pDst, Ipp32s len, Ipp8s * pMemUpdated,Ipp32s histLen) {
    Ipp32s nTaps;
    SynthesisFilterState *SynFltSt;
    SynFltSt=(SynthesisFilterState*)pMemUpdated;
    nTaps = SynFltSt->nTaps;
    if(histLen==0) ippsSynthesisFilter_G729E_16s(pLPC, 30, pSrc, pDst, len, SynFltSt->buffer);
    else
        ippsSynthesisFilter_NR_16s_Sfs(pLPC,pSrc,pDst, len, 12, SynFltSt->buffer+histLen);
    ippsCopy_16s((pDst+len-nTaps), SynFltSt->buffer, nTaps);
}

void SynthesisFilterSize_G729 (Ipp32s *pSize) {
    *pSize = sizeof(SynthesisFilterState)+BWLPCF_DIM*sizeof(Ipp16s);
    *pSize += 32;

}

void SynthesisFilterInit_G729 (Ipp8s * pMemUpdated) {
    SynthesisFilterState *SynFltSt;
    Ipp32s size;
    SynFltSt=(SynthesisFilterState*)pMemUpdated;
    size = sizeof(SynthesisFilterState);
    SynFltSt->buffer = (Ipp16s*)((Ipp8s*)SynFltSt + size);
    //SynFltSt->buffer = (Ipp16s*)(((((Ipp8s*)SynFltSt->buffer + 7) - (Ipp8s*)NULL)&~7));
    SynFltSt->buffer = IPP_ALIGNED_PTR(SynFltSt->buffer, 32);
    SynFltSt->nTaps = BWLPCF_DIM;
    ippsZero_16s(SynFltSt->buffer,BWLPCF_DIM);
}
void CodewordImpConv_G729(Ipp32s index, const Ipp16s *pSrc1,const Ipp16s *pSrc2,Ipp16s *pDst) {
    Ipp32s i;
    Ipp32s sign0, sign1, sign2, sign3;     /* 1, -1 */
    Ipp32s idx0, idx1, idx2, idx3;         /* position*/

    idx0 = index & 0x7;
    idx1 = (index>>3) & 0x7;
    idx2 = (index>>6) & 0x7;
    idx3 = index>>9;

    idx0 = (idx0<<2)+idx0;
    idx1 = (idx1<<2)+idx1+1;
    idx2 = (idx2<<2)+idx2+2;
    idx3 = ((idx3>>1)<<2)+(idx3>>1)+(idx3&1)+3;

    if(idx0>idx1) {
        i=idx0; idx0=idx1; idx1=i;
    }
    if(idx2>idx3) {
        i=idx2; idx2=idx3; idx3=i;
    }
    if(idx0>idx2) {
        i=idx0; idx0=idx2; idx2=i;
    }
    if(idx1>idx3) {
        i=idx1; idx1=idx3; idx3=i;
    }
    if(idx1>idx2) {
        i=idx1; idx1=idx2; idx2=i;
    }

    sign0 = ((pSrc1[idx0] >> 15)<<1)+1;
    sign1 = ((pSrc1[idx1] >> 15)<<1)+1;
    sign2 = ((pSrc1[idx2] >> 15)<<1)+1;
    sign3 = ((pSrc1[idx3] >> 15)<<1)+1;
    for(i=0; i<idx0; i++)
        pDst[i]=0;
    for(; i<idx1; i++)
        pDst[i]=(Ipp16s)(sign0*pSrc2[i-idx0]);
    for(; i<idx2; i++)
        pDst[i]=Cnvrt_32s16s(sign0*pSrc2[i-idx0]+sign1*pSrc2[i-idx1]);
    for(; i<idx3; i++)
        pDst[i]=Cnvrt_32s16s(sign0*pSrc2[i-idx0]+sign1*pSrc2[i-idx1]+sign2*pSrc2[i-idx2]);
    for(; i<LP_SUBFRAME_DIM; i++)
        pDst[i]=Cnvrt_32s16s(sign0*pSrc2[i-idx0]+sign1*pSrc2[i-idx1]+sign2*pSrc2[i-idx2]+sign3*pSrc2[i-idx3]);

}
/*=============================================================
// pSrc - reflection coefficients
// pDst - log area ratio coefficients
*/
void _ippsRCToLAR_G729_16s (const Ipp16s* pSrc, Ipp16s* pDst, Ipp32s len) {
    Ipp16s refC, Ax;
    Ipp32s   i, L_tmp, L_Bx;
    for(i=0; i<len; i++) {
        refC = Abs_16s(pSrc[i]);
        refC >>= 4;
        if(refC <= 1299  /* 0.6341 */) {
            pDst[i] = refC;
        } else {
            if(refC <= 1815 /* 0.8864 */) {
                Ax = 4567;
                L_Bx = 3271557L/* 0.78 */;
            } else if(refC <= 1944 /* 0.9490 */) {
                Ax = 11776               /* 5.75 */;
                L_Bx = 16357786L           /* 3.90 */;
            } else {
                Ax = 27443               /* 13.40 */;
                L_Bx = 46808433L           /* 11.16 */;
            }
            refC >>= 1;
            L_tmp = (refC * Ax)<<1;
            L_tmp=Sub_32s(L_tmp,L_Bx);
            pDst[i] = (Ipp16s)(L_tmp >> 11);
        }
        if(pSrc[i] < 0) {
            pDst[i]=Cnvrt_32s16s(-pDst[i]);
        }
    }
}
/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ownMakeDecision
//  Purpose:
//  Parameters:
//  bwf1    - banwidth expansion parameter
//  bwf2    - bandwidth expansion parameter
//  intLSF  - interpolated LSF
//  newLSF  - new LSF vector for 2nd subframe
//  pRc     - reflection coefficients
*/
void _ippsPWGammaFactor_G729_16s ( const Ipp16s *pLAR, const Ipp16s *pLSF,
                                   Ipp16s *flat, Ipp16s *pGamma1, Ipp16s *pGamma2, Ipp16s *pMem ) {

    Ipp16s  lar0, lar1, bwf1,bwf2;
    Ipp32s  i, L_tmp, L_min;

    lar0 = pLAR[0];
    lar1 = pLAR[1];

    if(*flat != 0) {
        if((lar0 < -3562 /* -1.74 */)&&(lar1 > 1336 /* 0.65 */)) {
            *flat = 0;
        }
    } else {
        if((lar0 > -3116 /* -1.52 */) || (lar1 < 890 /* 0.43 */) ) {
            *flat = 1;
        }
    }

    if(*flat == 0) {
        ippsLShiftC_16s(pLSF,1,pMem,LPF_DIM);
        bwf1 = N0_98 /* 0.98 */;
        L_min = pMem[1] - pMem[0];
        for(i=1; i<LPF_DIM-1; i++) {
            L_tmp = pMem[i+1] - pMem[i];
            if(L_tmp < L_min) {
                L_min = L_tmp;
            }
        }

        L_tmp = SIX_PI * L_min;
        L_tmp >>= 15;
        L_tmp = 1024 /* 1 */ - L_tmp;
        L_tmp = L_tmp << 5;
        if(L_tmp > BWF2 /* 0.70 */) {
            bwf2 = BWF2;
        } else if(L_tmp < 13107 /* 0.40 */) {
            bwf2 = 13107;
        } else {
            bwf2 = (Ipp16s)L_tmp;
        }
    } else {
        bwf1 = 30802 /* 0.94 */;
        bwf2 = 19661 /* 0.60 */;
    }
    *pGamma1 = bwf1;
    *pGamma2 = bwf2;

}

void PitchTracking_G729E( Ipp16s *delay,Ipp16s *delay_frac, Ipp16s *prevDelay, Ipp16s *stat_N,
                          Ipp16s *pitchStatDelayt, Ipp16s *pitchStatFractiont){
    Ipp16s dist, dist_min, pitch_mult;
    Ipp16s i, is_mult;

    dist = (Ipp16s)(delay[0] - prevDelay[0]);
    if(dist < 0) {
        dist = (Ipp16s)(-dist);
        is_mult = 0;
    } else {
        is_mult = 1;
    }
    if(dist < 5) {
        *stat_N += 1;
        if(*stat_N > 7) *stat_N = 7 ;
        *pitchStatDelayt = delay[0];
        *pitchStatFractiont = delay_frac[0];
    } else {

        dist_min =  dist;
        if( is_mult == 0) {
            pitch_mult = (Ipp16s)(delay[0] + delay[0]);
            for(i=2; i<5; i++) {
                dist = Abs_16s((Ipp16s)(pitch_mult - prevDelay[0]));
                if(dist <= dist_min) dist_min = dist;
                pitch_mult = (Ipp16s)(pitch_mult + delay[0]);
            }
        } else {
            pitch_mult = (Ipp16s)(prevDelay[0] + prevDelay[0]);
            for(i=2; i<5; i++) {
                dist = Abs_16s((Ipp16s)(pitch_mult - delay[0]));
                if(dist <= dist_min) dist_min = dist;
                pitch_mult = (Ipp16s)(pitch_mult + prevDelay[0]);
            }
        }
        if(dist_min < 5) {
            if(*stat_N > 0) {
                delay[0] = *pitchStatDelayt;
                delay_frac[0] = *pitchStatFractiont;
            }
            *stat_N -= 1;
            if(*stat_N < 0) *stat_N = 0 ;
        } else {
            *stat_N = 0;
            *pitchStatDelayt = delay[0];
            *pitchStatFractiont = delay_frac[0];
        }
    }
    prevDelay[0] = delay[0];
    return;
}

static void stat( Ipp16s prevB, Ipp16s prevF, Ipp16s mode, Ipp16s prevMode,
                       Ipp16s *statGlobal, Ipp16s *BWDFrameCounter, Ipp16s *val_BWDFrameCounter  ) {
    Ipp32s  i;
    if(mode == 1) {
        *BWDFrameCounter += 1;
        *val_BWDFrameCounter += 250;
        if(*BWDFrameCounter == 20)
            statGlobal[0] = Add_16s(statGlobal[0], 2500);
        else
            if(*BWDFrameCounter > 20)
            statGlobal[0] += 500;
    }else
        if((mode == 0)&&(prevMode == 1)) {
            if(*BWDFrameCounter < 20) {
                statGlobal[0] = (Ipp16s)(statGlobal[0] - 5000 + *val_BWDFrameCounter);
            }
        *BWDFrameCounter = *val_BWDFrameCounter = 0;
        }
    if(statGlobal[0] < 13000) {
        for(i=1; i<6;i++)
            if(prevB > prevF + gThrs[i]) {
                statGlobal[0] = (Ipp16s)(statGlobal[0] + gStats[i]);
                break;
            }
    }
    for(i=0; i<5;i++)
        if(prevB < prevF - gThrs[i]) {
            statGlobal[0] = (Ipp16s)(statGlobal[0] - gStats1[i]);
            break;
        }
    if( statGlobal[0] > 32000)
        statGlobal[0] = 32000;
    else
        if(statGlobal[0] < 0)
            statGlobal[0] = 0;
    return;
}

void SetLPCMode_G729E ( Ipp16s *pSignal,  Ipp16s *fwdA, Ipp16s *pBwdLPC, Ipp16s *mode,
                        Ipp16s *newLSP, Ipp16s *oldLSP, G729Encoder_Obj *encoderObj){
    Ipp16s i,gap,prevF,prevB,gpredB,temp;
    Ipp32s   L_threshLPC,LSPdist,sigEnergy;
    LOCAL_ALIGN_ARRAY(32, Ipp16s, res, LP_FRAME_DIM, encoderObj);
    sigEnergy = enerDB(pSignal, LP_FRAME_DIM);
    ippsResidualFilter_G729E_16s(&pBwdLPC[BWLPCF1_DIM], BWLPCF_DIM,pSignal, res, LP_FRAME_DIM);
    prevB = (Ipp16s)(sigEnergy - enerDB(res, LP_FRAME_DIM));
    encoderObj->interpCoeff2_2 -= 410;
    if( encoderObj->interpCoeff2_2 < 0) encoderObj->interpCoeff2_2 = 0;
    temp = (Ipp16s)((1<<12) - encoderObj->interpCoeff2_2);
    ippsInterpolateC_G729_16s_Sfs(pBwdLPC + BWLPCF1_DIM, temp,
                                  encoderObj->pPrevFilt, encoderObj->interpCoeff2_2, pBwdLPC + BWLPCF1_DIM, BWLPCF1_DIM, 12);
    ippsInterpolate_G729_16s
    (pBwdLPC + BWLPCF1_DIM, encoderObj->pPrevFilt, pBwdLPC, BWLPCF1_DIM);
    ippsResidualFilter_G729E_16s(pBwdLPC, BWLPCF_DIM,pSignal, res, LP_SUBFRAME_DIM);
    ippsResidualFilter_G729E_16s(&pBwdLPC[BWLPCF1_DIM], BWLPCF_DIM,&pSignal[LP_SUBFRAME_DIM], &res[LP_SUBFRAME_DIM], LP_SUBFRAME_DIM);
    gpredB = (Ipp16s)(sigEnergy - enerDB(res, LP_FRAME_DIM));
    ippsResidualFilter_G729E_16s(fwdA,LPF_DIM, pSignal, res, LP_SUBFRAME_DIM);
    ippsResidualFilter_G729E_16s(&fwdA[LPF_DIM+1], LPF_DIM,&pSignal[LP_SUBFRAME_DIM], &res[LP_SUBFRAME_DIM], LP_SUBFRAME_DIM);
    prevF = (Ipp16s)(sigEnergy - enerDB(res, LP_FRAME_DIM));
    temp = (Ipp16s)(encoderObj->statGlobal>>7);
    temp = (Ipp16s)(temp * 3);
    gap = (Ipp16s)(temp + 205);
    if( (gpredB > prevF - gap) &&
        (prevB > prevF - gap) && (prevB > 0) && (gpredB > 0) )
        mode[0] = 1;

    else
        mode[0] = 0;
    for(LSPdist=i=0; i<LPF_DIM; i++) {
        temp = (Ipp16s)(oldLSP[i] - newLSP[i]);
        LSPdist += temp * temp;
    }
    if(encoderObj->statGlobal < 13000)
        mode[0] = 0;
    if(encoderObj->statGlobal < 32000) {
        L_threshLPC = 0;
    } else {
        L_threshLPC = 32212255;
    }
    if((LSPdist < L_threshLPC)&&(mode[0] == 0)&&(encoderObj->prevLPmode == 1)&&(prevB > 0)&&(gpredB > 0)) {
        mode[0] = 1;
    }
    if(sigEnergy < BWF_HARMONIC_E) {
        mode[0] = 0;
        if(encoderObj->statGlobal > 13000)
            encoderObj->statGlobal = 13000;
    } else {
        tstDominantBWDmode(&encoderObj->BWDcounter2,&encoderObj->FWDcounter2,&encoderObj->dominantBWDmode, mode[0]);

        stat(prevB, prevF, mode[0], encoderObj->prevLPmode,
                  &encoderObj->statGlobal, &encoderObj->BWDFrameCounter, &encoderObj->val_BWDFrameCounter);
    }
    if(mode[0] == 0)
        encoderObj->interpCoeff2_2 = 4506;
    LOCAL_ALIGN_ARRAY_FREE(32, Ipp16s, res, LP_FRAME_DIM, encoderObj);
    return;
}

void tstDominantBWDmode(   Ipp16s *BWDcounter2,   Ipp16s *FWDcounter2,   Ipp16s *dominantBWDmode,  Ipp16s mode) {
    Ipp16s tmp, count_all;
    if(mode == 0)
        *FWDcounter2+=1;
    else *BWDcounter2+=1;
    count_all = (Ipp16s)(*BWDcounter2 + *FWDcounter2);
    if(count_all == 100) {
        count_all >>= 1;
        *BWDcounter2 >>= 1;
        *FWDcounter2 >>= 1;
    }
    *dominantBWDmode = 0; if(count_all >= 10) {
        tmp = (Ipp16s)(*FWDcounter2<<2);
        if(*BWDcounter2 > tmp) *dominantBWDmode = 1;
    }
    return;
}

Ipp16s enerDB(Ipp16s *synth, Ipp16s L) {

    Ipp16s i,energyLog,index;
    Ipp32s L_energy;
    ippsDotProd_16s32s_Sfs(synth,synth,L,&L_energy,0);
    if(L_energy > (IPP_MAX_32S>>1))
        L_energy = (IPP_MAX_32S>>1);
    for(i = 0; L_energy > 32; i++)
        L_energy >>= 1;
    index = (Ipp16s)(L_energy - (1<<4));
    energyLog = 617;
    if(index < 0)
        energyLog = 1;
    else {
        if(i > 1)
            energyLog = (Ipp16s)(i*617);
        energyLog = (Ipp16s)(energyLog + energyLogTable[index]);
    }
    return(energyLog);
}

static void NormBy0_32s(Ipp32s *src, Ipp32s *dst,Ipp32s len) {
    Ipp32s i, sum, norm;
    sum = src[0] + 1;
    norm = Norm_32s_I(&sum);
    dst[0] = sum;

    for(i = 1; i <len; i++) {
        sum = src[i] + 1;
        dst[i] = sum << norm;
    }
    return;
}

void BWDLagWindow(Ipp32s *pSrc, Ipp32s *pDst) {
    Ipp16s i;
    for(i=0; i<BWLPCF_DIM; i++) {
        pSrc[i+1] = Mul_32s(pSrc[i+1]>>1,wlag_bwd[i]>>1);
    }
    NormBy0_32s(pSrc,pDst,BWLPCF_DIM+1);
}

Ipp16s ExtractBitsG729( const Ipp8u **pBits, Ipp32s *nBit, Ipp32s Count) {
    Ipp32s i;
    Ipp16s temp, bits = 0;
    for( i = 0 ; i < Count ; i ++ ) {
        temp   = (Ipp16s)(((pBits[0])[(i + nBit[0])>>3] >> (7 - ((i + nBit[0]) & 7)) ) & 1);
        bits <<= 1 ;
        bits  = (Ipp16s)(bits + temp);
    }
    pBits[0] += (Count + nBit[0])>>3;
    nBit[0]   = (Count + nBit[0]) & 7;
    return bits;
}

