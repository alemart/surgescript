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
 * runtime/tag_system.c
 * SurgeScript Tag System
 */

#include <stdalign.h>
#include <stdint.h>
#include <stdbool.h>
#include "tag_system.h"
#include "../util/ssarray.h"
#include "../util/util.h"
#include "../third_party/uthash.h"

#define FASTHASH_INLINE
#include "../util/fasthash.h"

#define XXH_INLINE_ALL
#define XXH_FORCE_ALIGN_CHECK 1
#include "../third_party/xxhash.h"
#define WANT_FIXED_LENGTH_XXH 0

#if defined(__arm__) || ((defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)) && !(defined(__x86_64__) || defined(_M_X64)))
/* Use XXH32() only on 32-bit platforms */
#define XXH(input, len, seed) XXH32((input), (len), (seed))
typedef uint32_t xxhash_t;
#else
/* XXH64() is faster on 64-bit but slower on 32-bit platforms */
#define XXH(input, len, seed) (XXH64((input), (len), (seed)) & UINT64_C(0xFFFFFFFF)) /* we set the higher 32 bits to zero before computing signatures */
typedef uint64_t xxhash_t;
#endif


/* tag */
typedef xxhash_t surgescript_tag_t;
#define generate_tag(tag_name) (surgescript_tag_t)(XXH((tag_name), strlen(tag_name), 0))

/* tag tree: a binary tree of strings */
typedef struct surgescript_tagtree_t surgescript_tagtree_t;
struct surgescript_tagtree_t
{
    char* key;
    surgescript_tagtree_t* left;
    surgescript_tagtree_t* right;
};

static surgescript_tagtree_t* add_to_tree(surgescript_tagtree_t* tree, const char* key);
static surgescript_tagtree_t* remove_tree(surgescript_tagtree_t* tree);
static void traverse_tree(const surgescript_tagtree_t* tree, void* data, void (*callback)(const char*, void*));

/* inverse tag table: we'll get to know all objects that have a certain tag */
typedef struct surgescript_inversetagtable_t surgescript_inversetagtable_t;
struct surgescript_inversetagtable_t
{
    char* tag_name; /* key */
    surgescript_tagtree_t* objects; /* value */
    surgescript_tag_t tag;
    UT_hash_handle hh;
};

/* tag table: an object may hold an arbitrary number of tags */
typedef struct surgescript_tagtable_t surgescript_tagtable_t;
struct surgescript_tagtable_t
{
    surgescript_tag_t tag;
    char* object_name;
    char* tag_name;
};

static void destroy_tagtable_entry(void* e);

/* tag signature used in the tag table */
typedef uint64_t surgescript_tagsignature_t;

#if defined(__GNUC__)
static inline surgescript_tagsignature_t generate_tag_signature(const char* object_name, const char* tag_name) __attribute__((always_inline));
#else
static inline surgescript_tagsignature_t generate_tag_signature(const char* object_name, const char* tag_name);
#endif

/* bound tag system: helps to quickly test a particular class of objects */
struct surgescript_boundtagsystem_t
{
    char* object_name;
    uint64_t bitset;
    const surgescript_tagsystem_t* tag_system;

    UT_hash_handle hh;
};

static surgescript_boundtagsystem_t* find_bound_tag_system(surgescript_tagsystem_t* tag_system, const char* object_name);

#if defined(__GNUC__)
static inline uint64_t bitmask(const char* tag_name) __attribute__((always_inline));
#else
static inline uint64_t bitmask(const char* tag_name);
#endif

/* tag system */
struct surgescript_tagsystem_t
{
    fasthash_t* tag_table; /* tag table: object -> tags */
    surgescript_inversetagtable_t* inverse_tag_table; /* inverse tag table: tag -> objects */
    surgescript_tagtree_t* tag_tree; /* the set of all tags */
    surgescript_boundtagsystem_t* bound_tag_system; /* bound tag system */
};

/* misc */
static void foreach_tag_of_object(const char* tag_name, void* wrapper);




/*
 * surgescript_tagsystem_create()
 * Creates a Tag System instance
 */
