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
#define JUNE_UNREACHABLE for (;;)
#endif
#elif defined(JUNE_MSVC)
#define JUNE_UNREACHABLE __assume(0)
#else
#pragma message(                                                               \
    "WARNING: JUNE_UNREACHABLE is not implemented for this compiler")
#define JUNE_UNREACHABLE
#endif

#endif
