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
 * runtime/program_pool.c
 * SurgeScript program pool
 */

#include <stdalign.h>
#include <stdint.h>
#include <string.h>
#include "program_pool.h"
#include "program.h"
#include "../util/uthash.h"
#include "../util/util.h"
#include "../util/ssarray.h"

#define FASTHASH_INLINE
#include "../util/fasthash.h"

#define XXH_INLINE_ALL
#define XXH_FORCE_ALIGN_CHECK 1
#include "../util/xxhash.h"
#define WANT_FIXED_LENGTH_XXH 1 /* beneficial when XXH_INLINE_ALL is defined */

#if defined(__arm__) || ((defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)) && !(defined(__x86_64__) || defined(_M_X64)))
/* Use XXH32() only on 32-bit platforms */
#define XXH(input, len, seed) XXH32((input), (len), (seed))
typedef uint32_t xxhash_t;
#else
/* XXH64() is faster on 64-bit but slower on 32-bit platforms */
#define XXH(input, len, seed) (XXH64((input), (len), (seed)) & UINT64_C(0xFFFFFFFF)) /* we set the higher 32 bits to zero before computing signatures */
typedef uint64_t xxhash_t;
#endif


/*
 * Each function in SurgeScript defines a function signature
 * that depends on the containing object and on the function name
 */
typedef uint64_t surgescript_programpool_signature_t;
static inline surgescript_programpool_signature_t generate_signature(const char* object_name, const char* program_name); /* generates a function signature, given an object name and a program name */


/* metadata */
typedef struct surgescript_programpool_metadata_t surgescript_programpool_metadata_t;
struct surgescript_programpool_metadata_t /* we'll store the list of program names for each object */
{
    char* object_name;
    SSARRAY(char*, program_name);
    UT_hash_handle hh;
};

static void insert_metadata(surgescript_programpool_t* pool, const char* object_name, const char* program_name);
static void remove_metadata(surgescript_programpool_t* pool, const char* object_name, const char* program_name);
static void remove_object_metadata(surgescript_programpool_t* pool, const char* object_name);
static void clear_metadata(surgescript_programpool_t* pool);
static void traverse_metadata(surgescript_programpool_t* pool, const char* object_name, void* data, void (*callback)(const char*,void*));
static void traverse_adapter(const char* program_name, void* callback);


/* program pool hash type */
typedef struct surgescript_programpool_hashpair_t surgescript_programpool_hashpair_t;
struct surgescript_programpool_hashpair_t /* for each function signature, store a reference to its program */
{
    surgescript_programpool_signature_t signature; /* key */
    surgescript_program_t* program; /* value */
};


/* program pool */
struct surgescript_programpool_t
{
    fasthash_t* hash; /* a hash table of hashpair_t's */
    surgescript_programpool_metadata_t* meta;
};

/* misc */
static void delete_pair(void* pair);
static void delete_program(const char* program_name, void* data);




/* -------------------------------
 * public methods
 * ------------------------------- */

/*
 * surgescript_programpool_create()
 * Creates a new program pool
 */
surgescript_programpool_t* surgescript_programpool_create()
{
    surgescript_programpool_t* pool = ssmalloc(sizeof *pool);
    pool->hash = fasthash_create(delete_pair, 16);
    pool->meta = NULL;
    return pool;
}

/*
 * surgescript_programpool_destroy()
 * Destroys a program pool
 */
surgescript_programpool_t* surgescript_programpool_destroy(surgescript_programpool_t* pool)
{
    fasthash_destroy(pool->hash);
    clear_metadata(pool);
    return ssfree(pool);
}

/*
 * surgescript_programpool_exists()
 * Does the specified program exist in the pool?
 */
bool surgescript_programpool_exists(surgescript_programpool_t* pool, const char* object_name, const char* program_name)
{
    return surgescript_programpool_get(pool, object_name, program_name) != NULL;
}

/*
 * surgescript_programpool_shallowcheck()
 * Does the specified program, of EXACTLY the specified object (not a parent class), exist in the pool?
 */
bool surgescript_programpool_shallowcheck(surgescript_programpool_t* pool, const char* object_name, const char* program_name)
{
    surgescript_programpool_signature_t signature = generate_signature(object_name, program_name);
    surgescript_programpool_hashpair_t* pair = fasthash_get(pool->hash, signature);
    return pair != NULL;
}

/*
 * surgescript_programpool_put()
 * Puts a program in the pool
 */
