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
 * util/object.c
 * SurgeScript object
 */

#include <string.h>
#include "object.h"
#include "program_pool.h"
#include "tag_system.h"
#include "object_manager.h"
#include "program.h"
#include "heap.h"
#include "stack.h"
#include "renv.h"
#include "vm_time.h"
#include "../util/transform.h"
#include "../util/ssarray.h"
#include "../util/util.h"

/* object structure */
struct surgescript_object_t
{
    /* general properties */
    char* name; /* my name */
    surgescript_heap_t* heap; /* each object has its own heap */
    surgescript_renv_t* renv; /* runtime environment */

    /* object tree */
    unsigned handle; /* "this" pointer in the object manager */
    unsigned parent; /* handle to the parent in the object manager */
    SSARRAY(unsigned, child); /* handles to the children */
    int depth; /* object depth */

    /* inner state */
    surgescript_program_t* current_state; /* current state */
    char* state_name; /* current state name */
    bool is_active; /* can i run programs? */
    bool is_killed; /* am i scheduled to be destroyed? */
    bool is_reachable; /* is this object reachable through some other? (garbage-collection) */

    /* internal timer */
    const surgescript_vmtime_t* vmtime; /* VM time */
    uint64_t last_state_change; /* moment of the last state change */
    uint64_t time_spent; /* how much time did this object consume since the last state change */

    /* local transform */
    surgescript_transform_t* transform;

    /* user-data */
    void* user_data; /* custom user-data */
};

/* functions */
void surgescript_object_release(surgescript_object_t* object);

/* private stuff */
#define MAIN_STATE "main"
#define STATE2FUN_BUFFER_SIZE ((SS_NAMEMAX+1)+6) /* prefix a string with "state:" */
static char* state2fun(const char* state, char* buffer, size_t size);
static uint64_t run_current_state(const surgescript_object_t* object);
static surgescript_program_t* get_state_program(const surgescript_object_t* object, const char* state_name);
static bool object_exists(surgescript_programpool_t* program_pool, const char* object_name);
static bool simple_traversal(surgescript_object_t* object, void* data);

/* -------------------------------
 * public methods
 * ------------------------------- */




/* object manager - related */




/*
 * surgescript_object_create()
 * Creates a new blank object
 */
surgescript_object_t* surgescript_object_create(const char* name, unsigned handle, surgescript_objectmanager_t* object_manager, surgescript_programpool_t* program_pool, surgescript_stack_t* stack, const surgescript_vmtime_t* vmtime, void* user_data)
{
    surgescript_object_t* obj = ssmalloc(sizeof *obj);

    if(!object_exists(program_pool, name))
        ssfatal("Runtime Error: can't spawn object \"%s\" - it doesn't exist!", name);

    obj->name = ssstrdup(name);
    obj->heap = surgescript_heap_create();
    obj->renv = surgescript_renv_create(obj, stack, obj->heap, program_pool, object_manager, NULL);

    obj->handle = handle; /* handle == parent implies I am a root */
    obj->parent = handle;
    ssarray_init(obj->child);
    obj->depth = 0;

    obj->state_name = ssstrdup(MAIN_STATE);
    obj->current_state = get_state_program(obj, obj->state_name);
    obj->is_active = true;
    obj->is_killed = false;
    obj->is_reachable = false;

    obj->vmtime = vmtime;
    obj->last_state_change = surgescript_vmtime_time(obj->vmtime);
    obj->time_spent = 0;

    obj->transform = NULL;
    obj->user_data = user_data;

    return obj;
}

/*
 * surgescript_object_destroy()
 * Destroys an existing object
 */
surgescript_object_t* surgescript_object_destroy(surgescript_object_t* obj)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(obj->renv);
    int i;

    /* call destructor */
    surgescript_object_release(obj);

    /* am I root? */
    if(obj->parent != obj->handle) {
        surgescript_object_t* parent = surgescript_objectmanager_get(manager, obj->parent);
        surgescript_object_remove_child(parent, obj->handle); /* no? well, I am a root now! */
    }

    /* clear up the children */
    for(i = 0; i < ssarray_length(obj->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, obj->child[i]);
        child->parent = child->handle; /* the child is a root now */
        surgescript_objectmanager_delete(manager, child->handle); /* clear up everyone! */
    }
    ssarray_release(obj->child);

    /* clear up the local transform, if any */
    if(obj->transform != NULL)
        surgescript_transform_destroy(obj->transform);

    /* clear up some data */
    surgescript_renv_destroy(obj->renv);
    surgescript_heap_destroy(obj->heap);
    ssfree(obj->state_name);
    ssfree(obj->name);
    ssfree(obj);

    /* done! */
    return NULL;
}