surgescript_tagsystem_t* surgescript_tagsystem_create()
{
    surgescript_tagsystem_t* tag_system = ssmalloc(sizeof *tag_system);
    tag_system->tag_table = fasthash_create(destroy_tagtable_entry, 13);
    tag_system->inverse_tag_table = NULL;
    tag_system->tag_tree = NULL;
    tag_system->bound_tag_system = NULL;
    return tag_system;
}

/*
 * surgescript_tagsystem_destroy()
 * Destroys a Tag System instance
 */
surgescript_tagsystem_t* surgescript_tagsystem_destroy(surgescript_tagsystem_t* tag_system)
{
    surgescript_inversetagtable_t *iit, *itmp;
    surgescript_boundtagsystem_t *bit, *btmp;

    remove_tree(tag_system->tag_tree);
    fasthash_destroy(tag_system->tag_table);

    HASH_ITER(hh, tag_system->inverse_tag_table, iit, itmp) {
        HASH_DEL(tag_system->inverse_tag_table, iit);
        remove_tree(iit->objects);
        ssfree(iit->tag_name);
        ssfree(iit);
    }

    HASH_ITER(hh, tag_system->bound_tag_system, bit, btmp) {
        HASH_DEL(tag_system->bound_tag_system, bit);
        ssfree(bit->object_name);
        ssfree(bit);
    }

    return ssfree(tag_system);
}

/*
 * surgescript_tagsystem_add_tag()
 * Add tag_name to a certain class of objects
 */
void surgescript_tagsystem_add_tag(surgescript_tagsystem_t* tag_system, const char* object_name, const char* tag_name)
{
    surgescript_tagsignature_t signature = generate_tag_signature(object_name, tag_name);
    surgescript_tagtable_t* entry = fasthash_get(tag_system->tag_table, signature);
    surgescript_tag_t tag = generate_tag(tag_name);
    surgescript_inversetagtable_t* ientry = NULL;
    surgescript_boundtagsystem_t* bentry = NULL;

    /* add (object, tag) signature to tag_table */
    if(entry == NULL) {
        entry = ssmalloc(sizeof *entry);
        entry->tag = tag;
        entry->object_name = ssstrdup(object_name);
        entry->tag_name = ssstrdup(tag_name);
        fasthash_put(tag_system->tag_table, signature, entry);
    }
    else {
        /* conflict check */
        if(entry->tag != tag || strcmp(entry->object_name, object_name) != 0 || strcmp(entry->tag_name, tag_name) != 0)
            ssfatal("Tag \"%s\" of object \"%s\" conflicts with tag \"%s\" of object \"%s\"", entry->tag_name, entry->object_name, tag_name, object_name);
        else
            return; /* adding an existing tag */
    }

    /* add tag to inverse_tag_table */
    HASH_FIND(hh, tag_system->inverse_tag_table, tag_name, strlen(tag_name), ientry);
    if(ientry == NULL) {
        ientry = ssmalloc(sizeof *ientry);
        ientry->tag_name = ssstrdup(tag_name);
        ientry->objects = NULL;
        ientry->tag = tag;
        HASH_ADD_KEYPTR(hh, tag_system->inverse_tag_table, ientry->tag_name, strlen(ientry->tag_name), ientry);
    }

    /* add object to the tag entry of inverse_tag_table */
    ientry->objects = add_to_tree(ientry->objects, object_name);

    /* add tag to tag_tree */
    tag_system->tag_tree = add_to_tree(tag_system->tag_tree, tag_name);

    /* add the object to the bound tag system */
    bentry = find_bound_tag_system(tag_system, object_name);
    bentry->bitset |= bitmask(tag_name);
}

/*
 * surgescript_tagsystem_has_tag()
 * Is object_name tagged tag_name?
 */
bool surgescript_tagsystem_has_tag(const surgescript_tagsystem_t* tag_system, const char* object_name, const char* tag_name)
{
    /* This function must be fast!!! */
    surgescript_tagsignature_t signature = generate_tag_signature(object_name, tag_name);
    surgescript_tagtable_t* entry = fasthash_get(tag_system->tag_table, signature);

    return (entry != NULL) && (0 == strcmp(entry->object_name, object_name)) && (0 == strcmp(entry->tag_name, tag_name));
}

/*
 * surgescript_tagsystem_foreach_tag()
 * For each registered tag, calls callback(tag_name, data) in alphabetical order
 */