bool surgescript_programpool_put(surgescript_programpool_t* pool, const char* object_name, const char* program_name, surgescript_program_t* program)
{
    if(!surgescript_programpool_shallowcheck(pool, object_name, program_name)) {
        surgescript_programpool_hashpair_t* pair = ssmalloc(sizeof *pair);
        pair->signature = generate_signature(object_name, program_name);
        pair->program = program;
        fasthash_put(pool->hash, pair->signature, pair);
        insert_metadata(pool, object_name, program_name);
        return true;
    }
    else {
        ssfatal("Runtime Error: duplicate function \"%s\" in object \"%s\"", program_name, object_name);
        return false;
    }
}


/*
 * surgescript_programpool_get()
 * Gets a program from the pool (returns NULL if not found)
 * This needs to be fast!
 */
surgescript_program_t* surgescript_programpool_get(surgescript_programpool_t* pool, const char* object_name, const char* program_name)
{
    surgescript_programpool_signature_t signature = generate_signature(object_name, program_name);
    surgescript_programpool_hashpair_t* pair = fasthash_get(pool->hash, signature); /* find the program */
    
    /* if there is no such program */
    if(!pair) {
        /* try locating it in a common base for all objects */
        signature = generate_signature("Object", program_name);
        pair = fasthash_get(pool->hash, signature);

        /* really, the program doesn't exist */
        if(!pair)
            return NULL;
    }

    /* found it! */
    return pair->program;
}

/*
 * surgescript_programpool_foreach()
 * For each program of object_name, calls the callback
 */
void surgescript_programpool_foreach(surgescript_programpool_t* pool, const char* object_name, void (*callback)(const char*))
{
    traverse_metadata(pool, object_name, callback, traverse_adapter);
}

/*
 * surgescript_programpool_foreach_ex()
 * For each program of object_name, calls the callback with an added data parameter
 */
void surgescript_programpool_foreach_ex(surgescript_programpool_t* pool, const char* object_name, void* data, void (*callback)(const char*, void*))
{
    traverse_metadata(pool, object_name, data, callback);
}



/*
 * surgescript_programpool_replace()
 * Replaces a program in the pool (if it doesn't exist, create it)
 */
bool surgescript_programpool_replace(surgescript_programpool_t* pool, const char* object_name, const char* program_name, surgescript_program_t* program)
{
    surgescript_programpool_signature_t signature = generate_signature(object_name, program_name);
    surgescript_programpool_hashpair_t* pair = fasthash_get(pool->hash, signature); /* find the program */
    
    /* replace the program */
    if(pair != NULL) {
        surgescript_program_destroy(pair->program);
        pair->program = program;
        return true;
    }
    else
        return surgescript_programpool_put(pool, object_name, program_name, program); /* program doesn't exist yet */
}


/*
 * surgescript_programpool_purge()
 * Deletes all the programs of the specified object
 */
void surgescript_programpool_purge(surgescript_programpool_t* pool, const char* object_name)
{
    void* data[] = { pool, (void*)object_name };
    surgescript_programpool_foreach_ex(pool, object_name, data, delete_program);
    remove_object_metadata(pool, object_name);
}


/*
 * surgescript_programpool_delete()
 * Deletes a programs from the specified object
 */
void surgescript_programpool_delete(surgescript_programpool_t* pool, const char* object_name, const char* program_name)
{
    surgescript_programpool_signature_t signature = generate_signature(object_name, program_name);
    
    /* delete the program */
    fasthash_delete(pool->hash, signature);

    /* delete metadata */
    remove_metadata(pool, object_name, program_name);
}



/*
 * surgescript_programpool_is_compiled()
 * Is there any code for object_name?
 */
bool surgescript_programpool_is_compiled(surgescript_programpool_t* pool, const char* object_name)
{
    surgescript_programpool_metadata_t *m = NULL;
    HASH_FIND_STR(pool->meta, object_name, m);
    return (m != NULL) && (ssarray_length(m->program_name) > 0);
}



/* -------------------------------
 * private methods
 * ------------------------------- */


 /* metadata */
void insert_metadata(surgescript_programpool_t* pool, const char* object_name, const char* program_name)
{
    surgescript_programpool_metadata_t *m = NULL;
    HASH_FIND(hh, pool->meta, object_name, strlen(object_name), m);

    /* create the hash entry if it doesn't exist yet */
    if(m == NULL) {
        m = ssmalloc(sizeof *m);
        m->object_name = ssstrdup(object_name);
        ssarray_init(m->program_name);
        HASH_ADD_KEYPTR(hh, pool->meta, m->object_name, strlen(m->object_name), m);
    }

    /* no need to check for key uniqueness (it's checked before) */
    ssarray_push(m->program_name, ssstrdup(program_name));
}

