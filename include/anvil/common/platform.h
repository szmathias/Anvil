#ifndef ANVIL_PLATFORM_H
#define ANVIL_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

// Require C11 or higher
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
    #error "C11 or higher required"
#endif

// Platform detection - use CMake definitions if available, otherwise auto-detect
#if !defined(ANV_PLATFORM_WINDOWS) && !defined(ANV_PLATFORM_MACOS) && !defined(ANV_PLATFORM_LINUX)
    #if defined(_WIN32) || defined(_WIN64)
        #define ANV_PLATFORM_WINDOWS 1
    #elif defined(__APPLE__) || defined(__MACH__)
        #define ANV_PLATFORM_MACOS 1
    #elif defined(__linux__)
        #define ANV_PLATFORM_LINUX 1
    #else
        #define ANV_PLATFORM_UNKNOWN 1
    #endif
#endif

// API export/import macros
#ifdef ANV_PLATFORM_WINDOWS
    #ifdef ANV_BUILDING_DLL
        #define ANV_API __declspec(dllexport)
    #else
        #define ANV_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define ANV_API __attribute__((visibility("default")))
#else
    #define ANV_API
#endif

// Detect C23 for standard attributes
#if __STDC_VERSION__ >= 202311L
    #define ANV_COMPAT_C23 1
#endif

#if !defined(ANV_COMPAT_C23) || !defined(__cplusplus)
    #define nullptr NULL
    #define constexpr const
    #include <stdbool.h>
    #define static_assert(condition, message) _Static_assert(condition, message)
#endif

// Attribute compatibility macros
#ifdef ANV_COMPAT_C23
    #define ANV_NODISCARD [[nodiscard]]
    #define ANV_DEPRECATED [[deprecated]]
    #define ANV_NORETURN [[noreturn]]
#elif defined(__GNUC__) || defined(__clang__)
    #define ANV_NODISCARD __attribute__((warn_unused_result))
    #define ANV_DEPRECATED __attribute__((deprecated))
    #define ANV_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
    #define ANV_NODISCARD _Check_return_
    #define ANV_DEPRECATED __declspec(deprecated)
    #define ANV_NORETURN __declspec(noreturn)
#else
    #define ANV_NODISCARD
    #define ANV_DEPRECATED
    #define ANV_NORETURN
#endif

#ifdef __cplusplus
}
#endif

#endif // ANVIL_PLATFORM_H
