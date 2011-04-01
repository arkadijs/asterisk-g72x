/*
**
** File:        "util_cng.h"
**
** Description:     Function prototypes for "util_cng.c"
**
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

void Calc_Exc_Rand(Word16 cur_gain, Word16 *PrevExc, Word16 *DataExc,
                                      Word16 *nRandom, LINEDEF *Line);
Word16 Qua_SidGain(Word16 *Ener, Word16 *shEner, Word16 nq);
Word16 Dec_SidGain(Word16 i_gain);


