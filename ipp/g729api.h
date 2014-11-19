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
// Purpose: G.729/A/B/D/E speech codec: API header file.
//
*/

#ifndef __G729API_H__
#define __G729API_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _G729Codec_Type{
   G729_CODEC=0,
   G729A_CODEC=1,
   G729D_CODEC=2,
   G729E_CODEC=3,
   G729I_CODEC=4
}G729Codec_Type;

typedef enum _G729Encode_Mode{
   G729Encode_VAD_Enabled=1,
   G729Encode_VAD_Disabled=0
}G729Encode_Mode;

typedef enum _G729Decode_Mode{
   G729Decode_DefaultMode=0
}G729Decode_Mode;

typedef enum{
    APIG729_StsBadCodecType     =   -5,
    APIG729_StsNotInitialized   =   -4,
    APIG729_StsBadArgErr        =   -3,
    APIG729_StsDeactivated      =   -2,
    APIG729_StsErr              =   -1,
    APIG729_StsNoErr            =    0
}APIG729_Status;

typedef enum{
    G729_Bad_Frame           = -1,
    G729_Untransmitted_Frame = 0,
    G729_SID_Frame           = 1,
    G729D_Voice_Frame        = 2,
    G729_Voice_Frame         = 3,
    G729E_Voice_Frame        = 4,
    G729_FirstSID_Frame      = 5
}G729_FrameType;

struct _VADmemory;
struct _G729Encoder_Obj;
struct _G729Decoder_Obj;
typedef struct _VADmemory VADmemory_Obj;
typedef struct _G729Encoder_Obj G729Encoder_Obj;
typedef struct _G729Decoder_Obj G729Decoder_Obj;

#if defined( _WIN32 ) || defined ( _WIN64 )
  #define __STDCALL  __stdcall
  #define __CDECL    __cdecl
#else
  #define __STDCALL
  #define __CDECL
#endif

#define   G729_CODECAPI(type,name,arg)                extern type name arg;

/*
                   Functions declarations
*/
G729_CODECAPI( APIG729_Status, apiG729Encoder_Alloc,
         (G729Codec_Type codecType, Ipp32s *pCodecSize))
G729_CODECAPI( APIG729_Status, apiG729Decoder_Alloc,
         (G729Codec_Type codecType, Ipp32s *pCodecSize))
G729_CODECAPI( APIG729_Status, apiG729Codec_ScratchMemoryAlloc,
         (Ipp32s *pCodecSize))
G729_CODECAPI( APIG729_Status, apiG729Encoder_Init,
         (G729Encoder_Obj* encoderObj, G729Codec_Type codecType, G729Encode_Mode mode))
G729_CODECAPI( APIG729_Status, apiG729Decoder_Init,
         (G729Decoder_Obj* decoderObj, G729Codec_Type codecType))
G729_CODECAPI( APIG729_Status, apiG729Encoder_InitBuff,
         (G729Encoder_Obj* encoderObj, Ipp8s *buff))
G729_CODECAPI( APIG729_Status, apiG729Decoder_InitBuff,
         (G729Decoder_Obj* decoderObj, Ipp8s *buff))
G729_CODECAPI( APIG729_Status, apiG729Encode,
         (G729Encoder_Obj* encoderObj,const Ipp16s* src, Ipp8u* dst, G729Codec_Type codecType, Ipp32s *frametype))
G729_CODECAPI(  APIG729_Status, apiG729EncodeVAD,
         (G729Encoder_Obj* encoderObj,const Ipp16s* src, Ipp16s* dst, G729Codec_Type codecType, Ipp32s *pVAD ))
G729_CODECAPI( APIG729_Status, apiG729Decode,
         (G729Decoder_Obj* decoderObj,const Ipp8u* src, Ipp32s frametype, Ipp16s* dst))
G729_CODECAPI( APIG729_Status, apiG729Encoder_Mode,
         (G729Encoder_Obj* encoderObj, G729Encode_Mode mode))
G729_CODECAPI( APIG729_Status, apiG729Decoder_Mode,
               (G729Decoder_Obj* decoderObj, Ipp32s mode))

#ifdef __cplusplus
}
#endif

#endif /* __G729API_H__ */
