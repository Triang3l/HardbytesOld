#ifndef HbInclude_HbAtomic
#define HbInclude_HbAtomic
#include "HbCommon.h"
#if HbPlatform_OS_Windows
#include <Windows.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

// Note: acquire is used like "check atomic -> acquire -> read dependencies",
// release is used like "write dependencies -> release -> update atomic".

#if HbPlatform_OS_Windows
/******************************
* Windows and Visual C atomics
*******************************/

// Compare and swap.

HbForceInline int32_t HbAtomic_CompareAndSwapI32(int32_t volatile * location, int32_t setValue, int32_t compareValue) {
	return (int32_t) InterlockedCompareExchange((LONG volatile *) location, setValue, compareValue);
}
HbForceInline int64_t HbAtomic_CompareAndSwapI64(int64_t volatile * location, int64_t setValue, int64_t compareValue) {
	return (int64_t) InterlockedCompareExchange64((LONG64 volatile *) location, setValue, compareValue);
}

// Basic operations.

HbForceInline int32_t HbAtomic_IncrementI32(int32_t volatile * location) {
	return (int32_t) InterlockedIncrement((LONG volatile *) location);
}
HbForceInline int64_t HbAtomic_IncrementI64(int64_t volatile * location) {
	return (int64_t) InterlockedIncrement64((LONG64 volatile *) location);
}

#else
#error No atomic operations for the target OS.
#endif

/***************************************
 * Wrappers and higher-level operations
 ***************************************/

// Compare and swap.

HbForceInline uint32_t HbAtomic_CompareAndSwapU32(uint32_t volatile * location, uint32_t setValue, uint32_t compareValue) {
	return (uint32_t) HbAtomic_CompareAndSwapI32((int32_t volatile *) location, (int32_t) setValue, (int32_t) compareValue);
}
HbForceInline uint64_t HbAtomic_CompareAndSwapU64(uint64_t volatile * location, uint64_t setValue, uint64_t compareValue) {
	return (uint64_t) HbAtomic_CompareAndSwapI64((int64_t volatile *) location, (int64_t) setValue, (int64_t) compareValue);
}
HbForceInline size_t HbAtomic_CompareAndSwapSize(size_t volatile * location, size_t setValue, size_t compareValue) {
	#if HbPlatform_CPU_32Bit
	return (size_t) HbAtomic_CompareAndSwapU32((uint32_t volatile *) location, (uint32_t) setValue, (uint32_t) compareValue);
	#else
	return (size_t) HbAtomic_CompareAndSwapU64((uint64_t volatile *) location, (uint64_t) setValue, (uint64_t) compareValue);
	#endif
}
HbForceInline void * HbAtomic_CompareAndSwapPointer(void * volatile * location, void * setValue, void * compareValue) {
	return (void *) HbAtomic_CompareAndSwapU64((size_t volatile *) location, (size_t) setValue, (size_t) compareValue);
}

// Basic operations.

HbForceInline uint32_t HbAtomic_IncrementU32(uint32_t volatile * location) {
	return (uint32_t) HbAtomic_IncrementI32((int32_t volatile *) location);
}
HbForceInline uint64_t HbAtomic_IncrementU64(uint64_t volatile * location) {
	return (uint64_t) HbAtomic_IncrementI64((int64_t volatile *) location);
}
HbForceInline size_t HbAtomic_IncrementSize(size_t volatile * location) {
	#if HbPlatform_CPU_32Bit
	return (size_t) HbAtomic_IncrementU32((uint32_t volatile *) location);
	#else
	return (size_t) HbAtomic_IncrementU64((uint64_t volatile *) location);
	#endif
}

// Min/max.

HbForceInline void HbAtomic_MinI32(int32_t volatile * location, int32_t newBound) {
	int32_t oldValue = *location;
	while (oldValue > newBound && (oldValue = HbAtomic_CompareAndSwapI32(location, newBound, oldValue)) != oldValue) {}
}
HbForceInline void HbAtomic_MinI64(int64_t volatile * location, int64_t newBound) {
	int64_t oldValue = *location;
	while (oldValue > newBound && (oldValue = HbAtomic_CompareAndSwapI64(location, newBound, oldValue)) != oldValue) {}
}
HbForceInline void HbAtomic_MinU32(uint32_t volatile * location, uint32_t newBound) {
	uint32_t oldValue = *location;
	while (oldValue > newBound && (oldValue = HbAtomic_CompareAndSwapU32(location, newBound, oldValue)) != oldValue) {}
}
HbForceInline void HbAtomic_MinU64(uint64_t volatile * location, uint64_t newBound) {
	uint64_t oldValue = *location;
	while (oldValue > newBound && (oldValue = HbAtomic_CompareAndSwapU64(location, newBound, oldValue)) != oldValue) {}
}
HbForceInline void HbAtomic_MinSize(size_t volatile * location, size_t newBound) {
	#if HbPlatform_CPU_32Bit
	HbAtomic_MinU32((uint32_t volatile *) location, (uint32_t) newBound);
	#else
	HbAtomic_MinU64((uint64_t volatile *) location, (uint64_t) newBound);
	#endif
}

HbForceInline void HbAtomic_MaxI32(int32_t volatile * location, int32_t newBound) {
	int32_t oldValue = *location;
	while (oldValue < newBound && (oldValue = HbAtomic_CompareAndSwapI32(location, newBound, oldValue)) != oldValue) {}
}
HbForceInline void HbAtomic_MaxI64(int64_t volatile * location, int64_t newBound) {
	int64_t oldValue = *location;
	while (oldValue < newBound && (oldValue = HbAtomic_CompareAndSwapI64(location, newBound, oldValue)) != oldValue) {}
}
HbForceInline void HbAtomic_MaxU32(uint32_t volatile * location, uint32_t newBound) {
	uint32_t oldValue = *location;
	while (oldValue < newBound && (oldValue = HbAtomic_CompareAndSwapU32(location, newBound, oldValue)) != oldValue) {}
}
HbForceInline void HbAtomic_MaxU64(uint64_t volatile * location, uint64_t newBound) {
	uint64_t oldValue = *location;
	while (oldValue < newBound && (oldValue = HbAtomic_CompareAndSwapU64(location, newBound, oldValue)) != oldValue) {}
}
HbForceInline void HbAtomic_MaxSize(size_t volatile * location, size_t newBound) {
	#if HbPlatform_CPU_32Bit
	HbAtomic_MaxU32((uint32_t volatile *) location, (uint32_t) newBound);
	#else
	HbAtomic_MaxU64((uint64_t volatile *) location, (uint64_t) newBound);
	#endif
}

#ifdef __cplusplus
}
#endif
#endif
