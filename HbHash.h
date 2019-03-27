#ifndef HbInclude_HbHash
#define HbInclude_HbHash
#include "HbText.h"

#define HbHash_FNV1a_Basis 2166136261u
#define HbHash_FNV1a_Prime 16777619u

HbForceInline uint32_t HbHash_FNV1a_HashByte(uint32_t currentHash, uint8_t byte) {
	return (currentHash ^ byte) * HbHash_FNV1a_Prime;
}
inline uint32_t HbHash_FVN1a_HashTextA(char const * text) {
	uint32_t hash = HbHash_FNV1a_Basis;
	while (*text != '\0') {
		HbHash_FNV1a_HashByte(hash, (uint8_t) *(text++));
	}
	return hash;
}
inline uint32_t HbHash_FVN1a_HashTextACaseless(char const * text) {
	uint32_t hash = HbHash_FNV1a_Basis;
	while (*text != '\0') {
		HbHash_FNV1a_HashByte(hash, (uint8_t) HbTextA_CharToLower(*(text++)));
	}
	return hash;
}

// 8 (1 << 3) allows hash maps with no more than 4 entries, with a smaller limit the map needs to grow multiple times for a tiny count.
#define HbHash_Map_MinSizeLog2 3

uint32_t HbHash_Map_GetIndexMask32(uint32_t elementCount);

// For hash map collision resolution. Call iteratively until the collision has been resolved with the same parameters.
// Hash should initially point to the original 32-bit hash.
// Index should initially point to hash & mask.
// Mask is hash map size (power of 2) minus 1.
// From CPython dictobject.h.
// https://github.com/python/cpython/blob/master/LICENSE
HbForceInline void HbHash_Map_PerturbateIndex(uint32_t * hash, uint32_t * index, uint32_t mask) {
	*hash >>= 5;
	*index = ((*index * 5) + *hash + 1) & mask;
}

// DON'T ITERATE ON ALL BUCKETS USING A REGULAR FOR LOOP! With 0xFFFFFFFF index mask, the loop may be infinite (well, 4 billion iterations is too much anyway).

#endif
