#ifndef HbInclude_HbMath
#define HbInclude_HbMath
#include "HbCommon.h"
#if HbPlatform_CPU_x86
#include <pmmintrin.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

/* Some portions are taken from DirectXMath.
 *
 * Copyright (c) 2011-2019 Microsoft Corp
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
 * permit persons to whom the Software is furnished to do so, subject to the following 
 * conditions: 
 *
 * The above copyright notice and this permission notice shall be included in all copies 
 * or substantial portions of the Software.  
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// World coordinates are based on screen (device) coordinates:
// +X - right on screen, east in world.
// +Y - up on screen, up in world.
// +Z - forward on screen, north in world.

#define HbMath_VecAligned HbAligned(16)

#define HbMath_F64_Sqrt2 1.41421356237309504880
#define HbMath_F32_Sqrt2 ((float) HbMath_F64_Sqrt2)
#define HbMath_F64_InverseSqrt2 0.707106781186547524401
#define HbMath_F32_InverseSqrt2 ((float) HbMath_F64_InverseSqrt2)

#define HbMath_F64_Pi 3.14159265358979323846
#define HbMath_F32_Pi ((float) HbMath_F64_Pi)
#define HbMath_F64_InversePi 0.318309886183790671538
#define HbMath_F32_InversePi ((float) HbMath_F64_InversePi)
extern HbMath_VecAligned float const HbMath_F32x4_PiConstants[4]; // X: pi, Y: pi/2, Z: 2pi, W: 0.5/pi.
#define HbMath_F32_Deg2Rad (HbMath_F32_Pi * (1.0f / 180.0f))
#define HbMath_F32_Rad2Deg (180.0f * HbMath_F32_InversePi)

// For 11-degree sine and 10-degree cosine minimax approximations, from DirectXMath XMScalarSinCos/XMVectorSinCos.
#define HbMath_F32_Sin_C1 -2.3889859e-8f
#define HbMath_F32_Sin_C2 2.7525562e-6f
#define HbMath_F32_Sin_C3 -0.00019840874f
#define HbMath_F32_Sin_C4 0.008333331f
#define HbMath_F32_Sin_C5 -0.16666667f
#define HbMath_F32_Sin_C6 1.0f
#define HbMath_F32_Cos_C1 -2.6051615e-7f
#define HbMath_F32_Cos_C2 2.4760495e-5f
#define HbMath_F32_Cos_C3 -0.0013888378f
#define HbMath_F32_Cos_C4 0.041666638f
#define HbMath_F32_Cos_C5 -0.5f
#define HbMath_F32_Cos_C6 1.0f
extern HbMath_VecAligned float const HbMath_F32x4_Sin_Constants1[4]; // C1, C2, C3, C4.
extern HbMath_VecAligned float const HbMath_F32x4_Cos_Constants1[4]; // C1, C2, C3, C4.
extern HbMath_VecAligned float const HbMath_F32x4_SinCos_Constants2[4]; // Sin C5, Cos C5, 1 (C6), -1.

#if HbPlatform_CPU_x86

/******************
 * SSE3 intrinsics
 ******************/

typedef __m128 HbMath_F32x4;
typedef __m128i HbMath_S32x4;
typedef __m128i HbMath_U32x4;

#define HbMath_F32x4_LoadZero _mm_setzero_ps
#define HbMath_S32x4_LoadZero _mm_setzero_si128
#define HbMath_U32x4_LoadZero _mm_setzero_si128

#define HbMath_F32x4_LoadAligned _mm_load_ps
#define HbMath_S32x4_LoadAligned _mm_load_si128
#define HbMath_U32x4_LoadAligned HbMath_S32x4_LoadAligned
#define HbMath_F32x4_LoadUnaligned _mm_loadu_ps
#define HbMath_S32x4_LoadUnaligned _mm_loadu_si128
#define HbMath_U32x4_LoadUnaligned HbMath_S32x4_LoadUnaligned
#define HbMath_F32x4_LoadReplicated _mm_set_ps1
#define HbMath_S32x4_LoadReplicated _mm_set1_epi32
#define HbMath_U32x4_LoadReplicated(value) _mm_set1_epi32((int32_t) (value))
#define HbMath_F32x4_LoadX000 _mm_set_ss
#define HbMath_F32x4_StoreAligned _mm_store_ps
#define HbMath_S32x4_StoreAligned _mm_store_si128
#define HbMath_U32x4_StoreAligned HbMath_S32x4_StoreAligned
#define HbMath_F32x4_StoreUnaligned _mm_storeu_ps
#define HbMath_S32x4_StoreUnaligned _mm_storeu_si128
#define HbMath_U32x4_StoreUnaligned HbMath_S32x4_StoreUnaligned

