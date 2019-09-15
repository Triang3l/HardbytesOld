#ifndef HbInclude_HbFile_KV
#define HbInclude_HbFile_KV
#include "HbText.h"
#ifdef __cplusplus
extern "C" {
#endif

// Source Engine text key/value pairs, in UTF-8 or UTF-16 format (determined by the BOM).

typedef struct HbFile_KV_Read_String {
	size_t quoted : 1; // Use 2x less space than with a regular HbBool.
	size_t position : sizeof(size_t) * 8 - 1;
} HbFile_KV_Read_String;

typedef struct HbFile_KV_Read_Context {
	union {
		HbTextU8 const * u8;
		HbTextU16 const * u16;
	} data;
	size_t size;
	HbBool isU16;
	HbBool u16NonNativeEndian;
	HbBool useEscapeSequences;
	// Mutable part.
	uint32_t sectionDepth; // 0 when no { has been reached yet. Can't go below 0, } without an { will result in an "end" token.
	HbFile_KV_Read_String keyString; // Key or section name.
	HbFile_KV_Read_String valueString;
	// Internal mutable part.
	size_t readResumePosition;
} HbFile_KV_Read_Context;

// Input must be 2-aligned (may be UTF-16).
void HbFile_KV_Read_Init(HbFile_KV_Read_Context * context, void const * data, size_t size, HbBool useEscapeSequences);

HbTextU32 HbFile_KV_Read_GetStringCharacter(HbFile_KV_Read_Context const * context,
		HbFile_KV_Read_String const * kvString, size_t offset, uint32_t * bytesToNext);

size_t HbFile_KV_Read_StringLengthU8Elems(HbFile_KV_Read_Context const * context, HbFile_KV_Read_String const * kvString);
HbForceInline size_t HbFile_KV_Read_KeyLengthU8Elems(HbFile_KV_Read_Context const * context) {
	return HbFile_KV_Read_StringLengthU8Elems(context, &context->keyString);
}
HbForceInline size_t HbFile_KV_Read_ValueLengthU8Elems(HbFile_KV_Read_Context const * context) {
	return HbFile_KV_Read_StringLengthU8Elems(context, &context->valueString);
}

// Returns the number of copied UTF-8 elements (allocate HbFile_KV_Read_StringLengthU8Elems + 1 for the entire string).
size_t HbFile_KV_Read_CopyStringU8(HbFile_KV_Read_Context const * context, HbFile_KV_Read_String const * kvString,
		HbTextU8 * target, size_t targetBufferSizeElems);
HbForceInline size_t HbFile_KV_Read_CopyKeyU8(HbFile_KV_Read_Context const * context, HbTextU8 * target, size_t targetBufferSizeElems) {
	return HbFile_KV_Read_CopyStringU8(context, &context->keyString, target, targetBufferSizeElems);
}
HbForceInline size_t HbFile_KV_Read_CopyValueU8(HbFile_KV_Read_Context const * context, HbTextU8 * target, size_t targetBufferSizeElems) {
	return HbFile_KV_Read_CopyStringU8(context, &context->valueString, target, targetBufferSizeElems);
}

HbBool HbFile_KV_Read_StringEqualsCaselessA(HbFile_KV_Read_Context const * context, HbFile_KV_Read_String const * kvString, char const * compareString);
HbForceInline HbBool HbFile_KV_Read_KeyEquals(HbFile_KV_Read_Context const * context, char const * compareString) {
	return HbFile_KV_Read_StringEqualsCaselessA(context, &context->keyString, compareString);
}
HbForceInline HbBool HbFile_KV_Read_ValueEqualsCaselessA(HbFile_KV_Read_Context const * context, char const * compareString) {
	return HbFile_KV_Read_StringEqualsCaselessA(context, &context->valueString, compareString);
}

typedef enum HbFile_KV_Read_TokenType {
	HbFile_KV_Read_TokenType_End, // End of the file or syntax error.
	HbFile_KV_Read_TokenType_KeyValue, // key value.
	HbFile_KV_Read_TokenType_SectionStart, // key {, sectionDepth already incremented.
	HbFile_KV_Read_TokenType_SectionEnd, // }, sectionDepth already decremented.
} HbFile_KV_Read_TokenType;
HbFile_KV_Read_TokenType HbFile_KV_Read_Parse(HbFile_KV_Read_Context * context);

#ifdef __cplusplus
}
#endif
#endif
