/*/////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2011 Intel Corporation. All Rights Reserved.
//
//     Intel(R) Integrated Performance Primitives
//     USC - Unified Speech Codec interface library
//
// By downloading and installing USC codec, you hereby agree that the
// accompanying Materials are being provided to you under the terms and
// conditions of the End User License Agreement for the Intel(R) Integrated
// Performance Primitives product previously accepted by you. Please refer
// to the file ippEULA.rtf or ippEULA.txt located in the root directory of your Intel(R) IPP
// product installation for more information.
//
// A speech coding standards promoted by ITU, ETSI, 3GPP and other
// organizations. Implementations of these standards, or the standard enabled
// platforms may require licenses from various entities, including
// Intel Corporation.
//
//
// Purpose: G.723.1 speech codec: coder API header.
//
*/

#ifndef __G723API_H__
#define __G723API_H__

#include <ippdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define  G723_FrameSize       240
#define  G723_BitStreamFrameSizeMax 24

#define G723_ENC_KEY 0xecd723
#define G723_DEC_KEY 0xdec723

typedef struct {
   Ipp32s        objSize;
   Ipp32s        key;
   Ipp32u        mode;       /* mode's */
   Ipp32u        rat: 1;     /* encode rate: 0-63, 1-53, other bits reserved.*/
}G723_Obj_t;

typedef enum _G723Encode_Mode{
   G723Encode_DefaultMode=0, /* r63 ; HF, VAD disabled; */
   G723Encode_VAD_Enabled=1,
   G723Encode_HF_Enabled =2
}G723Encode_Mode;

typedef enum _G723Decode_Mode{
   G723Decode_DefaultMode=0, /* r63 ; PF disabled  */
   G723Decode_PF_Enabled =1
}G723Decode_Mode;

typedef enum{
    APIG723_StsBadCodecType     =   -5,
    APIG723_StsNotInitialized   =   -4,
    APIG723_StsBadArgErr        =   -3,
    APIG723_StsDeactivated      =   -2,
    APIG723_StsErr              =   -1,
    APIG723_StsNoErr            =    0
}APIG723_Status;

struct _G723VADmemory;
struct _G723Encoder_Obj;
struct _G723Decoder_Obj;

typedef struct _G723VADmemory G723VADmemory;
typedef struct _G723Encoder_Obj G723Encoder_Obj;
typedef struct _G723Decoder_Obj G723Decoder_Obj;

#define   G723_CODECAPI(type,name,arg)                extern type name arg;

/*
                   Functions declarations
*/
G723_CODECAPI( APIG723_Status, apiG723Codec_ScratchMemoryAlloc,(Ipp32s *pCodecSize))
G723_CODECAPI( APIG723_Status, apiG723Encoder_Alloc, (Ipp32s *pCodecSize))
G723_CODECAPI( APIG723_Status, apiG723Decoder_Alloc, (Ipp32s *pCodecSize))
G723_CODECAPI( APIG723_Status, apiG723Encoder_Init,
         (G723Encoder_Obj* encoderObj, Ipp32u mode))
G723_CODECAPI( APIG723_Status, apiG723Decoder_Init,
         (G723Decoder_Obj* decoderObj, Ipp32u mode))
G723_CODECAPI( APIG723_Status, apiG723Encoder_InitBuff,
         (G723Encoder_Obj* encoderObj, Ipp8s *buff))
G723_CODECAPI( APIG723_Status, apiG723Decoder_InitBuff,
         (G723Decoder_Obj* decoderObj, Ipp8s *buff))
G723_CODECAPI( APIG723_Status, apiG723Encode,
         (G723Encoder_Obj* encoderObj, const Ipp16s* src, Ipp16s rat, Ipp8s* dst ))
G723_CODECAPI( APIG723_Status, apiG723Decode,
         (G723Decoder_Obj* decoderObj, const Ipp8s* src, Ipp16s badFrameIndicator, Ipp16s* dst))
G723_CODECAPI( APIG723_Status, apiG723Encoder_Mode,
         (G723Encoder_Obj* encoderObj, Ipp32u mode))
G723_CODECAPI( APIG723_Status, apiG723Encoder_ControlMode,
         (G723Encoder_Obj* encoderObj, Ipp32u mode))
G723_CODECAPI( APIG723_Status, apiG723Decoder_ControlMode,
         (G723Decoder_Obj* decoderObj, Ipp32u mode))

#define G723_ENCODER_SCRATCH_MEMORY_SIZE        (5120+40)

#ifdef __cplusplus
}
#endif

#endif /* __G723API_H__ */