HbForceInline HbMath_F32x4 HbMath_F32x4_ReplicateX(HbMath_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0)); }
HbForceInline HbMath_F32x4 HbMath_F32x4_ReplicateY(HbMath_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)); }
HbForceInline HbMath_F32x4 HbMath_F32x4_ReplicateZ(HbMath_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)); }
HbForceInline HbMath_F32x4 HbMath_F32x4_ReplicateW(HbMath_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)); }
#define HbMath_S32x4_ReplicateX(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 0, 0, 0))
#define HbMath_U32x4_ReplicateX HbMath_S32x4_ReplicateX
#define HbMath_S32x4_ReplicateY(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(1, 1, 1, 1))
#define HbMath_U32x4_ReplicateY HbMath_S32x4_ReplicateY
#define HbMath_S32x4_ReplicateZ(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 2, 2, 2))
#define HbMath_U32x4_ReplicateZ HbMath_S32x4_ReplicateZ
#define HbMath_S32x4_ReplicateW(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 3, 3))
#define HbMath_U32x4_ReplicateW HbMath_S32x4_ReplicateW

#define HbMath_F32x4_StoreX _mm_store_ss
#define HbMath_F32x4_StoreY(p, v) _mm_store_ss(p, HbMath_F32x4_ReplicateY(v))
#define HbMath_F32x4_StoreZ(p, v) _mm_store_ss(p, HbMath_F32x4_ReplicateZ(v))
#define HbMath_F32x4_StoreW(p, v) _mm_store_ss(p, HbMath_F32x4_ReplicateW(v))

HbForceInline HbMath_F32x4 HbMath_F32x4_RotateYZWX(HbMath_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 3, 2, 1)); }
HbForceInline HbMath_F32x4 HbMath_F32x4_RotateZWXY(HbMath_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 0, 3, 2)); }
HbForceInline HbMath_F32x4 HbMath_F32x4_RotateWXYZ(HbMath_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 1, 0, 3)); }
#define HbMath_S32x4_RotateYZWX(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 3, 2, 1))
#define HbMath_U32x4_RotateYZWX HbMath_S32x4_RotateYZWX
#define HbMath_S32x4_RotateZWXY(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(1, 0, 3, 2))
#define HbMath_U32x4_RotateZWXY HbMath_S32x4_RotateZWXY
#define HbMath_S32x4_RotateWXYZ(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 1, 0, 3))
#define HbMath_U32x4_RotateWXYZ HbMath_S32x4_RotateWXYZ

#define HbMath_F32x4_ReplaceX _mm_move_ss

#define HbMath_F32x4_CombineXYXY _mm_movelh_ps
#define HbMath_F32x4_CombineZWZW(a, b) _mm_movehl_ps(b, a)

#define HbMath_F32x4_BitsAsS32x4 _mm_castps_si128
#define HbMath_F32x4_BitsAsU32x4 HbMath_F32x4_BitsAsS32x4
#define HbMath_S32x4_BitsAsF32x4 _mm_castsi128_ps
#define HbMath_S32x4_BitsAsU32x4(v) (v)
#define HbMath_U32x4_BitsAsF32x4 HbMath_S32x4_BitsAsF32x4
#define HbMath_U32x4_BitsAsS32x4(v) (v)
#define HbMath_F32x4_ConvertToS32x4 _mm_cvttps_epi32
#define HbMath_S32x4_ConvertToF32x4 _mm_cvtepi32_ps

#define HbMath_F32x4_CompareLess _mm_cmplt_ps
#define HbMath_F32x4_CompareEqual _mm_cmpeq_ps
#define HbMath_F32x4_CompareLessEqual _mm_cmple_ps
#define HbMath_F32x4_CompareGreater _mm_cmpgt_ps
#define HbMath_F32x4_CompareNotEqual _mm_cmpneq_ps
#define HbMath_F32x4_CompareGreaterEqual _mm_cmpge_ps

#define HbMath_S32x4_CompareLess _mm_cmplt_epi32
#define HbMath_S32x4_CompareEqual _mm_cmpeq_epi32
#define HbMath_U32x4_CompareEqual HbMath_S32x4_CompareEqual
#define HbMath_S32x4_CompareGreater _mm_cmpgt_epi32

#define HbMath_F32x4_And _mm_and_ps
#define HbMath_S32x4_And _mm_and_si128
#define HbMath_U32x4_And HbMath_S32x4_And
// a & ~b that makes sense, similar to NEON vbicq_u32.
#define HbMath_F32x4_AndNot(a, b) _mm_andnot_ps(b, a)
#define HbMath_S32x4_AndNot(a, b) _mm_andnot_si128(b, a)
#define HbMath_U32x4_AndNot HbMath_S32x4_AndNot
#define HbMath_F32x4_Or _mm_or_ps
#define HbMath_S32x4_Or _mm_or_si128
#define HbMath_U32x4_Or HbMath_S32x4_Or
#define HbMath_F32x4_Xor _mm_xor_ps
#define HbMath_S32x4_Xor _mm_xor_si128
#define HbMath_U32x4_Xor HbMath_S32x4_Xor
HbForceInline HbMath_F32x4 HbMath_F32x4_Select(HbMath_F32x4 mask, HbMath_F32x4 a, HbMath_F32x4 b) {
	return HbMath_F32x4_Or(HbMath_F32x4_And(a, mask), HbMath_F32x4_AndNot(b, mask));
}
HbForceInline HbMath_S32x4 HbMath_S32x4_Select(HbMath_S32x4 mask, HbMath_S32x4 a, HbMath_S32x4 b) {
	return HbMath_S32x4_Or(HbMath_S32x4_And(a, mask), HbMath_S32x4_AndNot(b, mask));
}
#define HbMath_U32x4_Select HbMath_S32x4_Select

