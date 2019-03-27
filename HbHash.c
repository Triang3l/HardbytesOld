#include "HbBit.h"
#include "HbHash.h"

uint32_t HbHash_Map_GetIndexMask32(uint32_t elementCount) {
	if (elementCount <= (uint32_t) 1 << HbHash_Map_MinSizeLog2) {
		return ((uint32_t) 1 << HbHash_Map_MinSizeLog2) - 1;
	}
	uint32_t lastElementIndex = elementCount - 1;
	uint32_t highestBitIndex = (uint32_t) HbBit_HighestOneU32(lastElementIndex);
	if (highestBitIndex >= 31) {
		// Can't make the storage even bigger in case of load above 2/3 in this case.
		return UINT32_MAX;
	}
	uint32_t mask = ((uint32_t) 1 << (highestBitIndex + 1)) - 1;
	// Used 2/3 or more of the storage - double the storage size to reduce density and thus collisions.
	// Multiplication by 2 is safe for values up to 0x7FFFFFFF.
	if (elementCount > (mask << 1) / 3) {
		mask = (mask << 1) | 1;
	}
	return mask;
}
