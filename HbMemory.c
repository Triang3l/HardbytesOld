#include "HbMemory.h"
#include "HbFeedback.h"
#include "HbText.h"

static HbParallel_Mutex HbMemoryi_TagListMutex;
HbMemory_Tag * HbMemoryi_TagFirst, * HbMemoryi_TagLast;

void HbMemory_Init() {
	if (!HbParallel_Mutex_Init(&HbMemoryi_TagListMutex)) {
		HbFeedback_Crash("HbMemory_Init", "Failed to initialize the global tag list mutex.");
	}
	HbMemoryi_TagFirst = HbMemoryi_TagLast = HbNull;
}

void HbMemory_Shutdown() {
	if (HbMemoryi_TagFirst != HbNull) {
		HbFeedback_Crash("HbMemory_Shutdown", "Not all memory tags were destroyed.");
	}
	HbParallel_Mutex_Destroy(&HbMemoryi_TagListMutex);
}

HbMemory_Tag * HbMemory_Tag_Create(char const * name) {
	HbMemory_Tag * tag = malloc(sizeof(HbMemory_Tag));
	if (tag == HbNull) {
		HbFeedback_Crash("HbMemory_Tag_Create", "Failed to allocate memory for a tag.");
	}
	if (!HbParallel_Mutex_Init(&tag->mutex)) {
		HbFeedback_Crash("HbMemory_Tag_Create", "Failed to initialize the mutex for a tag.");
	}
	if (name != HbNull) {
		HbTextA_Copy(tag->name, HbArrayLength(tag->name), name);
	} else {
		tag->name[0] = '\0';
	}
	tag->allocationFirst = tag->allocationLast = HbNull;
	tag->totalAllocatedSize = 0;

	HbParallel_Mutex_Lock(&HbMemoryi_TagListMutex);
	tag->globalTagPrevious = HbMemoryi_TagLast;
	tag->globalTagNext = HbNull;
	if (HbMemoryi_TagLast != HbNull) {
		HbMemoryi_TagLast->globalTagNext = tag;
	}
	HbMemoryi_TagLast = tag;
	if (HbMemoryi_TagFirst == HbNull) {
		HbMemoryi_TagFirst = tag;
	}
	HbParallel_Mutex_Unlock(&HbMemoryi_TagListMutex);

	return tag;
}

void HbMemory_Tag_Destroy(HbMemory_Tag * tag, HbBool leaksAreErrors) {
	HbParallel_Mutex_Lock(&HbMemoryi_TagListMutex);
	if (tag->globalTagPrevious != HbNull) {
		tag->globalTagPrevious->globalTagNext = tag->globalTagNext;
	} else {
		HbMemoryi_TagFirst = tag->globalTagNext;
	}
	if (tag->globalTagNext != HbNull) {
		tag->globalTagNext->globalTagPrevious = tag->globalTagPrevious;
	} else {
		HbMemoryi_TagLast = tag->globalTagPrevious;
	}
	HbParallel_Mutex_Unlock(&HbMemoryi_TagListMutex);

	// Free all allocations, and, if needed, report leaks and crash.
	uint32_t leakCount = 0;
	HbParallel_Mutex_Lock(&tag->mutex);
	HbMemoryi_Allocation * allocation = tag->allocationFirst;
	while (allocation != HbNull) {
		HbMemoryi_Allocation * allocationNext = allocation->tagAllocationNext;
		++leakCount;
		if (leaksAreErrors) {
			HbFeedback_DebugMessageForce("HbMemory_Tag_Destroy: leak at %s:%u (%zu bytes)",
					allocation->fileName, allocation->fileLine, allocation->size);
		}
		free(allocation);
		allocation = allocationNext;
	}
	HbParallel_Mutex_Unlock(&tag->mutex);
	if (leaksAreErrors && leakCount > 0) {
		HbFeedback_Crash("HbMemory_Tag_Destroy", "%u memory leaks in tag %s - see debug message log.",
				leakCount, tag->name != HbNull ? tag->name : "(unnamed)");
	}

	HbParallel_Mutex_Destroy(&tag->mutex);
	free(tag);
}

void * HbMemory_DoAlloc(HbMemory_Tag * tag, size_t size, HbBool align16,
		char const * fileName, uint32_t fileLine, HbBool crashOnFailure) {
	size_t mallocSize = sizeof(HbMemoryi_Allocation) + HbAlign(size, (size_t) 16); // Same as in DoRealloc.
	#if HbPlatform_CPU_32Bit
	if (align16) {
		// Alignment padding.
		mallocSize += 8;
	}
	#endif
	HbMemoryi_Allocation * allocation = malloc(mallocSize);
	if (allocation == HbNull) {
		if (crashOnFailure) {
			HbFeedback_Crash("HbMemory_DoAlloc", "Failed to allocate %zu bytes in tag %s at %s:%u.",
					size, tag->name != HbNull ? tag->name : "(unnamed)", fileName, fileLine);
		}
		return HbNull;
	}

	void * memory = allocation + 1;
	#if HbPlatform_CPU_32Bit
	if (align16 && ((uintptr_t) memory & 8) != 0) {
		*((uint8_t * *) &memory) += 8;
		// Put the marker in the padding (so DoRealloc and Free can find the header).
		((HbMemoryi_Allocation *) memory - 1)->marker = HbMemoryi_Allocation_Marker_GoBack8;
	}
	#endif

	allocation->tag = tag;
	allocation->size = size;
	allocation->fileName = fileName;
	allocation->fileLine = (uint16_t) fileLine;
	allocation->marker = align16 ? HbMemoryi_Allocation_Marker_Aligned16 : HbMemoryi_Allocation_Marker_Aligned8;

	HbParallel_Mutex_Lock(&tag->mutex);
	allocation->tagAllocationPrevious = tag->allocationLast;
	allocation->tagAllocationNext = HbNull;
	if (tag->allocationLast != HbNull) {
		tag->allocationLast->tagAllocationNext = allocation;
	}
	tag->allocationLast = allocation;
	if (tag->allocationFirst == HbNull) {
		tag->allocationFirst = allocation;
	}
	tag->totalAllocatedSize += size;
	HbParallel_Mutex_Unlock(&tag->mutex);

	return memory;
}

