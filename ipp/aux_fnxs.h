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
// Purpose: USC: auxiliary functions.
//
*/

#ifndef __AUX_FNXS__
#define __AUX_FNXS__

#if defined( _WIN32_WCE)
#pragma warning( disable : 4505 )
#endif

#include <ipps.h>
#include "scratchmem.h"

extern void InvSqrt_32s16s_I(Ipp32s *valFrac, Ipp16s *valExp);

/********************************************************
*      auxiliary inline functions declarations
*********************************************************/
__INLINE Ipp16s Cnvrt_32s16s(Ipp32s x){
   if (IPP_MAX_16S < x) return IPP_MAX_16S;
   else if (IPP_MIN_16S > x) return IPP_MIN_16S;
   return (Ipp16s)(x);
}

__INLINE Ipp16s Cnvrt_NR_32s16s(Ipp32s x) {
   Ipp16s s = IPP_MAX_16S;
   if(x<(Ipp32s)0x7fff8000) s = (Ipp16s)((x+0x8000)>>16);
   return s;
}

__INLINE Ipp32s Cnvrt_64s32s(__INT64 z) {
   if(IPP_MAX_32S < z) return IPP_MAX_32S;
   else if(IPP_MIN_32S > z) return IPP_MIN_32S;
   return (Ipp32s)z;
}

/* Random generator  */
__INLINE Ipp16s Rand_16s(Ipp16s *seed)
{
  *seed = (Ipp16s)(*seed * 31821 + 13849);

  return(*seed);
}

__INLINE Ipp16s  Rand2_16s( Ipp16s *pSeed )
{
    *pSeed = (Ipp16s)(*pSeed * 521 + 259);
    return *pSeed ;
}

__INLINE Ipp16s NormRand_16s(Ipp16s N, Ipp16s *nRandom)
{
    Ipp16s sTmp;

    sTmp = (Ipp16s)(Rand2_16s(nRandom) & 0x7FFF);
    sTmp = (Ipp16s)((sTmp * N)>>15);
    return sTmp;
}

__INLINE Ipp16s Abs_16s(Ipp16s x){
   if(x<0){
      if(IPP_MIN_16S == x) return IPP_MAX_16S;
      x = (Ipp16s)-x;
   }
   return x;
}
__INLINE Ipp32s Abs_32s(Ipp32s x){
   if(x<0){
      if(IPP_MIN_32S == x) return IPP_MAX_32S;
      x = -x;
   }
   return x;
}

__INLINE int Add_32s(int x, int y) {
   return Cnvrt_64s32s((__INT64)x + y);
}

extern CONST Ipp16s ExpPosNormTbl[256];
extern CONST Ipp16s ExpPosNormTbl2[256];

__INLINE Ipp16s Exp_16s_Pos(Ipp16u x)
{
   if((x>>8)==0)
      return ExpPosNormTbl2[x];
   else {
      return ExpPosNormTbl[(x>>8)];
   }
}

__INLINE Ipp16s Exp_16s(Ipp16s x){
   if (x == -1) return 15;
   if (x == 0) return 0;
   if (x < 0) x = (Ipp16s)(~x);
   return Exp_16s_Pos(x);
}

//__INLINE Ipp16s Exp_32s_Pos(Ipp32s x){
//   Ipp16s i;
//   if (x == 0) return 0;
//   for(i = 0; x < (Ipp32s)0x40000000; i++) x <<= 1;
//   return i;
//}
__INLINE short Exp_32s_Pos(unsigned int x){
   if (x == 0) return 0;
   if((x>>16)==0) return (short)(16+Exp_16s_Pos((unsigned short) x));
   return Exp_16s_Pos((unsigned short)(x>>16));
}

__INLINE Ipp16s Exp_32s(Ipp32s x){
   if (x == 0) return 0;
   if (x == -1) return 31;
   if (x < 0) x = ~x;
   return Exp_32s_Pos(x);
}