#define HbMath_F32x4_Add _mm_add_ps
#define HbMath_S32x4_Add _mm_add_epi32
#define HbMath_U32x4_Add HbMath_S32x4_Add
#define HbMath_F32x4_Subtract _mm_sub_ps
#define HbMath_S32x4_Subtract _mm_sub_epi32
#define HbMath_U32x4_Subtract HbMath_S32x4_Subtract
#define HbMath_F32x4_Multiply _mm_mul_ps
#define HbMath_U32x4_Multiply _mm_mul_epu32
#define HbMath_F32x4_MultiplyAdd(add, mul1, mul2) HbMath_F32x4_Add(add, HbMath_F32x4_Multiply(mul1, mul2))
// #define HbMath_F32x4_MultiplyAdd_Combined 0
#define HbMath_U32x4_MultiplyAdd(add, mul1, mul2) HbMath_U32x4_Add(add, HbMath_U32x4_Multiply(mul1, mul2))
// #define HbMath_U32x4_MultiplyAdd_Combined 0
#define HbMath_F32x4_Min _mm_min_ps
#define HbMath_F32x4_Max _mm_max_ps
#define HbMath_F32x4_Negate(v) HbMath_F32x4_Subtract(HbMath_F32x4_LoadZero(), v)
HbForceInline HbMath_F32x4 HbMath_F32x4_Absolute(HbMath_F32x4 v) { return HbMath_F32x4_Max(v, HbMath_F32x4_Negate(v)); }
#define HbMath_F32x4_InverseCoarse _mm_rcp_ps
#define HbMath_F32x4_InverseFine(v) _mm_div_ps(HbMath_F32x4_LoadReplicated(1.0f), v)
#define HbMath_F32x4_DivideCoarse(a, b) HbMath_F32x4_Multiply(a, HbMath_F32x4_InverseCoarse(b))
#define HbMath_F32x4_DivideFine _mm_div_ps
#define HbMath_F32x4_InverseSqrtCoarse _mm_rsqrt_ps
#define HbMath_F32x4_InverseSqrtFine(v) HbMath_F32x4_InverseFine(_mm_sqrt_ps(v))
HbForceInline HbMath_F32x4 HbMath_F32x4_SqrtCoarse(HbMath_F32x4 v) { return HbMath_F32x4_Multiply(v, HbMath_F32x4_InverseSqrtCoarse(v)); }
#define HbMath_F32x4_SqrtFine _mm_sqrt_ps

HbForceInline float HbMath_F32_InverseCoarse(float f) { float result; _mm_store_ss(&result, _mm_rcp_ss(_mm_set_ss(f))); return result; }
HbForceInline float HbMath_F32_InverseSqrtCoarse(float f) { float result; _mm_store_ss(&result, _mm_rsqrt_ss(_mm_set_ss(f))); return result; }

// Lane 3 will be 0 if a.w or b.w is 0.
HbForceInline HbMath_F32x4 HbMath_F32x4_Cross(HbMath_F32x4 a, HbMath_F32x4 b) {
	// x = a.y * b.z - a.z * b.y
	// y = a.z * b.x - a.x * b.z
	// z = a.x * b.y - a.y * b.x
	return HbMath_F32x4_Subtract(
			HbMath_F32x4_Multiply(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2))),
			HbMath_F32x4_Multiply(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1))));
}

#else
#error No HbMath vector intrinsics for the target platform.
#endif

HbForceInline float HbMath_F32_DivideCoarse(float a, float b) { return a * HbMath_F32_InverseCoarse(b); }
HbForceInline float HbMath_F32_SqrtCoarse(float f) { return f * HbMath_F32_InverseSqrtCoarse(f); }

// Only safe for values numbers whose absolute value is smaller than 2^31. Rounds the human way (0.5 to 1).
// Inspired by Stephanie Rancourt's method (under the Boost Software License), but with multiplication by 2 instead of 1.99999988079071044921875.
// http://dss.stephanierct.com/DevBlog/?p=8
// (Damn Visual Studio, why do you have to add the whole BOM just for a single e-acute in a comment?)
HbForceInline HbMath_F32x4 HbMath_F32x4_RoundToNearest(HbMath_F32x4 v) {
	HbMath_F32x4 truncated = HbMath_S32x4_ConvertToF32x4(HbMath_F32x4_ConvertToS32x4(v));
	HbMath_F32x4 fractional = HbMath_F32x4_Subtract(v, truncated);
	return HbMath_F32x4_Add(truncated, HbMath_S32x4_ConvertToF32x4(HbMath_F32x4_ConvertToS32x4(HbMath_F32x4_Add(fractional, fractional))));
}

