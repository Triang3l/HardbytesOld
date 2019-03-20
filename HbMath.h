#ifndef HbInclude_HbMath
#define HbInclude_HbMath
#include "HbCommon.h"

#if HbPlatform_CPU_x86
#include <emmintrin.h>

/******************
 * SSE2 intrinsics
 ******************/

typedef __m128 HbMath_F32x4;
typedef __m128i HbMath_S32x4;
typedef __m128i HbMath_U32x4;

#define HbMath_F32x4_LoadAligned _mm_load_ps
#define HbMath_S32x4_LoadAligned _mm_load_si128
#define HbMath_U32x4_LoadAligned _mm_load_si128
#define HbMath_F32x4_LoadUnaligned _mm_loadu_ps
#define HbMath_S32x4_LoadUnaligned _mm_loadu_si128
#define HbMath_U32x4_LoadUnaligned _mm_loadu_si128
HbForceInline HbMath_F32x4 HbMath_F32x4_LoadReplicated(float f) { return _mm_load_ps1(&f); }
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
#define HbMath_F32x4_ConvertToS32x4 _mm_cvtps_epi32
#define HbMath_S32x4_ConvertToF32x4 _mm_cvtepi32_ps

#define HbMath_F32x4_Add _mm_add_ps
#define HbMath_S32x4_Add _mm_add_epi32
#define HbMath_U32x4_Add _mm_add_epi32
#define HbMath_F32x4_Subtract _mm_sub_ps
#define HbMath_S32x4_Subtract _mm_sub_epi32
#define HbMath_U32x4_Subtract _mm_sub_epi32
#define HbMath_F32x4_Multiply _mm_mul_ps
#define HbMath_U32x4_Multiply _mm_mul_epu32
HbForceInline HbMath_F32x4 HbMath_F32x4_MultiplyAdd(HbMath_F32x4 add, HbMath_F32x4 mul1, HbMath_F32x4 mul2) {
	return HbMath_F32x4_Add(add, HbMath_F32x4_Multiply(mul1, mul2));
}
// #define HbMath_F32x4_MultiplyAdd_Combined 0
HbForceInline HbMath_U32x4 HbMath_U32x4_MultiplyAdd(HbMath_U32x4 add, HbMath_U32x4 mul1, HbMath_U32x4 mul2) {
	return HbMath_U32x4_Add(add, HbMath_U32x4_Multiply(mul1, mul2));
}
// #define HbMath_U32x4_MultiplyAdd_Combined 0

#else
#error No HbMath vector intrinsics for the target platform.
#endif

#endif