static inline HbMemoryi_Allocation * HbMemoryi_GetAllocation(void * memory) {
	if (memory == HbNull) {
		return HbNull;
	}
	HbMemoryi_Allocation * allocation = (HbMemoryi_Allocation *) memory - 1;
	#if HbPlatform_CPU_32Bit
	if (allocation->marker == HbMemoryi_Allocation_Marker_GoBack8) {
		*((uint8_t * *) &allocation) -= 8;
	}
	#endif
	if (allocation->marker != HbMemoryi_Allocation_Marker_Aligned8 &&
		allocation->marker != HbMemoryi_Allocation_Marker_Aligned16) {
		return HbNull;
	}
	return allocation;
}

HbBool HbMemory_DoRealloc(void * * memory, size_t size, char const * fileName, uint32_t fileLine, HbBool crashOnFailure) {
	HbMemoryi_Allocation * allocation = HbMemoryi_GetAllocation(*memory);
	if (allocation == HbNull) {
		HbFeedback_Crash("HbMemory_DoRealloc", "Tried to reallocate %p at %s:%u which was not allocated with HbMemory_Alloc.",
				*memory, fileName, fileLine);
	}

	// No need to lock here since only realloc can modify the size,
	// but reallocating the same memory on two threads at once is totally incorrect.
	size_t alignedSize = HbAlign(size, (size_t) 16); // Same as in DoAlloc.
	size_t oldSize = allocation->size;
	if (alignedSize == HbAlign(oldSize, (size_t) 16)) {
		// Same size - not reallocating.
		return HbTrue;
	}
	size_t mallocSize = sizeof(HbMemoryi_Allocation) + alignedSize;

	#if HbPlatform_CPU_32Bit
	HbBool align16 = allocation->marker == HbMemoryi_Allocation_Marker_Aligned16;
	HbBool wasPadded = ((uintptr_t) (allocation + 1) & 8) != 0;
	if (align16) {
		mallocSize += 8;
	}
	#endif

	HbMemory_Tag * tag = allocation->tag;

	HbParallel_Mutex_Lock(&tag->mutex);
	allocation = realloc(allocation, mallocSize);
	if (allocation == HbNull) {
		HbParallel_Mutex_Unlock(&tag->mutex);
		if (crashOnFailure) {
			HbFeedback_Crash("HbMemory_DoRealloc", "Failed to reallocate %zu->%zu bytes in tag %s at %s:%u.",
					oldSize, size, tag->name != HbNull ? tag->name : "(unnamed)", fileName, fileLine);
		}
		return HbFalse;
	}
	// The pointer to the allocation may now be different, so relink it.
	if (allocation->tagAllocationPrevious != HbNull) {
		allocation->tagAllocationPrevious->tagAllocationNext = allocation;
	} else {
		tag->allocationFirst = allocation;
	}
	if (allocation->tagAllocationNext != HbNull) {
		allocation->tagAllocationNext->tagAllocationPrevious = allocation;
	} else {
		tag->allocationLast = allocation;
	}
	allocation->size = size;
	allocation->fileName = fileName;
	allocation->fileLine = (uint16_t) fileLine;
	tag->totalAllocatedSize += size - oldSize;
	HbParallel_Mutex_Unlock(&tag->mutex);

	void * newMemory = allocation + 1;
	#if HbPlatform_CPU_32Bit
	if (align16) {
		// If the need for padding is changed, realign data.
		if (((uintptr_t) newMemory & 8) != 0) {
			if (!wasPadded) {
				// Need to insert the padding as the data is not 16-aligned anymore.
				memmove((uint8_t *) newMemory + 8, newMemory, HbMinI(oldSize, size));
				((HbMemoryi_Allocation *) newMemory - 1)->marker = HbMemoryi_Allocation_Marker_GoBack8;
			}
			*((uint8_t * *) &newMemory) += 8;
		} else {
			if (wasPadded) {
				// Need to remove the padding as the data is now 16-aligned.
				memmove(newMemory, (uint8_t *) newMemory + 8, HbMinI(oldSize, size));
			}
		}
	}
	#endif
	*memory = newMemory;

	return HbTrue;
}

void HbMemory_Free(void * memory) {
	if (memory == HbNull) {
		return; // To match C behavior, and for easier shutdown of things.
	}

	HbMemoryi_Allocation * allocation = HbMemoryi_GetAllocation(memory);
	if (allocation == HbNull) {
		HbFeedback_Crash("HbMemory_Free", "Tried to free %p which was not allocated with HbMemory_Alloc.", memory);
	}

	HbMemory_Tag * tag = allocation->tag;
	HbParallel_Mutex_Lock(&tag->mutex);
	if (allocation->tagAllocationPrevious != HbNull) {
		allocation->tagAllocationPrevious->tagAllocationNext = allocation->tagAllocationNext;
	} else {
		tag->allocationFirst = allocation->tagAllocationNext;
	}
	if (allocation->tagAllocationNext != HbNull) {
		allocation->tagAllocationNext->tagAllocationPrevious = allocation->tagAllocationPrevious;
	} else {
		tag->allocationLast = allocation->tagAllocationPrevious;
	}
	tag->totalAllocatedSize -= allocation->size;
	HbParallel_Mutex_Unlock(&tag->mutex);

	free(allocation);
}
