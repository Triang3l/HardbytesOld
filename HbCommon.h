#ifndef HbInclude_HbCommon
#define HbInclude_HbCommon

// Common top-level definitions - types, platform, useful macros.
// MUST be included in all deepest-level headers (those that don't include any other engine headers)!

#include <float.h> // Things like FLT_MAX.
#include <math.h> // fmin, fmax.
#include <stdarg.h>
#include <stdint.h> // Using int#_t types for consistency and because uint is shorter than unsigned int.
#include <string.h> // memcpy, memmove, memset.

#define HbNull ((void *) 0)

// Boolean with a consistent size, if C/C++ interoperability is needed.

typedef uint8_t HbBool;
#define HbFalse ((HbBool) 0)
#define HbTrue ((HbBool) 1)

// CPU architecture. Assuming little-endian.

#if defined(_M_AMD64) || defined(__x86_64__)
#define HbPlatform_CPU_x86 1
#define HbPlatform_CPU_x86_64 1
#elif defined(_M_IX86) || defined(__i386__)
#define HbPlatform_CPU_x86 1
#define HbPlatform_CPU_x86_32 1
#else
#error Unsupported target CPU.
#endif

#if defined(HbPlatform_CPU_x86_64) || defined(HbPlatform_CPU_Arm_64)
#define HbPlatform_CPU_64Bit 1
#else
#define HbPlatform_CPU_32Bit 1
#endif

// Operating system.

#if defined(_WIN32)
#define HbPlatform_OS_Windows 1
#define HbPlatform_OS_WindowsDesktop 1
#else
#error Unsupported target OS.
#endif

// Compiler.

#if defined(_MSC_VER)
#define HbPlatform_Compiler_MSVC 1
#elif defined(__GNUC__)
#define HbPlatform_Compiler_GNU 1
#else
#error Unsupported compiler.
#endif

// Alignment - HbAligned must be placed after the struct keyword.

#if defined(HbPlatform_Compiler_MSVC)
#define HbAligned(alignment) __declspec(align(alignment))
#elif defined(HbPlatform_Compiler_GNU)
#define HbAligned(alignment) __attribute__((aligned(alignment)))
#else
#error No HbAligned known for the current compiler.
#endif

// Force inlining.

#if defined(HbPlatform_Compiler_MSVC)
#define HbForceInline __forceinline
#elif defined(HbPlatform_Compiler_GNU)
#define HbForceInline __attribute__((always_inline))
#else
#error No HbForceInline known for the current compiler.
#endif

// Static array length.

#define HbArrayLength(arr) (sizeof(arr) / (sizeof((arr)[0])))

// Stack allocation.

#if defined(HbPlatform_OS_Windows)
#include <malloc.h>
#define HbStackAlloc _alloca
#else
#include <alloca.h>
#define HbStackAlloc alloca
#endif

// Common operations on numbers. For run-time floats, use fmin/fmax, not HbMinI/HbMaxI.

#define HbMinI(a, b) ((a) < (b) ? (a) : (b))
#define HbMaxI(a, b) ((a) > (b) ? (a) : (b))
#define HbClampI(value, low, high) (((value) > (high)) ? (high) : (((value) < (low)) ? (low) : (value)))
#define HbClampF(value, low, high) fminf(high, fmaxf(low, value))
#define HbAlign(value, alignment) (((value) + ((alignment) - 1u)) & ~((alignment) - 1u))

#endif