__INLINE Ipp16s Norm_32s_I(Ipp32s *x){
   Ipp16s i;
   if (*x == 0) return 0;
   if (*x < 0){
      for(i = 0; *x >= (Ipp32s)0xC0000000; i++) *x <<= 1;
   }else
      for(i = 0; *x < (Ipp32s)0x40000000; i++) *x <<= 1;
   return i;
}
__INLINE short Norm_16s_I(short *x){
   short i;
   short y = *x;
   if (y == 0) return 0;
   if (y < 0) y = (short)~y;
   i = Exp_16s_Pos(y);
   *x <<= i;
   return i;
}
__INLINE Ipp16s Norm_32s_Pos_I(Ipp32s *x){
   Ipp16s i;
   if (*x == 0) return 0;
   for(i = 0; *x < (Ipp32s)0x40000000; i++) *x <<= 1;
   return i;
}

__INLINE Ipp16s MulHR_16s(Ipp16s x, Ipp16s y) {
   return (Ipp16s)( ((((Ipp32s)x * (Ipp32s)y)) + 0x4000) >> 15 );
}
__INLINE Ipp16s Negate_16s(Ipp16s x) {
   if(IPP_MIN_16S == x)
      return IPP_MAX_16S;
   return (Ipp16s)-x;
}
__INLINE Ipp32s Negate_32s(Ipp32s x) {
   if(IPP_MIN_32S == x)
      return IPP_MAX_32S;
   return (Ipp32s)-x;
}
__INLINE Ipp16s Mul2_16s(Ipp16s x) {
    if(x > IPP_MAX_16S/2) return IPP_MAX_16S;
    else if( x < IPP_MIN_16S/2) return IPP_MIN_16S;
    x = (Ipp16s)(x << 1);
    return x;
}
__INLINE Ipp32s Mul2_32s(Ipp32s x) {
    if(x > IPP_MAX_32S/2) return IPP_MAX_32S;
    else if( x < IPP_MIN_32S/2) return IPP_MIN_32S;
    return x <<= 1;
}

__INLINE Ipp32s Mul16_32s(Ipp32s x) {
    if(x > (Ipp32s) 0x07ffffffL) return IPP_MAX_32S;
    else if( x < (Ipp32s) 0xf8000000L) return IPP_MIN_32S;
    return (x <<= 4);
}

__INLINE Ipp32s MulC_32s(Ipp16s val, Ipp32s x) {
   Ipp32s z ;
   Ipp32s xh, xl;
   xh  = x >> 16;
   xl  = x & 0xffff;
   z = 2*val*xh;
   z = Add_32s(z,(xl*val)>>15);

  return( z );
}

__INLINE Ipp32s ShiftL_32s(Ipp32s x, Ipp16u n)
{
   Ipp32s max = IPP_MAX_32S >> n;
   Ipp32s min = IPP_MIN_32S >> n;
   if(x > max) return IPP_MAX_32S;
   else if(x < min) return IPP_MIN_32S;
   return (x<<n);
}

__INLINE Ipp16s ShiftL_16s(Ipp16s n, Ipp16u x)
{
   Ipp16s max = (Ipp16s)(IPP_MAX_16S >> x);
   Ipp16s min = (Ipp16s)(IPP_MIN_16S >> x);
   if(n > max) return IPP_MAX_16S;
   else if(n < min) return IPP_MIN_16S;
   return (Ipp16s)(n<<x);
}

__INLINE Ipp16s ShiftR_NR_16s(Ipp16s x, Ipp16u n){
   return (Ipp16s)((x + (1<<(n-1)))>>n);
}

__INLINE Ipp16s ExtractHigh(Ipp32s x){
   return ((Ipp16s)(x >> 16));
}

__INLINE void Unpack_32s (Ipp32s number, Ipp16s *hi, Ipp16s *lo)
{
    *hi = (Ipp16s)(number >> 16);
    *lo = (Ipp16s)((number>>1)&0x7fff);
}
__INLINE Ipp32s Pack_16s32s(Ipp16s hi, Ipp16s lo)
{
    return ( ((Ipp32s)hi<<16) + ((Ipp32s)lo<<1) );
}
__INLINE Ipp32s Mpy_32_16 (Ipp16s hi, Ipp16s lo, Ipp16s n)
{
    return (2 * (hi * n) +  2 * ((lo * n) >> 15));
}