/***************
 * Trigonometry
 ***************/

// Based on DirectXMath XMScalar/VectorSinCos.

HbForceInline float HbMath_F32_AngleToPlusMinusPi(float angle) {
	return angle - roundf(angle * (0.5f * HbMath_F32_InversePi)) * (2.0f * HbMath_F32_Pi);
}
HbForceInline HbMath_F32x4 HbMath_F32x4_AnglesToPlusMinusPi_Loaded(HbMath_F32x4 angles, HbMath_F32x4 piConstants) {
	HbMath_F32x4 normalized = HbMath_F32x4_Multiply(angles, HbMath_F32x4_ReplicateW(piConstants));
	return HbMath_F32x4_Subtract(angles, HbMath_F32x4_Multiply(HbMath_F32x4_RoundToNearest(normalized), HbMath_F32x4_ReplicateZ(piConstants)));
}
HbForceInline HbMath_F32x4 HbMath_F32x4_AnglesToPlusMinusPi(HbMath_F32x4 angles) {
	return HbMath_F32x4_AnglesToPlusMinusPi_Loaded(angles, HbMath_F32x4_LoadAligned(HbMath_F32x4_PiConstants));
}

HbForceInline HbMath_F32x4 HbMath_F32x4_Sin_Loaded(HbMath_F32x4 x, HbMath_F32x4 piConstants, HbMath_F32x4 sinConstants1, HbMath_F32x4 sinCosConstants2) {
	// Map in [-pi, pi].
	x = HbMath_F32x4_AnglesToPlusMinusPi_Loaded(x, piConstants);
	// Map in [-pi/2, pi/2] with sin(y) = sin(x).
	HbMath_F32x4 xAbs = HbMath_F32x4_Absolute(x);
	HbMath_F32x4 xSigns = HbMath_F32x4_AndNot(x, xAbs);
	HbMath_F32x4 xReflected = HbMath_F32x4_Subtract(HbMath_F32x4_Or(HbMath_F32x4_ReplicateX(piConstants), xSigns), x);
	HbMath_F32x4 inLeftHalf = HbMath_F32x4_CompareGreater(xAbs, HbMath_F32x4_ReplicateY(piConstants));
	x = HbMath_F32x4_Select(inLeftHalf, xReflected, x);
	HbMath_F32x4 x2 = HbMath_F32x4_Multiply(x, x);
	// Compute polynomial approximation.
	HbMath_F32x4 result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateY(sinConstants1), HbMath_F32x4_ReplicateX(sinConstants1), x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateZ(sinConstants1), result, x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateW(sinConstants1), result, x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateX(sinCosConstants2), result, x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateZ(sinCosConstants2), result, x2);
	result = HbMath_F32x4_Multiply(result, x);
	return result;
}
HbForceInline HbMath_F32x4 HbMath_F32x4_Sin(HbMath_F32x4 x) {
	return HbMath_F32x4_Sin_Loaded(x, HbMath_F32x4_LoadAligned(HbMath_F32x4_PiConstants),
			HbMath_F32x4_LoadAligned(HbMath_F32x4_Sin_Constants1), HbMath_F32x4_LoadAligned(HbMath_F32x4_SinCos_Constants2));
}

HbForceInline HbMath_F32x4 HbMath_F32x4_Cos_Loaded(HbMath_F32x4 x, HbMath_F32x4 piConstants, HbMath_F32x4 cosConstants1, HbMath_F32x4 sinCosConstants2) {
	// Map in [-pi, pi].
	x = HbMath_F32x4_AnglesToPlusMinusPi_Loaded(x, piConstants);
	// Map in [-pi/2, pi/2] with cos(y) = (inLeftHalf ? -1 : 1) * cos(x).
	HbMath_F32x4 xAbs = HbMath_F32x4_Absolute(x);
	HbMath_F32x4 xSigns = HbMath_F32x4_AndNot(x, xAbs);
	HbMath_F32x4 xReflected = HbMath_F32x4_Subtract(HbMath_F32x4_Or(HbMath_F32x4_ReplicateX(piConstants), xSigns), x);
	HbMath_F32x4 inLeftHalf = HbMath_F32x4_CompareGreater(xAbs, HbMath_F32x4_ReplicateY(piConstants));
	x = HbMath_F32x4_Select(inLeftHalf, xReflected, x);
	HbMath_F32x4 x2 = HbMath_F32x4_Multiply(x, x);
	// Compute polynomial approximation.
	HbMath_F32x4 result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateY(cosConstants1), HbMath_F32x4_ReplicateX(cosConstants1), x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateZ(cosConstants1), result, x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateW(cosConstants1), result, x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateY(sinCosConstants2), result, x2);
	HbMath_F32x4 ones = HbMath_F32x4_ReplicateZ(sinCosConstants2);
	result = HbMath_F32x4_MultiplyAdd(ones, result, x2);
	result = HbMath_F32x4_Multiply(result, HbMath_F32x4_Select(inLeftHalf, HbMath_F32x4_ReplicateW(sinCosConstants2), ones));
	return result;
}
HbForceInline HbMath_F32x4 HbMath_F32x4_Cos(HbMath_F32x4 x) {
	return HbMath_F32x4_Cos_Loaded(x, HbMath_F32x4_LoadAligned(HbMath_F32x4_PiConstants),
			HbMath_F32x4_LoadAligned(HbMath_F32x4_Cos_Constants1), HbMath_F32x4_LoadAligned(HbMath_F32x4_SinCos_Constants2));
}

