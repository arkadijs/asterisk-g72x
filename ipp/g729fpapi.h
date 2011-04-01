/*/////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2004-2010 Intel Corporation. All Rights Reserved.
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
// Purpose: G.729 floating-point speech codec: API header file.
//
*/

#ifndef __G729FPAPI_H__
#define __G729FPAPI_H__

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
   G729Encode_VAD_Enabled =1,
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

struct _G729FPVADmemory_Obj;
struct _G729FPEncoder_Obj;
struct _G729FPDecoder_Obj;
typedef struct _G729FPVADmemory_Obj VADmemory;
typedef struct _G729FPEncoder_Obj G729FPEncoder_Obj;
typedef struct _G729FPDecoder_Obj G729FPDecoder_Obj;__G729FPAPI_H__

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
G729_CODECAPI( APIG729_Status, apiG729FPEncoder_Alloc,
         (G729Codec_Type codecType, Ipp32s *pCodecSize))
G729_CODECAPI( APIG729_Status, apiG729FPDecoder_Alloc,
         (G729Codec_Type codecType, Ipp32s *pCodecSize))
G729_CODECAPI( APIG729_Status, apiG729FPCodec_ScratchMemoryAlloc,
         (Ipp32s *pCodecSize))
G729_CODECAPI( APIG729_Status, apiG729FPEncoder_Init,
         (G729FPEncoder_Obj* encoderObj, G729Codec_Type codecType, G729Encode_Mode mode))
G729_CODECAPI( APIG729_Status, apiG729FPDecoder_Init,
         (G729FPDecoder_Obj* decoderObj, G729Codec_Type codecType))
G729_CODECAPI( APIG729_Status, apiG729FPEncoder_InitBuff,
         (G729FPEncoder_Obj* encoderObj, Ipp8s *buff))
G729_CODECAPI( APIG729_Status, apiG729FPDecoder_InitBuff,
         (G729FPDecoder_Obj* decoderObj, Ipp8s *buff))
G729_CODECAPI( APIG729_Status, apiG729FPEncode,
         (G729FPEncoder_Obj* encoderObj,const Ipp16s* src, Ipp8u* dst, G729Codec_Type codecType, Ipp32s *frametype))
G729_CODECAPI(  APIG729_Status, apiG729FPEncodeVAD,
         (G729FPEncoder_Obj* encoderObj,const Ipp16s* src, Ipp16s* dst, G729Codec_Type codecType, Ipp32s *pVAD ))
G729_CODECAPI( APIG729_Status, apiG729FPDecode,
         (G729FPDecoder_Obj* decoderObj,const Ipp8u* src, Ipp32s frametype, Ipp16s* dst))
G729_CODECAPI( APIG729_Status, apiG729FPEncoder_Mode,
         (G729FPEncoder_Obj* encoderObj, G729Encode_Mode mode))

#ifdef __cplusplus
}
#endif

#endif /* __G729FPAPI_H__ */
