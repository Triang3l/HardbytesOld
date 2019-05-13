#ifndef HbInclude_HbText
#define HbInclude_HbText
#include "HbCommon.h"

/*
 * Zero-terminated string manipulation.
 *
 * "Element" is a single char, HbTextU8, HbTextU16, HbTextU32.
 * Counts of elements are used for offsets, buffer sizes.
 * "Character" is a whole ASCII or Unicode code point, which may consists of multiple elements in UTF-8 and UTF-16.
 * It's used for iteration, for displaying.
 *
 * "Buffer size" in function prototypes includes the terminating element, "length" doesn't.
 *
 * Functions that may be useful for concatenation (copying, formatting) return the length of what they've done in elements usually.
 * Formatting functions may be called with null target, in this case they return the element count that would've been written.
 * Zero buffer size also counts as null target in formatting.
 */

typedef char HbTextU8; // So ASCII and UTF-8 literals can be written the same way.
typedef uint16_t HbTextU16;
typedef uint32_t HbTextU32; // Whole code point.

// Replacement character for anything invalid - must be a 7-bit ASCII character.
#define HbText_InvalidSubstitute '?'

// Maximum lengths of integers converted to strings.
#define HbText_Decimal_MaxLengthU8  3  // strlen("255")
#define HbText_Decimal_MaxLengthS8  4  // strlen("-128")
#define HbText_Decimal_MaxLengthU16 5  // strlen("65535")
#define HbText_Decimal_MaxLengthS16 6  // strlen("-32768")
#define HbText_Decimal_MaxLengthU32 10 // strlen("4294967295")
#define HbText_Decimal_MaxLengthS32 11 // strlen("-2147483648")
#define HbText_Decimal_MaxLengthU64 20 // strlen("18446744073709551615")
#define HbText_Decimal_MaxLengthS64 20 // strlen("-9223372036854775808")
#define HbText_Decimal_MaxLengthSize (sizeof(size_t) > 4 ? HbText_Decimal_MaxLengthU64 : HbText_Decimal_MaxLengthU32)

// Returns the number of BOM bytes to skip. For UTF-16 to be detectable, data must be 2-aligned.
uint32_t HbText_ClassifyUnicodeStream(void const * data, size_t size, HbBool * isU16, HbBool * u16NonNativeEndian);

/********
 * ASCII
 ********/

HbForceInline HbBool HbTextA_IsSpace(char character) {
	return character == ' ' || (character >= '\t' && character <= '\r');
}
HbForceInline char HbTextA_CharToLower(char character) {
	if (character >= 'A' && character <= 'Z') {
		character += 'a' - 'A';
	}
	return character;
}
HbForceInline char HbTextA_CharToUpper(char character) {
	if (character >= 'a' && character <= 'z') {
		character -= 'a' - 'A';
	}
	return character;
}
#define HbTextA_Length strlen
#define HbTextA_Compare strcmp
#define HbTextA_ComparePart strncmp
#if HbPlatform_OS_Windows
#define HbTextA_CompareCaseless _stricmp
#define HbTextA_ComparePartCaseless _strnicmp
#else
#define HbTextA_CompareCaseless strcasecmp
#define HbTextA_ComparePartCaseless strncasecmp
#endif
size_t HbTextA_Copy(char * target, size_t targetBufferSize, char const * source);
inline size_t HbTextA_CopyInto(char * target, size_t targetBufferSize, size_t targetOffset, char const * source) {
	return (targetOffset < targetBufferSize ?
			HbTextA_Copy(target + targetOffset, targetBufferSize - targetOffset, source) : 0u);
}
size_t HbTextA_FormatV(char * target, size_t targetBufferSize, char const * format, va_list arguments);
size_t HbTextA_Format(char * target, size_t targetBufferSize, char const * format, ...);

/*******************************************************************
 * Common Unicode
 * Not very strict handling, just storage and conversion safeguards
 *******************************************************************/

inline HbBool HbTextU32_IsCharValid(HbTextU32 character) {
	// Allow only characters that can be stored in UTF-8 and UTF-16, disallow BOM and surrogates.
	return character <= 0x10FFFF && (character & 0xFFFE) != 0xFFFE && (character & ~((HbTextU32) 0x7FF)) != 0xD800;
}
HbForceInline HbTextU32 HbTextU32_ValidateChar(HbTextU32 character) {
	return HbTextU32_IsCharValid(character) ? character : HbText_InvalidSubstitute;
}

HbForceInline HbBool HbTextU32_IsASCIISpace(HbTextU32 character) {
	return character == ' ' || (character >= '\t' && character <= '\r');
}
HbForceInline HbTextU32 HbTextU32_ASCIICharToLower(HbTextU32 character) {
	if (character >= 'A' && character <= 'Z') {
		character += 'a' - 'A';
	}
	return character;
}
HbForceInline HbTextU32 HbTextA_ASCIICharToUpper(HbTextU32 character) {
	if (character >= 'a' && character <= 'z') {
		character -= 'a' - 'A';
	}
	return character;
}

