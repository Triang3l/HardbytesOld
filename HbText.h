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

/********
 * ASCII
 ********/

#define HbTextA_Length strlen
#define HbTextA_Compare strcmp
#define HbTextA_ComparePart strncmp
#ifdef HbPlatform_OS_Windows
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

#endif