/* general properties */




/*
 * surgescript_object_name()
 * What's my name?
 */
const char* surgescript_object_name(const surgescript_object_t* object)
{
    return object->name;
}

/*
 * surgescript_object_heap()
 * Each object has its own heap. This gets mine.
 */
surgescript_heap_t* surgescript_object_heap(const surgescript_object_t* object)
{
    return object->heap;
}

/*
 * surgescript_object_manager()
 * Returns a pointer to the object manager
 */
surgescript_objectmanager_t* surgescript_object_manager(const surgescript_object_t* object)
{
    return surgescript_renv_objectmanager(object->renv);
}

/*
 * surgescript_object_userdata()
 * Custom user data (if any)
 */
void* surgescript_object_userdata(const surgescript_object_t* object)
{
    return object->user_data;
}

/*
 * surgescript_object_set_userdata()
 * Set custom user data
 */
void surgescript_object_set_userdata(surgescript_object_t* object, void* data)
{
    object->user_data = data;
}

/*
 * surgescript_object_has_tag()
 * Is this object tagged tag_name?
 */
bool surgescript_object_has_tag(const surgescript_object_t* object, const char* tag_name)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
    return surgescript_tagsystem_has_tag(surgescript_objectmanager_tagsystem(manager), object->name, tag_name);
}

/*
 * surgescript_object_has_function()
 * Checks if the object has the specified function
 */
bool surgescript_object_has_function(const surgescript_object_t* object, const char* fun_name)
{
    surgescript_programpool_t* pool = surgescript_renv_programpool(object->renv);
    return surgescript_programpool_exists(pool, object->name, fun_name);
}





/* object tree */


/*
 * surgescript_object_handle()
 * What's my handle in the object manager?
 */
unsigned surgescript_object_handle(const surgescript_object_t* object)
{
    return object->handle;
}

/*
 * surgescript_object_parent()
 * A handle to my parent in the object manager
 */
unsigned surgescript_object_parent(const surgescript_object_t* object)
{
    return object->parent;
}

/*
 * surgescript_object_nth_child()
 * Gets the handle to the index-th child in the object manager
 */
unsigned surgescript_object_nth_child(const surgescript_object_t* object, int index)
{
    if(index >= 0 && index < ssarray_length(object->child))
        return object->child[index];
    else
        return surgescript_objectmanager_null(surgescript_renv_objectmanager(object->renv));
}

/*
 * surgescript_object_child_count()
 * How many children do I have?
 */
int surgescript_object_child_count(const surgescript_object_t* object)
{
    return ssarray_length(object->child);
}

/*
 * surgescript_object_child()
 * Gets a handle to a child named name
 */
unsigned surgescript_object_child(const surgescript_object_t* object, const char* name)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);

    for(int i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        if(strcmp(name, child->name) == 0)
            return child->handle;
    }

    return surgescript_objectmanager_null(manager);
}

/*
 * surgescript_object_children()
 * Gets the handles to all the direct children named name.
 * Returns the number of direct children named name.
 */
int surgescript_object_children(const surgescript_object_t* object, const char* name, void* data, void (*callback)(unsigned,void*))
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
    int count = 0;

    for(int i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        if(strcmp(name, child->name) == 0) {
            ++count;
            callback(child->handle, data);
        }
    }

    return count;
}

/*
 * surgescript_object_tagged_child()
 * Gets a handle to a child having the specified tag
 */
unsigned surgescript_object_tagged_child(const surgescript_object_t* object, const char* tag_name)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);

    for(int i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        if(surgescript_object_has_tag(child, tag_name))
            return child->handle;
    }

    return surgescript_objectmanager_null(manager);
}

/*
 * surgescript_object_tagged_children()
 * Gets the handles to all the direct children tagged tag_name.
 * Returns the number of all such direct children.
 */
