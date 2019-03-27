#include "HbHash.h"
#include "HbPack.h"

char const HbPack_HeaderID[12] = { 'H', 'a', 'r', 'd', 'b', 'y', 't', 'e', 's', 'P', 'a', 'k' };

HbBool HbPack_GetInfo(void const * pack, uint32_t packSize, HbPack_Info * info, HbBool validateDirectory) {
	if (((uintptr_t) pack & 15) != 0 || packSize < sizeof(HbPack_Header)) {
		return HbFalse;
	}
	HbPack_Header const * header = (HbPack_Header *) pack;
	if (memcmp(header->id, HbPack_HeaderID, sizeof(HbPack_HeaderID)) != 0) {
		return HbFalse;
	}
	uint32_t itemCount = header->itemCount;
	if (itemCount == 0) {
		return HbFalse;
	}
	if ((packSize - sizeof(HbPack_Header)) / sizeof(HbPack_DirectoryEntry) < itemCount) {
		return HbFalse;
	}
	if (validateDirectory) {
		HbPack_DirectoryEntry const * directory = (HbPack_DirectoryEntry const *) ((uint8_t const *) pack + sizeof(HbPack_Header));
		for (uint32_t itemIndex = 0; itemIndex < itemCount; ++itemCount) {
			HbPack_DirectoryEntry const * directoryEntry = &directory[itemIndex];
			if (directoryEntry->name[0] == '\0' || directoryEntry->name[HbPack_MaxItemNameSize - 1] != '\0' ||
					(directoryEntry->offset & 15) != 0 || directoryEntry->offset > packSize ||
					(packSize - directoryEntry->offset) < directoryEntry->size) {
				return HbFalse;
			}
		}
	}
	if (header->hashMapPresent) {
		uint32_t hashMapOffset = sizeof(HbPack_Header) + itemCount * sizeof(HbPack_DirectoryEntry);
		uint32_t hashMapIndexMask = HbHash_Map_GetIndexMask32(itemCount);
		// Validation.
		uint32_t hashMapMaxSize = (packSize - hashMapOffset) / sizeof(uint32_t);
		if (hashMapMaxSize == 0 || (hashMapMaxSize - 1) < hashMapIndexMask) {
			return HbFalse;
		}
		if (validateDirectory) {
			uint32_t const * hashMap = (uint32_t const *) ((uint8_t const *) pack + hashMapOffset);
			uint32_t hashMapIndex = 0;
			for (;;) { // Not a true for loop for safety in case of 0xFFFFFFFF mask.
				uint32_t hashMapEntry = hashMap[hashMapIndex];
				if (hashMapEntry != HbPack_InvalidItemIndex && hashMapEntry >= itemCount) {
					return HbFalse;
				}
				if (hashMapIndex >= hashMapIndexMask) {
					break;
				}
				++hashMapIndex;
			}
		}
		info->hashMapOffset = hashMapOffset;
		info->hashMapIndexMask = hashMapIndexMask;
	} else {
		info->hashMapOffset = 0;
		info->hashMapIndexMask = 0;
	}
	info->start = pack;
	info->size = packSize;
	info->itemCount = itemCount;
	return HbTrue;
}

uint32_t HbPack_FindFirstPrefixed(HbPack_Info const * info, char const * namePrefix) {
	size_t namePrefixLength = HbTextA_Length(namePrefix);
	HbPack_DirectoryEntry const * directory = HbPack_GetDirectory(info);
	uint32_t lowerBound = 0, upperBound = info->itemCount;
	while (lowerBound < upperBound) {
		uint32_t entryIndex = lowerBound + ((upperBound - lowerBound) >> 1);
		int32_t comparison = HbTextA_ComparePartCaseless(directory[entryIndex].name, namePrefix, namePrefixLength);
		if (comparison < 0) {
			upperBound = entryIndex;
		} else if (comparison > 0) {
			lowerBound = entryIndex + 1;
		} else {
			// Rewind to the first with the prefix.
			while (entryIndex > 0) {
				if (HbTextA_ComparePartCaseless(directory[entryIndex - 1].name, namePrefix, namePrefixLength) == 0) {
					--entryIndex;
				} else {
					break;
				}
			}
			return entryIndex;
		}
	}
	return HbPack_InvalidItemIndex;
}

HbPack_DirectoryEntry const * HbPack_Find(HbPack_Info const * info, char const * name) {
	HbPack_DirectoryEntry const * directory = HbPack_GetDirectory(info);
	if (info->hashMapOffset != 0) {
		uint32_t hashIndexMask = info->hashMapIndexMask;
		uint32_t const * hashMap = (uint32_t const *) (info->start + info->hashMapOffset);
		uint32_t hash = HbHash_FVN1a_HashTextACaseless(name);
		uint32_t hashIndex = hash & hashIndexMask;
		uint32_t entryIndex;
		while ((entryIndex = hashMap[hashIndex]) != HbPack_InvalidItemIndex) {
			if (HbTextA_CompareCaseless(directory[entryIndex].name, name) == 0) {
				return &directory[entryIndex];
			}
			HbHash_Map_PerturbateIndex(&hash, &hashIndex, hashIndexMask);
		}
	} else {
		uint32_t lowerBound = 0, upperBound = info->itemCount;
		while (lowerBound < upperBound) {
			uint32_t entryIndex = lowerBound + ((upperBound - lowerBound) >> 1);
			int32_t comparison = HbTextA_CompareCaseless(directory[entryIndex].name, name);
			if (comparison < 0) {
				upperBound = entryIndex;
			} else if (comparison > 0) {
				lowerBound = entryIndex + 1;
			} else {
				return &directory[entryIndex];
			}
		}
	}
	return HbNull;
}
