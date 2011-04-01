/*
**
** File:        "lsp.h"
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

void AtoLsp( Word16 *LspVect, Word16 *Lpc, Word16 *PrevLsp );
Word32   Lsp_Qnt( Word16 *CurrLsp, Word16 *PrevLsp );
Word32   Lsp_Svq( Word16 *Tv, Word16 *Wvect );
void Lsp_Inq( Word16 *Lsp, Word16 *PrevLsp, Word32 LspId, Word16 Crc );
void  Lsp_Int( Word16 *QntLpc, Word16 *CurrLsp, Word16 *PrevLsp );
void  LsptoA( Word16 *Lsp );
