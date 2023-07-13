/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2023 Alexandre Martins <alemartf(at)gmail(dot)com>
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

#define _GNU_SOURCE /* required for strtod_l() ? */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <locale.h>

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__EMSCRIPTEN__) || defined(__IBMCPP__)
#include <xlocale.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <wchar.h>
#endif

#include "util.h"
#include "../third_party/gettimeofday.h"

/* private stuff */
static void mem_crash(const char* file, int line);
static void my_log(const char* message);
static void my_fatal(const char* message);
static void (*log_function)(const char* message) = my_log;
static void (*fatal_function)(const char* message) = my_fatal;



/* -------------------------------
 * public methods
 * ------------------------------- */

/*
 * surgescript_util_malloc()
 * Memory allocation routine
 */
void* surgescript_util_malloc(size_t bytes, const char* file, int line)
{
    void *m = malloc(bytes);

    if(m == NULL)
        mem_crash(file, line);

    return m;
}

/*
 * surgescript_util_realloc()
 * Memory reallocation routine
 */
void* surgescript_util_realloc(void* ptr, size_t bytes, const char* file, int line)
{
    void *m = realloc(ptr, bytes);

    if(m == NULL)
        mem_crash(file, line);

    return m;
}

/*
 * surgescript_util_free()
 * Memory deallocation routine
 */
void* surgescript_util_free(void* ptr)
{
    if(ptr != NULL)
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
 * surgescript_util_versioncode()
 * Converts a SurgeScript version string to a comparable number
 * Note: if you pass NULL to version, it returns a comparable
 * number of the current version of SurgeScript
 */
int surgescript_util_versioncode(const char* version)
{
    static const int p[] = { 1, 100, 10000, 1000000 }; /* 100^i, up to i = limit-1 */
    int limit = 4; /* read up to 4 numbers */
    int code = 0, x = 0;

    if(!version)
        version = surgescript_util_version();

    for(; limit; version++) {
        if(isdigit(*version))
            x = x * 10 + (*version - '0');
        else if(*version == '.')
            code = code * 100 + x, x = 0, --limit;
        else if(*version == '\0')
            code = (code * 100 + x) * p[--limit], x = 0, limit = 0;
    }

    return code;
}

/*
 * surgescript_util_version()
 * Returns the current version of SurgeScript
 */
const char* surgescript_util_version()
{
    extern const char SURGESCRIPT_VERSION[];
    return SURGESCRIPT_VERSION;
}

/*
 * surgescript_util_year()
 * Returns a year string of the SurgeScript runtime
 */
const char* surgescript_util_year()
{
    extern const char SURGESCRIPT_YEARS[];
    return SURGESCRIPT_YEARS;
}

/*
 * surgescript_util_website()
 * Returns a string of the project website
 */
const char* surgescript_util_website()
{
    extern const char SURGESCRIPT_WEBSITE[];
    return SURGESCRIPT_WEBSITE;
}

/*
 * surgescript_util_authors()
 * Author string
 */
const char* surgescript_util_authors()
{
    extern const char SURGESCRIPT_AUTHORS[];
    return SURGESCRIPT_AUTHORS;
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
char* surgescript_util_strdup(const char* str, const char* file, int line)
{
    size_t size = (1 + strlen(str)) * sizeof(char);
    char* new_str = surgescript_util_malloc(size, file, line);
    return memcpy(new_str, str, size); /* include '\0' */
}

/*
 * surgescript_util_strtod()
 * Convert a string to a floating-point number in a locale-independent manner
 * This works just like strtod() from libc
 */
double surgescript_util_strtod(const char* str, char** endptr)
{
    double x;

#if defined(__GLIBC__)

    locale_t loc = newlocale(LC_NUMERIC, "C", (locale_t)0);
    x = strtod_l(str, endptr, loc);
    freelocale(loc);

#elif defined(_MSC_VER)

    _locale_t loc = _create_locale(LC_NUMERIC, "C");
    x = _strtod_l(str, endptr, loc);
    _free_locale(loc);

#else

    /* this routine assumes that the decimal point is always
       a single character, regardless of the current locale */
    const char* dec = localeconv()->decimal_point;

    if(*dec == '.') {

        /* '.' is already the decimal point, as expected by SurgeScript */
        x = strtod(str, endptr);

    }
    else {
        #define SS_STRTOD_TOKENSIZE 127
        char buf[1 + SS_STRTOD_TOKENSIZE];

        /* copy str to buf */
        strncpy(buf, str, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        /* replace '.' by the decimal point of the current locale */
        char* p = strchr(buf, '.');
        if(p != NULL)
            *p = *dec;

        /* convert the modified string to a floating-point number */
        x = strtod(buf, endptr);
    }

#endif

    return x;
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
 * Returns the number of milliseconds since the Unix Epoch
 */
uint64_t surgescript_util_gettickcount()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return ((uint64_t)now.tv_sec * 1000) + ((uint64_t)now.tv_usec / 1000);
}

/*
 * surgescript_util_srand()
 * Sets the seed of the pseudo-random number generator
 */
void surgescript_util_srand(uint64_t seed)
{
    /* using splitmix64 to seed the generator */
    extern uint64_t* xor_seed;
    for(int i = 0; i <= 1; i++) {
        uint64_t x = (seed += UINT64_C(0x9e3779b97f4a7c15));
        x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
        x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
        xor_seed[i] = x ^ (x >> 31);
    }
}

/*
 * surgescript_util_random64()
 * Generates a pseudo-random 64-bit unsigned integer
 */
uint64_t surgescript_util_random64()
{
    extern uint64_t (*xor_next)(void);
    return xor_next();
}

/*
 * surgescript_util_random()
 * Generates a pseudo-random double in the [0,1) range
 */
double surgescript_util_random()
{
    /* assuming IEEE-754 */
    uint64_t x = surgescript_util_random64();
    x = (x >> 12) | UINT64_C(0x3FF0000000000000); /* sign bit = 0; exponent = 1023 */
    return *((double*)&x) - 1.0;
}

/*
 * surgescript_util_fopen_utf8()
 * fopen() with UTF-8 support for filenames
 */
FILE* surgescript_util_fopen_utf8(const char* filepath, const char* mode)
{
#if defined(_WIN32)
    FILE* fp;
    int wpath_size = MultiByteToWideChar(CP_UTF8, 0, filepath, -1, NULL, 0);
    int wmode_size = MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);

    if(wpath_size > 0 && wmode_size > 0) {
        wchar_t* wpath = ssmalloc(wpath_size * sizeof(*wpath));
        wchar_t* wmode = ssmalloc(wmode_size * sizeof(*wmode));

        MultiByteToWideChar(CP_UTF8, 0, filepath, -1, wpath, wpath_size);
        MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, wmode_size);
        fp = _wfopen(wpath, wmode);

        ssfree(wmode);
        ssfree(wpath);
    }
    else {
        surgescript_util_log("%s(\"%s\", \"%s\") ERROR %d", __func__, filepath, mode, GetLastError());
        fp = fopen(filepath, mode);
    }

    return fp;
#else
    return fopen(filepath, mode);
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

void mem_crash(const char* file, int line) /* out of memory error */
{
    static char buf[1024] = "Out of memory in ";
    static const int prefix_len = 17;

    snprintf(buf + prefix_len, sizeof(buf) - prefix_len, "%s:%d", file, line);
    fatal_function(buf);

    exit(1); /* just in case */
}