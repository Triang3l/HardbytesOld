#include "HbBit.h"
#include "HbFeedback.h"
#include "HbHash.h"
#include "HbMemory.h"

uint32_t HbHash_MapUtil_GetNeededEntriesLog2(uint32_t usedCount) {
	if (usedCount == 0) {
		return 0;
	}
	if (usedCount > HbHash_MapUtil_MaxUsedEntries) {
		return UINT32_MAX;
	}
	uint32_t entryCount = usedCount + 1;
	uint32_t indexBitCount = HbBit_Log2CeilU32(entryCount);
	// If able to expand to reduce collisions, try to expand when load is above 2/3. Safe to multiply 0x3FFFFFFFu by 3.
	if (indexBitCount < HbHash_MapUtil_MaxEntriesLog2 && (usedCount * 3) >= ((uint32_t) 1 << (indexBitCount + 1))) {
		++indexBitCount;
	}
	return HbMaxU32(indexBitCount, HbHash_MapUtil_MinEntriesLog2);
}

void HbHash_Map_DoInit(HbHash_Map * map, HbMemory_Tag * tag, size_t elementSize,
		HbHash_Map_Functions const * functions, char const * fileName, uint32_t fileLine) {
	map->tag = tag;
	map->fileName = fileName;
	map->fileLine = fileLine;
	map->elementSize = elementSize;
	map->functions = functions;
	map->elementsAndTypeBits = NULL;
	map->indexBitCount = 0;
	map->elementCount = 0;
	map->elementAndRemovedCount = 0;
}

void HbHash_Map_Destroy(HbHash_Map * map) {
	HbMemory_Free(map->elementsAndTypeBits);
}

HbForceInline uint32_t const * HbHashi_Map_GetEntryTypesC(HbHash_Map const * map) {
	return (uint32_t const *) ((uint8_t const *) map->elementsAndTypeBits +
			HbAlignSize(map->elementSize << map->indexBitCount, sizeof(uint32_t)));
}

HbForceInline uint32_t * HbHashi_Map_GetEntryTypes(HbHash_Map * map) {
	return (uint32_t *) HbHashi_Map_GetEntryTypesC(map);
}

// Reserves and/or defragments.
static HbBool HbHashi_Map_Reallocate(HbHash_Map * map, uint32_t newIndexBitCount) {
	HbFeedback_Assert(newIndexBitCount <= HbHash_MapUtil_MaxEntriesLog2, "HbHashi_Map_Reallocate",
			"Too many bits requested for indices (%u) in the hash map created at %s:%u.",
			newIndexBitCount, map->fileName, map->fileLine);
	newIndexBitCount = HbMaxU32(newIndexBitCount, HbHash_MapUtil_GetNeededEntriesLog2(map->elementCount));
	if (newIndexBitCount == map->indexBitCount && map->elementCount == map->elementAndRemovedCount) {
		return HbFalse;
	}
	uint8_t * newElementsAndTypeBits = NULL;
	if (newIndexBitCount != 0) {
		size_t newElementsSize = map->elementSize << newIndexBitCount;
		size_t newEntryTypesOffset = HbAlignSize(newElementsSize, sizeof(uint32_t));
		// Groups of two uint32 bit masks for 32 entries.
		size_t newEntryTypesSize = (size_t) ((uint32_t) 1 << (HbMaxU32(newIndexBitCount, 5) - 5)) * (sizeof(uint32_t) * 2);
		newElementsAndTypeBits = (uint8_t *) HbMemory_DoAlloc(
				map->tag, newEntryTypesOffset + newEntryTypesSize, HbTrue, map->fileName, map->fileLine, HbTrue);
		uint32_t * newEntryTypeBits = (uint32_t *) ((uint8_t *) newElementsAndTypeBits + newEntryTypesOffset);
		memset(newEntryTypeBits, 0, newEntryTypesSize);

		// Defragment the old map into the new one - copy actually existing elements.
		uint8_t const * oldElements = (uint8_t const *) map->elementsAndTypeBits;
		uint32_t const * oldEntryTypeBits = HbHashi_Map_GetEntryTypesC(map);
		uint32_t oldEntryTypeBlockOffset = 0;
		uint32_t newIndexMask = ((uint32_t) 1 << newIndexBitCount) - 1;
		uint32_t elementsRemaining = map->elementCount;
		while (elementsRemaining != 0) {
			// [1], [3]... contain which elements actually exist.
			uint32_t oldEntryTypeBlock = oldEntryTypeBits[oldEntryTypeBlockOffset * 2 + 1];
			while (oldEntryTypeBlock != 0) {
				uint32_t oldEntryIndexInBlock = HbBit_LowestOneU32(oldEntryTypeBlock);
				oldEntryTypeBlock &= ~((uint32_t) 1 << oldEntryIndexInBlock);
				// Insert into the new hash map.
				uint8_t const * oldElement = oldElements + ((oldEntryTypeBlockOffset << 5) + oldEntryIndexInBlock) * map->elementSize;
				uint32_t elementHash = map->functions->hashKey(map->functions->elementKey(oldElement));
				uint32_t newElementIndex = elementHash & newIndexMask;
				while (newEntryTypeBits[newElementIndex >> 5] & ((uint32_t) 1 << (newElementIndex & 31))) {
					HbHash_MapUtil_PerturbateIndex(&elementHash, &newElementIndex, newIndexMask);
				}
				memcpy(newElementsAndTypeBits + newElementIndex * map->elementSize, oldElement, map->elementSize);
				uint32_t newElementIndexInBlock = (uint32_t) 1 << (newElementIndex & 31);
				newEntryTypeBits[newElementIndex >> 5] |= newElementIndexInBlock; // Used.
				newEntryTypeBits[(newElementIndex >> 5) + 1] |= newElementIndexInBlock; // Existing element.
				--elementsRemaining;
			}
			++oldEntryTypeBlockOffset;
		}
	}
	HbMemory_Free(map->elementsAndTypeBits);
	map->elementsAndTypeBits = newElementsAndTypeBits;
	map->indexBitCount = newIndexBitCount;
	map->elementAndRemovedCount = map->elementCount; // Was defragmented.
	return HbTrue;
}