// X1, X2, Y1, Y2 -> sin(X1), sin(X2), cos(Y1), cos(Y2). For two sines/cosines, call with X1 == Y1, X2 == Y2.
HbForceInline HbMath_F32x4 HbMath_F32x4_SinCosX2_Loaded(HbMath_F32x4 x, HbMath_F32x4 piConstants,
		HbMath_F32x4 sinConstants1, HbMath_F32x4 cosConstants1, HbMath_F32x4 sinCosConstants2) {
	// Map in [-pi, pi].
	x = HbMath_F32x4_AnglesToPlusMinusPi_Loaded(x, piConstants);
	// Map in [-pi/2, pi/2] with sin(y) = sin(x), cos(y) = (inLeftHalf ? -1 : 1) * cos(x).
	HbMath_F32x4 xAbs = HbMath_F32x4_Absolute(x);
	HbMath_F32x4 xSigns = HbMath_F32x4_AndNot(x, xAbs);
	HbMath_F32x4 xReflected = HbMath_F32x4_Subtract(HbMath_F32x4_Or(HbMath_F32x4_ReplicateX(piConstants), xSigns), x);
	HbMath_F32x4 inLeftHalf = HbMath_F32x4_CompareGreater(xAbs, HbMath_F32x4_ReplicateY(piConstants));
	x = HbMath_F32x4_Select(inLeftHalf, xReflected, x);
	HbMath_F32x4 x2 = HbMath_F32x4_Multiply(x, x);
	// Compute polynomial approximations.
	#if HbPlatform_CPU_x86
	HbMath_F32x4 result = HbMath_F32x4_MultiplyAdd(_mm_shuffle_ps(sinConstants1, cosConstants1, _MM_SHUFFLE(1, 1, 1, 1)),
			_mm_shuffle_ps(sinConstants1, cosConstants1, _MM_SHUFFLE(0, 0, 0, 0)), x2);
	result = HbMath_F32x4_MultiplyAdd(_mm_shuffle_ps(sinConstants1, cosConstants1, _MM_SHUFFLE(2, 2, 2, 2)), result, x2);
	result = HbMath_F32x4_MultiplyAdd(_mm_shuffle_ps(sinConstants1, cosConstants1, _MM_SHUFFLE(3, 3, 3, 3)), result, x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_CombineXYXY(sinCosConstants2, sinCosConstants2), result, x2);
	HbMath_F32x4 ones = HbMath_F32x4_ReplicateZ(sinCosConstants2);
	result = HbMath_F32x4_MultiplyAdd(ones, result, x2);
	result = HbMath_F32x4_Multiply(result,
			_mm_shuffle_ps(x, HbMath_F32x4_Select(inLeftHalf, HbMath_F32x4_ReplicateW(sinCosConstants2), ones), _MM_SHUFFLE(3, 2, 1, 0)));
	#else
	#error No shuffles for the target CPU in HbMath_F32x4_SinCosX2_Loaded.
	#endif
	return result;
}
HbForceInline HbMath_F32x4 HbMath_F32x4_SinCosX2(HbMath_F32x4 x) {
	return HbMath_F32x4_SinCosX2_Loaded(x, HbMath_F32x4_LoadAligned(HbMath_F32x4_PiConstants), HbMath_F32x4_LoadAligned(HbMath_F32x4_Sin_Constants1),
			HbMath_F32x4_LoadAligned(HbMath_F32x4_Cos_Constants1), HbMath_F32x4_LoadAligned(HbMath_F32x4_SinCos_Constants2));
}