/*************************************************************************
 * UTF-8 - assuming no incomplete characters
 * (invalid characters are treated as sequences of substitute characters)
 *************************************************************************/

#define HbTextU8_BOM_0 0xEF
#define HbTextU8_BOM_1 0xBB
#define HbTextU8_BOM_2 0xBF

#define HbTextU8_MaxCharElems 4

inline uint32_t HbTextU8_ValidCharElemCount(HbTextU32 character) {
	return (character > 0) + (character > 0x7F) + (character > 0x7FF) + (character > 0xFFFF);
}
inline uint32_t HbTextU8_CharElemCount(HbTextU32 character) {
	return HbTextU32_IsCharValid(character) ? HbTextU8_ValidCharElemCount(character) : 1;
}

#define HbTextU8_LengthElems HbTextA_Length
#define HbTextU8_Compare HbTextA_Compare
#define HbTextU8_CompareCaselessEnglish HbTextA_CompareCaseless

// 0 when no characters left. Advances the cursor.
// maxElems is for non-null-terminated buffers, to prevent buffer overflow when the last character is truncated
// (size_t so it can be calculated from size_t easily). It should be HbTextU8_MaxCharElems for null-terminated strings.
HbTextU32 HbTextU8_NextChar(HbTextU8 const * * cursor, size_t maxElems);
inline size_t HbTextU8_LengthChars(HbTextU8 const * text) {
	size_t length = 0;
	while (HbTextU8_NextChar(&text, HbTextU8_MaxCharElems) != '\0') {
		++length;
	}
	return length;
};
inline size_t HbTextU8_LengthU16Elems(HbTextU8 const * text) {
	size_t length = 0;
	HbTextU32 character;
	while ((character = HbTextU8_NextChar(&text, HbTextU8_MaxCharElems)) != '\0') {
		length += 1 + ((character >> 16) != 0);
	}
	return length;
}

// Places a character in the buffer if possible, returning the number of elements actually written.
// Does not null-terminate (and the buffer size doesn't include the terminator)!
uint32_t HbTextU8_WriteValidChar(HbTextU8 * target, size_t targetBufferSizeElems, HbTextU32 character);

// Allocate HbTextU16_LengthU8Elems elements for this.
size_t HbTextU8_FromU16(HbTextU8 * target, size_t targetBufferSizeElems, HbTextU16 const * source, HbBool nonNativeEndian);

/*********
 * UTF-16
 *********/

#define HbTextU16_BOM_Native 0xFEFF
#define HbTextU16_BOM_NonNative 0xFFFE

#define HbTextU16_MaxCharElems 2

HbTextU32 HbTextU16_NextChar(HbTextU16 const * * cursor, size_t maxElems, HbBool nonNativeEndian); // 0 when no characters left. Advances the cursor.

// No validation - returns real data size (for appending)!
inline size_t HbTextU16_LengthElems(HbTextU16 const * text) {
	HbTextU16 const * start = text;
	while (*(text++) != '\0') {}
	return (size_t) (text - start);
}
inline size_t HbTextU16_LengthChars(HbTextU16 const * text, HbBool nonNativeEndian) {
	size_t length = 0;
	while (HbTextU16_NextChar(&text, HbTextU16_MaxCharElems, nonNativeEndian) != '\0') {
		++length;
	}
	return length;
};
inline size_t HbTextU16_LengthU8Elems(HbTextU16 const * text, HbBool nonNativeEndian) {
	size_t length = 0;
	HbTextU32 character;
	while ((character = HbTextU16_NextChar(&text, HbTextU16_MaxCharElems, nonNativeEndian)) != '\0') {
		length += HbTextU8_ValidCharElemCount(character);
	}
	return length;
}

// Places a character in the buffer if possible, returning the number of elements actually written.
// Does not null-terminate (and the buffer size doesn't include the terminator)!
uint32_t HbTextU16_WriteValidChar(HbTextU16 * target, size_t targetBufferSizeElems, HbTextU32 character, HbBool nonNativeEndian);

// This actually validates characters - size in elements may be changed if there are invalid characters!
size_t HbTextU16_Copy(HbTextU16 * target, size_t targetBufferSizeElems, HbTextU16 const * source, HbBool nonNativeEndian);

// Allocate HbTextU8_LengthU16Elems elements for this.
size_t HbTextU16_FromU8(HbTextU16 * target, size_t targetBufferSizeElems, HbTextU8 const * source, HbBool nonNativeEndian);

#endif
