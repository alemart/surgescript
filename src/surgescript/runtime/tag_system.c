/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2022 Alexandre Martins <alemartf(at)gmail(dot)com>
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

#include <stdint.h>
#include <stdbool.h>
#include "tag_system.h"
#include "../util/ssarray.h"
#include "../util/util.h"
#include "../util/uthash.h"
#define XXH_INLINE_ALL
#include "../util/xxhash.h"

#define USE_FAST_TAGS /* use faster algorithms */
#define generate_tag(tag_name) XXH64(tag_name, strlen(tag_name), 0)
typedef uint64_t surgescript_tag_t;
typedef struct surgescript_tagtable_t surgescript_tagtable_t;
typedef struct surgescript_inversetagtable_t surgescript_inversetagtable_t;
typedef struct surgescript_tagtree_t surgescript_tagtree_t;

#if defined(USE_FAST_TAGS)
typedef uint64_t surgescript_tagsignature_t;
#  define FASTHASH_INLINE
#  include "../util/fasthash.h"
#endif

/* tag system */
struct surgescript_tagsystem_t
{
#if defined(USE_FAST_TAGS)
    fasthash_t* tag_table; /* tag table: object -> tags */
#else
    surgescript_tagtable_t* tag_table; /* tag table: object -> tags */
#endif
    surgescript_inversetagtable_t* inverse_tag_table; /* inverse tag table: tag -> objects */
    surgescript_tagtree_t* tag_tree; /* the set of all tags */
};

/* tag table: an object may hold an arbitrary number of tags */
#if defined(USE_FAST_TAGS)
struct surgescript_tagtable_t
{
    surgescript_tag_t tag;
    char* object_name;
    char* tag_name;
};

static void destroy_tagtable_entry(void* e);
static inline surgescript_tagsignature_t generate_tag_signature(const char* object_name, const char* tag_name);
#else
struct surgescript_tagtable_t
{
    char* object_name; /* key */
    SSARRAY(surgescript_tag_t, tag); /* values */
    UT_hash_handle hh;
};
#endif

/* inverse tag table: we'll get to know all objects that have a certain tag */
struct surgescript_inversetagtable_t
{
    char* tag_name; /* key */
    surgescript_tagtree_t* objects; /* value */
    surgescript_tag_t tag;
    UT_hash_handle hh;
};

/* tag tree: a binary tree of strings */
struct surgescript_tagtree_t
{
    char* key;
    surgescript_tagtree_t* left;
    surgescript_tagtree_t* right;
};

static surgescript_tagtree_t* add_to_tree(surgescript_tagtree_t* tree, const char* key);
static void remove_tree(surgescript_tagtree_t* tree);
static void traverse_tree(const surgescript_tagtree_t* tree, void* data, void (*callback)(const char*, void*));


/*
 * surgescript_tagsystem_create()
 * Creates a Tag System instance
 */
surgescript_tagsystem_t* surgescript_tagsystem_create()
{
    surgescript_tagsystem_t* tag_system = ssmalloc(sizeof *tag_system);
#if defined(USE_FAST_TAGS)
    tag_system->tag_table = fasthash_create(destroy_tagtable_entry, 13);
    tag_system->inverse_tag_table = NULL;
    tag_system->tag_tree = NULL;
#else
    tag_system->tag_table = NULL;
    tag_system->inverse_tag_table = NULL;
    tag_system->tag_tree = NULL;
#endif
    return tag_system;
}

/*
 * surgescript_tagsystem_destroy()
 * Destroys a Tag System instance
 */
surgescript_tagsystem_t* surgescript_tagsystem_destroy(surgescript_tagsystem_t* tag_system)
{
#if defined(USE_FAST_TAGS)
    surgescript_inversetagtable_t *iit, *itmp;

    remove_tree(tag_system->tag_tree);
    fasthash_destroy(tag_system->tag_table);

    HASH_ITER(hh, tag_system->inverse_tag_table, iit, itmp) {
        HASH_DEL(tag_system->inverse_tag_table, iit);
        remove_tree(iit->objects);
        ssfree(iit->tag_name);
        ssfree(iit);
    }
#else
    surgescript_tagtable_t *it, *tmp;
    surgescript_inversetagtable_t *iit, *itmp;

    remove_tree(tag_system->tag_tree);

    HASH_ITER(hh, tag_system->inverse_tag_table, iit, itmp) {
        HASH_DEL(tag_system->inverse_tag_table, iit);
        remove_tree(iit->objects);
        ssfree(iit->tag_name);
        ssfree(iit);
    }

    HASH_ITER(hh, tag_system->tag_table, it, tmp) {
        HASH_DEL(tag_system->tag_table, it);
        ssarray_release(it->tag);
        ssfree(it->object_name);
        ssfree(it);
    }
#endif

    return ssfree(tag_system);
}