int surgescript_object_tagged_children(const surgescript_object_t* object, const char* tag_name, void* data, void (*callback)(unsigned,void*))
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
    int count = 0;

    for(int i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        if(surgescript_object_has_tag(child, tag_name)) {
            ++count;
            callback(child->handle, data);
        }
    }

    return count;
}

/*
 * surgescript_object_find_descendant()
 * Find a descendant whose name matches the name parameter.
 * This might be slow, so it's recommended to cache the objects.
 */
unsigned surgescript_object_find_descendant(const surgescript_object_t* object, const char* name)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);
    int i;

    for(i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        if(strcmp(name, child->name) == 0)
            return child->handle;
    }

    for(i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        unsigned handle = surgescript_object_find_descendant(child, name);
        if(handle != null_handle)
            return handle;
    }

    return null_handle;
}

/*
 * surgescript_object_find_descendants()
 * Finds all descendants named name, calling callback for each one.
 * Returns the number of matching descendants.
 * This might be slow, so it's recommended to cache the objects.
 */
int surgescript_object_find_descendants(const surgescript_object_t* object, const char* name, void* data, void (*callback)(unsigned,void*))
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);
    int i, count = 0;

    for(i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        if(strcmp(name, child->name) == 0) {
            ++count;
            callback(child->handle, data);
        }
    }

    for(i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        count += surgescript_object_find_descendants(child, name, data, callback);
    }

    return count;
}

/*
 * surgescript_object_find_tagged_descendant()
 * Find a descendant tagged tag_name.
 * This might be slow, so it's recommended to cache the objects.
 */
unsigned surgescript_object_find_tagged_descendant(const surgescript_object_t* object, const char* tag_name)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);
    int i;

    for(i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        if(surgescript_object_has_tag(child, tag_name))
            return child->handle;
    }

    for(i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        unsigned handle = surgescript_object_find_tagged_descendant(child, tag_name);
        if(handle != null_handle)
            return handle;
    }

    return null_handle;
}

/*
 * surgescript_object_find_tagged_descendants()
 * Finds all descendants tagged tag_name, calling callback for each one.
 * Returns the number of matching descendants.
 * This might be slow, so it's recommended to cache the objects.
 */
int surgescript_object_find_tagged_descendants(const surgescript_object_t* object, const char* tag_name, void* data, void (*callback)(unsigned,void*))
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);
    int i, count = 0;

    for(i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        if(surgescript_object_has_tag(child, tag_name)) {
            ++count;
            callback(child->handle, data);
        }
    }

    for(i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        count += surgescript_object_find_tagged_descendants(child, tag_name, data, callback);
    }

    return count;
}

/*
 * surgescript_object_add_child()
 * Adds a child to this object (adds the link)
 */
void surgescript_object_add_child(surgescript_object_t* object, unsigned child_handle)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
    surgescript_object_t* child;

    /* check if it doesn't exist already */
    for(int i = 0; i < ssarray_length(object->child); i++) {
        if(object->child[i] == child_handle)
            return;
    }

    /* check if the child isn't myself */
    if(object->handle == child_handle) {
        ssfatal("Runtime Error: object 0x%X (\"%s\") can't be a child of itself.", object->handle, object->name);
        return;
    }

    /* check if the child belongs to someone else */
    child = surgescript_objectmanager_get(manager, child_handle);
    if(child->parent != child->handle) {
        ssfatal("Runtime Error: can't add child 0x%X (\"%s\") to object 0x%X (\"%s\") - child already registered", child->handle, child->name, object->handle, object->name);
        return;
    }

    /* add it */
    ssarray_push(object->child, child->handle);
    child->parent = object->handle;
    child->depth = 1 + object->depth;
}

/*
 * surgescript_object_remove_child()
 * Removes a child having this handle from this object (removes the link only)
 */
bool surgescript_object_remove_child(surgescript_object_t* object, unsigned child_handle)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);

    /* find the child */
    for(int i = 0; i < ssarray_length(object->child); i++) {
        if(object->child[i] == child_handle) {
            surgescript_object_t* child = surgescript_objectmanager_get(manager, child_handle);
            ssarray_remove(object->child, i);
            child->parent = child->handle; /* the child is now a root */
            return true;
        }
    }

    /* child not found */
    sslog("Can't remove child 0x%X of object 0x%X (\"%s\"): child not found", child_handle, object->handle, object->name);
    return false;
}