__INLINE Ipp16s Mul_16s_Sfs(Ipp16s x, Ipp16s y, Ipp32s scaleFactor) {
   return (Ipp16s)((x * y) >> scaleFactor);
}
__INLINE Ipp32s Mul_32s(Ipp32s x,Ipp32s y) {
   Ipp32s z,z1,z2;
   Ipp16s xh, xl, yh, yl;
   xh  = (Ipp16s)(x >> 15);
   yh  = (Ipp16s)(y >> 15);
   xl  = (Ipp16s)(x & 0x7fff);
   yl  = (Ipp16s)(y & 0x7fff);
   z1  = Mul_16s_Sfs(xh,yl,15);
   z2  = Mul_16s_Sfs(xl,yh,15);
   z   = xh * yh;
   z   += z1;
   z   += z2;
   return (z<<1);
}
__INLINE Ipp32s Mul4_32s(Ipp32s x) {
    if(x > IPP_MAX_32S/4) return IPP_MAX_32S;
    else if( x < IPP_MIN_32S/4) return IPP_MIN_32S;
    return x <<= 2;
}
__INLINE Ipp32s Mul_32s16s(Ipp32s x, Ipp16s y) {
   short xh, xl;
   xh  = (short)(x >> 15);
   xl  = (short)(x & 0x7fff);
   return ((xh*y)+((xl*y)>>15));
}
__INLINE Ipp16s Add_16s(Ipp16s x, Ipp16s y) {
   return Cnvrt_32s16s((Ipp32s)x + (Ipp32s)y);
}
__INLINE Ipp16s Sub_16s(Ipp16s x, Ipp16s y) {
   return Cnvrt_32s16s((Ipp32s)x - (Ipp32s)y);
}

__INLINE Ipp32s Sub_32s(Ipp32s x,Ipp32s y){
   return Cnvrt_64s32s((__INT64)x - y);
}

__INLINE Ipp32s ownSqrt_32s( Ipp32s n )
{
    Ipp32s   i  ;

    Ipp16s  x =  0 ;
    Ipp16s  y =  0x4000 ;

    Ipp32s   z ;

    for ( i = 0 ; i < 14 ; i ++ ) {
        z = (x + y) * (x + y ) ;
        if ( n >= z )
            x = (Ipp16s)(x + y);
        y >>= 1 ;
    }
    return x;
}

__INLINE Ipp32s Exp_64s_Pos(Ipp64u x)
{
   if (x == 0) return 0;
   if((x>>32)==0) return (Ipp32s)(32+Exp_32s_Pos((Ipp32u) x));
   return (Ipp32s)Exp_32s_Pos((Ipp32u)(x>>32));
}


__INLINE Ipp32s Norm_64s(Ipp64s x){
   Ipp32s i;
   if (x == 0) return 0;
   if (x == -1) return 63;
   if (x < 0) x = ~x;
   i = (Ipp32s)Exp_64s_Pos((Ipp64u)x);
   return i;
}
__INLINE Ipp32s Norm_32s(Ipp32s x){
   Ipp32s i;
   if (x == 0) return 0;
   if (x == -1) return 31;
   if (x < 0) x = ~x;
   i = (Ipp32s)Exp_32s_Pos((Ipp32u)x);
   return i;
}

__INLINE Ipp16s Div_16s(Ipp16s num, Ipp16s den)
{
   if ((num < den) && (num > 0) && (den > 0)) {
      return (Ipp16s)( (((Ipp32s)num)<<15) / den);
   } else if ((den != 0) && (num == den)) {
      return IPP_MAX_16S;
   }
   return 0;
}
__INLINE short Div_32s16s(int num32, short den)
{
  return (den>0) ? (short)((num32>>1)/den) : IPP_MAX_16S; /* div: den # 0*/
}

extern CONST short LostFrame[200];

#endif /* __AUX_FNXS__ */
