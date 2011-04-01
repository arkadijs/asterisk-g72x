/*
**
** File:        "exc_lbc.h"
**
** Description:     Function prototypes for "exc_lbc.c"
**  
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/


Word16   Estim_Pitch( Word16 *Dpnt, Word16 Start );
PWDEF Comp_Pw( Word16 *Dpnt, Word16 Start, Word16 Olp );
void  Filt_Pw( Word16 *DataBuff, Word16 *Dpnt, Word16 Start, PWDEF Pw );
void  Find_Fcbk( Word16 *Dpnt, Word16 *ImpResp, LINEDEF *Line, Word16 Sfc );
void  Gen_Trn( Word16 *Dst, Word16 *Src, Word16 Olp );
void  Find_Best( BESTDEF *Best, Word16 *Tv, Word16 *ImpResp, Word16 Np,
Word16 Olp );
void  Fcbk_Pack( Word16 *Dpnt, SFSDEF *Sfs, BESTDEF *Best, Word16 Np );
void  Fcbk_Unpk( Word16 *Tv, SFSDEF Sfs, Word16 Olp, Word16 Sfc );
void  Find_Acbk( Word16 *Tv, Word16 *ImpResp, Word16 *PrevExc, LINEDEF
*Line, Word16 Sfc );
void  Get_Rez( Word16 *Tv, Word16 *PrevExc, Word16 Lag );
void  Decod_Acbk( Word16 *Tv, Word16 *PrevExc, Word16 Olp, Word16 Lid,
Word16 Gid );
Word16 Comp_Info( Word16 *Buff, Word16 Olp, Word16 *Gain, Word16 *ShGain);
void     Regen( Word16 *DataBuff, Word16 *Buff, Word16 Lag, Word16 Gain,
Word16 Ecount, Word16 *Sd );
PFDEF Comp_Lpf( Word16 *Buff, Word16 Olp, Word16 Sfc );
Word16   Find_B( Word16 *Buff, Word16 Olp, Word16 Sfc );
Word16   Find_F( Word16 *Buff, Word16 Olp, Word16 Sfc );
PFDEF Get_Ind( Word16 Ind, Word16 Ten, Word16 Ccr, Word16 Enr );
void  Filt_Lpf( Word16 *Tv, Word16 *Buff, PFDEF Pf, Word16 Sfc );
void reset_max_time(void);
Word16 search_T0 ( Word16 T0, Word16 Gid, Word16 *gain_T0);
Word16 ACELP_LBC_code(Word16 X[], Word16 h[], Word16 T0, Word16 code[],
            Word16 *gain, Word16 *shift, Word16 *sign, Word16 gain_T0);
void   Cor_h(Word16 *H, Word16 *rr);
void   Cor_h_X(Word16 h[], Word16 X[], Word16 D[]);
Word16 D4i64_LBC(Word16 Dn[], Word16 rr[], Word16 h[], Word16 cod[],
                 Word16 y[], Word16 *code_shift, Word16 *sign);
Word16 G_code(Word16 X[], Word16 Y[], Word16 *gain_q);

