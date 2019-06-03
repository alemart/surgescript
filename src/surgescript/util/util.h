/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2018 Alexandre Martins <alemartf(at)gmail(dot)com>
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

#include <stdlib.h>
#include <stdint.h>

/* macros */
#define ssmin(a, b)                 ((a) < (b) ? (a) : (b))
#define ssmax(a, b)                 ((a) >= (b) ? (a) : (b))
#define ssclamp(x, min, max)        ssmax(ssmin((x), (max)), (min))
#define sssign(x)                   ((x) >= 0 ? 1 : -1)
#define sstok(x)                    #x
#define ssstr(x)                    sstok(x)
#define ssassert(expr)              do { if(!(expr)) ssfatal("%s", "In " __FILE__ ":" ssstr(__LINE__) ": assertion `" sstok(expr) "` failed."); } while(0)

/* common aliases */
#define ssmalloc(n)                 surgescript_util_malloc((n), __FILE__ ":" ssstr(__LINE__))
#define ssrealloc(p, n)             surgescript_util_realloc((p), (n), __FILE__ ":" ssstr(__LINE__))
#define ssfree                      surgescript_util_free
#define sslog                       surgescript_util_log
#define ssfatal                     surgescript_util_fatal
#define ssstrdup(str)               surgescript_util_strdup((str), __FILE__ ":" ssstr(__LINE__))

/* public routines */
const char* surgescript_util_version(); /* compiled version of SurgeScript */
int surgescript_util_versioncode(const char* version); /* converts a version string to a comparable number */

void* surgescript_util_malloc(size_t bytes, const char* location); /* memory allocation */
void* surgescript_util_realloc(void* ptr, size_t bytes, const char* location); /* memory reallocation */
void* surgescript_util_free(void* ptr); /* memory deallocation */

void surgescript_util_log(const char* fmt, ...); /* logs a message */
void surgescript_util_fatal(const char* fmt, ...); /* logs a message and kills the app */
void surgescript_util_set_error_functions(void (*log)(const char*), void (*fatal)(const char*)); /* set custom error functions */

char* surgescript_util_strncpy(char* dst, const char* src, size_t n); /* strcpy */
char* surgescript_util_strdup(const char* src, const char* location); /* strdup */
uint32_t surgescript_util_str2hash(const char* str); /* converts a string to a 32-bit hash */
const char* surgescript_util_basename(const char* path); /* basename */
char* surgescript_util_accessorfun(const char* prefix, const char* text); /* getter/setter prefixing function */

unsigned surgescript_util_htob(unsigned x); /* host to big-endian */
unsigned surgescript_util_btoh(unsigned x); /* big to host-endian */
uint64_t surgescript_util_gettickcount(); /* number of milliseconds since some arbitrary zero */

#endif
