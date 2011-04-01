/*
**
** File:        "util_lbc.h"
**
** Description:     Function prototypes for "util_lbc.c"
**
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)

    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/


void    Rem_Dc( Word16 *Dpnt );
Word16  Vec_Norm( Word16 *Vect, Word16 Len );
void    Mem_Shift( Word16 *PrevDat, Word16 *DataBuff );
void    Line_Pack( LINEDEF *Line, char *Vout, Word16 Ftyp );
Word16* Par2Ser( Word32 Inp, Word16 *Pnt, int BitNum );
LINEDEF  Line_Unpk( char *Vinp, Word16 *Ftyp, Word16 Crc );
Word32  Ser2Par( Word16 **Pnt, int Count );
Word32  Comp_En( Word16 *Dpnt );
Word16  Sqrt_lbc( Word32 Num );
Word16  Rand_lbc( Word16 *p );
void    Scale( Word16 *Tv, Word32 Sen );
