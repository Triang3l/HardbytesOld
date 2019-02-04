#include "HbText.h"
#include <stdio.h>

/********
 * ASCII
 ********/

size_t HbTextA_Copy(char * target, size_t targetBufferSize, char const * source) {
	char * originalTarget = target;
	if (targetBufferSize != 0) {
		while (targetBufferSize != 0 && *source != '\0') {
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

HbTextU32 HbTextU8_NextChar(HbTextU8 const * * cursor) {
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
		if ((text[1] >> 6) == 2) {
			*cursor += 2;
			return ((HbTextU32) (first & 31) << 6) | (text[1] & 63);
		}
	}
	if ((first >> 4) == 14) {
		if ((text[1] >> 6) == 2 && (text[2] >> 6) == 2) {
			*cursor += 3;
			return HbTextU32_ValidateChar(
					((HbTextU32) (first & 15) << 12) | ((HbTextU32) (text[1] & 63) << 6) | (text[2] & 63));
		}
	}
	if ((first >> 3) == 30) {
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

/*********
 * UTF-16
 *********/

HbTextU32 HbTextU16_NextChar(HbTextU16 const * * cursor) {
	HbTextU16 first = (*cursor)[0];
	if (first == '\0') {
		return '\0';
	}
	++(*cursor);
	HbTextU32 character;
	if ((first >> 10) == (0xD800 >> 10)) {
		HbTextU16 second = (*cursor)[0];
		if ((second >> 10) == (0xDC00 >> 10)) {
			return HbText_InvalidSubstitute;
		}
		++(*cursor);
		character = ((HbTextU32) (first & 0x3FF) << 10) | (second & 0x3FF);
	} else {
		character = first;
	}
	return HbTextU32_ValidateChar(character);
}

uint32_t HbTextU16_WriteValidChar(HbTextU16 * target, size_t targetBufferSizeElems, HbTextU32 character) {
	if (targetBufferSizeElems == 0) {
		return 0;
	}
	if ((character >> 16) != 0) {
		if (targetBufferSizeElems <= 1) {
			return 0;
		}
		target[0] = 0xD800 | ((character >> 10) - (0x10000 >> 10));
		target[1] = 0xDC00 | (character & 0x3FF);
		return 2;
	}
	target[0] = (HbTextU16) character;
	return 1;
}

size_t HbTextU16_Copy(HbTextU16 * target, size_t targetBufferSizeElems, HbTextU16 const * source) {
	HbTextU16 * targetStart = target;
	if (targetBufferSizeElems != 0) {
		--targetBufferSizeElems;
		HbTextU32 character;
		while (targetBufferSizeElems != 0 && (character = HbTextU16_NextChar(&source)) != '\0') {
			uint32_t written = HbTextU16_WriteValidChar(target, targetBufferSizeElems, character);
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

size_t HbTextU16_FromU8(HbTextU16 * target, size_t targetBufferSizeElems, HbTextU8 const * source) {
	HbTextU16 * targetStart = target;
	if (targetBufferSizeElems != 0) {
		--targetBufferSizeElems;
		HbTextU32 character;
		while (targetBufferSizeElems != 0 && (character = HbTextU8_NextChar(&source)) != '\0') {
			uint32_t written = HbTextU16_WriteValidChar(target, targetBufferSizeElems, character);
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