void * HbHash_Map_FindOrCreate(HbHash_Map * map, void const * key, HbBool * created) {
	if (map->indexBitCount == 0) {
		// Adding the first element.
		HbHashi_Map_Reallocate(map, HbHash_MapUtil_MinEntriesLog2);
	}
	uint32_t keyHash = map->functions->hashKey(key);
	// Try to find an existing entry.
	uint32_t * entryTypes = HbHashi_Map_GetEntryTypes(map);
	uint32_t indexMask = ((uint32_t) 1 << map->indexBitCount) - 1;
	uint32_t hash = keyHash;
	uint32_t index = hash & indexMask;
	uint32_t insertIndex = UINT32_MAX; // With the shortest path.
	HbBool replaceRemoved = HbFalse;
	for (;;) {
		uint32_t const * entryTypesBlock = &(entryTypes[(index >> 5) * 2]);
		uint32_t entryTypeBit = (uint32_t) 1 << (index & 31);
		if ((entryTypesBlock[0] & entryTypeBit) == 0) {
			if (insertIndex == UINT32_MAX) {
				insertIndex = index;
			}
			break;
		}
		if ((entryTypesBlock[1] & entryTypeBit) != 0) {
			void * element = (uint8_t *) map->elementsAndTypeBits + index * map->elementSize;
			if (map->functions->compareKeys(map->functions->elementKey(element), key)) {
				if (created != NULL) {
					*created = HbFalse;
				}
				return element;
			}
		} else {
			if (insertIndex == UINT32_MAX) {
				insertIndex = index;
				replaceRemoved = HbTrue;
			}
		}
		HbHash_MapUtil_PerturbateIndex(&hash, &index, indexMask);
	}
	if (!replaceRemoved) {
		// Need to create a new element, increasing the load. Check if it's time to expand the storage.
		HbBool reallocationNeeded = HbFalse;
		uint32_t indexBitCountNeeded = HbHash_MapUtil_GetNeededEntriesLog2(map->elementAndRemovedCount + 1);
		uint32_t indexBitCountNeededForElements = HbHash_MapUtil_GetNeededEntriesLog2(map->elementCount + 1);
		if (indexBitCountNeeded == UINT32_MAX) {
			if (indexBitCountNeededForElements == UINT32_MAX) {
				HbFeedback_Crash("HbHash_Map_FindOrCreate", "Too many elements added to a hash map created at %s:%u, maximum 0x%X.",
						map->fileName, map->fileLine, HbHash_MapUtil_MaxUsedEntries);
			}
			// Need to defragment the map anyway - no path containing removed elements found, so have to use a new entry, but no space for one.
			reallocationNeeded = HbTrue;
			indexBitCountNeeded = indexBitCountNeededForElements;
		}
		// As entries might have been explicitly reserved, only increase the size of the table.
		if (indexBitCountNeeded > map->indexBitCount) {
			reallocationNeeded = HbTrue;
			// In case too many elements have been removed, actually increasing the size may be unnecessary, so only defragment.
			if (indexBitCountNeededForElements < map->indexBitCount) {
				indexBitCountNeeded = map->indexBitCount;
			}
		}
		if (reallocationNeeded && HbHashi_Map_Reallocate(map, indexBitCountNeeded)) {
			// No removed elements now, find only the tail.
			entryTypes = HbHashi_Map_GetEntryTypes(map);
			indexMask = ((uint32_t) 1 << map->indexBitCount) - 1;
			hash = keyHash;
			index = hash & indexMask;
			for (;;) {
				if ((entryTypes[(index >> 5) * 2] & ((uint32_t) 1 << (index & 31))) == 0) {
					insertIndex = index;
					break;
				}
				HbHash_MapUtil_PerturbateIndex(&hash, &index, indexMask);
			}
		}
	}
	uint32_t * newEntryTypesBlock = &(entryTypes[(insertIndex >> 5) * 2]);
	uint32_t newEntryTypeBit = (uint32_t) 1 << (insertIndex & 31);
	newEntryTypesBlock[0] |= newEntryTypeBit; // Used entry.
	newEntryTypesBlock[1] |= newEntryTypeBit; // Element.
	if (created != NULL) {
		*created = HbTrue;
	}
	return (uint8_t *) map->elementsAndTypeBits + insertIndex * map->elementSize;
}

