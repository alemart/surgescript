/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2019 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * util/util.c
 * SurgeScript utilities
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "util.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

/* private stuff */
static void mem_crash(const char* location);
static void my_log(const char* message);
static void my_fatal(const char* message);
static void (*log_function)(const char* message) = my_log;
static void (*fatal_function)(const char* message) = my_fatal;

/* current version of SurgeScript */
static const char* version_string = "0.5.4-dev";



/* -------------------------------
 * public methods
 * ------------------------------- */

/*
 * surgescript_util_malloc()
 * Memory allocation routine
 */
void* surgescript_util_malloc(size_t bytes, const char* location)
{
    void *m = malloc(bytes);

    if(m == NULL)
        mem_crash(location);

    return m;
}

/*
 * surgescript_util_realloc()
 * Memory reallocation routine
 */
void* surgescript_util_realloc(void* ptr, size_t bytes, const char* location)
{
    void *m = realloc(ptr, bytes);

    if(m == NULL)
        mem_crash(location);

    return m;
}

/*
 * surgescript_util_free()
 * Memory deallocation routine
 */
void* surgescript_util_free(void* ptr)
{
    if(ptr)
        free(ptr);

    return NULL;
}

/*
 * surgescript_util_log()
 * Logs a message
 */
void surgescript_util_log(const char* fmt, ...)
{
    char buf[1024] = "[surgescript] ";
    int len = strlen(buf);
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf+len, sizeof(buf)-len, fmt, args);
    va_end(args);

    log_function(buf);
}

/*
 * surgescript_util_fatal()
 * Displays a fatal error and kills the app
 */
void surgescript_util_fatal(const char* fmt, ...)
{
    char buf[1024] = "[surgescript-error] ";
    int len = strlen(buf);
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf+len, sizeof(buf)-len, fmt, args);
    va_end(args);

    fatal_function(buf);
    exit(1); /* just in case */
}

/*
 * surgescript_util_version()
 * Returns the current version of SurgeScript
 */
const char* surgescript_util_version()
{
    return version_string;
}

/*
 * surgescript_util_versioncode()
 * Converts a SurgeScript version string to a comparable number
 * Note: if you pass NULL to version, it returns a comparable
 * number of the current version of SurgeScript
 */
int surgescript_util_versioncode(const char* version)
{
    int code = 0, x = 0;

    if(!version)
        version = surgescript_util_version();

    for(; *version; version++) {
        if(isdigit(*version))
            x = x * 10 + (*version - '0');
        else if(*version == '.')
            code = code * 100 + x, x = 0;
        else
            break;
    }

    return code * 100 + x;
}

/*
 * surgescript_util_set_error_functions()
 * Customize the error messages
 */
void surgescript_util_set_error_functions(void (*log)(const char*), void (*fatal)(const char*))
{
    log_function = log ? log : my_log;
    fatal_function = fatal ? fatal : my_fatal;
}

/*
 * surgescript_util_strncpy()
 * Copies src to dst, limited to n > 0 bytes (this puts the ending '\0' on dst)
 */
char* surgescript_util_strncpy(char* dst, const char* src, size_t n)
{
    char *p = dst;

    if(n && src) {
        while(*src && --n)
            *(dst++) = *(src++);
    }

    *dst = 0;
    return p;
}

/*
 * surgescript_util_strdup()
 * Copies a string into another, allocating the required memory
 */
char* surgescript_util_strdup(const char* src, const char* location)
{
    char* str = surgescript_util_malloc(sizeof(char) * (1 + strlen(src)), location);
    return strcpy(str, src);
}

/*
 * surgescript_util_str2hash()
 * Converts a string to a 32-bit hash
 * This implements Jenkins' One-at-a-Time hash function
 */
uint32_t surgescript_util_str2hash(const char* str)
{
    const unsigned char* p = (const unsigned char*)str;
    uint32_t hash = 0;

    if(p != NULL) {
        while(*p) {
            hash += *(p++);
            hash += hash << 10;
            hash ^= hash >> 6;
        }
        hash += hash << 3;
        hash ^= hash >> 11;
        hash += hash << 15;
    }

    return hash;
}

/*
 * surgescript_util_htob()
 * Convert a 32-bit number from host to big-endian notation
 */
unsigned surgescript_util_htob(unsigned x)
{
    static const union { int i; char c[sizeof(int)]; } u = { .i = 1 };
    return *(u.c) ? ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24) : x;
}

/*
 * surgescript_util_btoh()
 * Convert a 32-bit number from big to host-endian notation
 */
unsigned surgescript_util_btoh(unsigned x)
{
    return surgescript_util_htob(x);
}

/*
 * surgescript_util_basename()
 * similar to basename(), but without the odd semantics. No strings are allocated.
 */
const char* surgescript_util_basename(const char* path)
{
    const char* p;

    if(!(p = strrchr(path, '/'))) {
        if(!(p = strrchr(path, '\\')))
            return path;
    }

    return p + 1;
}

/*
 * surgescript_util_acessor2fun()
 * Getter/setter prefixing function: returns "prefix_text".
 * You need to ssfree() this.
 */
char* surgescript_util_accessorfun(const char* prefix, const char* text)
{
    char* str = ssmalloc((strlen(prefix) + strlen(text) + 2) * sizeof(char));
    char tmp[2] = { '_', '\0' };

    strcpy(str, prefix);
    strcat(str, tmp);
    strcat(str, text);

    return str;
}

/*
 * surgescript_util_gettickcount()
 * Returns the number of milliseconds since some arbitrary zero
 * This is a system-specific routine
 */
uint64_t surgescript_util_gettickcount()
{
#ifndef _WIN32
    struct timeval now;
    gettimeofday(&now, NULL);
    return ((uint64_t)now.tv_sec * 1000) + ((uint64_t)now.tv_usec / 1000);
    /*return 1000 * clock() / CLOCKS_PER_SEC;*/ /* not very accurate */
#else
    return GetTickCount(); /*GetTickCount64()*/
#endif
}


/* -------------------------------
 * private methods
 * ------------------------------- */
void my_log(const char* message)
{
    printf("%s\n", message);
}

void my_fatal(const char* message)
{
    fprintf(stderr, "%s\n", message);
}

void mem_crash(const char* location) /* out of memory error */
{
    static char buf[128] = "Out of memory in ";
    surgescript_util_strncpy(buf + 17, location, sizeof(buf) - 17);
    fatal_function(buf);
    exit(1); /* just in case */
}
