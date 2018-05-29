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
 * runtime/variable.c
 * SurgeScript variables
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include "variable.h"
#include "object.h"
#include "object_manager.h"
#include "../util/util.h"
#include "../util/utf8.h"


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
    /* data */
    union {
        char* string;
        double number;
        unsigned handle:32;
        bool boolean;
        int64_t raw;
    };

    /* metadata */
    enum surgescript_vartype_t type;
};

/* var pool */
/*#define DISABLE_VARPOOL*/
#ifndef DISABLE_VARPOOL

typedef struct surgescript_varpool_t surgescript_varpool_t;
typedef struct surgescript_varbucket_t surgescript_varbucket_t;
struct surgescript_varpool_t
{
    /* a pool is a collection of buckets */
    #define VARPOOL_NUM_BUCKETS 937

    struct surgescript_varbucket_t {
        union {
            /* the 1st element of the bucket (var) shares
               the same address as the bucket itself */
            surgescript_var_t var; /* var data */
            surgescript_varbucket_t* next; /* free list */
        };
        bool in_use; /* is this bucket currently in use? */
    } bucket[VARPOOL_NUM_BUCKETS];

    surgescript_varpool_t* next;
};
static surgescript_varpool_t* new_varpool(surgescript_varpool_t* next);
static surgescript_varpool_t* delete_varpools(surgescript_varpool_t* head);
static surgescript_varbucket_t* get_1stbucket(surgescript_varpool_t* pool);
static inline surgescript_varbucket_t* allocate_bucket();
static inline void free_bucket(surgescript_varbucket_t* bucket);
static surgescript_varpool_t* varpool = NULL;
static surgescript_varbucket_t* varpool_currbucket = NULL;

#endif

/* helpers */
#define RELEASE_DATA(var)       if((var)->type == SSVAR_STRING) \
                                    (var)->string = ssfree((var)->string); /* this will clear all bits */
static inline bool is_number(const char* str);
static const int typecode[] = { 0, 'b', 'n', 's', 'o' };

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
#ifndef DISABLE_VARPOOL
    surgescript_var_t* var = (surgescript_var_t*)allocate_bucket();
    var->type = SSVAR_NULL;
    var->raw = 0;
    return var;
#else
    surgescript_var_t* var = ssmalloc(sizeof *var);
    var->type = SSVAR_NULL;
    var->raw = 0;
    return var;
#endif
}

/*
 * surgescript_var_destroy()
 * Destroys an existing variable from memory
 */
surgescript_var_t* surgescript_var_destroy(surgescript_var_t* var)
{
#ifndef DISABLE_VARPOOL
    RELEASE_DATA(var);
    free_bucket((surgescript_varbucket_t*)var);
    return NULL;
#else
    RELEASE_DATA(var);
    ssfree(var);
    return NULL;
#endif
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
    var->raw = 0; /* must clear up all bits; see get_rawbits() below */
    var->boolean = boolean; /* stdbool.h guarantees: expands to 1 or 0 */
    return var;
}

/*
 * surgescript_var_set_number()
 * Sets the variable to a numeric variable
 */
surgescript_var_t* surgescript_var_set_number(surgescript_var_t* var, double number)
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
    static const int MAXLEN = 1048576 - 1; /* 1 MB */

    RELEASE_DATA(var);
    if(string && strlen(string) <= MAXLEN) {
        var->type = SSVAR_STRING;
        var->string = IsUTF8((uint8_t*)string) ? ssstrdup(string) : str2utf8(string); /* ensures all strings are UTF-8 encoded */
    }
    else if(!string) {
        var->type = SSVAR_STRING;
        var->string = ssstrdup("");
    }
    else {
        ((char*)string)[128] = '\0';
        ssfatal("Runtime Error: string \"%s...\" is too large!", string);
    }

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
            return var->raw && fpclassify(var->number) != FP_ZERO;
        case SSVAR_STRING:
            return *(var->string) != 0;
        case SSVAR_NULL:
            return false;
        case SSVAR_OBJECTHANDLE:
            return var->handle != 0;
    }

    return false;
}

/*
 * surgescript_var_get_number()
 * Gets the numeric value of a variable
 */
double surgescript_var_get_number(const surgescript_var_t* var)
{
    switch(var->type) {
        case SSVAR_NUMBER:
            return var->number;
        case SSVAR_BOOL:
            return var->boolean ? 1.0 : 0.0;
        case SSVAR_STRING:
            return is_number(var->string) ? atof(var->string) : NAN;
        case SSVAR_NULL:
            return 0.0;
        case SSVAR_OBJECTHANDLE:
            return NAN;
    }

    return 0.0;
}

