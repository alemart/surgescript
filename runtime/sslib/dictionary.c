/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/dictionary.c
 * SurgeScript standard library: Dictionary data structure
 */

#include <string.h>
#include "../vm.h"
#include "../heap.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"

/* private stuff */

/* Dictionary */
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getsize(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_clear(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_delete(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_has(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_iterator(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* DictionaryIterator */
static surgescript_var_t* fun_it_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_it_next(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_it_hasnext(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_it_getvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* BSTNode: native implementation of a Binary Search Tree in SurgeScript */
static surgescript_var_t* fun_bst_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_getkey(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_getvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_getleft(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_getright(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_setvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_count(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_find(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_insert(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_remove(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* utilities */
static const surgescript_heapptr_t BST_KEY = 0; /* address of the 'key' field in the BST */
static const surgescript_heapptr_t BST_VALUE = 1; /* and so on */
static const surgescript_heapptr_t BST_LEFT = 2;
static const surgescript_heapptr_t BST_RIGHT = 3;

static surgescript_var_t* var2string(const surgescript_var_t* ssvar);
static surgescript_objecthandle_t new_bst_node(const surgescript_object_t* parent, const surgescript_var_t* key, const surgescript_var_t* value);
static int bst_count(const surgescript_objectmanager_t* manager, const surgescript_object_t* object);
static surgescript_var_t* bst_remove(surgescript_object_t* object, const char* param_key, int depth);
static surgescript_var_t* bst_removeroot(surgescript_object_t* object);

/*
 * surgescript_sslib_register_dictionary()
 * Register methods
 */
void surgescript_sslib_register_dictionary(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "BSTNode", "__constructor", fun_bst_constructor, 0);
    surgescript_vm_bind(vm, "BSTNode", "state:main", fun_bst_main, 0);
    surgescript_vm_bind(vm, "BSTNode", "getKey", fun_bst_getkey, 0);
    surgescript_vm_bind(vm, "BSTNode", "getValue", fun_bst_getvalue, 0);
    surgescript_vm_bind(vm, "BSTNode", "setValue", fun_bst_setvalue, 1);
    surgescript_vm_bind(vm, "BSTNode", "getLeft", fun_bst_getleft, 0);
    surgescript_vm_bind(vm, "BSTNode", "getRight", fun_bst_getright, 0);
    surgescript_vm_bind(vm, "BSTNode", "count", fun_bst_count, 0);
    surgescript_vm_bind(vm, "BSTNode", "find", fun_bst_find, 1);
    surgescript_vm_bind(vm, "BSTNode", "insert", fun_bst_insert, 2);
    surgescript_vm_bind(vm, "BSTNode", "remove", fun_bst_remove, 1);
}



/* --- BSTNode functions --- */

/* constructor: initialize data */
surgescript_var_t* fun_bst_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);

    ssassert(BST_KEY == surgescript_heap_malloc(heap));
    ssassert(BST_VALUE == surgescript_heap_malloc(heap));
    ssassert(BST_LEFT == surgescript_heap_malloc(heap));
    ssassert(BST_RIGHT == surgescript_heap_malloc(heap));

    surgescript_var_set_string(surgescript_heap_at(heap, BST_KEY), "[undefined]");
    surgescript_var_set_null(surgescript_heap_at(heap, BST_VALUE));
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_LEFT), null_handle);
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_RIGHT), null_handle);

    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
}

/* main state */
surgescript_var_t* fun_bst_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_set_active(object, false); /* optimization; we don't need to spend time updating this object */
    return NULL;
}

/* get the key */
surgescript_var_t* fun_bst_getkey(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, BST_KEY));
}

/* get the value */
surgescript_var_t* fun_bst_getvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, BST_VALUE));
}

/* get the left node */
surgescript_var_t* fun_bst_getleft(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, BST_LEFT));
}

/* get the right node */
surgescript_var_t* fun_bst_getright(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, BST_RIGHT));
}

/* set the value; param[0] can be of any type */
surgescript_var_t* fun_bst_setvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_var_copy(surgescript_heap_at(heap, BST_VALUE), param[0]);
    return NULL;
}

/* count the number of nodes */
surgescript_var_t* fun_bst_count(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    return surgescript_var_set_number(surgescript_var_create(), bst_count(manager, object));
}

