/*
 * Copyright (c) 2026-present, PowerTech.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ZASTER_DEFS_H
#define ZASTER_DEFS_H

/* =========================================================================
 * Common macros for all Zaster modules.
 *
 * Provides:
 *   - Compiler detection helpers
 *   - Inline / noinline attributes
 *   - Branch prediction hints
 *   - API visibility
 *   - Static assertion wrapper
 * ========================================================================= */

#include <stddef.h>   /* size_t, offsetof */
#include <stdint.h>   /* uint8_t, uintptr_t */
#include <stdio.h>    /* fprintf (for ZASTER_FATAL) */
#include <stdlib.h>   /* abort  (for ZASTER_FATAL) */

/* -------------------------------------------------------------------------
 * Compiler detection
 * ------------------------------------------------------------------------- */

#if defined(__GNUC__) && !defined(__clang__)
#   define ZASTER_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#else
#   define ZASTER_GCC_VERSION 0
#endif

/* -------------------------------------------------------------------------
 * Inline / Noinline
 * ------------------------------------------------------------------------- */

#if defined(__GNUC__) || defined(__clang__)
#   define ZASTER_INLINE       static inline __attribute__((always_inline))
#   define ZASTER_NOINLINE     __attribute__((noinline))
#   define ZASTER_UNUSED       __attribute__((unused))
#elif defined(_MSC_VER)
#   define ZASTER_INLINE       static __forceinline
#   define ZASTER_NOINLINE     __declspec(noinline)
#   define ZASTER_UNUSED
#else
#   define ZASTER_INLINE       static inline
#   define ZASTER_NOINLINE
#   define ZASTER_UNUSED
#endif

/* ZASTER_ALWAYS_INLINE — same as ZASTER_INLINE but without `static`.
 * Use for C++ class member functions where `static` is invalid. */
#if defined(__GNUC__) || defined(__clang__)
#   define ZASTER_ALWAYS_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#   define ZASTER_ALWAYS_INLINE __forceinline
#else
#   define ZASTER_ALWAYS_INLINE inline
#endif

/* -------------------------------------------------------------------------
 * Branch prediction hints
 * ------------------------------------------------------------------------- */

#if defined(__GNUC__) || defined(__clang__)
#   define ZASTER_LIKELY(x)    __builtin_expect(!!(x), 1)
#   define ZASTER_UNLIKELY(x)  __builtin_expect(!!(x), 0)
#else
#   define ZASTER_LIKELY(x)    (x)
#   define ZASTER_UNLIKELY(x)  (x)
#endif

/* -------------------------------------------------------------------------
 * API visibility
 *
 * ZASTER_API marks functions exported from the shared library.
 * For static linking the macro resolves to nothing significant.
 *
 * Control defines (set by the build system):
 *   ZASTER_DLL_EXPORT — when building the shared library
 *   ZASTER_DLL_IMPORT — when consuming the shared library
 * ------------------------------------------------------------------------- */

#if defined(_WIN32) || defined(__CYGWIN__)
#   if defined(ZASTER_DLL_EXPORT)
#       define ZASTER_VISIBLE __declspec(dllexport)
#   elif defined(ZASTER_DLL_IMPORT)
#       define ZASTER_VISIBLE __declspec(dllimport)
#   else
#       define ZASTER_VISIBLE
#   endif
#   define ZASTER_HIDDEN
#elif (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
#   define ZASTER_VISIBLE __attribute__((visibility("default")))
#   define ZASTER_HIDDEN  __attribute__((visibility("hidden")))
#else
#   define ZASTER_VISIBLE
#   define ZASTER_HIDDEN
#endif

#define ZASTER_API ZASTER_VISIBLE

/* -------------------------------------------------------------------------
 * Cold attribute — hint that a function is rarely called
 * ------------------------------------------------------------------------- */

#if defined(__GNUC__) || defined(__clang__)
#   define ZASTER_COLD __attribute__((cold))
#else
#   define ZASTER_COLD
#endif

/* -------------------------------------------------------------------------
 * Static assert (portable across C11 / C99 / C++)
 * ------------------------------------------------------------------------- */

#if defined(__cplusplus)
#   define ZASTER_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#   define ZASTER_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#   define ZASTER_STATIC_ASSERT(cond, msg) typedef char zaster_static_assert_##__LINE__[(cond) ? 1 : -1]
#endif

/* -------------------------------------------------------------------------
 * Boolean type
 * ------------------------------------------------------------------------- */

typedef uint8_t ZasterBool;
#define ZASTER_TRUE  ((ZasterBool)1)
#define ZASTER_FALSE ((ZasterBool)0)

/* -------------------------------------------------------------------------
 * Fatal error — prints message to stderr and aborts
 * ------------------------------------------------------------------------- */

#define ZASTER_FATAL(fmt, ...) \
    do { fprintf(stderr, "zaster: " fmt "\n", ##__VA_ARGS__); abort(); } while (0)

#endif /* ZASTER_DEFS_H */
