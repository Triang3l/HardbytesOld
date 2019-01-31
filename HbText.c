#include "HbText.h"
#include <stdio.h>

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
