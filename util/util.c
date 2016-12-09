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
void* surgescript_util_malloc(size_t bytes)
{
    void *m = malloc(bytes);

    if(m == NULL)
        surgescript_util_fatal(__FILE__ ": Out of memory");

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
 * surgescript_util_realloc()
 * Memory reallocation routine
 */
void* surgescript_util_realloc(void* ptr, size_t bytes)
{
    void *m = realloc(ptr, bytes);

    if(m == NULL)
        surgescript_util_fatal(__FILE__ ": Out of memory");

    return m;
}

/*
 * surgescript_util_log()
 * Logs a message
 */
void surgescript_util_log(const char* fmt, ...)
{
    char buf[1024] = "surgescript: ";
    int len = strlen(buf);
    va_list args;

    va_start(args, fmt);
    vsprintf(buf+len, fmt, args);
    va_end(args);

    log_function(buf);
}

/*
 * surgescript_util_fatal()
 * Displays a fatal error and kills the app
 */
void surgescript_util_fatal(const char* fmt, ...)
{
    char buf[1024] = "surgescript error! ";
    int len = strlen(buf);
    va_list args;

    va_start(args, fmt);
    vsprintf(buf+len, fmt, args);
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
 * Copies src to dst, limited to n > 0 bytes
 */
char* surgescript_util_strncpy(char* dst, const char* src, size_t n)
{
    char *p = dst;

    while(*src && --n)
        *(dst++) = *(src++);

    *dst = 0;
    return p;
}

/*
 * surgescript_util_strdup()
 * Copies a string into another, allocating the required memory
 */
char* surgescript_util_strdup(const char* src)
{
    char* str = ssmalloc(sizeof(char) * (1 + strlen(src)));
    return strcpy(str, src);
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
