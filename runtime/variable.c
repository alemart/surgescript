/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016-2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/variable.c
 * SurgeScript variables
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include "variable.h"
#include "object.h"
#include "../util/util.h"


/* private stuff */

/* possible variable types */
enum surgescript_vartype_t {
    SSVAR_NULL,
    SSVAR_BOOL,
    SSVAR_NUMBER,
    SSVAR_STRING,
    SSVAR_OBJECTHANDLE
};

/* the variable struct */
struct surgescript_var_t
{
    union {
        char* string;
        float number;
        unsigned handle;
        bool boolean;
        unsigned long raw;
    };
    enum surgescript_vartype_t type;
};

/* helpers */
#define RELEASE_DATA(var)       if((var)->type == SSVAR_STRING) \
                                    ssfree((var)->string);

/* privates */
static inline const char* typeof_var(const surgescript_var_t* var);

/* -------------------------------
 * public methods
 * ------------------------------- */

/* create & destroy variables */

/*
 * surgescript_var_create()
 * Creates an empty, null variable
 */
surgescript_var_t* surgescript_var_create()
{
    surgescript_var_t* var = ssmalloc(sizeof *var);
    var->type = SSVAR_NULL;
    var->raw = 0;
    return var;
}

/*
 * surgescript_var_destroy()
 * Destroys an existing variable from memory
 */
surgescript_var_t* surgescript_var_destroy(surgescript_var_t* var)
{
    RELEASE_DATA(var);
    ssfree(var);
    return NULL;
}




/* sets the value of a variable */

/*
 * surgescript_var_set_null()
 * Sets the variable to null
 */
surgescript_var_t* surgescript_var_set_null(surgescript_var_t* var)
{
    RELEASE_DATA(var);
    var->type = SSVAR_NULL;
    var->raw = 0; /* clear up all bits */
    return var;
}

/*
 * surgescript_var_set_bool()
 * Sets the variable to a boolean variable
 */
surgescript_var_t* surgescript_var_set_bool(surgescript_var_t* var, bool boolean)
{
    RELEASE_DATA(var);
    var->type = SSVAR_BOOL;
    var->raw = (unsigned long)boolean; /* must clear up all bits; see get_rawbits() below */
    return var;
}

/*
 * surgescript_var_set_number()
 * Sets the variable to a numeric variable
 */
surgescript_var_t* surgescript_var_set_number(surgescript_var_t* var, float number)
{
    RELEASE_DATA(var);
    var->type = SSVAR_NUMBER;
    var->number = number;
    return var;
}

/*
 * surgescript_var_set_string()
 * Sets the variable to a (valid, not-NULL) text variable
 */
surgescript_var_t* surgescript_var_set_string(surgescript_var_t* var, const char* string)
{
    static const int MAXLEN = 0xFFFE;
    
    RELEASE_DATA(var);
    if(strlen(string) <= MAXLEN) {
        var->type = SSVAR_STRING;
        var->string = ssstrdup(string);
    }
    else
        ssfatal("Runtime Error: string too large!");

    return var;
}

/*
 * surgescript_var_set_objecthandle()
 * Sets the variable to an object handle
 */
surgescript_var_t* surgescript_var_set_objecthandle(surgescript_var_t* var, unsigned handle)
{
    RELEASE_DATA(var);
    var->type = SSVAR_OBJECTHANDLE;
    var->handle = handle;
    return var;
}



/* retrieve the value stored in a variable */

/*
 * surgescript_var_is_null()
 * Is this variable null?
 */
bool surgescript_var_is_null(const surgescript_var_t* var)
{
    return var->type == SSVAR_NULL;
}

/*
 * surgescript_var_get_bool()
 * Gets the boolean value of a variable
 */
bool surgescript_var_get_bool(const surgescript_var_t* var)
{
    switch(var->type) {
        case SSVAR_BOOL:
            return var->boolean;
        case SSVAR_NUMBER:
            return var->number != 0 ? true : false;
        case SSVAR_STRING:
            return *(var->string) != 0 ? true : false;
        case SSVAR_NULL:
            return false;
        case SSVAR_OBJECTHANDLE:
            return true;
    }

    return false;
}

