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

/* tag signature */
typedef uint64_t surgescript_tagsignature_t;
static inline surgescript_tagsignature_t generate_tag_signature(const char* object_name, const char* tag_name);

/* tag system */
struct surgescript_tagsystem_t
{
    fasthash_t* tag_table; /* tag table: object -> tags */
    surgescript_inversetagtable_t* inverse_tag_table; /* inverse tag table: tag -> objects */
    surgescript_tagtree_t* tag_tree; /* the set of all tags */
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
    return tag_system;
}

/*
 * surgescript_tagsystem_destroy()
 * Destroys a Tag System instance
 */
surgescript_tagsystem_t* surgescript_tagsystem_destroy(surgescript_tagsystem_t* tag_system)
{
    surgescript_inversetagtable_t *iit, *itmp;

    remove_tree(tag_system->tag_tree);
    fasthash_destroy(tag_system->tag_table);

    HASH_ITER(hh, tag_system->inverse_tag_table, iit, itmp) {
        HASH_DEL(tag_system->inverse_tag_table, iit);
        remove_tree(iit->objects);
        ssfree(iit->tag_name);
        ssfree(iit);
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