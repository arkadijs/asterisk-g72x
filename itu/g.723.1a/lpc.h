/*
**
** File:        "lpc.h"
**
** Description:     Function prototypes for "lpc.c"
**  
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/


void    Comp_Lpc( Word16 *UnqLpc, Word16 *PrevDat, Word16 *DataBuff );
Word16  Durbin( Word16 *Lpc, Word16 *Corr, Word16 Err, Word16 *Pk2 );
void    Wght_Lpc( Word16 *PerLpc, Word16 *UnqLpc );
void    Error_Wght( Word16 *Dpnt, Word16 *PerLpc );
void    Comp_Ir( Word16 *ImpResp, Word16 *QntLpc, Word16 *PerLpc, PWDEF Pw );
void    Sub_Ring( Word16 *Dpnt, Word16 *QntLpc, Word16 *PerLpc, Word16 *PrevErr, PWDEF Pw );
void    Upd_Ring( Word16 *Dpnt, Word16 *QntLpc, Word16 *PerLpc, Word16 *PrevErr );
void    Synt( Word16 *Dpnt, Word16 *Lpc );
Word32  Spf( Word16 *Tv, Word16 *Lpc );