/*
 * surgescript_var_get_number()
 * Gets the numeric value of a variable
 */
float surgescript_var_get_number(const surgescript_var_t* var)
{
    switch(var->type) {
        case SSVAR_NUMBER:
            return var->number;
        case SSVAR_BOOL:
            return var->boolean ? 1.0f : 0.0f;
        case SSVAR_STRING:
            return atof(var->string);
        case SSVAR_NULL:
            return 0.0f;
        case SSVAR_OBJECTHANDLE:
            return 1.0f;
    }

    return 0.0f;
}

/*
 * surgescript_var_get_string()
 * Gets the contents of a string variable. You have to ssfree() this after use.
 */
char* surgescript_var_get_string(const surgescript_var_t* var)
{
    switch(var->type) {
        case SSVAR_STRING:
            return ssstrdup(var->string);
        case SSVAR_BOOL:
            return ssstrdup(var->boolean ? "true" : "false");
        case SSVAR_NULL:
            return ssstrdup("null");
        default: {
            char buf[128];
            surgescript_var_to_string(var, buf, sizeof(buf) / sizeof(char));
            return ssstrdup(buf);
        }
    }
}

/*
 * surgescript_var_get_objecthandle()
 * Gets the object handle
 */
unsigned surgescript_var_get_objecthandle(const surgescript_var_t* var)
{
    /* return null if var doesn't store a handle */
    return var->type == SSVAR_OBJECTHANDLE ? var->handle : 0;
}


/* misc */

/*
 * surgescript_var_copy()
 * Copies src into dst, without creating a new variable.
 * Returns dst. Similar to strcpy()
 */
surgescript_var_t* surgescript_var_copy(surgescript_var_t* dst, const surgescript_var_t* src)
{
    dst->type = src->type;

    switch(src->type) {
        case SSVAR_BOOL:
            dst->boolean = src->boolean;
            break;
        case SSVAR_NUMBER:
            dst->number = src->number;
            break;
        case SSVAR_STRING:
            dst->string = ssstrdup(src->string);
            break;
        case SSVAR_OBJECTHANDLE:
            dst->handle = src->handle;
            break;
        default:
            break;
    }

    return dst;
}

/*
 * surgescript_var_clone()
 * Creates a copy of a variable
 */
surgescript_var_t* surgescript_var_clone(const surgescript_var_t* var)
{
    surgescript_var_t* copy = ssmalloc(sizeof *copy);
    return surgescript_var_copy(copy, var);
}

#if 0
/*
 * surgescript_var_typename()
 * What's the name of the type of the variable?
 */
const char* surgescript_var_typename(const surgescript_var_t* var)
{
    /* all first letters should be different; see typecode() below */
    switch(var->type) {
        case SSVAR_NUMBER:
            return "number";
        case SSVAR_BOOL:
            return "boolean";
        case SSVAR_STRING:
            return "string";
        case SSVAR_OBJECTHANDLE:
            return "object";
        case SSVAR_NULL:
            return ""; /* '\0' is the first character */
    }
    
    return "unknown";
}
#endif

/*
 * surgescript_var_typecode()
 * Returns an integer representing the type of the variable
 * (each type has its own code)
 */
int surgescript_var_typecode(const surgescript_var_t* var)
{
    static const int code[] = { 0, 'b', 'n', 's', 'o' };
    return code[(int)(var->type)];
}

/*
 * surgescript_var_type2code()
 * Given a type_name (that may be NULL), return its corresponding type code
 * PS: typename must be lowercase
 */
int surgescript_var_type2code(const char* type_name)
{
    return type_name ? *type_name : 0;
}

/*
 * surgescript_var_to_string()
 * Converts a variable of any type to a string to be stored in a buffer of bufsize bytes
 */
