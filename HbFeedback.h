#ifndef HbInclude_HbFeedback
#define HbInclude_HbFeedback
#include "HbCommon.h"
#if HbPlatform_OS_Windows
#include <intrin.h>
#endif

#ifdef _DEBUG
#define HbFeedback_DebugBuild 1
#endif

#if HbPlatform_Compiler_MSVC
#define HbFeedback_StaticAssert static_assert
#else
#define HbFeedback_StaticAssert _Static_assert
#endif

#if HbPlatform_OS_Windows
#define HbFeedback_Break __debugbreak
#else
#error No HbFeedback_Break implementation for the target OS.
#endif

// OS-specific.
void HbFeedback_DebugMessageForceV(char const * format, va_list arguments);
void HbFeedback_CrashV(HbBool isAssert, char const * functionName, char const * messageFormat, va_list messageArguments);

// Non-OS-specific.
void HbFeedback_DebugMessageForce(char const * format, ...);
void HbFeedback_Crash(char const * functionName, char const * messageFormat, ...);
void HbFeedback_AssertCrash(char const * functionName, char const * messageFormat, ...);
#if HbFeedback_DebugBuild
#define HbFeedback_DebugMessage(format, ...) HbFeedback_DebugMessageForce((format), __VA_ARGS__)
#define HbFeedback_Assert(condition, functionName, messageFormat, ...) \
	{ if (!(condition)) HbFeedback_AssertCrash((functionName), (messageFormat), __VA_ARGS__); }
#else
#define HbFeedback_DebugMessage(format, ...) {}
#define HbFeedback_Assert(condition, functionName, messageFormat, ...) {}
#endif

#endif
