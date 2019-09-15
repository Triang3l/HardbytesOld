#ifndef HbInclude_HbHash
#define HbInclude_HbHash
#include "HbBit.h"
#include "HbMemory.h"
#include "HbText.h"
#ifdef __cplusplus
extern "C" {
#endif

#define HbHash_FNV1a_Basis 2166136261u
#define HbHash_FNV1a_Prime 16777619u

HbForceInline uint32_t HbHash_FNV1a_HashByte(uint32_t currentHash, uint8_t byte) {
	return (currentHash ^ byte) * HbHash_FNV1a_Prime;
}
inline uint32_t HbHash_FNV1a_HashTextA(char const * text) {
	uint32_t hash = HbHash_FNV1a_Basis;
	while (*text != '\0') {
		HbHash_FNV1a_HashByte(hash, (uint8_t) *(text++));
	}
	return hash;
}
inline uint32_t HbHash_FNV1a_HashTextACaseless(char const * text) {
	uint32_t hash = HbHash_FNV1a_Basis;
	while (*text != '\0') {
		HbHash_FNV1a_HashByte(hash, (uint8_t) HbTextA_CharToLower(*(text++)));
	}
	return hash;
}

#define HbHash_FNV1a_CaseKey1(c0) \
		((HbHash_FNV1a_Basis ^ (uint8_t) HbTextA_CharToLowerDefine(c0)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey2(c0, c1) \
		((HbHash_FNV1a_CaseKey1(c0) ^ (uint8_t) HbTextA_CharToLowerDefine(c1)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey3(c0, c1, c2) \
		((HbHash_FNV1a_CaseKey2(c0, c1) ^ (uint8_t) HbTextA_CharToLowerDefine(c2)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey4(c0, c1, c2, c3) \
		((HbHash_FNV1a_CaseKey3(c0, c1, c2) ^ (uint8_t) HbTextA_CharToLowerDefine(c3)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey5(c0, c1, c2, c3, c4) \
		((HbHash_FNV1a_CaseKey4(c0, c1, c2, c3) ^ (uint8_t) HbTextA_CharToLowerDefine(c4)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey6(c0, c1, c2, c3, c4, c5) \
		((HbHash_FNV1a_CaseKey5(c0, c1, c2, c3, c4) ^ (uint8_t) HbTextA_CharToLowerDefine(c5)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey7(c0, c1, c2, c3, c4, c5, c6) \
		((HbHash_FNV1a_CaseKey6(c0, c1, c2, c3, c4, c5) ^ (uint8_t) HbTextA_CharToLowerDefine(c6)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey8(c0, c1, c2, c3, c4, c5, c6, c7) \
		((HbHash_FNV1a_CaseKey7(c0, c1, c2, c3, c4, c5, c6) ^ (uint8_t) HbTextA_CharToLowerDefine(c7)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey9(c0, c1, c2, c3, c4, c5, c6, c7, c8) \
		((HbHash_FNV1a_CaseKey8(c0, c1, c2, c3, c4, c5, c6, c7) ^ (uint8_t) HbTextA_CharToLowerDefine(c8)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey10(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9) \
		((HbHash_FNV1a_CaseKey9(c0, c1, c2, c3, c4, c5, c6, c7, c8) ^ (uint8_t) HbTextA_CharToLowerDefine(c9)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey11(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) \
		((HbHash_FNV1a_CaseKey10(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9) ^ (uint8_t) HbTextA_CharToLowerDefine(c10)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey12(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
		((HbHash_FNV1a_CaseKey11(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10) ^ (uint8_t) HbTextA_CharToLowerDefine(c11)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey13(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) \
		((HbHash_FNV1a_CaseKey12(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) ^ \
				(uint8_t) HbTextA_CharToLowerDefine(c12)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey14(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) \
		((HbHash_FNV1a_CaseKey13(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12) ^ \
				(uint8_t) HbTextA_CharToLowerDefine(c13)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey15(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) \
		((HbHash_FNV1a_CaseKey14(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13) ^ \
				(uint8_t) HbTextA_CharToLowerDefine(c14)) * HbHash_FNV1a_Prime)
#define HbHash_FNV1a_CaseKey16(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
		((HbHash_FNV1a_CaseKey15(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14) ^ \
				(uint8_t) HbTextA_CharToLowerDefine(c15)) * HbHash_FNV1a_Prime)

// !!!
// Hash maps can be stored in files, if these are changed, files containing hash maps need to be updated too.
// Storing can be done for both HbHash_Map instances and in-place hash maps that use HbHash_MapUtil.
// !!!

// The hard minimum is 4 entries for anything meaningful (at least 2 elements).
// 4 would cause expansion for the 3rd entry, so doesn't make much sense too.
#define HbHash_MapUtil_MinEntriesLog2 3

// 1 << 32 is undefined, so stay within the safe mask values.
#define HbHash_MapUtil_MaxEntriesLog2 (sizeof(uint32_t) * 8 - 1)

// At least one free entry is required, so search is guaranteed to be stopped if no element has been found.
#define HbHash_MapUtil_MaxUsedEntries ((UINT32_MAX >> 1) - 1)

// Returns 0 for no entries, at least HbHash_MapUtil_MinEntriesLog2, but UINT32_MAX if too many entries 
uint32_t HbHash_MapUtil_GetNeededEntriesLog2(uint32_t usedCount);

// For hash map collision resolution. Call iteratively until the collision has been resolved with the same parameters.
// Hash should initially point to the original 32-bit hash.
// Index should initially point to hash & mask.
// Mask is hash map size (power of 2) minus 1.
// From CPython dictobject.h.
// https://github.com/python/cpython/blob/master/LICENSE
HbForceInline void HbHash_MapUtil_PerturbateIndex(uint32_t * hash, uint32_t * index, uint32_t indexMask) {
	*hash >>= 5;
	*index = ((*index * 5) + *hash + 1) & indexMask;
}

// Hash map from CPython dictobject and libRocket Dictionary.

typedef struct {
	uint32_t (* hashKey)(void const * key);
	void const * (* elementKey)(void const * element);
	HbBool (* compareKeys)(void const * elementKey, void const * searchKey);
} HbHash_Map_Functions;

typedef struct HbHash_Map {
	HbMemory_Tag * tag;
	char const * fileName;
	size_t elementSize;
	HbHash_Map_Functions const * functions;
	// 16-aligned allocation - not reallocating.
	// First, elementSize << indexBitCount elements.
	// Then, 32-bit-aligned AoSoA type bits.
	// The first uint32 bit mask contains whether each entry is used, for either an actual or a removed element.
	// The second uint32 bit mask contains whether each entry has an actual element.
	void * elementsAndTypeBits;

	uint32_t fileLine;
	uint32_t indexBitCount; // 31 max, 1 << 32 is undefined.
	uint32_t elementCount;
	uint32_t elementAndRemovedCount;
} HbHash_Map;

void HbHash_Map_DoInit(HbHash_Map * map, HbMemory_Tag * tag, size_t elementSize,
		HbHash_Map_Functions const * functions, char const * fileName, uint32_t fileLine);
#define HbHash_Map_Init(map, tag, elementSize, hashFunction, compareFunction) \
		HbHash_Map_DoInit(map, tag, elementSize, hashFunction, compareFunction, __FILE__, __LINE__)
void HbHash_Map_Destroy(HbHash_Map * map);
void * HbHash_Map_FindOrCreate(HbHash_Map * map, void const * key, HbBool * created);
void const * HbHash_Map_FindC(HbHash_Map const * map, void const * key);
// Returns the removed element (if found) so it can be destroyed safely.
void * HbHash_Map_Remove(HbHash_Map * map, void const * key);
// Increases the storage size if needed. Returns true if reallocation happened.
HbBool HbHash_Map_Reserve(HbHash_Map * map, uint32_t elementCount);
// Returns true if reallocation and defragmentation happened.
HbBool HbHash_Map_Defragment(HbHash_Map * map, uint32_t reserveElementCount);

#ifdef __cplusplus
}
#endif
#endif
