#pragma once

#include "snfile/platform.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#if defined(SN_FILE_STATIC)
    #define SN_API
#else
    #ifdef SN_EXPORT
        #if defined(SN_OS_LINUX) || defined(SN_OS_MAC)
            #define SN_API __attribute__((visibility("default")))
        #elif defined(SN_OS_WINDOWS)
            #define SN_API __declspec(dllexport)
        #else
            #error "Should not reach here!"
        #endif
    #else
        #if defined(SN_OS_LINUX) || defined(SN_OS_MAC)
            #define SN_API
        #elif defined(SN_OS_WINDOWS)
            #define SN_API __declspec(dllimport)
        #else
            #error "Should not reach here!"
        #endif
    #endif
#endif

#define SN_INLINE static inline

#if defined(SN_COMPILER_MSVC)
    #define SN_FORCE_INLINE static __forceinline
#else
    #define SN_FORCE_INLINE static inline __attribute__((always_inline))
#endif

#define SN_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

#define SN_ASSERT(x) assert(x)

#define SN_SHOULD_NOT_REACH_HERE (SN_ASSERT(false))

#define SN_UNUSED(x) (void)(x)

#define SN_ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

#define SN_MAX(a, b) ((a) > (b) ? (a) : (b))

#define SN_MIN(a, b) ((a) < (b) ? (a) : (b))

#define SN_CLAMP(x, min, max) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x))

#define SN_BIT_FLAG(n) (1 << (n))

#define SN_BIT_SET(x, n) ((x) |= SN_BIT_FLAG((n)))

#define SN_BIT_CLEAR(x, n) ((x) &= ~SN_BIT_FLAG((n)))

#define SN_BIT_TOGGLE(x, n) ((x) ^= SN_BIT_FLAG((n)))

#define SN_BIT_CHECK(x, n) ((x) & SN_BIT_FLAG((n)))

#define SN_BIT_SET_VALUE(x, n) ((x) | SN_BIT_FLAG((n)))

#define SN_BIT_CLEARED_VALUE(x, n) ((x) & ~SN_BIT_FLAG((n)))

#define SN_BIT_TOGGLED_VALUE(x, n) ((x) ^ SN_BIT_FLAG((n)))

