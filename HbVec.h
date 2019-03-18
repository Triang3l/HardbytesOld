#ifndef HbInclude_HbVec
#define HbInclude_HbVec
#include "HbCommon.h"

#if HbPlatform_CPU_x86
#include <emmintrin.h>

/******************
 * SSE2 intrinsics
 ******************/

typedef __m128 HbVec_F32x4;
typedef __m128i HbVec_S32x4;
typedef __m128i HbVec_U32x4;

#define HbVec_F32x4_LoadAligned _mm_load_ps
#define HbVec_S32x4_LoadAligned _mm_load_si128
#define HbVec_U32x4_LoadAligned _mm_load_si128
#define HbVec_F32x4_LoadUnaligned _mm_loadu_ps
#define HbVec_S32x4_LoadUnaligned _mm_loadu_si128
#define HbVec_U32x4_LoadUnaligned _mm_loadu_si128
HbForceInline HbVec_F32x4 HbVec_F32x4_LoadReplicated(float f) { return _mm_load_ps1(&f); }
#define HbVec_F32x4_StoreAligned _mm_store_ps
#define HbVec_S32x4_StoreAligned _mm_store_si128
#define HbVec_U32x4_StoreAligned _mm_store_si128
#define HbVec_F32x4_StoreUnaligned _mm_storeu_ps
#define HbVec_S32x4_StoreUnaligned _mm_storeu_si128
#define HbVec_U32x4_StoreUnaligned _mm_storeu_si128

HbForceInline HbVec_F32x4 HbVec_F32x4_ReplicateX(HbVec_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0)); }
HbForceInline HbVec_F32x4 HbVec_F32x4_ReplicateY(HbVec_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)); }
HbForceInline HbVec_F32x4 HbVec_F32x4_ReplicateZ(HbVec_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)); }
HbForceInline HbVec_F32x4 HbVec_F32x4_ReplicateW(HbVec_F32x4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)); }
#define HbVec_S32x4_ReplicateX(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 0, 0, 0))
#define HbVec_U32x4_ReplicateX(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 0, 0, 0))
#define HbVec_S32x4_ReplicateY(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(1, 1, 1, 1))
#define HbVec_U32x4_ReplicateY(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(1, 1, 1, 1))
#define HbVec_S32x4_ReplicateZ(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 2, 2, 2))
#define HbVec_U32x4_ReplicateZ(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 2, 2, 2))
#define HbVec_S32x4_ReplicateW(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 3, 3))
#define HbVec_U32x4_ReplicateW(v) _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 3, 3))

#define HbVec_F32x4_BitsAsS32x4 _mm_castps_si128
#define HbVec_F32x4_BitsAsU32x4 _mm_castps_si128
#define HbVec_S32x4_BitsAsF32x4 _mm_castsi128_ps
#define HbVec_S32x4_BitsAsU32x4(v) (v)
#define HbVec_U32x4_BitsAsF32x4 _mm_castsi128_ps
#define HbVec_U32x4_BitsAsS32x4(v) (v)
#define HbVec_F32x4_ConvertToS32x4 _mm_cvtps_epi32
#define HbVec_S32x4_ConvertToF32x4 _mm_cvtepi32_ps

#define HbVec_F32x4_Add _mm_add_ps
#define HbVec_S32x4_Add _mm_add_epi32
#define HbVec_U32x4_Add _mm_add_epi32
#define HbVec_F32x4_Subtract _mm_sub_ps
#define HbVec_S32x4_Subtract _mm_sub_epi32
#define HbVec_U32x4_Subtract _mm_sub_epi32
#define HbVec_F32x4_Multiply _mm_mul_ps
#define HbVec_U32x4_Multiply _mm_mul_epu32
HbForceInline HbVec_F32x4 HbVec_F32x4_MultiplyAdd(HbVec_F32x4 add, HbVec_F32x4 mul1, HbVec_F32x4 mul2) {
	return HbVec_F32x4_Add(add, HbVec_F32x4_Multiply(mul1, mul2));
}
// #define HbVec_F32x4_MultiplyAdd_Combined 0
HbForceInline HbVec_U32x4 HbVec_U32x4_MultiplyAdd(HbVec_U32x4 add, HbVec_U32x4 mul1, HbVec_U32x4 mul2) {
	return HbVec_U32x4_Add(add, HbVec_U32x4_Multiply(mul1, mul2));
}
// #define HbVec_U32x4_MultiplyAdd_Combined 0

#else
#error No HbVec vector intrinsics for the target platform.
#endif

#endif
