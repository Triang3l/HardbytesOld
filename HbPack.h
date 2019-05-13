#ifndef HbInclude_HbPack
#define HbInclude_HbPack
#include "HbCommon.h"

/*
 * Custom package format for assets of different types.
 *
 * Features and restrictions:
 * - 16 byte alignment of files (items), allowing for SIMD usage when loading files.
 * - Alphabetically sorted (ignoring case for ASCII) central directory for quick binary search and folder contents iteration.
 * - Optional hash map (in HbHash_Map_* format, but without dummy entries) for even quicker lookups.
 * - Directory record and hash map located in the beginning for sequential reading.
 *
 * Can be used with both whole-file reading and with file memory mapping.
 *
 * The format is designed for use both as a package for multiple assets and as a container for other asset formats.
 * This is similar to .*x files used by Microsoft Office. Path names are a more flexible and extensible alternative to things like FourCC.
 *
 * Overall structure:
 * - HbPack_Header.
 * - HbPack_DirectoryEntry[item count].
 * - Optional hash map with uint32_t indexes of directory entries. The mask of the hash is HbHash_Map_GetIndexMask32(item count).
 * - Item contents, each 16-aligned.
 */

// Must be in the beginning.
extern char const HbPack_HeaderID[12];

typedef struct HbAligned(16) HbPack_Header {
	char id[12]; // HbPack_HeaderID.
	uint32_t itemCount : 31;
	uint32_t hashMapPresent : 1; // The index mask is calculated as HbHash_Map_GetIndexMask32(itemCount).
} HbPack_Header;

#define HbPack_InvalidItemIndex UINT32_MAX

#define HbPack_MaxItemNameSize 56 // Including the zero terminator.

typedef struct HbAligned(16) HbPack_DirectoryEntry {
	char name[HbPack_MaxItemNameSize]; // The tail must be zero-filled. Use HbTextA_CompareCaseless for comparison. Path separator is /.
	uint32_t offset;
	uint32_t size;
} HbPack_DirectoryEntry;

typedef struct HbPack_Info {
	uint8_t const * start;
	uint32_t size;
	uint32_t itemCount;
	// Can be calculated from itemCount, but for faster access later.
	uint32_t hashMapOffset; // 0 if no hash map (if using binary search only for more compact storage).
	uint32_t hashMapIndexMask;
} HbPack_Info;

// validateDirectory must be true for user-generated content, with false only the size of the header will be validated.
// With validateDirectory, alignment, item offsets, sizes and hash map indexes will be validated.
HbBool HbPack_GetInfo(void const * pack, uint32_t packSize, HbPack_Info * info, HbBool validateDirectory);

HbForceInline HbPack_DirectoryEntry const * HbPack_GetDirectory(HbPack_Info const * info) {
	return (HbPack_DirectoryEntry const *) (info->start + sizeof(HbPack_Header));
}

// Uses binary search to find the first item with the specified prefix. For the first item in a folder, end the prefix with a /.
uint32_t HbPack_FindFirstPrefixed(HbPack_Info const * info, char const * prefix);
HbPack_DirectoryEntry const * HbPack_Find(HbPack_Info const * info, char const * path);

#endif
