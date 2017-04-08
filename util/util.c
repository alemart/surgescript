/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/util.c
 * SurgeScript utilities
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "util.h"

/* private stuff */
static void crash(const char* location);
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
void* surgescript_util_malloc(size_t bytes, const char* location)
{
    void *m = malloc(bytes);

    if(m == NULL)
        crash(location);

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
        crash(location);

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

    log_function(buf);
    fatal_function(buf);
    exit(1); /* just in case */
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
    const char* p = str;
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
unsigned long surgescript_util_htob(unsigned long x)
{
    static const union { int i; char c[sizeof(int)]; } u = { .i = 1 };
    return *(u.c) ? ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24) : x;
}

/*
 * surgescript_util_btoh()
 * Convert a 32-bit number from big to host-endian notation
 */

unsigned long surgescript_util_btoh(unsigned long x)
{
    return surgescript_util_htob(x);
}


/* -------------------------------
 * private methods
 * ------------------------------- */
void my_log(const char* message)
{
    fprintf(stderr, "%s\n", message);
}

void my_fatal(const char* message)
{
    ;
}

void crash(const char* location) /* out of memory error */
{
    static char buf[64] = "Out of memory in ";
    
    strncat(buf, location, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;

    log_function(buf);
    fatal_function(buf);
    exit(1); /* just in case */
}