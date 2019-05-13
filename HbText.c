#include "HbText.h"
#include <stdio.h>

uint32_t HbText_ClassifyUnicodeStream(void const * data, size_t size, HbBool * isU16, HbBool * u16NonNativeEndian) {
	if (size >= 2) {
		if (((size_t) data & 1) == 0) {
			uint16_t potentialBOM = *((uint16_t const *) data);
			if (potentialBOM == HbTextU16_BOM_Native || potentialBOM == HbTextU16_BOM_NonNative) {
				*isU16 = HbTrue;
				*u16NonNativeEndian = potentialBOM == HbTextU16_BOM_NonNative;
				return sizeof(HbTextU16);
			}
		}
		if (size >= 3) {
			uint8_t const * potentialBOM = (uint8_t const *) data;
			if (potentialBOM[0] == HbTextU8_BOM_0 && potentialBOM[1] == HbTextU8_BOM_1 && potentialBOM[2] == HbTextU8_BOM_2) {
				*isU16 = HbFalse;
				*u16NonNativeEndian = HbFalse;
				return 3;
			}
		}
	}
	*isU16 = HbFalse;
	*u16NonNativeEndian = HbFalse;
	return 0;
}

/********
 * ASCII
 ********/

size_t HbTextA_Copy(char * target, size_t targetBufferSize, char const * source) {
	char * originalTarget = target;
	if (targetBufferSize != 0) {
		while (--targetBufferSize != 0 && *source != '\0') {
			*(target++) = *(source++);
		}
		*target = '\0';
	}
	return (size_t) (target - originalTarget);
}

size_t HbTextA_FormatV(char * target, size_t targetBufferSize, char const * format, va_list arguments) {
	if (target == HbNull || targetBufferSize == 0) {
		// Normalize both arguments.
		target = HbNull;
		targetBufferSize = 0;
	}
	int length = vsnprintf(target, targetBufferSize, format, arguments);
	if (length < 0) {
		// Failure.
		if (targetBufferSize != 0) {
			target[0] = '\0';
		}
		return 0;
	}
	if (targetBufferSize != 0) {
		// Terminate in case it was cut off.
		target[targetBufferSize - 1] = '\0';
	}
	return (size_t) length;
}

size_t HbTextA_Format(char * target, size_t targetBufferSize, char const * format, ...) {
	va_list arguments;
	va_start(arguments, format);
	size_t length = HbTextA_FormatV(target, targetBufferSize, format, arguments);
	va_end(arguments);
	return length;
}

/********
 * UTF-8
 ********/

HbTextU32 HbTextU8_NextChar(HbTextU8 const * * cursor, size_t maxElems) {
	if (maxElems == 0) {
		return '\0';
	}
	// Force unsigned (char signedness is target-dependent).
	uint8_t const * text = (uint8_t const *) *cursor, first = text[0];
	if (first == '\0') {
		return '\0';
	}
	if ((first >> 7) == 0) {
		++(*cursor);
		return first;
	}
	// Doing && sequences in order is safe due to early exit.
	if ((first >> 5) == 6) {
		if (maxElems < 2) {
			return '\0';
		}
		if ((text[1] >> 6) == 2) {
			*cursor += 2;
			return ((HbTextU32) (first & 31) << 6) | (text[1] & 63);
		}
	}
	if ((first >> 4) == 14) {
		if (maxElems < 3) {
			return '\0';
		}
		if ((text[1] >> 6) == 2 && (text[2] >> 6) == 2) {
			*cursor += 3;
			return HbTextU32_ValidateChar(
					((HbTextU32) (first & 15) << 12) | ((HbTextU32) (text[1] & 63) << 6) | (text[2] & 63));
		}
	}
	if ((first >> 3) == 30) {
		if (maxElems < 4) {
			return '\0';
		}
		if ((text[1] >> 6) == 2 && (text[2] >> 6) == 2 && (text[3] >> 6) == 2) {
			*cursor += 4;
			return HbTextU32_ValidateChar(
					((HbTextU32) (first & 7) << 18) | ((HbTextU32) (text[1] & 63) << 12) |
							((HbTextU32) (text[2] & 63) << 6) | (text[3] & 63));
		}
	}
	++(*cursor);
	return HbText_InvalidSubstitute;
}