/*
 * surgescript_var_get_string()
 * Gets the contents of a string variable. You have to ssfree() this after use.
 * You may pass NULL as the object manager parameter. However,
 * If you want to perform an object->string conversion, then you must pass a valid manager.
 */
char* surgescript_var_get_string(const surgescript_var_t* var, const surgescript_objectmanager_t* manager)
{
    switch(var->type) {
        case SSVAR_NULL:
            return ssstrdup("null");
        case SSVAR_BOOL:
            return ssstrdup(var->boolean ? "true" : "false");
        case SSVAR_STRING:
            return ssstrdup(var->string);
        case SSVAR_NUMBER: {
            char buf[32];
            surgescript_var_to_string(var, buf, sizeof(buf));
            return ssstrdup(buf);
        }
        case SSVAR_OBJECTHANDLE: {
            if(manager != NULL) {
                surgescript_object_t* obj = surgescript_objectmanager_get(manager, var->handle);
                surgescript_var_t* tmp = surgescript_var_create(); char* str;
                surgescript_object_call_function(obj, "toString", NULL, 0, tmp);
                str = surgescript_var_get_string(tmp, NULL); /* discard manager */
                surgescript_var_destroy(tmp);
                return str;
            }
            else
                return ssstrdup("[object]");
        }
        default:
            return ssstrdup("");
    }
}

/*
 * surgescript_var_get_objecthandle()
 * Gets the object handle
 */
unsigned surgescript_var_get_objecthandle(const surgescript_var_t* var)
{
    /* will return the primitive wrapper if var doesn't store a handle */
    switch(var->type) {
        case SSVAR_OBJECTHANDLE:
            return var->handle;
        case SSVAR_NUMBER:
            return surgescript_objectmanager_system_object(NULL, "Number");
        case SSVAR_STRING:
            return surgescript_objectmanager_system_object(NULL, "String");
        case SSVAR_BOOL:
            return surgescript_objectmanager_system_object(NULL, "Boolean");
        default:
            return surgescript_objectmanager_null(NULL);
    }
}


/* misc */

/*
 * surgescript_var_copy()
 * Copies src into dst, without creating a new variable.
 * Returns dst. Similar to strcpy()
 */
surgescript_var_t* surgescript_var_copy(surgescript_var_t* dst, const surgescript_var_t* src)
{
    RELEASE_DATA(dst);
    dst->type = src->type;
    dst->raw = 0; /* must clear up all bits */

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
            dst->raw = src->raw;
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
    surgescript_var_t* copy = surgescript_var_create();
    return surgescript_var_copy(copy, var);
}

/*
 * surgescript_var_typecode()
 * Returns an integer representing the type of the variable
 * (each type has its own code)
 */
int surgescript_var_typecode(const surgescript_var_t* var)
{
    return typecode[(int)(var->type)];
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
 * surgescript_var_typecheck()
 * Returns zero if, and only if, var has the given type code
 */
int surgescript_var_typecheck(const surgescript_var_t* var, int code)
{
    return typecode[(int)(var->type)] ^ code;
}

/*
 * surgescript_var_to_string()
 * Converts a variable of any primitive type to a string to be stored in a buffer of bufsize bytes
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
            if(var->number == ceil(var->number)) /* integer check */
                snprintf(tmp, sizeof(tmp), "%.0lf", var->number);
            else
                snprintf(tmp, sizeof(tmp), "%lf", var->number);
            return surgescript_util_strncpy(buf, tmp, bufsize);
        }
    }

    return buf;
}

/*
 * surgescript_var_fast_get_string()
 * gets the string contents of var without performing any type conversion
 */
