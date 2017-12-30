/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016-2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/util.h
 * SurgeScript utilities
 */

#ifndef _SURGESCRIPT_UTIL_H
#define _SURGESCRIPT_UTIL_H

#include <stdlib.h>
#include <stdint.h>

/* SurgeScript info */
#define SSPROJECT                   "SurgeScript"
#define SSVERSION                   "0.5.0"
#ifndef SSBUILD
#define SSBUILD                     "custom build"
#endif
#define SSINFO                      SSPROJECT " " SSVERSION " - " SSBUILD

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
void* surgescript_util_malloc(size_t bytes, const char* location); /* memory allocation */
void* surgescript_util_realloc(void* ptr, size_t bytes, const char* location); /* memory reallocation */
void* surgescript_util_free(void* ptr); /* memory deallocation */

void surgescript_util_log(const char* fmt, ...); /* logs a message */
void surgescript_util_fatal(const char* fmt, ...); /* logs a message and kills the app */
void surgescript_util_set_error_functions(void (*log)(const char*), void (*fatal)(const char*)); /* set custom error functions */

char* surgescript_util_strncpy(char* dst, const char* src, size_t n); /* strcpy */
char* surgescript_util_strdup(const char* src, const char* location); /* strdup */
uint32_t surgescript_util_str2hash(const char* str); /* converts a string to a 32-bit hash */
uint32_t surgescript_util_strpair2hash(const char* str1, const char* str2); /* a variant of str2hash() for an ordered pair of strings */

unsigned long surgescript_util_htob(unsigned long x); /* host to big-endian */
unsigned long surgescript_util_btoh(unsigned long x); /* big to host-endian */

const char* surgescript_util_basename(const char* path); /* basename */
char* surgescript_util_camelcaseprefix(const char* prefix, const char* text); /* camelCase prefixing function */

float surgescript_util_gettickcount(); /* number of seconds since the start of the program */

#endif