void surgescript_tagsystem_foreach_tag(const surgescript_tagsystem_t* tag_system, void* data, void (*callback)(const char*,void*))
{
    traverse_tree(tag_system->tag_tree, data, callback);
}

/*
 * surgescript_tagsystem_foreach_tagged_object()
 * For each object tagged tag_name, calls callback(object_name, data)
 */
void surgescript_tagsystem_foreach_tagged_object(const surgescript_tagsystem_t* tag_system, const char* tag_name, void* data, void (*callback)(const char*,void*))
{
    surgescript_inversetagtable_t* ientry = NULL;

    HASH_FIND(hh, tag_system->inverse_tag_table, tag_name, strlen(tag_name), ientry);
    if(ientry != NULL) {
        /* objects are called in alphabetical order */
        traverse_tree(ientry->objects, data, callback);
    }
}

/*
 * surgescript_tagsystem_foreach_tag_of_object()
 * For each tag of object named object_name, calls callback(tag_name, data)
 */
void surgescript_tagsystem_foreach_tag_of_object(const surgescript_tagsystem_t* tag_system, const char* object_name, void* data, void (*callback)(const char*,void*))
{
    /* this operation can be made faster with a dedicated data
       structure, but there aren't too many tags, are there? */
    void* wrapper[] = { (void*)tag_system, (void*)object_name, data, callback };
    surgescript_tagsystem_foreach_tag(tag_system, wrapper, foreach_tag_of_object);
}

/*
 * surgescript_tagsystem_bind()
 * Get a bound tag system bound to object_name
 */
const surgescript_boundtagsystem_t* surgescript_tagsystem_bind(surgescript_tagsystem_t* tag_system, const char* object_name)
{
    return find_bound_tag_system(tag_system, object_name);
}

/*
 * surgescript_boundtagsystem_has_tag()
 * Super quick tag test
 */
bool surgescript_boundtagsystem_has_tag(const surgescript_boundtagsystem_t* bound_tag_system, const char* tag_name)
{
    /* we can check super quickly if the bound class of objects is NOT tagged tag_name */
    if(bound_tag_system->bitset & bitmask(tag_name) == 0)
        return false;

    /* run a standard tag test */
    return surgescript_tagsystem_has_tag(bound_tag_system->tag_system, bound_tag_system->object_name, tag_name);
}


/* private stuff */

/* adds a tag to the tag tree */
surgescript_tagtree_t* add_to_tree(surgescript_tagtree_t* tree, const char* key)
{
    if(tree == NULL) {
        surgescript_tagtree_t* node = ssmalloc(sizeof *node);
        node->key = ssstrdup(key);
        node->left = NULL;
        node->right = NULL;
        return node;
    }
    else {
        int cmp = strcmp(key, tree->key);
        if(cmp < 0)
            tree->left = add_to_tree(tree->left, key);
        else if(cmp > 0)
            tree->right = add_to_tree(tree->right, key);
        return tree;
    }
}

/* removes the whole tag tree */
surgescript_tagtree_t* remove_tree(surgescript_tagtree_t* tree)
{
    if(tree != NULL) {
        remove_tree(tree->left);
        remove_tree(tree->right);
        ssfree(tree->key);
        ssfree(tree);
    }

    return NULL;
}

/* traverses the tag tree, in alphabetical order */
void traverse_tree(const surgescript_tagtree_t* tree, void* data, void (*callback)(const char*, void*))
{
    if(tree != NULL) {
        traverse_tree(tree->left, data, callback);
        callback(tree->key, data);
        traverse_tree(tree->right, data, callback);
    }
}

