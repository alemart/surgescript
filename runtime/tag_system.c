/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/tag_system.c
 * SurgeScript Tag System
 */

#include <stdint.h>
#include <stdbool.h>
#include "tag_system.h"
#include "../util/ssarray.h"
#include "../util/uthash.h"
#include "../util/util.h"

#define generate_tag(tag_name) surgescript_util_str2hash(tag_name)

typedef uint32_t surgescript_tag_t;
typedef struct surgescript_tagtable_t surgescript_tagtable_t;
typedef struct surgescript_inversetagtable_t surgescript_inversetagtable_t;
typedef struct surgescript_tagtree_t surgescript_tagtree_t;

/* tag system */
struct surgescript_tagsystem_t
{
    surgescript_tagtable_t* tag_table; /* tag table: object -> tags */
    surgescript_inversetagtable_t* inverse_tag_table; /* inverse tag table: tag -> objects */
    surgescript_tagtree_t* tag_tree; /* the set of all tags */
};

/* tag table: an object may hold an arbitrary number of tags */
struct surgescript_tagtable_t
{
    char* object_name; /* key */
    SSARRAY(surgescript_tag_t, tag); /* values */
    UT_hash_handle hh;
};

struct surgescript_inversetagtable_t
{
    surgescript_tag_t tag; /* key */
    surgescript_tagtree_t* objects;
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
    tag_system->tag_table = NULL;
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
    surgescript_tagtable_t *it, *tmp;
    surgescript_inversetagtable_t *iit, *itmp;

    remove_tree(tag_system->tag_tree);

    HASH_ITER(hh, tag_system->inverse_tag_table, iit, itmp) {
        HASH_DEL(tag_system->inverse_tag_table, iit);
        remove_tree(iit->objects);
        ssfree(iit);
    }

    HASH_ITER(hh, tag_system->tag_table, it, tmp) {
        HASH_DEL(tag_system->tag_table, it);
        ssfree(it->object_name);
        ssfree(it);
    }

    return ssfree(tag_system);
}

/*
 * surgescript_tagsystem_add_tag()
 * Add tag_name to a certain class of objects
 */
void surgescript_tagsystem_add_tag(surgescript_tagsystem_t* tag_system, const char* object_name, const char* tag_name)
{
    surgescript_tagtable_t* entry = NULL;
    surgescript_inversetagtable_t* ientry = NULL;
    surgescript_tag_t tag = generate_tag(tag_name);

    HASH_FIND(hh, tag_system->tag_table, object_name, strlen(object_name), entry);
    if(entry == NULL) {
        entry = ssmalloc(sizeof *entry);
        entry->object_name = ssstrdup(object_name);
        ssarray_init(entry->tag);
        HASH_ADD_KEYPTR(hh, tag_system->tag_table, entry->object_name, strlen(entry->object_name), entry);
    }

    HASH_FIND(hh, tag_system->inverse_tag_table, &tag, sizeof(tag), ientry);
    if(ientry == NULL) {
        ientry = ssmalloc(sizeof *ientry);
        ientry->tag = tag;
        ientry->objects = NULL;
        HASH_ADD(hh, tag_system->inverse_tag_table, tag, sizeof(tag), ientry);
    }

    ssarray_push(entry->tag, tag);
    ientry->objects = add_to_tree(ientry->objects, object_name);
    tag_system->tag_tree = add_to_tree(tag_system->tag_tree, tag_name);
}

/*
 * surgescript_tagsystem_has_tag()
 * Is object_name tagged tag_name?
 */
bool surgescript_tagsystem_has_tag(const surgescript_tagsystem_t* tag_system, const char* object_name, const char* tag_name)
{
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
    surgescript_tag_t tag = generate_tag(tag_name);

    HASH_FIND(hh, tag_system->inverse_tag_table, &tag, sizeof(tag), ientry);
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