uint32_t HbTextU8_WriteValidChar(HbTextU8 * target, size_t targetBufferSizeElems, HbTextU32 character) {
	uint32_t elemCount = HbTextU8_ValidCharElemCount(character);
	if (targetBufferSizeElems < elemCount) {
		return 0;
	}
	switch (elemCount) {
	case 1:
		target[0] = (HbTextU8) character;
		break;
	case 2:
		target[0] = (HbTextU8) ((6 << 5) | (character >> 6));
		target[1] = (HbTextU8) ((2 << 6) | (character & 63));
		break;
	case 3:
		target[0] = (HbTextU8) ((14 << 4) | (character >> 12));
		target[1] = (HbTextU8) ((2 << 6) | ((character >> 6) & 63));
		target[2] = (HbTextU8) ((2 << 6) | (character & 63));
		break;
	case 4:
		target[0] = (HbTextU8) ((30 << 3) | (character >> 18));
		target[1] = (HbTextU8) ((2 << 6) | ((character >> 12) & 63));
		target[2] = (HbTextU8) ((2 << 6) | ((character >> 6) & 63));
		target[3] = (HbTextU8) ((2 << 6) | (character & 63));
		break;
	// Otherwise may be 0 (for the null character).
	}
	return elemCount;
}

size_t HbTextU8_FromU16(HbTextU8 * target, size_t targetBufferSizeElems, HbTextU16 const * source, HbBool nonNativeEndian) {
	HbTextU8 * targetStart = target;
	if (targetBufferSizeElems != 0) {
		--targetBufferSizeElems;
		HbTextU32 character;
		while (targetBufferSizeElems != 0 && (character = HbTextU16_NextChar(&source, nonNativeEndian)) != '\0') {
			uint32_t written = HbTextU8_WriteValidChar(target, targetBufferSizeElems, character);
			if (written == 0) {
				break;
			}
			target += written;
			targetBufferSizeElems -= written;
		}
		*target = '\0';
	}
	return (size_t) (target - targetStart);
}

/*********
 * UTF-16
 *********/

HbTextU32 HbTextU16_NextChar(HbTextU16 const * * cursor, HbBool nonNativeEndian) {
	HbTextU16 first = (*cursor)[0];
	if (first == '\0') {
		return '\0';
	}
	++(*cursor);
	if (nonNativeEndian) {
		first = HbByteSwapU16(first);
	}
	HbTextU32 character;
	if ((first >> 10) == (0xD800 >> 10)) {
		HbTextU16 second = (*cursor)[0];
		if ((second >> 10) == (0xDC00 >> 10)) {
			return HbText_InvalidSubstitute;
		}
		++(*cursor);
		if (nonNativeEndian) {
			second = HbByteSwapU16(second);
		}
		character = ((HbTextU32) (first & 0x3FF) << 10) | (second & 0x3FF);
	} else {
		character = first;
	}
	return HbTextU32_ValidateChar(character);
}

uint32_t HbTextU16_WriteValidChar(HbTextU16 * target, size_t targetBufferSizeElems, HbTextU32 character, HbBool nonNativeEndian) {
	if (targetBufferSizeElems == 0) {
		return 0;
	}
	if ((character >> 16) != 0) {
		if (targetBufferSizeElems <= 1) {
			return 0;
		}
		HbTextU16 surrogate1 = (HbTextU16) (0xD800 | ((character >> 10) - (0x10000 >> 10)));
		HbTextU16 surrogate2 = (HbTextU16) (0xDC00 | (character & 0x3FF));
		if (nonNativeEndian) {
			surrogate1 = HbByteSwapU16(surrogate1);
			surrogate2 = HbByteSwapU16(surrogate2);
		}
		target[0] = surrogate1;
		target[1] = surrogate2;
		return 2;
	}
	target[0] = nonNativeEndian ? HbByteSwapU16((HbTextU16) character) : (HbTextU16) character;
	return 1;
}

size_t HbTextU16_Copy(HbTextU16 * target, size_t targetBufferSizeElems, HbTextU16 const * source, HbBool nonNativeEndian) {
	HbTextU16 * targetStart = target;
	if (targetBufferSizeElems != 0) {
		--targetBufferSizeElems;
		HbTextU32 character;
		while (targetBufferSizeElems != 0 && (character = HbTextU16_NextChar(&source, nonNativeEndian)) != '\0') {
			uint32_t written = HbTextU16_WriteValidChar(target, targetBufferSizeElems, character, nonNativeEndian);
			if (written == 0) {
				break;
			}
			target += written;
			targetBufferSizeElems -= written;
		}
		*target = '\0';
	}
	return (size_t) (target - targetStart);
}

size_t HbTextU16_FromU8(HbTextU16 * target, size_t targetBufferSizeElems, HbTextU8 const * source, HbBool nonNativeEndian) {
	HbTextU16 * targetStart = target;
	if (targetBufferSizeElems != 0) {
		--targetBufferSizeElems;
		HbTextU32 character;
		while (targetBufferSizeElems != 0 && (character = HbTextU8_NextChar(&source, HbTextU8_MaxCharElems)) != '\0') {
			uint32_t written = HbTextU16_WriteValidChar(target, targetBufferSizeElems, character, nonNativeEndian);
			if (written == 0) {
				break;
			}
			target += written;
			targetBufferSizeElems -= written;
		}
		*target = '\0';
	}
	return (size_t) (target - targetStart);
}
