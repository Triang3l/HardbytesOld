#include "HbFeedback.h"
#include "HbText.h"

#if defined(HbPlatform_OS_Windows)

/********************
 * Windows-specific.
 ********************/

#include <Windows.h>

void HbFeedback_DebugMessageForceV(char const * format, va_list arguments) {
	char message[1024];
	HbTextA_FormatV(message, HbArrayLength(message), format, arguments);
	size_t messageLength = HbTextA_Length(message);
	messageLength = HbMinI(HbArrayLength(message) - 2, messageLength);
	message[messageLength] = '\n';
	message[messageLength + 1] = '\0';
	OutputDebugStringA(message);
}

#if defined(HbPlatform_OS_WindowsDesktop)
void HbFeedback_CrashV(HbBool isAssert, char const * functionName, char const * messageFormat, va_list messageArguments) {
	char message[1024];
	size_t written = HbTextA_Copy(message, HbArrayLength(message), functionName);
	written += HbTextA_CopyInto(message, HbArrayLength(message), written, isAssert ? " (assertion): " : ": ");
	HbTextA_FormatV(message + written, HbArrayLength(message) - written, messageFormat, messageArguments);
	HbFeedback_DebugMessageForce("Fatal error: %s", message);
	MessageBoxA(HbNull, message, "Fatal error", MB_OK);
	HbFeedback_Break();
	_exit(EXIT_FAILURE);
}
#else
#error No HbFeedback_CrashV for the target Windows application model.
#endif

#else
#error No HbFeedback output functions for the target OS.
#endif

/******************
 * OS-independent.
 ******************/

void HbFeedback_DebugMessageForce(char const * format, ...) {
	va_list arguments;
	va_start(arguments, format);
	HbFeedback_DebugMessageForceV(format, arguments);
	va_end(arguments);
}

void HbFeedback_Crash(char const * functionName, char const * messageFormat, ...) {
	va_list messageArguments;
	va_start(messageArguments, messageFormat);
	HbFeedback_CrashV(HbFalse, functionName, messageFormat, messageArguments);
	va_end(messageArguments);
}

void HbFeedback_AssertCrash(char const * functionName, char const * messageFormat, ...) {
	va_list messageArguments;
	va_start(messageArguments, messageFormat);
	HbFeedback_CrashV(HbTrue, functionName, messageFormat, messageArguments);
	va_end(messageArguments);
}