void const * HbHash_Map_FindC(HbHash_Map const * map, void const * key) {
	if (map->indexBitCount == 0) {
		return NULL;
	}
	uint32_t hash = map->functions->hashKey(key);
	uint32_t indexMask = ((uint32_t) 1 << map->indexBitCount) - 1;
	uint32_t index = hash & indexMask;
	uint32_t const * entryTypes = HbHashi_Map_GetEntryTypesC(map);
	for (;;) {
		uint32_t const * entryTypesBlock = &(entryTypes[(index >> 5) * 2]);
		uint32_t entryTypeBit = (uint32_t) 1 << (index & 31);
		if ((entryTypesBlock[0] & entryTypeBit) == 0) {
			return NULL; // Reached an unused entry - end of search.
		}
		if ((entryTypesBlock[1] & entryTypeBit) != 0) {
			void const * element = (uint8_t const *) map->elementsAndTypeBits + index * map->elementSize;
			if (map->functions->compareKeys(map->functions->elementKey(element), key)) {
				return element;
			}
		}
		HbHash_MapUtil_PerturbateIndex(&hash, &index, indexMask);
	}
	return NULL;
}

void * HbHash_Map_Remove(HbHash_Map * map, void const * key) {
	if (map->indexBitCount == 0) {
		return NULL;
	}
	uint32_t hash = map->functions->hashKey(key);
	uint32_t indexMask = ((uint32_t) 1 << map->indexBitCount) - 1;
	uint32_t index = hash & indexMask;
	uint32_t * entryTypes = HbHashi_Map_GetEntryTypes(map);
	for (;;) {
		uint32_t * entryTypesBlock = &(entryTypes[(index >> 5) * 2]);
		uint32_t entryTypeBit = (uint32_t) 1 << (index & 31);
		if ((entryTypesBlock[0] & entryTypeBit) == 0) {
			return NULL; // Reached an unused entry - end of search.
		}
		if ((entryTypesBlock[1] & entryTypeBit) != 0) {
			void * element = (uint8_t *) map->elementsAndTypeBits + index * map->elementSize;
			if (map->functions->compareKeys(map->functions->elementKey(element), key)) {
				entryTypesBlock[1] &= ~entryTypeBit;
				return element;
			}
		}
		HbHash_MapUtil_PerturbateIndex(&hash, &index, indexMask);
	}
	return NULL;
}

HbBool HbHash_Map_Reserve(HbHash_Map * map, uint32_t elementCount) {
	uint32_t indexBitCountNeeded = HbHash_MapUtil_GetNeededEntriesLog2(elementCount);
	if (indexBitCountNeeded == UINT32_MAX) {
		HbFeedback_Crash("HbHash_Map_Reserve", "Too many elements (0x%X) requested in a hash map created at %s:%u, maximum 0x%X.",
				elementCount, map->fileName, map->fileLine, HbHash_MapUtil_MaxUsedEntries);
	}
	if (indexBitCountNeeded <= map->indexBitCount) {
		return HbFalse;
	}
	return HbHashi_Map_Reallocate(map, indexBitCountNeeded);
}

HbBool HbHash_Map_Defragment(HbHash_Map * map, uint32_t reserveElementCount) {
	uint32_t indexBitCountNeeded = HbHash_MapUtil_GetNeededEntriesLog2(reserveElementCount);
	if (indexBitCountNeeded == UINT32_MAX) {
		HbFeedback_Crash("HbHash_Map_Defragment", "Too many elements (0x%X) requested in a hash map created at %s:%u, maximum 0x%X.",
				reserveElementCount, map->fileName, map->fileLine, HbHash_MapUtil_MaxUsedEntries);
	}
	indexBitCountNeeded = HbMaxU32(HbHash_MapUtil_GetNeededEntriesLog2(map->elementCount), indexBitCountNeeded);
	return HbHashi_Map_Reallocate(map, indexBitCountNeeded);
}
