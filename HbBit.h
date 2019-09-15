#ifndef HbInclude_HbBit
#define HbInclude_HbBit
#include "HbCommon.h"
#if HbPlatform_Compiler_MSVC
#include <intrin.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

#if HbPlatform_Compiler_MSVC
HbForceInline int32_t HbBit_LowestOneU32(uint32_t value) {
	if (value == 0) {
		return -1;
	}
	unsigned long index;
	_BitScanForward(&index, (unsigned long) value);
	return (int) index;
}

HbForceInline int32_t HbBit_LowestOneU64(uint64_t value) {
	unsigned long index;
	#if HbPlatform_CPU_32Bit
	uint32_t part = (uint32_t) value;
	if (part != 0) {
		_BitScanForward(&index, (unsigned long) part);
		return (int) index;
	}
	part = (uint32_t) (value >> 32);
	if (part != 0) {
		_BitScanForward(&index, (unsigned long) part);
		return (int) index + 32;
	}
	return -1;
	#else
	if (value == 0) {
		return -1;
	}
	_BitScanForward64(&index, (unsigned __int64) value);
	return (int) index;
	#endif
}

HbForceInline int32_t HbBit_HighestOneU32(uint32_t value) {
	if (value == 0) {
		return -1;
	}
	unsigned long index;
	_BitScanReverse(&index, (unsigned long) value);
	return (int) index;
}

HbForceInline int32_t HbBit_HighestOneU64(uint64_t value) {
	unsigned long index;
	#if HbPlatform_CPU_32Bit
	uint32_t part = (uint32_t) (value >> 32);
	if (part != 0) {
		_BitScanReverse(&index, (unsigned long) part);
		return (int) index + 32;
	}
	part = (uint32_t) value;
	if (part != 0) {
		_BitScanReverse(&index, (unsigned long) part);
		return (int) index;
	}
	return -1;
	#else
	if (value == 0) {
		return -1;
	}
	_BitScanReverse64(&index, (unsigned __int64) value);
	return (int) index;
	#endif
}

#if HbPlatform_CPU_32Bit
HbForceInline int32_t HbBit_LowestOneSize(size_t value) {
	return HbBit_LowestOneU32((uint32_t) value);
}
HbForceInline int32_t HbBit_HighestOneSize(size_t value) {
	return HbBit_HighestOneU32((uint32_t) value);
}
#else
HbForceInline int32_t HbBit_LowestOneSize(size_t value) {
	return HbBit_LowestOneU64((uint64_t) value);
}
HbForceInline int32_t HbBit_HighestOneSize(size_t value) {
	return HbBit_HighestOneU64((uint64_t) value);
}
#endif

#else
#error No bitwise math functions (HbBit) for the target compiler.
#endif

HbForceInline HbBool HbBit_IsPO2U32(uint32_t value) { return (value & (value - 1)) == 0; }
HbForceInline HbBool HbBit_IsPO2U64(uint64_t value) { return (value & (value - 1)) == 0; }
HbForceInline HbBool HbBit_IsPO2Size(size_t value) { return (value & (value - 1)) == 0; }

// Returns 0 for both 0 and 1.
HbForceInline uint32_t HbBit_Log2CeilU32(uint32_t value) {
	if (value == 0) {
		return 0;
	}
	return (uint32_t) HbBit_HighestOneU32(value) + !HbBit_IsPO2U32(value);
}
HbForceInline uint32_t HbBit_Log2CeilU64(uint64_t value) {
	if (value == 0) {
		return 0;
	}
	return (uint32_t) HbBit_HighestOneU64(value) + !HbBit_IsPO2U64(value);
}
HbForceInline uint32_t HbBit_Log2CeilSize(size_t value) {
	if (value == 0) {
		return 0;
	}
	return (uint32_t) HbBit_HighestOneSize(value) + !HbBit_IsPO2Size(value);
}

HbForceInline uint32_t HbBit_Log2CeilSaturatedU32(uint32_t value) {
	return HbMinU32(HbBit_Log2CeilU32(value), sizeof(value) * 8 - 1);
}
HbForceInline uint32_t HbBit_Log2CeilSaturatedU64(uint64_t value) {
	return HbMinU32(HbBit_Log2CeilU64(value), sizeof(value) * 8 - 1);
}
HbForceInline uint32_t HbBit_Log2CeilSaturatedSize(size_t value) {
	return HbMinU32(HbBit_Log2CeilSize(value), sizeof(value) * 8 - 1);
}

#ifdef __cplusplus
}
#endif
#endif