/*
 * surgescript_object_depth()
 * The depth of the object in the object tree (the root has depth zero)
 */
int surgescript_object_depth(const surgescript_object_t* object)
{
    return object->depth;
}

/*
 * surgescript_object_is_ascendant()
 * Checks if parameter ascendant is an ascendant of the given object
 */
bool surgescript_object_is_ascendant(const surgescript_object_t* object, const surgescript_object_t* ascendant)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
    surgescript_objecthandle_t target_handle = ascendant->handle;

    if(object->handle == ascendant->handle)
        return false;

    while(object->handle != target_handle && object->handle != object->parent)
        object = surgescript_objectmanager_get(manager, object->parent);

    return object->handle == target_handle;
}


/* life operations */



/*
 * surgescript_object_is_active()
 * Am i active? an object runs its programs iff it's active
 */
bool surgescript_object_is_active(const surgescript_object_t* object)
{
    return object->is_active;
}

/*
 * surgescript_object_set_active()
 * sets whether i am active or not; default is true
 */
void surgescript_object_set_active(surgescript_object_t* object, bool active)
{
    object->is_active = active;
}

/*
 * surgescript_object_state()
 * each object is a state machine. in which state am i in?
 */
const char* surgescript_object_state(const surgescript_object_t *object)
{
    return object->state_name;
}

/*
 * surgescript_object_set_state()
 * sets a state; default is "main"
 */
void surgescript_object_set_state(surgescript_object_t* object, const char* state_name)
{
    if(strcmp(object->state_name, state_name) != 0) {
        ssfree(object->state_name);
        object->state_name = ssstrdup(state_name ? state_name : MAIN_STATE);
        object->current_state = get_state_program(object, object->state_name);
        object->last_state_change = surgescript_vmtime_time(object->vmtime);
        object->time_spent = 0;
    }
}

/*
 * surgescript_object_elapsed_time()
 * elapsed time (in seconds) since last state change
 */
double surgescript_object_elapsed_time(const surgescript_object_t* object)
{
    return (surgescript_vmtime_time(object->vmtime) - object->last_state_change) * 0.001;
}

/*
 * surgescript_object_is_killed()
 * has this object been killed?
 */
bool surgescript_object_is_killed(const surgescript_object_t* object)
{
    return object->is_killed;
}

/*
 * surgescript_object_kill()
 * will destroy the object as soon as the opportunity arises
 */
void surgescript_object_kill(surgescript_object_t* object)
{
    object->is_killed = true;
}

/*
 * surgescript_object_is_reachable()
 * Is this object reachable through some other? garbage-collector stuff
 */
bool surgescript_object_is_reachable(const surgescript_object_t* object)
{
    return object->is_reachable;
}

/*
 * surgescript_object_set_reachable()
 * Sets whether this object is reachable through some other or not
 */
void surgescript_object_set_reachable(surgescript_object_t* object, bool reachable)
{
    object->is_reachable = reachable;
}


/* misc */

/*
 * surgescript_object_peek_transform()
 * Reads the local transform of this object, storing the result in *transform
 */
void surgescript_object_peek_transform(const surgescript_object_t* object, surgescript_transform_t* transform)
{
    if(object->transform == NULL)
        surgescript_transform_reset(transform);
    else
        surgescript_transform_copy(transform, object->transform);
}

/*
 * surgescript_object_poke_transform()
 * Sets the local transform of this object
 */
void surgescript_object_poke_transform(surgescript_object_t* object, const surgescript_transform_t* transform)
{
    if(object->transform == NULL)
        object->transform = surgescript_transform_create();

    surgescript_transform_copy(object->transform, transform);
}

/*
 * surgescript_object_transform()
 * Returns the inner pointer to the local transform. If there is no local
 * transform allocated, this function will allocate one.
 * Usage of surgescript_object_peek_transform() is preferred over this, as
 * it saves space. Only use this function if you are going to modify the
 * transform and want to optimize for speed.
 */
surgescript_transform_t* surgescript_object_transform(surgescript_object_t* object)
{
    if(object->transform == NULL)
        object->transform = surgescript_transform_create();

    return object->transform;
}

/*
 * surgescript_object_transform_changed()
 * Returns true if the transform of this object has ever been changed
 * If this returns false, its transform is the identity transform
 */
