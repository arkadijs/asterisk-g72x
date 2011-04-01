/*
**
** File:        "cod_cng.h"
**
** Description:     Function prototypes for "cod_cng.c"
**
*/
/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

void Init_Cod_Cng(void);
void Cod_Cng(Word16 *DataExc, Word16 *Ftyp, LINEDEF *Line, Word16 *QntLpc);
void Update_Acf(Word16 *Acfsf, Word16 *Shsf);
extern  __thread CODCNGDEF   *CodCng;

