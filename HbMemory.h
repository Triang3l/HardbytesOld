#ifndef HbInclude_HbMemory
#define HbInclude_HbMemory
#include "HbFeedback.h"
#include "HbParallel.h"
#ifdef __cplusplus
extern "C" {
#endif

// More or less arbitrary value, but dlmalloc doesn't allocate smaller than this.
#define HbMemory_MinReasonableAllocLog2 5
#define HbMemory_MinReasonableAlloc ((size_t) 1 << HbMemory_MinReasonableAllocLog2)

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

// 16-alignment is completely free on 64-bit platforms, free as long as reallocs aren't done on 32-bit (realloc may cause moving).
void * HbMemory_DoAlloc(HbMemory_Tag * tag, size_t size, HbBool align16,
		char const * fileName, uint32_t fileLine, HbBool crashOnFailure);
#define HbMemory_Alloc(tag, size, align16) HbMemory_DoAlloc(tag, size, align16, __FILE__, __LINE__, HbTrue)
#define HbMemory_TryAlloc(tag, size, align16) HbMemory_DoAlloc(tag, size, align16, __FILE__, __LINE__, HbFalse)
HbBool HbMemory_DoRealloc(void * * memory, size_t size, char const * fileName, uint32_t fileLine, HbBool crashOnFailure);
#define HbMemory_Realloc(memory, size) HbMemory_DoRealloc(memory, size, __FILE__, __LINE__, HbTrue)
#define HbMemory_TryRealloc(memory, size) HbMemory_DoRealloc(memory, size, __FILE__, __LINE__, HbFalse)
size_t HbMemory_GetAllocationSize(void const * memory);
void HbMemory_Free(void * memory);

/**************************************************************************
 * Two-level dynamic array
 * Piece allocations are 16-aligned because realloc is never done for them
 **************************************************************************/

#define HbMemory_Array2L_PieceCountFewLog2 ((uint32_t) 2)
#define HbMemory_Array2L_MaxLength ((size_t) ((SIZE_MAX >> 1) + 1))
typedef struct HbMemory_Array2L {
	HbMemory_Tag * tag;
	size_t elementSize;
	char const * fileName;
	union {
		void * few[1 << HbMemory_Array2L_PieceCountFewLog2];
		void * * many;
	} pieces;
	size_t pieceElementIndexMask; // (1 << pieceElementCountLog2) - 1
	size_t length;

	uint32_t pieceElementCountLog2;
	uint32_t fileLine;
	uint32_t pieceCountLog2;
} HbMemory_Array2L;

void HbMemory_Array2L_DoInit(HbMemory_Array2L * array2L, HbMemory_Tag * tag, size_t elementSize,
		uint32_t pieceElementCountLog2, char const * fileName, uint32_t fileLine);
#define HbMemory_Array2L_Init(array2L, tag, elementSize, pieceElementCountLog2) \
		HbMemory_Array2L_DoInit(array2L, tag, elementSize, pieceElementCountLog2, __FILE__, __LINE__)
void HbMemory_Array2L_Destroy(HbMemory_Array2L * array2L);
void HbMemory_Array2L_ReservePiecePointers(HbMemory_Array2L * array2L, size_t elementCount);
void HbMemory_Array2L_Resize(HbMemory_Array2L * array2L, size_t elementCount, HbBool onlyReserveMemory);
HbForceInline void const * const * HbMemory_Array2L_GetPiecesC(HbMemory_Array2L const * array2L) {
	return array2L->pieceCountLog2 <= HbMemory_Array2L_PieceCountFewLog2 ? array2L->pieces.few : array2L->pieces.many;
}
HbForceInline void * * HbMemory_Array2L_GetPieces(HbMemory_Array2L * array2L) {
	return (void * *) HbMemory_Array2L_GetPiecesC((HbMemory_Array2L const *) array2L);
}
void const * HbMemory_Array2L_GetC(HbMemory_Array2L const * array2L, size_t offset, size_t * remainingInPiece);
HbForceInline void * HbMemory_Array2L_Get(HbMemory_Array2L * array2L, size_t offset, size_t * remainingInPiece) {
	return (void *) HbMemory_Array2L_GetC((HbMemory_Array2L const *) array2L, offset, remainingInPiece);
}
inline void * HbMemory_Array2L_Append(HbMemory_Array2L * array2L) {
	HbMemory_Array2L_Resize(array2L, array2L->length + 1, HbFalse);
	return HbMemory_Array2L_Get(array2L, array2L->length - 1, NULL);
}
void HbMemory_Array2L_RemoveUnsorted(HbMemory_Array2L * array2L, size_t index);
void HbMemory_Array2L_FillBytes(HbMemory_Array2L * array2L, size_t offset, size_t elementCount, uint8_t value);

/*****************************************************************************
 * Free list-based allocation with persistent indices (locators) from Array2L
 *****************************************************************************/

// For more compact storage (8 bytes instead of 16 per element on 64-bit),
// limiting the maximum number of elements on 64-bit systems.

typedef struct HbMemory_Pool_Locator {
	uint32_t entryIndex;
	uint32_t revision;
} HbMemory_Pool_Locator;

typedef struct HbMemory_Pool_Entry {
	uint32_t locatorRevision;
	uint32_t nextFree; // If == index of this, this entry is occupied.
} HbMemory_Pool_Entry;

typedef struct HbMemory_Pool {
	HbMemory_Array2L entries;
	HbMemory_Array2L elementData;
	uint32_t firstFree;
} HbMemory_Pool;

void HbMemory_Pool_DoInit(HbMemory_Pool * pool, HbMemory_Tag * tag, size_t elementSize,
		uint32_t array2LPieceElementCountLog2, char const * fileName, uint32_t fileLine);
#define HbMemory_Pool_Init(pool, tag, elementSize, array2LPieceElementCountLog2) \
		HbMemory_Pool_DoInit(pool, tag, elementSize, array2LPieceElementCountLog2, __FILE__, __LINE__)
void HbMemory_Pool_Destroy(HbMemory_Pool * pool);
void HbMemory_Pool_Reserve(HbMemory_Pool * pool, uint32_t elementCount);
void const * HbMemory_Pool_GetC(HbMemory_Pool const * pool, HbMemory_Pool_Locator locator);
HbForceInline void * HbMemory_Pool_Get(HbMemory_Pool * pool, HbMemory_Pool_Locator locator) {
	return (void *) HbMemory_Pool_GetC((HbMemory_Pool const *) pool, locator);
}
HbMemory_Pool_Locator HbMemory_Pool_Alloc(HbMemory_Pool * pool);
void HbMemory_Pool_Free(HbMemory_Pool * pool, HbMemory_Pool_Locator locator);

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

#ifdef __cplusplus
}
#endif
#endif
