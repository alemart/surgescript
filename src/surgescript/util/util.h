/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2024 Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * util/util.h
 * SurgeScript utilities
 */

#ifndef _SURGESCRIPT_UTIL_H
#define _SURGESCRIPT_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* helpers */
#define SSTOK(x)                    #x
#define SSSTR(x)                    SSTOK(x)
#define SSCAT_(x, y)                x##y
#define SSCAT(x, y)                 SSCAT_(x, y)

/* macros */
#define ssmin(a, b)                 ((a) < (b) ? (a) : (b))
#define ssmax(a, b)                 ((a) >= (b) ? (a) : (b))
#define ssclamp(x, min, max)        ssmax(ssmin((x), (max)), (min))
#define sssign(x)                   ((x) >= 0 ? 1 : -1)
#define ssassert(expr)              do { if(!(expr)) ssfatal("In %s:%d: %s", __FILE__, __LINE__, ": assertion `" SSTOK(expr) "` failed."); } while(0)

/* common aliases */
#define ssmalloc(n)                 surgescript_util_malloc((n), __FILE__, __LINE__)
#define ssrealloc(p, n)             surgescript_util_realloc((p), (n), __FILE__, __LINE__)
#define ssfree                      surgescript_util_free
#define sslog                       surgescript_util_log
#define ssfatal                     surgescript_util_fatal
#define ssstrdup(str)               surgescript_util_strdup((str), __FILE__, __LINE__)
#define ssatof(str)                 surgescript_util_strtod((str), NULL)

/* constants */
#define SS_NAMEMAX                  63 /* names can't be larger than this (computes hashes quickly) */

/* compile-time assertions */
#define SS_STATIC_ASSERT(expr, ...) \
    struct SSCAT(static_assert_at_ ## __VA_ARGS__ ## _line_, __LINE__) { int x: !!(expr); }

/* inlining hints */
#if defined(__GNUC__) || defined(__clang__)
#define SS_FORCE_INLINE             inline __attribute__((always_inline))
#define SS_NO_INLINE                __attribute__((noinline))
#elif defined(_MSC_VER)
#define SS_FORCE_INLINE             __forceinline /* MSVC */
#define SS_NO_INLINE                __declspec(noinline)
#else
#define SS_FORCE_INLINE             inline
#define SS_NO_INLINE
#endif

/* public routines */
int surgescript_util_versioncode(const char* version); /* converts a version string to a comparable number */
const char* surgescript_util_version(); /* compiled version of SurgeScript */
const char* surgescript_util_years(); /* years string of the SurgeScript runtime */
const char* surgescript_util_website(); /* project website */
const char* surgescript_util_author(); /* project author */

void* surgescript_util_malloc(size_t bytes, const char* file, int line); /* memory allocation */
void* surgescript_util_realloc(void* ptr, size_t bytes, const char* file, int line); /* memory reallocation */
void* surgescript_util_free(void* ptr); /* memory deallocation */

void surgescript_util_log(const char* fmt, ...); /* logs a message */
void surgescript_util_fatal(const char* fmt, ...); /* logs a message and kills the app */
void surgescript_util_set_log_function(void (*fn)(const char*,void*), void* context); /* set a custom log function */
void surgescript_util_set_crash_function(void (*fn)(const char*,void*), void* context); /* set a custom crash function */
void surgescript_util_set_error_functions(void (*log)(const char*), void (*crash)(const char*)); /* (obsolete) set custom log & crash functions */

char* surgescript_util_strncpy(char* dst, const char* src, size_t n); /* strcpy */
char* surgescript_util_strdup(const char* str, const char* file, int line); /* strdup */
double surgescript_util_strtod(const char* str, char** endptr); /* convert a string to a floating-point number in a locale-independent manner */
const char* surgescript_util_basename(const char* path); /* basename */
char* surgescript_util_accessorfun(const char* prefix, const char* text); /* getter/setter prefixing function */

unsigned surgescript_util_htob(unsigned x); /* host to big-endian */
unsigned surgescript_util_btoh(unsigned x); /* big to host-endian */

void surgescript_util_srand(uint64_t seed); /* sets the seed of the pseudo-random number generator */
uint64_t surgescript_util_random64(); /* generates a pseudo-random 64-bit unsigned integer */
double surgescript_util_random(); /* generates a pseudo-random double in the [0,1) range */

uint64_t surgescript_util_gettickcount(); /* number of milliseconds since some arbitrary zero */

FILE* surgescript_util_fopen_utf8(const char* filepath, const char* mode); /* fopen() with UTF-8 support for filenames */

#endif