/* finds a node having a given key, param[0] */
surgescript_var_t* fun_bst_find(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t left_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_LEFT));
    surgescript_objecthandle_t right_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_RIGHT));
    const char* key = surgescript_var_fast_get_string(surgescript_heap_at(heap, BST_KEY));
    const char* search_key = surgescript_var_fast_get_string(param[0]);
    int cmp = strcmp(search_key, key);

    if(cmp == 0) /* found it */
        return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
    else if(cmp < 0 && surgescript_objectmanager_exists(manager, left_handle)) /* maybe it's on the left */
        return fun_bst_find(surgescript_objectmanager_get(manager, left_handle), param, num_params);
    else if(cmp > 0 && surgescript_objectmanager_exists(manager, right_handle)) /* maybe it's on the right */
        return fun_bst_find(surgescript_objectmanager_get(manager, right_handle), param, num_params);
    else /* key not found */
        return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_objectmanager_null(manager));
}

/* inserts a (key, value) pair into the BST and returns the new BSTNode; key = param[0] must be a string, value = param[1] can be of any type */
surgescript_var_t* fun_bst_insert(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t left_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_LEFT));
    surgescript_objecthandle_t right_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_RIGHT));
    const char* key = surgescript_var_fast_get_string(surgescript_heap_at(heap, BST_KEY));
    const char* new_key = surgescript_var_fast_get_string(param[0]); /* thus param[0] must be a string */
    int cmp = strcmp(new_key, key);

    if(cmp == 0) {
        /* the key was already in the BST */
        surgescript_var_copy(surgescript_heap_at(heap, BST_VALUE), param[1]);
        return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
    }
    else if(cmp < 0) {
        if(!surgescript_objectmanager_exists(manager, left_handle)) {
            /* insert on the left */
            surgescript_objecthandle_t new_node = new_bst_node(object, param[0], param[1]);
            surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_LEFT), new_node);
            return surgescript_var_set_objecthandle(surgescript_var_create(), new_node);
        }
        else
            return fun_bst_insert(surgescript_objectmanager_get(manager, left_handle), param, num_params);
    }
    else {
        if(!surgescript_objectmanager_exists(manager, right_handle)) {
            /* insert on the right */
            surgescript_objecthandle_t new_node = new_bst_node(object, param[0], param[1]);
            surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_RIGHT), new_node);
            return surgescript_var_set_objecthandle(surgescript_var_create(), new_node);
        }
        else
            return fun_bst_insert(surgescript_objectmanager_get(manager, right_handle), param, num_params);
    }
}

/* removes an entry (key = param[0]) from the BST, and returns the root of the modified BST */
surgescript_var_t* fun_bst_remove(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    const char* key = surgescript_var_fast_get_string(surgescript_heap_at(heap, BST_KEY));
    return bst_remove(object, key, 0);
}





/* --- Utilities --- */

/* clones a var, transforming it into a (surgescript) string */
surgescript_var_t* var2string(const surgescript_var_t* ssvar)
{
    char* buf = surgescript_var_get_string(ssvar);
    surgescript_var_t* clone = surgescript_var_set_string(surgescript_var_create(), buf);
    ssfree(buf);
    return clone;
}

/* spawns a new BSTNode */
surgescript_objecthandle_t new_bst_node(const surgescript_object_t* parent, const surgescript_var_t* key, const surgescript_var_t* value)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(parent);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);
    surgescript_objecthandle_t new_node = surgescript_objectmanager_spawn(manager, surgescript_object_handle(parent), "BSTNode", NULL);
    surgescript_object_t* new_obj = surgescript_objectmanager_get(manager, new_node);
    surgescript_heap_t* heap = surgescript_object_heap(new_obj);

    surgescript_var_copy(surgescript_heap_at(heap, BST_KEY), key); /* key must be a string */
    surgescript_var_copy(surgescript_heap_at(heap, BST_VALUE), value); /* value can be of any type */
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_LEFT), null_handle);
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_RIGHT), null_handle);

    return new_node;
}