/*
 * surgescript_tagsystem_add_tag()
 * Add tag_name to a certain class of objects
 */
void surgescript_tagsystem_add_tag(surgescript_tagsystem_t* tag_system, const char* object_name, const char* tag_name)
{
#if defined(USE_FAST_TAGS)
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
#else
    surgescript_tagtable_t* entry = NULL;
    surgescript_inversetagtable_t* ientry = NULL;
    surgescript_tag_t tag = generate_tag(tag_name);

    if(surgescript_tagsystem_has_tag(tag_system, object_name, tag_name))
        return;

    HASH_FIND(hh, tag_system->tag_table, object_name, strlen(object_name), entry);
    if(entry == NULL) {
        entry = ssmalloc(sizeof *entry);
        entry->object_name = ssstrdup(object_name);
        ssarray_init(entry->tag);
        HASH_ADD_KEYPTR(hh, tag_system->tag_table, entry->object_name, strlen(entry->object_name), entry);
    }

    HASH_FIND(hh, tag_system->inverse_tag_table, tag_name, strlen(tag_name), ientry);
    if(ientry == NULL) {
        ientry = ssmalloc(sizeof *ientry);
        ientry->tag_name = ssstrdup(tag_name);
        ientry->objects = NULL;
        ientry->tag = tag;
        HASH_ADD_KEYPTR(hh, tag_system->inverse_tag_table, ientry->tag_name, strlen(ientry->tag_name), ientry);
    }

    ssarray_push(entry->tag, tag);
    ientry->objects = add_to_tree(ientry->objects, object_name);
    tag_system->tag_tree = add_to_tree(tag_system->tag_tree, tag_name);
#endif
}

/*
 * surgescript_tagsystem_has_tag()
 * Is object_name tagged tag_name?
 */
bool surgescript_tagsystem_has_tag(const surgescript_tagsystem_t* tag_system, const char* object_name, const char* tag_name)
{
#if defined(USE_FAST_TAGS)
    /* This function must be fast!!! */
    surgescript_tagsignature_t signature = generate_tag_signature(object_name, tag_name);
    surgescript_tagtable_t* entry = fasthash_get(tag_system->tag_table, signature);

    if(entry != NULL)
        return (entry->tag == generate_tag(tag_name));
    else
        return false;
#else
    surgescript_tagtable_t* entry = NULL;

    HASH_FIND(hh, tag_system->tag_table, object_name, strlen(object_name), entry);
    if(entry != NULL) {
        surgescript_tag_t tag = generate_tag(tag_name);
        for(int i = 0; i < ssarray_length(entry->tag); i++) {
            if(entry->tag[i] == tag)
                return true;
        }
    }

    return false;
#endif
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
void remove_tree(surgescript_tagtree_t* tree)
{
    if(tree != NULL) {
        remove_tree(tree->left);
        remove_tree(tree->right);
        ssfree(tree->key);
        ssfree(tree);
    }
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

#if defined(USE_FAST_TAGS)
/* signature generator */
surgescript_tagsignature_t generate_tag_signature(const char* object_name, const char* tag_name)
{
    char buf[2 * SS_NAMEMAX + 2] = { 0 };
    uint32_t ha = 0, hb = 0;
    size_t l1 = strlen(object_name), l2 = strlen(tag_name);
    memcpy(buf, object_name, l1);
    memcpy(buf + l1 + 1, tag_name, l2);
    ha = XXH32(buf, l1 + 1, l1) + (uint8_t)tag_name[0];
    hb = XXH32(buf, l1 + l2 + 1, ha + (uint8_t)object_name[0]);
    return (uint64_t)hb | (((uint64_t)ha) << 32); /* probably unique */
}

void destroy_tagtable_entry(void* e)
{
    surgescript_tagtable_t* entry = (surgescript_tagtable_t*)e;
    ssfree(entry->object_name);
    ssfree(entry->tag_name);
    ssfree(entry);
}
#endif