HbForceInline void HbMath_F32x4_SinCosX4_Loaded(HbMath_F32x4 x, HbMath_F32x4 * sine, HbMath_F32x4 * cosine,
		HbMath_F32x4 piConstants, HbMath_F32x4 sinConstants1, HbMath_F32x4 cosConstants1, HbMath_F32x4 sinCosConstants2) {
	// Map in [-pi, pi].
	x = HbMath_F32x4_AnglesToPlusMinusPi_Loaded(x, piConstants);
	// Map in [-pi/2, pi/2] with sin(y) = sin(x), cos(y) = (inLeftHalf ? -1 : 1) * cos(x).
	HbMath_F32x4 xAbs = HbMath_F32x4_Absolute(x);
	HbMath_F32x4 xSigns = HbMath_F32x4_AndNot(x, xAbs);
	HbMath_F32x4 xReflected = HbMath_F32x4_Subtract(HbMath_F32x4_Or(HbMath_F32x4_ReplicateX(piConstants), xSigns), x);
	HbMath_F32x4 inLeftHalf = HbMath_F32x4_CompareGreater(xAbs, HbMath_F32x4_ReplicateY(piConstants));
	x = HbMath_F32x4_Select(inLeftHalf, xReflected, x);
	HbMath_F32x4 x2 = HbMath_F32x4_Multiply(x, x);
	HbMath_F32x4 ones = HbMath_F32x4_ReplicateZ(sinCosConstants2);
	// Compute polynomial approximation for sine.
	HbMath_F32x4 result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateY(sinConstants1), HbMath_F32x4_ReplicateX(sinConstants1), x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateZ(sinConstants1), result, x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateW(sinConstants1), result, x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateX(sinCosConstants2), result, x2);
	result = HbMath_F32x4_MultiplyAdd(ones, result, x2);
	result = HbMath_F32x4_Multiply(result, x);
	*sine = result;
	// Compute polynomial approximation for cosine.
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateY(cosConstants1), HbMath_F32x4_ReplicateX(cosConstants1), x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateZ(cosConstants1), result, x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateW(cosConstants1), result, x2);
	result = HbMath_F32x4_MultiplyAdd(HbMath_F32x4_ReplicateY(sinCosConstants2), result, x2);
	result = HbMath_F32x4_MultiplyAdd(ones, result, x2);
	result = HbMath_F32x4_Multiply(result, HbMath_F32x4_Select(inLeftHalf, HbMath_F32x4_ReplicateW(sinCosConstants2), ones));
	*cosine = result;
}
HbForceInline void HbMath_F32x4_SinCosX4(HbMath_F32x4 x, HbMath_F32x4 * sine, HbMath_F32x4 * cosine) {
	HbMath_F32x4_SinCosX4_Loaded(x, sine, cosine, HbMath_F32x4_LoadAligned(HbMath_F32x4_PiConstants), HbMath_F32x4_LoadAligned(HbMath_F32x4_Sin_Constants1),
			HbMath_F32x4_LoadAligned(HbMath_F32x4_Cos_Constants1), HbMath_F32x4_LoadAligned(HbMath_F32x4_SinCos_Constants2));
}

HbForceInline float HbMath_F32_Sin(float x) {
	// Map in [-pi, pi], x = 2*pi*quotient + remainder.
	x -= 2.0f * HbMath_F32_Pi * (float) (int32_t) ((0.5f * HbMath_F32_InversePi * x) + (x >= 0.0f ? 0.5f : -0.5f));
	// Map in [-pi/2,pi/2] with sin(y) = sin(x).
	if (x > 0.5f * HbMath_F32_Pi) {
		x = HbMath_F32_Pi - x;
	} else if (x < -0.5f * HbMath_F32_Pi) {
		x = -HbMath_F32_Pi - x;
	}
	// 11-degree minimax approximation.
	float x2 = x * x;
	return (((((HbMath_F32_Sin_C1 * x2 + HbMath_F32_Sin_C2) * x2 + HbMath_F32_Sin_C3) * x2 + HbMath_F32_Sin_C4) * x2 + HbMath_F32_Sin_C5) * x2 + HbMath_F32_Sin_C6) * x;
}
HbForceInline float HbMath_F32_Cos(float x) {
	// Map in [-pi, pi], x = 2*pi*quotient + remainder.
	x -= 2.0f * HbMath_F32_Pi * (float) (int32_t) ((0.5f * HbMath_F32_InversePi * x) + (x >= 0.0f ? 0.5f : -0.5f));
	// Map in [-pi/2,pi/2] with cos(y) = sign*cos(x).
	float sign;
	if (x > 0.5f * HbMath_F32_Pi) {
		x = HbMath_F32_Pi - x;
		sign = -1.0f;
	} else if (x < -0.5f * HbMath_F32_Pi) {
		x = -HbMath_F32_Pi - x;
		sign = -1.0f;
	} else {
		sign = 1.0f;
	}
	// 11-degree minimax approximation.
	float x2 = x * x;
	return (((((HbMath_F32_Cos_C1 * x2 + HbMath_F32_Cos_C2) * x2 + HbMath_F32_Cos_C3) * x2 + HbMath_F32_Cos_C4) * x2 + HbMath_F32_Cos_C5) * x2 + HbMath_F32_Cos_C6) * sign;
}
HbForceInline void HbMath_F32_SinCos(float x, float * sine, float * cosine) {
	HbMath_F32x4 sinCosVec = HbMath_F32x4_SinCosX2(HbMath_F32x4_LoadReplicated(x));
	HbMath_F32x4_StoreX(sine, sinCosVec);
	HbMath_F32x4_StoreZ(cosine, sinCosVec);
}

/************************************************
 * Quaternion, as WXYZ (so _ss can be used on W)
 ************************************************/

