/*
**
** File:        "dec_cng.h"
**
** Description:     Function prototypes for "dec_cng.c"
**
*/
/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

void Init_Dec_Cng(void);
void Dec_Cng(Word16 Ftyp, LINEDEF *Line, Word16 *DataExc,
                                                    Word16 *QntLpc);

extern __thread  DECCNGDEF *DecCng;

