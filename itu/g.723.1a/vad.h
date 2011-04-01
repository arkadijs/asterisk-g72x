/*
**
** File:        "vad.h"
**
** Description:     Function prototypes for "vad.c"
**  
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)

    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

void    Init_Vad(void) ;
Flag Comp_Vad( Word16 *Dpnt);
extern __thread VADSTATDEF  *VadStat ;