#define HbMath_F32x4_Quat_ReplicateW HbMath_F32x4_ReplicateX
#define HbMath_F32x4_Quat_ReplicateX HbMath_F32x4_ReplicateY
#define HbMath_F32x4_Quat_ReplicateY HbMath_F32x4_ReplicateZ
#define HbMath_F32x4_Quat_ReplicateZ HbMath_F32x4_ReplicateW

HbForceInline HbMath_F32x4 HbMath_F32x4_Quat_Conjugate(HbMath_F32x4 q) {
	#if HbPlatform_CPU_x86
	return HbMath_F32x4_ReplaceX(HbMath_F32x4_Negate(q), q);
	#else
	#error No HbMath_F32x4_QuatConjugate for the target CPU.
	#endif
}

// b * a
// w = b.w * a.w - b.x * a.x - b.y * a.y - b.z * a.z
// x = b.w * a.x + b.x * a.w + b.y * a.z - b.z * a.y
// y = b.w * a.y - b.x * a.z + b.y * a.w + b.z * a.x
// z = b.w * a.z + b.x * a.y - b.y * a.x + b.z * a.w
// or:
// w = (b.w * a.w - b.x * a.x) - (b.y * a.y + b.z * a.z)
// x = (b.w * a.x + b.x * a.w) + (b.y * a.z - b.z * a.y)
// y = (b.w * a.y - b.x * a.z) + (b.y * a.w + b.z * a.x)
// z = (b.w * a.z + b.x * a.y) - (b.y * a.x - b.z * a.w)
HbForceInline HbMath_F32x4 HbMath_F32x4_Quat_Concatenate(HbMath_F32x4 a, HbMath_F32x4 b) {
	HbMath_F32x4 axwzy = _mm_shuffle_ps(a, a, _MM_SHUFFLE(2, 3, 0, 1));
	#if HbPlatform_CPU_x86
	// t0[0] = b.w * a.w - b.x * a.x (for W)
	// t0[1] = b.w * a.x + b.x * a.w (for X)
	// t0[2] = b.w * a.y - b.x * a.z (for Y)
	// t0[3] = b.w * a.z + b.x * a.y (for Z)
	HbMath_F32x4 t0 = _mm_addsub_ps(
			HbMath_F32x4_Multiply(HbMath_F32x4_Quat_ReplicateW(b), a),
			HbMath_F32x4_Multiply(HbMath_F32x4_Quat_ReplicateX(b), axwzy));
	// t1[0] = b.y * a.x - b.z * a.w (for Z)
	// t1[1] = b.y * a.w + b.z * a.x (for Y)
	// t1[2] = b.y * a.z - b.z * a.y (for X)
	// t1[3] = b.y * a.y + b.z * a.z (for W)
	HbMath_F32x4 t1 = _mm_addsub_ps(
			HbMath_F32x4_Multiply(HbMath_F32x4_Quat_ReplicateY(b), axwzy),
			HbMath_F32x4_Multiply(HbMath_F32x4_Quat_ReplicateZ(b), a));
	// t0[0] = (b.w * a.w - b.x * a.x) - (b.y * a.y + b.z * a.z) (for W)
	// t0[1] = (b.w * a.x + b.x * a.w) + (b.y * a.z - b.z * a.y) (for X)
	// t0[2] = (b.w * a.z + b.x * a.y) - (b.y * a.x - b.z * a.w) (for Z)
	// to[3] = (b.w * a.y - b.x * a.z) + (b.y * a.w + b.z * a.x) (for Y)
	t0 = _mm_addsub_ps(_mm_shuffle_ps(t0, t0, _MM_SHUFFLE(2, 3, 1, 0)), _mm_shuffle_ps(t1, t1, _MM_SHUFFLE(1, 0, 2, 3)));
	return _mm_shuffle_ps(t0, t0, _MM_SHUFFLE(2, 3, 1, 0));
	#else
	#error No HbMath_F32x4_Quat_Concatenate for the target CPU.
	#endif
}

// Cross product of lanes 123 of Q and 012 of V, returned in lanes 012. Lane 3 will have 0 if v.w is 0.
HbForceInline HbMath_F32x4 HbMath_F32x4_Quat_CrossVector(HbMath_F32x4 q, HbMath_F32x4 v) {
	// x = q.y * v.z - q.z * v.y
	// y = q.z * v.x - q.x * v.z
	// z = q.x * v.y - q.y * v.x
	#if HbPlatform_CPU_x86
	return HbMath_F32x4_Subtract(
			HbMath_F32x4_Multiply(_mm_shuffle_ps(q, q, _MM_SHUFFLE(0, 1, 3, 2)), _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 1, 0, 2))),
			HbMath_F32x4_Multiply(_mm_shuffle_ps(q, q, _MM_SHUFFLE(0, 2, 1, 3)), _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 2, 1))));
	#else
	#error No HbMath_F32x4_Quat_CrossVector for the target CPU.
	#endif
}