bool surgescript_object_transform_changed(const surgescript_object_t* object)
{
    return object->transform != NULL;
}


/* life-cycle */

/*
 * surgescript_object_init()
 * Initializes this object
 */
void surgescript_object_init(surgescript_object_t* object)
{
    static const char* CONSTRUCTOR_FUN = "constructor"; /* regular constructor */
    static const char* PRE_CONSTRUCTOR_FUN = "__ssconstructor"; /* a constructor reserved for the VM */
    surgescript_stack_t* stack = surgescript_renv_stack(object->renv);
    surgescript_programpool_t* program_pool = surgescript_renv_programpool(object->renv);
    surgescript_stack_push(stack, surgescript_var_set_objecthandle(surgescript_var_create(), object->handle));

    if(surgescript_programpool_exists(program_pool, object->name, PRE_CONSTRUCTOR_FUN)) {
        surgescript_program_t* pre_constructor = surgescript_programpool_get(program_pool, object->name, PRE_CONSTRUCTOR_FUN);
        surgescript_program_call(pre_constructor, object->renv, 0);
    }

    if(surgescript_programpool_exists(program_pool, object->name, CONSTRUCTOR_FUN)) {
        surgescript_program_t* constructor = surgescript_programpool_get(program_pool, object->name, CONSTRUCTOR_FUN);
        if(surgescript_program_arity(constructor) != 0)
            ssfatal("Runtime Error: Object \"%s\"'s %s() cannot receive parameters", object->name, CONSTRUCTOR_FUN);
        surgescript_program_call(constructor, object->renv, 0);
    }

    surgescript_stack_pop(stack);
}

/*
 * surgescript_object_release()
 * Releases this object (program-wise)
 */
void surgescript_object_release(surgescript_object_t* object)
{
    static const char* DESTRUCTOR_FUN = "destructor";
    surgescript_programpool_t* program_pool = surgescript_renv_programpool(object->renv);

    if(surgescript_programpool_exists(program_pool, object->name, DESTRUCTOR_FUN)) {
        surgescript_stack_t* stack = surgescript_renv_stack(object->renv);
        surgescript_program_t* destructor = surgescript_programpool_get(program_pool, object->name, DESTRUCTOR_FUN);
        
        if(surgescript_program_arity(destructor) != 0)
            ssfatal("Runtime Error: Object \"%s\"'s %s() cannot receive parameters", object->name, DESTRUCTOR_FUN);

        surgescript_stack_push(stack, surgescript_var_set_objecthandle(surgescript_var_create(), object->handle));
        surgescript_program_call(destructor, object->renv, 0);
        surgescript_stack_pop(stack);
    }
}

/*
 * surgescript_object_update()
 * Updates this object; runs the current state and returns true if my children should be updated too
 */
bool surgescript_object_update(surgescript_object_t* object)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);

    /* check if I am destroyed */
    if(object->is_killed) {
        surgescript_objectmanager_delete(manager, object->handle); /* children are deleted too */
        return false;
    }

    /* update myself */
    if(object->is_active) {
        object->time_spent += run_current_state(object);
        return object->is_active; /* will generally be true, but not necessarily */
    }

    /* optimize; don't update my children */
    return false;
}

/*
 * surgescript_object_traverse_tree()
 * Traverses the object tree, calling the callback function for each object
 * If the callback returns false, the traversal doesn't visit the children
 */
bool surgescript_object_traverse_tree(surgescript_object_t* object, bool (*callback)(surgescript_object_t*))
{
    return surgescript_object_traverse_tree_ex(object, callback, simple_traversal);
}

/*
 * surgescript_object_traverse_tree_ex()
 * Traverses the object tree, calling the callback function for each object
 * If the callback returns false, the traversal doesn't visit the children
 * Accepts an extra data parameter
 */
bool surgescript_object_traverse_tree_ex(surgescript_object_t* object, void* data, bool (*callback)(surgescript_object_t*,void*))
{
    if(callback(object, data)) {
        surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
        for(int i = 0; i < ssarray_length(object->child); i++) {
            surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
            surgescript_object_traverse_tree_ex(child, data, callback);
        }
        return true;
    }

    return false;
}


/*
 * surgescript_object_call_function()
 * Call SurgeScript functions from C (you may pass NULL to return_value;
 * you may also pass NULL to param iff num_params is 0)
 */
