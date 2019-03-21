#ifndef HbInclude_HbMath
#define HbInclude_HbMath
#include "HbCommon.h"

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

#define HbMath_VecAligned HbAligned(16)

#define HbMath_F64_Pi 3.14159265358979323846
#define HbMath_F32_Pi ((float) HbMath_F64_Pi)
#define HbMath_F64_InvPi 0.318309886183790671538
#define HbMath_F32_InvPi ((float) HbMath_F64_InvPi)
extern HbMath_VecAligned float const HbMath_F32x4_PiConstants[4]; // X: pi, Y: pi/2, Z: 2pi, W: 0.5/pi.

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
#include <emmintrin.h>

/******************
 * SSE2 intrinsics
 ******************/

typedef __m128 HbMath_F32x4;
typedef __m128i HbMath_S32x4;
typedef __m128i HbMath_U32x4;

#define HbMath_F32x4_Zero _mm_setzero_ps()
#define HbMath_S32x4_Zero _mm_setzero_si128()
#define HbMath_U32x4_Zero _mm_setzero_si128()

#define HbMath_F32x4_LoadAligned _mm_load_ps
#define HbMath_S32x4_LoadAligned _mm_load_si128
#define HbMath_U32x4_LoadAligned _mm_load_si128
#define HbMath_F32x4_LoadUnaligned _mm_loadu_ps
#define HbMath_S32x4_LoadUnaligned _mm_loadu_si128
#define HbMath_U32x4_LoadUnaligned _mm_loadu_si128
#define HbMath_F32x4_LoadReplicated _mm_set_ps1
#define HbMath_S32x4_LoadReplicated _mm_set1_epi32
#define HbMath_U32x4_LoadReplicated(value) _mm_set1_epi32((int32_t) (value))
#define HbMath_F32x4_StoreAligned _mm_store_ps
#define HbMath_S32x4_StoreAligned _mm_store_si128
#define HbMath_U32x4_StoreAligned _mm_store_si128
#define HbMath_F32x4_StoreUnaligned _mm_storeu_ps
#define HbMath_S32x4_StoreUnaligned _mm_storeu_si128
#define HbMath_U32x4_StoreUnaligned _mm_storeu_si128

HbForceInline HbMath_F32x4 HbMath_F32x4_ReplicateX(HbMath_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0)); }
HbForceInline HbMath_F32x4 HbMath_F32x4_ReplicateY(HbMath_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)); }
HbForceInline HbMath_F32x4 HbMath_F32x4_ReplicateZ(HbMath_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)); }
HbForceInline HbMath_F32x4 HbMath_F32x4_ReplicateW(HbMath_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)); }
#define HbMath_S32x4_ReplicateX(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 0, 0, 0))
#define HbMath_U32x4_ReplicateX(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 0, 0, 0))
#define HbMath_S32x4_ReplicateY(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(1, 1, 1, 1))
#define HbMath_U32x4_ReplicateY(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(1, 1, 1, 1))
#define HbMath_S32x4_ReplicateZ(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 2, 2, 2))
#define HbMath_U32x4_ReplicateZ(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 2, 2, 2))
#define HbMath_S32x4_ReplicateW(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 3, 3))
#define HbMath_U32x4_ReplicateW(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 3, 3))

#define HbMath_F32x4_BitsAsS32x4 _mm_castps_si128
#define HbMath_F32x4_BitsAsU32x4 _mm_castps_si128
#define HbMath_S32x4_BitsAsF32x4 _mm_castsi128_ps
#define HbMath_S32x4_BitsAsU32x4(v) (v)
#define HbMath_U32x4_BitsAsF32x4 _mm_castsi128_ps
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
#define HbMath_U32x4_CompareEqual _mm_cmpeq_epi32
#define HbMath_S32x4_CompareGreater _mm_cmpgt_epi32

#define HbMath_F32x4_And _mm_and_ps
#define HbMath_S32x4_And _mm_and_si128
#define HbMath_U32x4_And _mm_and_si128
// a & ~b that makes sense, similar to NEON vbicq_u32.
#define HbMath_F32x4_AndNot(a, b) _mm_andnot_ps(b, a)
#define HbMath_S32x4_AndNot(a, b) _mm_andnot_si128(b, a)
#define HbMath_U32x4_AndNot(a, b) _mm_andnot_si128(b, a)
#define HbMath_F32x4_Or _mm_or_ps
#define HbMath_S32x4_Or _mm_or_si128
#define HbMath_U32x4_Or _mm_or_si128
#define HbMath_F32x4_Xor _mm_xor_ps
#define HbMath_S32x4_Xor _mm_xor_si128
#define HbMath_U32x4_Xor _mm_xor_si128
HbForceInline HbMath_F32x4 HbMath_F32x4_Select(HbMath_F32x4 mask, HbMath_F32x4 a, HbMath_F32x4 b) {
	return HbMath_F32x4_Or(HbMath_F32x4_And(a, mask), HbMath_F32x4_AndNot(b, mask));
}
HbForceInline HbMath_S32x4 HbMath_S32x4_Select(HbMath_S32x4 mask, HbMath_S32x4 a, HbMath_S32x4 b) {
	return HbMath_S32x4_Or(HbMath_S32x4_And(a, mask), HbMath_S32x4_AndNot(b, mask));
}
#define HbMath_U32x4_Select HbMath_S32x4_Select

#define HbMath_F32x4_Add _mm_add_ps
#define HbMath_S32x4_Add _mm_add_epi32
#define HbMath_U32x4_Add _mm_add_epi32
#define HbMath_F32x4_Subtract _mm_sub_ps
#define HbMath_S32x4_Subtract _mm_sub_epi32
#define HbMath_U32x4_Subtract _mm_sub_epi32
#define HbMath_F32x4_Multiply _mm_mul_ps
#define HbMath_U32x4_Multiply _mm_mul_epu32
#define HbMath_F32x4_MultiplyAdd(add, mul1, mul2) HbMath_F32x4_Add(add, HbMath_F32x4_Multiply(mul1, mul2))
// #define HbMath_F32x4_MultiplyAdd_Combined 0
#define HbMath_U32x4_MultiplyAdd(add, mul1, mul2) HbMath_U32x4_Add(add, HbMath_U32x4_Multiply(mul1, mul2))
// #define HbMath_U32x4_MultiplyAdd_Combined 0
#define HbMath_F32x4_Min _mm_min_ps
#define HbMath_F32x4_Max _mm_max_ps
#define HbMath_F32x4_Negate(v) HbMath_F32x4_Subtract(HbMath_F32x4_Zero, v)
HbForceInline HbMath_F32x4 HbMath_F32x4_Absolute(HbMath_F32x4 v) { return HbMath_F32x4_Max(v, HbMath_F32x4_Negate(v)); }

#else
#error No HbMath vector intrinsics for the target platform.
#endif

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
	result = HbMath_F32x4_MultiplyAdd(_mm_shuffle_ps(sinCosConstants2, sinCosConstants2, _MM_SHUFFLE(1, 0, 1, 0)), result, x2);
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
	HbMath_F32x4_SinCosX2_Loaded(x, HbMath_F32x4_LoadAligned(HbMath_F32x4_PiConstants), HbMath_F32x4_LoadAligned(HbMath_F32x4_Sin_Constants1),
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

#endif
