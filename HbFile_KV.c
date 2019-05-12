#include "HbFeedback.h"
#include "HbFile_KV.h"

void HbFile_KV_Read_Init(HbFile_KV_Read_Context * context, void const * data, size_t size, HbBool useEscapeSequences) {
	uint32_t bomSize = HbText_ClassifyUnicodeStream(data, size, &context->isU16, &context->u16NonNativeEndian);
	context->data.u8 = (uint8_t const *) data + bomSize;
	context->size = size - bomSize;
	if (context->isU16) {
		context->size &= ~(sizeof(HbTextU16) - 1);
	}
	context->useEscapeSequences = useEscapeSequences;
	context->sectionDepth = 0;
	context->keyString.position = SIZE_MAX;
	context->keyString.quoted = HbFalse;
	context->valueString.position = SIZE_MAX;
	context->valueString.quoted = HbFalse;
	context->readResumePosition = 0;
}

static HbTextU32 HbFile_KV_Read_GetCharacter(HbFile_KV_Read_Context const * context, size_t positionBytes, uint32_t * bytesToNext) {
	if (positionBytes >= context->size) {
		if (bytesToNext != HbNull) {
			*bytesToNext = 0;
		}
		return '\0';
	}
	HbTextU32 character;
	if (context->isU16) {
		HbFeedback_Assert((positionBytes & (sizeof(HbTextU16) - 1)) == 0, "HbFile_KV_Read_GetCharacter",
				"Position must be UTF-16 element-aligned.");
		HbTextU16 const * cursorStart = &context->data.u16[positionBytes / sizeof(HbTextU16)], * cursor = cursorStart;
		character = HbTextU16_NextChar(&cursor, context->u16NonNativeEndian);
		if (character == '\r') {
			HbTextU16 const * cursorLF = cursor;
			if (HbTextU16_NextChar(&cursorLF, context->u16NonNativeEndian) == '\n') {
				character = '\n';
				cursor = cursorLF;
			}
		}
		if (bytesToNext != HbNull) {
			*bytesToNext = (uint32_t) ((cursor - cursorStart) * sizeof(HbTextU16));
		}
	} else {
		HbTextU8 const * cursorStart = &context->data.u8[positionBytes], * cursor = cursorStart;
		character = HbTextU8_NextChar(&cursor);
		if (character == '\r') {
			HbTextU8 const * cursorLF = cursor;
			if (HbTextU8_NextChar(&cursorLF) == '\n') {
				character = '\n';
				cursor = cursorLF;
			}
		}
		if (bytesToNext != HbNull) {
			*bytesToNext = (uint32_t) (cursor - cursorStart);
		}
	}
	return character;
}

HbTextU32 HbFile_KV_Read_GetStringCharacter(HbFile_KV_Read_Context const * context,
		HbFile_KV_Read_String const * kvString, size_t offset, uint32_t * bytesToNext) {
	offset += kvString->position;
	uint32_t characterSize;
	HbTextU32 character = HbFile_KV_Read_GetCharacter(context, offset, &characterSize);
	if (kvString->quoted) {
		if (character == '"') {
			character = '\0';
		} else if (character == '\\' && context->useEscapeSequences) {
			uint32_t escapedCharacterSize;
			HbTextU32 escapedCharacter = HbFile_KV_Read_GetCharacter(context, offset + characterSize, &escapedCharacterSize);
			switch (escapedCharacter) {
			case 'n': character = '\n'; break;
			case 't': character = '\t'; break;
			case 'v': character = '\v'; break;
			case 'b': character = '\b'; break;
			case 'r': character = '\r'; break;
			case 'f': character = '\f'; break;
			case 'a': character = '\a'; break;
			case '\\': character = '\\'; break;
			case '?': character = '?'; break;
			case '\'': character = '\''; break;
			case '"': character = '"'; break;
			default: escapedCharacterSize = 0; break; // Just return the backslash itself.
			}
			characterSize += escapedCharacterSize;
		}
	} else {
		if (HbTextU32_IsASCIISpace(character) || character == '"' || character == '{' || character == '}') {
			character = '\0';
		}
	}
	if (bytesToNext != HbNull) {
		*bytesToNext = character != '\0' ? characterSize : 0;
	}
	return character;
}

size_t HbFile_KV_Read_StringLengthU8Elems(HbFile_KV_Read_Context const * context, HbFile_KV_Read_String const * kvString) {
	size_t offset = 0, elems = 0;
	HbTextU32 character;
	uint32_t bytesToNext;
	while ((character = HbFile_KV_Read_GetStringCharacter(context, kvString, offset, &bytesToNext)) != '\0') {
		offset += bytesToNext;
		elems += HbTextU8_CharElemCount(character);
	}
	return elems;
}

size_t HbFile_KV_Read_CopyStringU8(HbFile_KV_Read_Context const * context, HbFile_KV_Read_String const * kvString,
		HbTextU8 * target, size_t targetBufferSizeElems) {
	HbTextU8 * originalTarget = target;
	size_t sourceOffset = 0;
	if (targetBufferSizeElems != 0) {
		--targetBufferSizeElems;
		HbTextU32 character;
		uint32_t bytesToNext;
		while (targetBufferSizeElems != 0 && (character = HbFile_KV_Read_GetStringCharacter(context, kvString, sourceOffset, &bytesToNext)) != '\0') {
			sourceOffset += bytesToNext;
			uint32_t written = HbTextU8_WriteValidChar(target, targetBufferSizeElems, character);
			if (written == 0) {
				break;
			}
			target += written;
			targetBufferSizeElems -= written;
		}
		*target = '\0';
	}
	return (size_t) (target - originalTarget);
}