char* surgescript_var_to_string(const surgescript_var_t* var, char* buf, size_t bufsize)
{
    switch(var->type) {
        case SSVAR_STRING:
            return surgescript_util_strncpy(buf, var->string, bufsize);
        case SSVAR_BOOL:
            return surgescript_util_strncpy(buf, var->boolean ? "true" : "false", bufsize);
        case SSVAR_NULL:
            return surgescript_util_strncpy(buf, "null", bufsize);
        case SSVAR_OBJECTHANDLE:
            return surgescript_util_strncpy(buf, "[object]", bufsize);
        case SSVAR_NUMBER: {
            char tmp[32];
            if(var->number <= LONG_MIN || var->number >= LONG_MAX || var->number == (long)(var->number))
                snprintf(tmp, sizeof(tmp), "%ld", (long)(var->number)); /* it is an integer */
            else
                snprintf(tmp, sizeof(tmp), "%f", var->number);
            return surgescript_util_strncpy(buf, tmp, bufsize);
        }
    }

    return buf;
}

/*
 * surgescript_var_compare()
 * Compares a to b. Returns:
 * 0   if a == b
 * <0  if a < b
 * >0  if a > b
 */
int surgescript_var_compare(const surgescript_var_t* a, const surgescript_var_t* b)
{
    if(a->type == b->type) {
        switch(a->type) {
            case SSVAR_BOOL:
                return (int)(a->boolean) - (int)(b->boolean);
            case SSVAR_NUMBER:
                if(a->number > b->number)
                    return a->number - b->number < FLT_EPSILON * fabsf(a->number) ? 0 : 1;
                else if(a->number < b->number)
                    return b->number - a->number < FLT_EPSILON * fabsf(b->number) ? 0 : -1;
                else
                    return 0;
            case SSVAR_OBJECTHANDLE:
                return a->handle != b->handle ? (a->handle > b->handle ? 1 : -1) : 0;
            case SSVAR_STRING:
                return strcmp(a->string, b->string);
            default:
                return 0;
        }
    }
    else {
        if(a->type == SSVAR_NULL || b->type == SSVAR_NULL) {
            return (a->raw > 0) - (b->raw > 0);
        }
        else if(a->type == SSVAR_STRING || b->type == SSVAR_STRING) {
            char buf[128];
            if(a->type == SSVAR_STRING) {
                surgescript_var_to_string(b, buf, sizeof(buf) / sizeof(char));
                return strcmp(a->string, buf);
            }
            else {
                surgescript_var_to_string(a, buf, sizeof(buf) / sizeof(char));
                return strcmp(buf, b->string);
            }
        }
        else if(a->type == SSVAR_NUMBER || b->type == SSVAR_NUMBER) {
            float x = surgescript_var_get_number(a);
            float y = surgescript_var_get_number(b);
            if(x > y)
                return x - y < FLT_EPSILON * fabsf(x) ? 0 : 1;
            else if(x < y)
                return y - x < FLT_EPSILON * fabsf(y) ? 0 : -1;
            else
                return 0;
        }
        else if(a->type == SSVAR_BOOL || b->type == SSVAR_BOOL) {
            bool x = surgescript_var_get_bool(a);
            bool y = surgescript_var_get_bool(b);
            return (int)x - (int)y;
        }
        else if(a->type == SSVAR_OBJECTHANDLE || b->type == SSVAR_OBJECTHANDLE) {
            unsigned long x = surgescript_var_get_objecthandle(a);
            unsigned long y = surgescript_var_get_objecthandle(b);
            return x != y ? (x > y ? 1 : -1) : 0;
        }
        else
            return 0; /* this shouldn't happen */
    }
}

/*
 * surgescript_var_swap()
 * Swaps the contents of a and b (faster than copying stuff with surgescript_var_copy)
 */
void surgescript_var_swap(surgescript_var_t* a, surgescript_var_t* b)
{
    surgescript_var_t t = *a;
    *a = *b;
    *b = t;
}

/*
 * surgescript_var_get_rawbits()
 * Returns the binary value stored in the variable
 */
unsigned long surgescript_var_get_rawbits(const surgescript_var_t* var)
{
    return var->raw;
}

/*
 * surgescript_var_set_rawbits()
 * Sets the binary value of the variable
 */
void surgescript_var_set_rawbits(surgescript_var_t* var, unsigned long raw)
{
    RELEASE_DATA(var);
    var->type = SSVAR_NUMBER;
    var->raw = raw;
}