#ifndef HbInclude_HbCommon
#define HbInclude_HbCommon

// Common top-level definitions - types, platform, useful macros.
// MUST be included in all deepest-level headers (those that don't include any other engine headers)!

#include <float.h> // Things like FLT_MAX.
#include <math.h> // fmin, fmax.
#include <stdarg.h>
#include <stdint.h> // Using int#_t types for consistency and because uint is shorter than unsigned int.
#include <stdlib.h> // Things like abs.
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

#if HbPlatform_CPU_x86_64 || HbPlatform_CPU_Arm_64
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

#if HbPlatform_Compiler_MSVC
#define HbAligned(alignment) __declspec(align(alignment))
#elif HbPlatform_Compiler_GNU
#define HbAligned(alignment) __attribute__((aligned(alignment)))
#else
#error No HbAligned known for the current compiler.
#endif

// Force inlining.

#if HbPlatform_Compiler_MSVC
#define HbForceInline __forceinline
#elif HbPlatform_Compiler_GNU
#define HbForceInline __attribute__((always_inline))
#else
#error No HbForceInline known for the current compiler.
#endif

// Static array length.

#define HbArrayLength(arr) (sizeof(arr) / (sizeof((arr)[0])))

// Stack allocation.
// WARNING: Don't call with 0, it may cause freeing all the `alloca`ted memory on some compilers.

#if HbPlatform_OS_Windows
#include <malloc.h>
#define HbStackAlloc _alloca
#else
#include <alloca.h>
#define HbStackAlloc alloca
#endif

// Byte swapping.
#if HbPlatform_Compiler_MSVC
#define HbByteSwapU16 _byteswap_ushort
#define HbByteSwapU32 _byteswap_ulong
#define HbByteSwapU64 _byteswap_uint64
#else
#error No HbByteSwap for the current compiler.
#endif

// Common operations on numbers. For run-time floats, use fmin/fmax, not HbMinI/HbMaxI.
// Force-inline versions to evaluate each argument only once.

#define HbMinI(a, b) ((a) < (b) ? (a) : (b))
HbForceInline int32_t HbMinI32(int32_t a, int32_t b) { return HbMinI(a, b); }
HbForceInline uint32_t HbMinU32(uint32_t a, uint32_t b) { return HbMinI(a, b); }
HbForceInline size_t HbMinSize(size_t a, size_t b) { return HbMinI(a, b); }
#define HbMaxI(a, b) ((a) > (b) ? (a) : (b))
HbForceInline int32_t HbMaxI32(int32_t a, int32_t b) { return HbMaxI(a, b); }
HbForceInline uint32_t HbMaxU32(uint32_t a, uint32_t b) { return HbMaxI(a, b); }
HbForceInline size_t HbMaxSize(size_t a, size_t b) { return HbMaxI(a, b); }
#define HbClampI(value, low, high) (((value) > (high)) ? (high) : (((value) < (low)) ? (low) : (value)))
HbForceInline int32_t HbClampI32(int32_t value, int32_t low, int32_t high) { return HbClampI(value, low, high); }
HbForceInline uint32_t HbClampU32(uint32_t value, uint32_t low, uint32_t high) { return HbClampI(value, low, high); }
HbForceInline size_t HbClampSize(size_t value, size_t low, size_t high) { return HbClampI(value, low, high); }
#define HbClampF(value, low, high) fminf(high, fmaxf(low, value))

#define HbAlign(value, alignment) (((value) + ((alignment) - 1u)) & ~((alignment) - 1u))
HbForceInline uint32_t HbAlignU32(uint32_t value, uint32_t alignment) {
	--alignment;
	return (value + alignment) & ~alignment;
}
HbForceInline size_t HbAlignSize(size_t value, size_t alignment) {
	--alignment;
	return (value + alignment) & ~alignment;
}

#endif
