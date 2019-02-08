#ifndef HbInclude_HbMemory
#define HbInclude_HbMemory
#include "HbParallel.h"

typedef enum HbMemoryi_Allocation_Marker {
	HbMemoryi_Allocation_Marker_Aligned8 = 0xA110, // Header of a 8-aligned allocation.
	HbMemoryi_Allocation_Marker_Aligned16, // Header of a 16-aligned allocation.
	HbMemoryi_Allocation_Marker_GoBack8, // Alignment padding, real header is 8 bytes before.
} HbMemoryi_Allocation_Marker;

#if HbPlatform_CPU_32Bit
typedef struct HbAligned(8) HbMemoryi_Allocation {
#else
typedef struct HbAligned(16) HbMemoryi_Allocation {
#endif
	struct HbMemory_Tag * tag; // 4 (32-bit) / 8 (64-bit) bytes.
	struct HbMemoryi_Allocation * tagAllocationPrevious; // 8/16 bytes, protected by the tag's mutex.
	struct HbMemoryi_Allocation * tagAllocationNext; // 12/24 bytes, protected by the tag's mutex.
	size_t size; // 16/32 bytes, protected by the tag's mutex due to realloc.
	char const * fileName; // 20/40 bytes, immutable string, protected by the tag's mutex due to realloc.
	uint16_t fileLine; // 22/42 bytes, protected by the tag's mutex due to realloc.
	uint16_t marker; // 24/44 bytes (sentinel + what is actually located here). Must be the last field!
} HbMemoryi_Allocation;

typedef struct HbMemory_Tag {
	char name[64];
	HbParallel_Mutex mutex;
	HbMemoryi_Allocation * allocationFirst, * allocationLast; // Protected by the mutex.
	size_t totalAllocatedSize; // Protected by the mutex.
	struct HbMemory_Tag * globalTagPrevious, * globalTagNext; // Protected by HbMemoryi_TagListMutex.
} HbMemory_Tag;

void HbMemory_Init();
void HbMemory_Shutdown();

// Create rather than Init to make sure a tag is never created within memory allocated in another tag :O
HbMemory_Tag * HbMemory_Tag_Create(char const * name);
void HbMemory_Tag_Destroy(HbMemory_Tag * tag, HbBool leaksAreErrors);

void * HbMemory_DoAlloc(HbMemory_Tag * tag, size_t size, HbBool align16,
		char const * fileName, uint32_t fileLine, HbBool crashOnFailure);
#define HbMemory_Alloc(tag, size, align16) HbMemory_DoAlloc(tag, size, align16, __FILE__, __LINE__, HbTrue)
#define HbMemory_TryAlloc(tag, size, align16) HbMemory_DoAlloc(tag, size, align16, __FILE__, __LINE__, HbFalse)
HbBool HbMemory_DoRealloc(void * * memory, size_t size, char const * fileName, uint32_t fileLine, HbBool crashOnFailure);
#define HbMemory_Realloc(memory, size) HbMemory_DoRealloc(memory, size, __FILE__, __LINE__, HbTrue)
#define HbMemory_TryRealloc(memory, size) HbMemory_DoRealloc(memory, size, __FILE__, __LINE__, HbFalse)
void HbMemory_Free(void * memory);

#endif