/* counts the number of nodes in a BST */
int bst_count(const surgescript_objectmanager_t* manager, const surgescript_object_t* object)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objecthandle_t left_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_LEFT));
    surgescript_objecthandle_t right_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_RIGHT));
    int count = 1;

    if(surgescript_objectmanager_exists(manager, left_handle))
        count += bst_count(manager, surgescript_objectmanager_get(manager, left_handle));
    if(surgescript_objectmanager_exists(manager, right_handle))
        count += bst_count(manager, surgescript_objectmanager_get(manager, right_handle));

    return count;
}

/* removes the root of the BST and returns a handle to the new root. Performs any required modifications. */
surgescript_var_t* bst_removeroot(surgescript_object_t* object)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t left_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_LEFT));

    if(!surgescript_objectmanager_exists(manager, left_handle)) {
        surgescript_objecthandle_t right_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_RIGHT));
        surgescript_object_kill(object);
        return surgescript_var_set_objecthandle(surgescript_var_create(), right_handle);
    }
    else {
        surgescript_object_t* node = object;
        surgescript_heap_t* node_heap = heap;
        surgescript_objecthandle_t child_handle = left_handle;
        surgescript_object_t* child = surgescript_objectmanager_get(manager, child_handle);
        surgescript_heap_t* child_heap = surgescript_object_heap(child);
        surgescript_objecthandle_t grand_child = surgescript_var_get_objecthandle(surgescript_heap_at(child_heap, BST_RIGHT));

        /* the right-most guy (that is to the left of the root) will be the new root */
        while(surgescript_objectmanager_exists(manager, grand_child)) {
            node = child;
            node_heap = surgescript_object_heap(node);
            child_handle = grand_child;
            child = surgescript_objectmanager_get(manager, child_handle);
            child_heap = surgescript_object_heap(child);
            grand_child = surgescript_var_get_objecthandle(surgescript_heap_at(child_heap, BST_RIGHT));
        }

        if(node != object) {
            surgescript_var_copy(surgescript_heap_at(node_heap, BST_RIGHT), surgescript_heap_at(child_heap, BST_LEFT));
            surgescript_var_copy(surgescript_heap_at(child_heap, BST_LEFT), surgescript_heap_at(heap, BST_LEFT));
            surgescript_var_copy(surgescript_heap_at(child_heap, BST_RIGHT), surgescript_heap_at(heap, BST_RIGHT));
        }
        else
            surgescript_var_copy(surgescript_heap_at(child_heap, BST_RIGHT), surgescript_heap_at(heap, BST_RIGHT));

        surgescript_object_kill(object);
        return surgescript_var_set_objecthandle(surgescript_var_create(), child_handle);
    }
}



/* removes a node from the BST; returns the root of the modified BST */
surgescript_var_t* bst_remove(surgescript_object_t* object, const char* param_key, int depth)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t left_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_LEFT));
    surgescript_objecthandle_t right_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_RIGHT));
    const char* key = surgescript_var_fast_get_string(surgescript_heap_at(heap, BST_KEY));
    int cmp = strcmp(param_key, key);

    if(cmp == 0) {
        surgescript_var_t* new_root = bst_removeroot(object);
        return new_root;
    }
    else if((cmp < 0 && surgescript_objectmanager_exists(manager, left_handle)) || (cmp > 0 && surgescript_objectmanager_exists(manager, right_handle))) {
        surgescript_objecthandle_t child_handle = (cmp < 0) ? left_handle : right_handle;
        surgescript_object_t* child = surgescript_objectmanager_get(manager, child_handle);
        surgescript_heap_t* child_heap = surgescript_object_heap(child);
        const char* child_key = surgescript_var_fast_get_string(surgescript_heap_at(child_heap, BST_KEY));
        if(0 == strcmp(param_key, child_key)) {
            surgescript_var_t* new_root = bst_removeroot(child);
            surgescript_var_copy(surgescript_heap_at(heap, (cmp < 0) ? BST_LEFT : BST_RIGHT), new_root);
            return new_root;
        }
        else {
            surgescript_var_t* new_root = bst_remove(child, param_key, depth + 1);
            if(depth == 0 && new_root == NULL) /* will only create var on 1st call */
                new_root = surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object)); /* the root hasn't changed */
            return new_root;
        }
    }
    else
        return NULL; /* key not found */
}