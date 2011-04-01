/*
**
** File:        "decod.h"
**
** Description:     Function prototypes and external declarations 
**          for "decod.c"
**  
*/


/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/


void  Init_Decod(void);
Flag    Decod( Word16 *DataBuff, char *Vinp, Word16 Crc );
extern __thread DECSTATDEF  *DecStat  ;



