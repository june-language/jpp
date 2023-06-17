#ifndef _defines_h
#define _defines_h

#if defined(_WIN32) || defined(_WIN64)
#define JUNE_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
#define JUNE_APPLE
#else
#define JUNE_NIX
#endif

#ifndef __funcname__
#ifdef JUNE_WINDOWS
#define __funcname__ __FUNCTION__
#else
#define __funcname__ __func__
#endif
#endif

#if defined(__clang__)
#define JUNE_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#define JUNE_GCC
#elif defined(_MSC_VER)
#define JUNE_MSVC
#endif

#if defined(JUNE_CLANG) || defined(JUNE_GCC)
#define JUNE_DEPRECATED __attribute__((deprecated))
#elif defined(JUNE_MSVC)
#define JUNE_DEPRECATED __declspec(deprecated)
#else
#pragma message("WARNING: JUNE_DEPRECATED is not implemented for this compiler")
#define JUNE_DEPRECATED
#endif

#if defined(JUNE_CLANG) || defined(JUNE_GCC)
#define JUNE_NORETURN __attribute__((noreturn))
#elif defined(JUNE_MSVC)
#define JUNE_NORETURN __declspec(noreturn)
#else
#pragma message("WARNING: JUNE_NORETURN is not implemented for this compiler")
#define JUNE_NORETURN
#endif

#if __cplusplus
#define JUNEC extern "C" {
#define JUNEC_END }
#else
#define JUNEC
#define JUNEC_END
#endif

#if defined(JUNE_CLANG) || defined(JUNE_GCC)
#if __has_builtin(__builtin_unreachable)
#define JUNE_UNREACHABLE __builtin_unreachable()
#else
#define JUNE_UNREACHABLE for (;;) // technically UB, but works
#endif
#elif defined(JUNE_MSVC)
#define JUNE_UNREACHABLE __assume(0)
#else
#pragma message(                                                               \
    "WARNING: JUNE_UNREACHABLE is not implemented for this compiler")
#define JUNE_UNREACHABLE
#endif

// BSWAP functions

#if defined(__builtin_bswap16)
#define JUNE_BSWAP16 __builtin_bswap16
#elif defined(JUNE_MSVC)
#define JUNE_BSWAP16 _byteswap_ushort
#else
#define JUNE_BSWAP16(x) (((x) << 8) | ((x) >> 8))
#endif

#if defined(__builtin_bswap32)
#define JUNE_BSWAP32 __builtin_bswap32
#elif defined(JUNE_MSVC)
#define JUNE_BSWAP32 _byteswap_ulong
#else
#define JUNE_BSWAP32(x)                                                      \
    (((x) << 24) | (((x) << 8) & 0x00FF0000) | (((x) >> 8) & 0x0000FF00) |     \
     ((x) >> 24))
#endif

#if defined(__builtin_bswap64)
#define JUNE_BSWAP64 __builtin_bswap64
#elif defined(JUNE_MSVC)
#define JUNE_BSWAP64 _byteswap_uint64
#else
#define JUNE_BSWAP64(x)                                                      \
    (((x) << 56) | (((x) << 40) & 0x00FF000000000000) |                        \
     (((x) << 24) & 0x0000FF0000000000) | (((x) << 8) & 0x000000FF00000000) | \
     (((x) >> 8) & 0x00000000FF000000) | (((x) >> 24) & 0x0000000000FF0000) | \
     (((x) >> 40) & 0x000000000000FF00) | ((x) >> 56))
#endif

#define JUNE_BSWAP32F(x) JUNE_BSWAP32(*((uint32_t *)&x))
#define JUNE_BSWAP64F(x) JUNE_BSWAP64(*((uint64_t *)&x))

// Endianness
#if defined(JUNE_WINDOWS)
#define JUNE_LE
#else
#include <sys/types.h>
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define JUNE_LE
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define JUNE_BE
#else
#error "Unknown endianness"
#endif
#endif

#endif
