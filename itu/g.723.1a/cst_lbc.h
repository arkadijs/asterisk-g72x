/*
**
** File:        "cst_lbc.h"
**
** Description:  This file contains global definition of the SG15
**    LBC Coder for 6.3/5.3 kbps.
**
*/


/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

#define  False 0
#define  True  1

/* Definition of the working mode */
enum  Wmode { Both, Cod, Dec } ;

/* Coder rate */
enum  Crate    { Rate63, Rate53 } ;

/* Coder global constants */
#define  Frame       240
#define  LpcFrame    180
#define  SubFrames   4
#define  SubFrLen    (Frame/SubFrames)

/* LPC constants */
#define  LpcOrder          10
#define  RidgeFact         10
#define  CosineTableSize   512
#define  PreCoef           (Word16) 0xc000            /* -0.25*2 */

#define  LspPrd0           12288
#define  LspPrd1           23552

#define  LspQntBands       3
#define  LspCbSize         256
#define  LspCbBits         8

/* LTP constants */
#define  PitchMin          18
#define  PitchMax          (PitchMin+127)
#define  PwConst           (Word16) 0x2800
#define  PwRange           3
#define  ClPitchOrd        5
#define  Pstep             1
#define NbFilt085          85
#define NbFilt170          170

/* MP-MLQ constants */
#define  Sgrid             2
#define  MaxPulseNum       6
#define  MlqSteps          2

/* acelp constants */
#define SubFrLen2          (SubFrLen +4)
#define DIM_RR             416
#define NB_POS             8
#define STEP               8
#define MSIZE              64
#define threshold          16384  /* 0.5 = 16384 in Q15 */
#define max_time           120

/* Gain constant */
#define  NumOfGainLev      24

/* FER constant */
#define  ErrMaxNum         3

/* CNG constants  */
#define NbAvAcf            3  /* Nb of frames for Acf average               */
#define NbAvGain           3  /* Nb of frames for gain average              */
#define ThreshGain         3  /* Theshold for quantized gains               */
#define FracThresh         7000   /* Itakura dist threshold: frac. part     */
#define NbPulsBlk          11 /* Nb of pulses in 2-subframes blocks         */

#define InvNbPulsBlk       2979 /* 32768/NbPulsBlk                          */
#define NbFilt             50 /* number of filters for CNG exc generation   */
#define LpcOrderP1         (LpcOrder+1)
#define SizAcf             ((NbAvAcf+1)*LpcOrderP1) /* size of array Acf    */
#define SubFrLenD          (2*SubFrLen)
#define Gexc_Max           5000  /* Maximum gain for fixed CNG excitation   */

/* Taming constants */
#define NbFilt085_min      51
#define NbFilt170_min      93
#define SizErr             5
#define Err0               (Word32)4  /* scaling factor */
#define ThreshErr          0x40000000L
#define DEC                (30 - 7)

/*
   Used structures
*/
typedef  struct   {
   /* High pass variables */
   Word16   HpfZdl   ;
   Word32   HpfPdl   ;

   /* Sine wave detector */
   Word16   SinDet   ;

   /* Lsp previous vector */
   Word16   PrevLsp[LpcOrder] ;

   /* All pitch operation buffers */
   Word16   PrevWgt[PitchMax] ;
   Word16   PrevErr[PitchMax] ;
   Word16   PrevExc[PitchMax] ;

   /* Required memory for the delay */
   Word16   PrevDat[LpcFrame-SubFrLen] ;

   /* Used delay lines */
   Word16   WghtFirDl[LpcOrder] ;
   Word16   WghtIirDl[LpcOrder] ;
   Word16   RingFirDl[LpcOrder] ;
   Word16   RingIirDl[LpcOrder] ;

   /* Taming procedure errors */
   Word32 Err[SizErr];

   } CODSTATDEF  ;

typedef  struct   {
   Word16   Ecount ;
   Word16   InterGain ;
   Word16   InterIndx ;
   Word16   Rseed ;
   Word16   Park  ;
   Word16   Gain  ;
   /* Lsp previous vector */
   Word16   PrevLsp[LpcOrder] ;

   /* All pitch operation buffers */
   Word16   PrevExc[PitchMax] ;

   /* Used delay lines */
   Word16   SyntIirDl[LpcOrder] ;
   Word16   PostFirDl[LpcOrder] ;
   Word16   PostIirDl[LpcOrder] ;

   } DECSTATDEF  ;

   /* subframe coded parameters */
typedef  struct   {
   Word16   AcLg  ;
   Word16   AcGn  ;
   Word16   Mamp  ;
   Word16   Grid  ;
   Word16   Tran  ;
   Word16   Pamp  ;
   Word32   Ppos  ;
   } SFSDEF ;

   /* frame coded parameters */
typedef  struct   {
   Word16   Crc   ;
   Word32   LspId ;
   Word16   Olp[SubFrames/2] ;
   SFSDEF   Sfs[SubFrames] ;
   } LINEDEF ;

   /* harmonic noise shaping filter parameters */
typedef  struct   {
   Word16   Indx  ;
   Word16   Gain  ;
   } PWDEF  ;

    /* pitch postfilter parameters */
typedef  struct   {
   Word16   Indx  ;
   Word16   Gain  ;
   Word16   ScGn  ;
   } PFDEF  ;

    /* best excitation vector parameters for the high rate */
typedef  struct {
   Word32   MaxErr   ;
   Word16   GridId   ;
   Word16   MampId   ;
   Word16   UseTrn   ;
   Word16   Ploc[MaxPulseNum] ;
   Word16   Pamp[MaxPulseNum] ;
   } BESTDEF ;

    /* VAD static variables */
typedef struct {
    Word16  Hcnt ;
    Word16  Vcnt ;
    Word32  Penr ;
    Word32  Nlev ;
    Word16  Aen ;
    Word16  Polp[4] ;
    Word16  NLpc[LpcOrder] ;
} VADSTATDEF ;


/* CNG features */

/* Coder part */
typedef struct {
    Word16 CurGain;
    Word16 PastFtyp;
    Word16 Acf[SizAcf];
    Word16 ShAcf[NbAvAcf+1];
    Word16 LspSid[LpcOrder] ;
    Word16 SidLpc[LpcOrder] ;
    Word16 RC[LpcOrderP1];
    Word16 ShRC;
    Word16 Ener[NbAvGain];
    Word16 NbEner;
    Word16 IRef;
    Word16 SidGain;
    Word16 RandSeed;
} CODCNGDEF;

/* Decoder part */
typedef struct {
    Word16 CurGain;
    Word16 PastFtyp;
    Word16 LspSid[LpcOrder] ;
    Word16 SidGain;
    Word16 RandSeed;
} DECCNGDEF;