const char* surgescript_var_fast_get_string(const surgescript_var_t* var)
{
    return var->type == SSVAR_STRING ? var->string : "";
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
            case SSVAR_OBJECTHANDLE:
                return a->handle != b->handle ? (a->handle > b->handle ? 1 : -1) : 0;
            case SSVAR_STRING:
                return strcmp(a->string, b->string);
            case SSVAR_NUMBER: {
                double ma = fabs(a->number), mb = fabs(b->number);
                if(a->number > b->number)
                    return a->number - b->number <= FLT_EPSILON * ssmax(ma, mb) ? 0 : 1;
                else if(a->number < b->number)
                    return b->number - a->number <= FLT_EPSILON * ssmax(ma, mb) ? 0 : -1;
                else
                    return 0;
            }
            default:
                return 0;
        }
    }
    else {
        if(a->type == SSVAR_NULL || b->type == SSVAR_NULL) {
            return (a->raw != 0) - (b->raw != 0);
        }
        else if(a->type == SSVAR_STRING || b->type == SSVAR_STRING) {
            char buf[128];
            if(a->type == SSVAR_STRING) {
                surgescript_var_to_string(b, buf, sizeof(buf));
                return strcmp(a->string, buf);
            }
            else {
                surgescript_var_to_string(a, buf, sizeof(buf));
                return strcmp(buf, b->string);
            }
        }
        else if(a->type == SSVAR_NUMBER || b->type == SSVAR_NUMBER) {
            double x = surgescript_var_get_number(a);
            double y = surgescript_var_get_number(b);
            double mx = fabs(x), my = fabs(y);
            if(x > y)
                return x - y <= FLT_EPSILON * ssmax(mx, my) ? 0 : 1;
            else if(x < y)
                return y - x <= FLT_EPSILON * ssmax(mx, my) ? 0 : -1;
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
int64_t surgescript_var_get_rawbits(const surgescript_var_t* var)
{
    return var->raw;
}

/*
 * surgescript_var_set_rawbits()
 * Sets the binary value of the variable (for internal use only)
 */
surgescript_var_t* surgescript_var_set_rawbits(surgescript_var_t* var, int64_t raw)
{
    RELEASE_DATA(var);
    var->type = SSVAR_NUMBER;
    var->raw = raw;
    return var;
}


/*
 * surgescript_var_size()
 * Used memory in user space, in bytes
 */
size_t surgescript_var_size(const surgescript_var_t* var)
{
    if(var->type == SSVAR_STRING)
        return sizeof(surgescript_var_t) + (1 + strlen(var->string)) * sizeof(char);
    else
        return sizeof(surgescript_var_t);
}





/* var pooling */

/*
 * surgescript_var_init_pool()
 * Initializes the pool
 */
void surgescript_var_init_pool()
{
#ifndef DISABLE_VARPOOL
    if(varpool == NULL) {
        varpool = new_varpool(NULL);
        varpool_currbucket = get_1stbucket(varpool);
    }
#else
    sslog("Warning: SurgeScript has been compiled with disabled var pooling.");
#endif
}

/*
 * surgescript_var_release_pool()
 * Releases the pool
 */
void surgescript_var_release_pool()
{
#ifndef DISABLE_VARPOOL
    if(varpool != NULL) {
        varpool_currbucket = NULL;
        varpool = delete_varpools(varpool);
    }
#endif
}


/* private section */

bool is_number(const char* str)
{
    if(str == NULL)
        return false;
    if(*str == '-' || *str == '+')
        str++;
    while(*str) {
        if(*str == '.') {
            str++;
            break;
        }
        else if(isdigit(*str))
            str++;
        else
            return false;
    }
    while(*str) {
        if(isdigit(*str))
            str++;
        else
            return false;
    }
    return true;
}


/* private var pool routines */
#ifndef DISABLE_VARPOOL

/* Creates a new var pool */
surgescript_varpool_t* new_varpool(surgescript_varpool_t* next)
{
    surgescript_varpool_t* pool;
    sslog("Allocating a new var pool...");

    pool = ssmalloc(sizeof *pool);
    for(int i = 0; i < VARPOOL_NUM_BUCKETS - 1; i++) {
        pool->bucket[i].next = &(pool->bucket[i + 1]);
        pool->bucket[i].in_use = false;
    }
    pool->bucket[VARPOOL_NUM_BUCKETS - 1].next = NULL;
    pool->bucket[VARPOOL_NUM_BUCKETS - 1].in_use = false;
    pool->next = next;

    return pool;
}

/* Deletes all var pools */
surgescript_varpool_t* delete_varpools(surgescript_varpool_t* head)
{
    if(head->next)
        delete_varpools(head->next);
    return ssfree(head);
}

/* Gets the 1st bucket of a pool */
surgescript_varbucket_t* get_1stbucket(surgescript_varpool_t* pool)
{
    return &(pool->bucket[0]);
}

/* Allocates a bucket (must be fast) */
surgescript_varbucket_t* allocate_bucket()
{
    surgescript_varbucket_t* bucket = varpool_currbucket;

    /* consistency check */
    /*ssassert(bucket && !bucket->in_use);*/

    /* select bucket */
    if(bucket->next == NULL)
        bucket->next = get_1stbucket(varpool = new_varpool(varpool));
    varpool_currbucket = bucket->next;
    bucket->in_use = true;

    /* done! */
    return bucket;
}

/* Deallocates a bucket (must be fast) */
void free_bucket(surgescript_varbucket_t* bucket)
{
    bucket->in_use = false;
    bucket->next = varpool_currbucket;
    varpool_currbucket = bucket;
}

#endif