/* signature generator */
surgescript_tagsignature_t generate_tag_signature(const char* object_name, const char* tag_name)
{
    /* Our app must enforce signature uniqueness */
    alignas(8) char buf[2 * SS_NAMEMAX + 2] = { 0 };
    size_t l1 = strlen(object_name), l2 = strlen(tag_name);
    xxhash_t ha, hb;

#if WANT_FIXED_LENGTH_XXH
    /* version with compile-time constant lengths and inline xxhash functions
       buf[] is pre-initialized with zeros */
    enum { SIZE = sizeof(buf) / 2, DBL_SIZE = sizeof(buf) };
    memcpy(buf, object_name, l1 <= SIZE ? l1 : SIZE);
    memcpy(buf + SIZE, tag_name, l2 <= SIZE ? l2 : SIZE);
    ha = XXH(buf, SIZE, *tag_name);
    hb = XXH(buf, DBL_SIZE, ha + *object_name);
#else
    if(l1 > SS_NAMEMAX) l1 = SS_NAMEMAX;
    if(l2 > SS_NAMEMAX) l2 = SS_NAMEMAX;
    memcpy(buf, object_name, l1);
    memcpy(buf + l1 + 1, tag_name, l2);
    ha = XXH(buf, l1 + 1, l1) + (uint8_t)tag_name[0];
    hb = XXH(buf, l1 + l2 + 1, ha + (uint8_t)object_name[0]);
#endif

    return (uint64_t)hb | (((uint64_t)ha) << 32); /* probably unique */
}

void destroy_tagtable_entry(void* e)
{
    surgescript_tagtable_t* entry = (surgescript_tagtable_t*)e;
    ssfree(entry->object_name);
    ssfree(entry->tag_name);
    ssfree(entry);
}

/* calls a callback for each tag of object named object_name */
void foreach_tag_of_object(const char* tag_name, void* wrapper)
{
    const surgescript_tagsystem_t* tag_system = (const surgescript_tagsystem_t*)(((void**)wrapper)[0]);
    const char* object_name = (const char*)(((void**)wrapper)[1]);
    void* data = ((void**)wrapper)[2];
    void (*callback)(const char*,void*) = (void(*)(const char*,void*))(((void**)wrapper)[3]);

    if(surgescript_tagsystem_has_tag(tag_system, object_name, tag_name))
        callback(tag_name, data);
}

/* find or create a surgescript_boundtagsystem_t entry */
surgescript_boundtagsystem_t* find_bound_tag_system(surgescript_tagsystem_t* tag_system, const char* object_name)
{
    surgescript_boundtagsystem_t* entry = NULL;

    HASH_FIND(hh, tag_system->bound_tag_system, object_name, strlen(object_name), entry);
    if(entry == NULL) {
        entry = ssmalloc(sizeof *entry);
        entry->object_name = ssstrdup(object_name);
        entry->bitset = 0;
        entry->tag_system = tag_system;
        HASH_ADD_KEYPTR(hh, tag_system->bound_tag_system, entry->object_name, strlen(entry->object_name), entry);
    }

    return entry;
}

/* compute a bit mask for a tag name VERY QUICKLY */
uint64_t bitmask(const char* tag_name)
{
    /*

    vowels (lowercase only)
    ------
    'a' == 0b 011 00001 -> 1
    'e' == 0b 011 00101 -> 5
    'i' == 0b 011 01001 -> 9
    'o' == 0b 011 01111 -> 15
    'u' == 0b 011 10101 -> 21

    same 011 prefix, different 5-bit suffix

    bit mask:
    0b1000001000001000100010 == 2130466

    */
    int c0 = *tag_name;
    int is_vowel = (2130466 >> (c0 & 31)) & 1; /* 1 if tag_name[0] is a vowel; 0 otherwise */

    /* take two characters of the string */
    int c1 = tag_name[is_vowel]; /* c1 is probably the first consonant */
    int c2 = tag_name[(c0 != 0 && tag_name[1] != 0) << 1]; /* tag_name[2] unless strlen(tag_name) <= 1 */

    /*

    this code will read tag_name[0] and may read tag_name[1] and tag_name[2],
    depending on the length of the string and on whether or not the first
    character is a lowercase vowel.

    assuming that tag_name[0..2] has only lowercase letters (the convention
    for tag names), c1 and c2 are probably the first two consonants, or maybe
    the first consonant and the next vowel, respectively.

    */

    /* compute a hash in [0, 63] */
    int h = (c2 * 31 + c1) & 63; /* c1 in the lower-order bits */

    /* testing / visual inspection */
    /*printf("%s => %c%c => %d\n", tag_name, c1, c2, h);*/

    /* return the 64-bit (1 << h) mask, or zero if we
       received an empty string (branchless code) */
    return ((uint64_t)(c0 != 0)) << h;
}