void surgescript_object_call_function(surgescript_object_t* object, const char* fun_name, const surgescript_var_t* param[], int num_params, surgescript_var_t* return_value)
{
    surgescript_program_t* program = surgescript_programpool_get(surgescript_renv_programpool(object->renv), object->name, fun_name);
    surgescript_stack_t* stack = surgescript_renv_stack(object->renv);
    int i;

    /* sanity check */
    if(num_params < 0)
        num_params = 0;

    /* parameters are stacked left-to-right */
    surgescript_stack_push(stack, surgescript_var_set_objecthandle(surgescript_var_create(), object->handle));
    for(i = 0; i < num_params; i++)
        surgescript_stack_push(stack, surgescript_var_clone(param[i]));

    /* call the program */
    if(program != NULL) {
        surgescript_program_call(program, object->renv, num_params);
        if(return_value != NULL)
            surgescript_var_copy(return_value, *(surgescript_renv_tmp(object->renv) + 0)); /* the return value of the function (if any) */
    }
    else
        ssfatal("Runtime Error: function %s.%s/%d doesn't exist.", object->name, fun_name, num_params);

    /* pop stuff from the stack */
    surgescript_stack_popn(stack, 1 + num_params);
}

/*
 * surgescript_object_call_state()
 * Runs the code of a state of the given object from C
 */
void surgescript_object_call_state(surgescript_object_t* object, const char* state_name)
{
    char buffer[STATE2FUN_BUFFER_SIZE];
    char* fun_name = state2fun(state_name, buffer, sizeof(buffer));
    surgescript_object_call_function(object, fun_name, NULL, 0, NULL);
}


/*
 * surgescript_object_timespent()
 * Average time consumption in the current state (in seconds)
 */
double surgescript_object_timespent(const surgescript_object_t* object)
{
    uint64_t now = surgescript_util_gettickcount();
    uint64_t dt = now > object->last_state_change ? now - object->last_state_change : 1;
    return ((double)(object->time_spent) * 0.001) / ((double)dt);
}

/*
 * surgescript_object_memspent()
 * Memory consumption at the last frame (in bytes)
 */
size_t surgescript_object_memspent(const surgescript_object_t* object)
{
    return surgescript_heap_memspent(object->heap);
}

/* private stuff */
char* state2fun(const char* state, char* buffer, size_t size)
{
    /* return (buffer = prefix + state); */
    const char prefix[] = "state:";
    const size_t prefix_size = sizeof(prefix) - 1; /* 6, assumed to be less than size */
    size_t suffix_size = strlen(state);

    memcpy(buffer, prefix, prefix_size);
    memcpy(buffer + prefix_size, state, size - prefix_size);

    if(prefix_size + suffix_size < size)
        buffer[prefix_size + suffix_size] = '\0';
    else
        buffer[size - 1] = '\0';

    return buffer;
}

uint64_t run_current_state(const surgescript_object_t* object)
{
    uint64_t start = surgescript_util_gettickcount(), end;
    surgescript_stack_t* stack = surgescript_renv_stack(object->renv);
    surgescript_stack_push(stack, surgescript_var_set_objecthandle(surgescript_var_create(), object->handle));
    surgescript_program_call(object->current_state, object->renv, 0);
    surgescript_stack_pop(stack);
    end = surgescript_util_gettickcount();
    return end > start ? end - start : 0;
}

surgescript_program_t* get_state_program(const surgescript_object_t* object, const char* state_name)
{
    char buffer[STATE2FUN_BUFFER_SIZE];
    char* fun_name = state2fun(state_name, buffer, sizeof(buffer));
    surgescript_programpool_t* program_pool = surgescript_renv_programpool(object->renv);
    surgescript_program_t* program = surgescript_programpool_get(program_pool, object->name, fun_name);

    if(program == NULL)
        ssfatal("Runtime Error: state \"%s\" of object \"%s\" doesn't exist.", state_name, object->name);

    return program;
}

bool object_exists(surgescript_programpool_t* program_pool, const char* object_name)
{
    return NULL != surgescript_programpool_get(program_pool, object_name, "state:" MAIN_STATE);
}

bool simple_traversal(surgescript_object_t* object, void* callback)
{
    return ((bool (*)(surgescript_object_t*))callback)(object);
}
