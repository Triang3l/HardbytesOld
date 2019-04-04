#ifndef HbInclude_HbMemory
#define HbInclude_HbMemory
#include "HbFeedback.h"
#include "HbParallel.h"

/***************************************
 * Tag-based memory allocation tracking
 ***************************************/

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

/************************************************************************
 * Power of two (buddy) allocator for data indexes (not for data itself)
 ************************************************************************/

// For (1 << n) nodes at the deepest level, (n - 1) bits are needed to store the index.
// A special value (-1), however, is needed for the beginning/end of the free list, so 1 bit is reserved.
// 2 bits are used to store the node type, so 30 bits are available for the links.
// 30 minus 1 (for null links) plus 1 (because they are indexes) is 30.
#define HbMemoryi_PO2Alloc_MaxLevels 30

typedef enum HbMemoryi_PO2Alloc_Node_Type {
	// Doesn't exist yet (no split parent), 0 for memset.
	HbMemoryi_PO2Alloc_Node_Type_NonExistent,
	// Has a split parent (except for when it's the top node) **and previous/next free** (or -1 if in the ends of the free list).
	HbMemoryi_PO2Alloc_Node_Type_Free,
	// Has two children, each either free or with data.
	HbMemoryi_PO2Alloc_Node_Type_Split,
	// Allocation here.
	HbMemoryi_PO2Alloc_Node_Type_Data,
	// 2 bits needed.
} HbMemoryi_PO2Alloc_Node_Type;

typedef struct HbMemoryi_PO2Alloc_Node {
	uint32_t type : 2; // HbMemoryi_PO2Alloc_Node_Type as unsigned.
	// Free list links, relative to the first node on the level.
	int32_t previousFreeOnLevel : HbMemoryi_PO2Alloc_MaxLevels;
	int32_t nextFreeOnLevel : HbMemoryi_PO2Alloc_MaxLevels;
} HbMemoryi_PO2Alloc_Node;

HbFeedback_StaticAssert(sizeof(HbMemoryi_PO2Alloc_Node) <= 2 * sizeof(int32_t),
		"Check HbMemoryi_PO2Alloc_MaxLevels, one power-of-2 allocator node became pretty large.");

typedef struct HbMemory_PO2Alloc {
	HbMemoryi_PO2Alloc_Node * nodes;
	uint32_t deepestLevel;
	uint32_t smallestNodeSizeLog2;
	// Absolute indexes of first free nodes on each level.
	// Level 0 is the head level, level deepestLevel contains the smallest allocations.
	int32_t firstFree[HbMemoryi_PO2Alloc_MaxLevels];
} HbMemory_PO2Alloc;

void HbMemory_PO2Alloc_Init(HbMemory_PO2Alloc * allocator, HbMemory_Tag * tag,
		uint32_t largestNodeSizeLog2, uint32_t smallestNodeSizeLog2);
void HbMemory_PO2Alloc_Destroy(HbMemory_PO2Alloc * allocator);
#define HbMemory_PO2Alloc_TryAllocFailed UINT32_MAX
uint32_t HbMemory_PO2Alloc_TryAlloc(HbMemory_PO2Alloc * allocator, uint32_t count);
uint32_t HbMemory_PO2Alloc_Alloc(HbMemory_PO2Alloc * allocator, uint32_t count);
void HbMemory_PO2Alloc_Free(HbMemory_PO2Alloc * allocator, uint32_t index);

#endif