void remove_metadata(surgescript_programpool_t* pool, const char* object_name, const char* program_name)
{
    surgescript_programpool_metadata_t *m = NULL;
    HASH_FIND(hh, pool->meta, object_name, strlen(object_name), m);

    if(m != NULL) {
        int index = -1;

        /* find the program */
        for(int i = 0; i < ssarray_length(m->program_name) && index < 0; i++) {
            if(strcmp(m->program_name[i], program_name) == 0)
                index = i;
        }

        /* delete the program */
        if(index >= 0) {
            ssfree(m->program_name[index]);
            ssarray_remove(m->program_name, index);
        }
    }
}

void remove_object_metadata(surgescript_programpool_t* pool, const char* object_name)
{
    surgescript_programpool_metadata_t *m = NULL;
    HASH_FIND(hh, pool->meta, object_name, strlen(object_name), m);

    /* delete all programs of object_name */
    if(m != NULL) {
        HASH_DEL(pool->meta, m);
        for(int i = 0; i < ssarray_length(m->program_name); i++)
            ssfree(m->program_name[i]);
        ssarray_release(m->program_name);
        ssfree(m->object_name);
        ssfree(m);
    }
}

void clear_metadata(surgescript_programpool_t* pool)
{
    surgescript_programpool_metadata_t *it, *tmp;

    /* delete all programs of all objects */
    HASH_ITER(hh, pool->meta, it, tmp) {
        HASH_DEL(pool->meta, it);
        for(int i = 0; i < ssarray_length(it->program_name); i++)
            ssfree(it->program_name[i]);
        ssarray_release(it->program_name);
        ssfree(it->object_name);
        ssfree(it);
    }
}

void traverse_metadata(surgescript_programpool_t* pool, const char* object_name, void* data, void (*callback)(const char*,void*))
{
    surgescript_programpool_metadata_t *m = NULL;
    HASH_FIND_STR(pool->meta, object_name, m);

    if(m != NULL) {
        for(int i = 0; i < ssarray_length(m->program_name); i++)
            callback(m->program_name[i], data);
    }
}

void traverse_adapter(const char* program_name, void* callback)
{
    ((void (*)(const char*))callback)(program_name);
}


/* utilities */
void delete_pair(void* pair)
{
    surgescript_programpool_hashpair_t* p = (surgescript_programpool_hashpair_t*)pair;
    surgescript_program_destroy(p->program);
    ssfree(p);
}

void delete_program(const char* program_name, void* data)
{
    surgescript_programpool_t* pool = (surgescript_programpool_t*)(((void**)data)[0]);
    const char* object_name = (const char*)(((void**)data)[1]);
    surgescript_programpool_signature_t signature = generate_signature(object_name, program_name);

    /* delete the program */
    fasthash_delete(pool->hash, signature);
}


/* program signature generator: must be extremely fast */
surgescript_programpool_signature_t generate_signature(const char* object_name, const char* program_name)
{
    /* Our app must enforce signature uniqueness */
    alignas(8) char buf[2 * SS_NAMEMAX + 2] = { 0 };
    size_t l1 = strlen(object_name), l2 = strlen(program_name);
    xxhash_t ha, hb;

#if WANT_FIXED_LENGTH_XXH
    /* version with compile-time constant lengths and inline xxhash functions
       buf[] is pre-initialized with zeros */
    enum { SIZE = sizeof(buf) / 2, DBL_SIZE = sizeof(buf) };
    memcpy(buf, object_name, l1 <= SIZE ? l1 : SIZE);
    memcpy(buf + SIZE, program_name, l2 <= SIZE ? l2 : SIZE);
    ha = XXH(buf, SIZE, *program_name);
    hb = XXH(buf, DBL_SIZE, ha + *object_name);
#else
    if(l1 > SS_NAMEMAX) l1 = SS_NAMEMAX;
    if(l2 > SS_NAMEMAX) l2 = SS_NAMEMAX;
    memcpy(buf, object_name, l1);
    memcpy(buf + l1 + 1, program_name, l2);
    ha = XXH(buf, l1 + 1, l1) + (uint8_t)program_name[0];
    hb = XXH(buf, l1 + l2 + 1, ha + (uint8_t)object_name[0]);
#endif

    return (uint64_t)hb | (((uint64_t)ha) << 32); /* probably unique */
}