HbBool HbFile_KV_Read_StringEqualsCaselessA(HbFile_KV_Read_Context const * context, HbFile_KV_Read_String const * kvString, char const * compareString) {
	size_t characterIndex = 0;
	size_t kvOffset = 0;
	for (;;) {
		uint32_t bytesToNext;
		char compareChar = HbTextA_CharToLower(compareString[characterIndex]);
		if (HbTextU32_ASCIICharToLower(HbFile_KV_Read_GetStringCharacter(context, kvString, kvOffset, &bytesToNext)) != compareChar) {
			return HbFalse;
		}
		if (compareChar == '\0') {
			return HbTrue;
		}
		++characterIndex;
		kvOffset += bytesToNext;
	}
}

static void HbFile_KV_Read_SkipSeparators(HbFile_KV_Read_Context * context) {
	HbTextU32 character;
	uint32_t characterSize;
	HbBool inComment = HbFalse;
	while ((character = HbFile_KV_Read_GetCharacter(context, context->readResumePosition, &characterSize)) != '\0') {
		if (inComment) {
			if (character == '\r' || character == '\n') {
				inComment = HbFalse;
			}
		} else {
			if (character == '/') {
				uint32_t secondCommentCharacterSize;
				if (HbFile_KV_Read_GetCharacter(context, context->readResumePosition + characterSize, &secondCommentCharacterSize) == '/') {
					inComment = HbTrue;
					characterSize += secondCommentCharacterSize;
				} else {
					break;
				}
			} else if (!HbTextU32_IsASCIISpace(character)) {
				break;
			}
		}
		context->readResumePosition += characterSize;
	}
}

HbFile_KV_Read_TokenType HbFile_KV_Read_Parse(HbFile_KV_Read_Context * context) {
	// Clear the token state.
	context->keyString.position = SIZE_MAX;
	context->keyString.quoted = HbFalse;
	context->valueString.position = SIZE_MAX;
	context->valueString.quoted = HbFalse;

	// Try to read the key or }.
	HbFile_KV_Read_SkipSeparators(context);
	uint32_t bytesToNext;
	HbTextU32 character = HbFile_KV_Read_GetCharacter(context, context->readResumePosition, &bytesToNext);
	if (character == '\0') {
		return HbFile_KV_Read_TokenType_End;
	}
	if (character == '{') {
		// Section without a key - corrupt file.
		context->readResumePosition = context->size;
		return HbFile_KV_Read_TokenType_End;
	}
	if (character == '}') {
		if (context->sectionDepth == 0) {
			// Unbalanced sections - corrupt file.
			context->readResumePosition = context->size;
			return HbFile_KV_Read_TokenType_End;
		}
		context->readResumePosition += bytesToNext;
		--context->sectionDepth;
		return HbFile_KV_Read_TokenType_SectionEnd;
	}
	HbFile_KV_Read_String keyString;
	keyString.quoted = character == '"';
	if (keyString.quoted) {
		context->readResumePosition += bytesToNext;
	}
	keyString.position = context->readResumePosition;
	while ((character = HbFile_KV_Read_GetStringCharacter(
			context, &keyString, context->readResumePosition - keyString.position, &bytesToNext)) != '\0') {
		context->readResumePosition += bytesToNext;
	}
	if (keyString.quoted) {
		character = HbFile_KV_Read_GetCharacter(context, context->readResumePosition, &bytesToNext);
		if (character != '"') {
			// Unclosed quoted string - corrupt file.
			context->readResumePosition = context->size;
			return HbFile_KV_Read_TokenType_End;
		}
		context->readResumePosition += bytesToNext;
	}

	// Try to read the value or {.
	HbFile_KV_Read_SkipSeparators(context);
	character = HbFile_KV_Read_GetCharacter(context, context->readResumePosition, &bytesToNext);
	if (character == '\0') {
		return HbFile_KV_Read_TokenType_End;
	}
	if (character == '{') {
		context->readResumePosition += bytesToNext;
		++context->sectionDepth;
		context->keyString = keyString;
		return HbFile_KV_Read_TokenType_SectionStart;
	}
	HbFile_KV_Read_String valueString;
	valueString.quoted = character == '"';
	if (valueString.quoted) {
		context->readResumePosition += bytesToNext;
	}
	valueString.position = context->readResumePosition;
	while ((character = HbFile_KV_Read_GetStringCharacter(
			context, &valueString, context->readResumePosition - valueString.position, &bytesToNext)) != '\0') {
		context->readResumePosition += bytesToNext;
	}
	if (valueString.quoted) {
		character = HbFile_KV_Read_GetCharacter(context, context->readResumePosition, &bytesToNext);
		if (character != '"') {
			// Unclosed quoted string - corrupt file.
			context->readResumePosition = context->size;
			return HbFile_KV_Read_TokenType_End;
		}
		context->readResumePosition += bytesToNext;
	}
	context->keyString = keyString;
	context->valueString = valueString;
	return HbFile_KV_Read_TokenType_KeyValue;
}