// Cross product of lanes 123 of conjugate(Q) and 012 of V, returned in lanes 012. Lane 3 will have 0 if v.w is 0.
HbForceInline HbMath_F32x4 HbMath_F32x4_Quat_CrossVectorConjugate(HbMath_F32x4 q, HbMath_F32x4 v) {
	// x = -q.y * v.z - -q.z * v.y
	// y = -q.z * v.x - -q.x * v.z
	// z = -q.x * v.y - -q.y * v.x
	// Or:
	// x = q.z * v.y - q.y * v.z
	// y = q.x * v.z - q.z * v.x
	// z = q.y * v.x - q.x * v.y
	#if HbPlatform_CPU_x86
	return HbMath_F32x4_Subtract(
			HbMath_F32x4_Multiply(_mm_shuffle_ps(q, q, _MM_SHUFFLE(0, 2, 1, 3)), _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 2, 1))),
			HbMath_F32x4_Multiply(_mm_shuffle_ps(q, q, _MM_SHUFFLE(0, 1, 3, 2)), _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 1, 0, 2))));
	#else
	#error No HbMath_F32x4_Quat_CrossVector for the target CPU.
	#endif
}

// Lane 3 will have 0 if v.w is 0.
HbForceInline HbMath_F32x4 HbMath_F32x4_Quat_Apply(HbMath_F32x4 q, HbMath_F32x4 v) {
	// By Fabian @rygorous Giesen.
	// https://fgiesen.wordpress.com/2019/02/09/rotating-a-single-vector-using-a-quaternion/
	HbMath_F32x4 t = HbMath_F32x4_Quat_CrossVector(q, v);
	t = HbMath_F32x4_Add(t, t);
	return HbMath_F32x4_Add(HbMath_F32x4_Add(v, HbMath_F32x4_Multiply(HbMath_F32x4_Quat_ReplicateW(q), t)), HbMath_F32x4_Quat_CrossVector(q, t));
}

// Lane 3 will have 0 if v.w is 0.
HbForceInline HbMath_F32x4 HbMath_F32x4_Quat_ApplyConjugate(HbMath_F32x4 q, HbMath_F32x4 v) {
	// By Fabian @rygorous Giesen.
	// https://fgiesen.wordpress.com/2019/02/09/rotating-a-single-vector-using-a-quaternion/
	HbMath_F32x4 t = HbMath_F32x4_Quat_CrossVectorConjugate(q, v);
	t = HbMath_F32x4_Add(t, t);
	return HbMath_F32x4_Add(HbMath_F32x4_Add(v, HbMath_F32x4_Multiply(HbMath_F32x4_Quat_ReplicateW(q), t)), HbMath_F32x4_Quat_CrossVectorConjugate(q, t));
}

// Concatenated in the following order: roll around Z, then pitch around X, then yaw around Y.
// halfYawPitchRoll should preferably contain 0 in lane 3 to avoid calculations with bad floating-point values.
HbForceInline HbMath_F32x4 HbMath_F32x4_Quat_FromEulerHalf(HbMath_F32x4 halfYawPitchRoll) {
	// 1) [cz | 0 | 0 | sz] .. [cx | sx | 0 | 0] = [cx*cz | sx*cz | -sx*sz | cx*sz]
	// 2) [cx*cz | sx*cz | -sx*sz | cx*sz] .. [cy | 0 | sy | 0] =
	//    [cy*cx*cz + sy*sx*sz | cy*sx*cz + sy*cx*sz | - cy*sx*sz + sy*cx*cz | cy*cx*sz - sy*sx*cz]
	#if HbPlatform_CPU_x86
	HbMath_F32x4 sines, cosines;
	HbMath_F32x4_SinCosX4(halfYawPitchRoll, &sines, &cosines);
	HbMath_F32x4 x = _mm_addsub_ps(HbMath_F32x4_LoadZero(), _mm_shuffle_ps(sines, cosines, _MM_SHUFFLE(1, 1, 1, 1))); // [-sx | sx | -cx | cx]
	HbMath_F32x4 t0 = HbMath_F32x4_Multiply(HbMath_F32x4_ReplicateX(cosines), _mm_shuffle_ps(x, x, _MM_SHUFFLE(3, 0, 1, 3))); // [cy*cx | cy*sx | -cy*sx | cy*cx]
	t0 = HbMath_F32x4_Multiply(t0, _mm_shuffle_ps(cosines, sines, _MM_SHUFFLE(2, 2, 2, 2)));
	HbMath_F32x4 t1 = HbMath_F32x4_Multiply(HbMath_F32x4_ReplicateX(sines), _mm_shuffle_ps(x, x, _MM_SHUFFLE(0, 3, 3, 1))); // [sy*sx | sy*cx | sy*cx | -sy*sx]
	t1 = HbMath_F32x4_Multiply(t1, _mm_shuffle_ps(sines, cosines, _MM_SHUFFLE(2, 2, 2, 2)));
	return HbMath_F32x4_Add(t0, t1);
	#else
	#error No HbMath_F32x4_Quat_FromEulerHalf for the target CPU.
	#endif
}

#ifdef __cplusplus
}
#endif
#endif
