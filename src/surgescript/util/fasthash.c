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
 * util/fasthash.c
 * A fast hash table with integer keys and linear probing
 */
#include <stdlib.h>
#include <stdio.h>
#include "fasthash.h"
#include "util.h"

/* types */
typedef enum fasthash_entry_state_t fasthash_entry_state_t;
typedef struct fasthash_entry_t fasthash_entry_t;

enum fasthash_entry_state_t {
    BLANK,
    ACTIVE,
    DELETED
};

struct fasthash_entry_t
{
    uint64_t key;
    fasthash_entry_state_t state;
    void* value;
};

struct fasthash_t
{
    size_t length;
    size_t capacity;
    uint32_t cap_mask; /* capacity - 1 */
    fasthash_entry_t* data;
    void (*destructor)(void*); /* element destructor */
};

/* static data */
static const unsigned SPARSITY = 4; /* 1 / load_factor */
static fasthash_entry_t BLANK_ENTRY = { 0, BLANK, NULL };
static inline uint32_t hash(uint64_t x, uint32_t m);
static inline void grow(fasthash_t* hashtable);
static void empty_destructor(void* data);



/* ----- public API ----- */

/*
 * fasthash_create()
 * Create a new hash table
 * Function element_destructor deallocates individual elements
 * (it may be set to NULL if no destructor is desired)
 * The initial capacity of the hash table will be set to 2^lg2_cap
 * (it grows as needed)
 */
fasthash_t* fasthash_create(void (*element_destructor)(void*), size_t lg2_cap)
{
    fasthash_t* hashtable = ssmalloc(sizeof(fasthash_t));
    int i;

    hashtable->length = 0;
    hashtable->capacity = 1 << ssmin(16, lg2_cap); /* no more than 64K */
    hashtable->cap_mask = hashtable->capacity - 1;
    hashtable->destructor = element_destructor ? element_destructor : empty_destructor;
    hashtable->data = ssmalloc(hashtable->capacity * sizeof(fasthash_entry_t));
    for(i = 0; i < hashtable->capacity; i++)
        hashtable->data[i] = BLANK_ENTRY;

    return hashtable;
}

/*
 * fasthash_destroy()
 * Destroys an existing hash table
 */
fasthash_t* fasthash_destroy(fasthash_t* hashtable)
{
    /* destroy the remaining elements */
    for(int i = 0; i < hashtable->capacity; i++) {
        if(hashtable->data[i].state == ACTIVE)
            hashtable->destructor(hashtable->data[i].value);
    }
    
    /* release the hash table */
    free(hashtable->data);
    free(hashtable);

    /* omit warnings */
    (void)fasthash_get;
    (void)fasthash_put;
    (void)fasthash_delete;
    (void)fasthash_find;

    /* done */
    return NULL;
}

/*
 * fasthash_get()
 * Gets an element from the hash table
 * Returns NULL if the element doesn't exist
 */
void* fasthash_get(fasthash_t* hashtable, uint64_t key)
{
    uint32_t k = hash(key, hashtable->cap_mask);
    uint32_t marker = hashtable->capacity;

    while(hashtable->data[k].state != BLANK) {
        if(hashtable->data[k].state == ACTIVE) {
            if(hashtable->data[k].key == key) {
                /* swap marker */
                if(marker < hashtable->capacity) {
                    /* remove deleted entry */
                    hashtable->data[marker] = hashtable->data[k];
                    hashtable->data[k] = BLANK_ENTRY;
                    hashtable->length--;
                    return hashtable->data[marker].value;
                }

                /* return element */
                return hashtable->data[k].value;
            }
        }
        else if(marker == hashtable->capacity)
            marker = k; /* save first deleted entry */

        /* probe */
        ++k; k &= hashtable->cap_mask;
    }

    return NULL;
}

/*
 * fasthash_put()
 * Puts an element into the hash table
 */
void fasthash_put(fasthash_t* hashtable, uint64_t key, void* value)
{
    if(hashtable->length < hashtable->capacity / SPARSITY) { /* make it sparse */
        uint32_t k = hash(key, hashtable->cap_mask);

        /* won't accept NULL values */
        if(value == NULL)
            return;

        while(hashtable->data[k].state != BLANK) {
            if(hashtable->data[k].state == DELETED) {
                /* replace deleted element */
                hashtable->data[k].key = key;
                hashtable->data[k].value = value;
                hashtable->data[k].state = ACTIVE;
                return;
            }
            else if(hashtable->data[k].key == key) {
                /* replace active element */
                if(value != hashtable->data[k].value) {
                    hashtable->destructor(hashtable->data[k].value); /* TODO: save until later? */
                    hashtable->data[k].value = value;
                }
                return;
            }

            /* probe */
            ++k; k &= hashtable->cap_mask;
        }

        /* insert new element */
        hashtable->data[k].key = key;
        hashtable->data[k].value = value;
        hashtable->data[k].state = ACTIVE;
        hashtable->length++;
    }
    else {
        grow(hashtable);
        fasthash_put(hashtable, key, value);
    }
}

/*
 * fasthash_delete()
 * Deletes an element from the hash table
 * Returns true on success
 */
bool fasthash_delete(fasthash_t* hashtable, uint64_t key)
{
    uint32_t k = hash(key, hashtable->cap_mask);

    while(hashtable->data[k].state != BLANK) {
        if(hashtable->data[k].key == key) {
            if(hashtable->data[k].state == ACTIVE) {
                /* lazy removal of the entry */
                hashtable->data[k].state = DELETED;
                hashtable->destructor(hashtable->data[k].value);
                return true;
            }
            else
                return false; /* duplicate removal */
        }

        /* probe */
        ++k; k &= hashtable->cap_mask;
    }

    /* key not found */
    return false;
}

/*
 * fasthash_find()
 * Finds an element value such that test(value, data) is true
 * data is a generic pointer given as a parameter
 * If no element satisfies the given test function, NULL is returned
 */
void* fasthash_find(fasthash_t* hashtable, bool (*test)(const void*,void*), void* data)
{
    /* search the entire table */
    for(int i = 0; i < hashtable->capacity; i++) {
        if(hashtable->data[i].state == ACTIVE) {
            if(test(hashtable->data[i].value, data))
                return hashtable->data[i].value;
        }
    }

    /* no element passes the test */
    return NULL;
}


/* ----- private ----- */

void grow(fasthash_t* hashtable)
{
    size_t old_cap = hashtable->capacity;
    int i;

    hashtable->capacity *= 2;
    hashtable->cap_mask = (hashtable->cap_mask << 1) | 1;
    hashtable->data = ssrealloc(hashtable->data, hashtable->capacity * sizeof(fasthash_entry_t));
    for(i = old_cap; i < hashtable->capacity; i++)
        hashtable->data[i] = BLANK_ENTRY;
}

uint32_t hash(uint64_t x, uint32_t m)
{
	x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
	x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
	return (uint32_t)(x ^ (x >> 31)) & m; /* m = 2^k - 1 */
}

void empty_destructor(void* data)
{
    ; /* do nothing */
}