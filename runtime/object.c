/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016-2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/object.c
 * SurgeScript object
 */

#include <time.h>
#include <string.h>
#include "object.h"
#include "program_pool.h"
#include "tag_system.h"
#include "object_manager.h"
#include "program.h"
#include "heap.h"
#include "stack.h"
#include "renv.h"
#include "../util/transform.h"
#include "../util/ssarray.h"
#include "../util/util.h"

typedef struct surgescript_object_exportedvar_t surgescript_object_exportedvar_t;

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

    /* inner state */
    char* state_name; /* current state name */
    bool is_active; /* can i run programs? */
    bool is_killed; /* am i scheduled to be destroyed? */
    bool is_reachable; /* is this object reachable through some other? (garbage-collection) */
    clock_t state_time; /* time since last state change */

    /* exported variables */
    SSARRAY(surgescript_object_exportedvar_t, exported_var);

    /* local transform */
    surgescript_transform_t* transform;

    /* user-data */
    void* user_data; /* custom user-data */
};

/* exported variable: pair<string,heapptr> */
struct surgescript_object_exportedvar_t
{
    char* var_name;
    surgescript_heapptr_t var_addr;
};

/* functions */
void surgescript_object_release(surgescript_object_t* object);

/* private stuff */
#define MAIN_STATE "main"
static char* state2fun(const char* state);
static void run_state(surgescript_object_t* object, const char* state_name);
static bool object_exists(surgescript_programpool_t* program_pool, const char* object_name);

/* -------------------------------
 * public methods
 * ------------------------------- */




/* object manager - related */




/*
 * surgescript_object_create()
 * Creates a new blank object
 */
surgescript_object_t* surgescript_object_create(const char* name, unsigned handle, surgescript_objectmanager_t* object_manager, surgescript_programpool_t* program_pool, surgescript_stack_t* stack, void* user_data)
{
    surgescript_object_t* obj = ssmalloc(sizeof *obj);

    obj->name = ssstrdup(name);
    obj->heap = surgescript_heap_create();
    obj->renv = surgescript_renv_create(obj, stack, obj->heap, program_pool, object_manager, NULL);

    obj->handle = handle; /* handle == parent implies I am a root */
    obj->parent = handle;
    ssarray_init(obj->child);

    obj->state_name = ssstrdup(MAIN_STATE);
    obj->is_active = true;
    obj->is_killed = false;
    obj->is_reachable = false;
    obj->state_time = clock();

    ssarray_init(obj->exported_var);
    obj->transform = NULL;
    obj->user_data = user_data;

    /* validation procedure */
    if(!object_exists(program_pool, name))
        ssfatal("Runtime Error: object \"%s\" doesn't exist", name);

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

    /* clear up any exported variables */
    for(i = 0; i < ssarray_length(obj->exported_var); i++)
        ssfree(obj->exported_var[i].var_name);
    ssarray_release(obj->exported_var);

    /* clear up the local transform, if any */
    if(obj->transform != NULL)
        surgescript_transform_destroy(obj->transform);

    /* call destructor */
    surgescript_object_release(obj);

    /* clear up some data */
    ssfree(obj->name);
    surgescript_renv_destroy(obj->renv);
    surgescript_heap_destroy(obj->heap);
    ssfree(obj->state_name);
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
 * Custom user-data (if any)
 */
void* surgescript_object_userdata(const surgescript_object_t* object)
{
    return object->user_data;
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
    else if(index >= 0)
        ssfatal("Runtime Error: can't obtain child #%d of object 0x%X (\"%s\"): object has %d %s", index, object->handle, object->name, ssarray_length(object->child), ssarray_length(object->child) != 1 ? "children" : "child");
    else
        ssfatal("Runtime Error: can't obtain child #%d of object 0x%X (\"%s\"): negative index", index, object->handle, object->name);

    return 0;
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
 * gets a handle to the 1st child named name
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
 * surgescript_object_find_child()
 * find 1st child (or grand-child, or grand-grand-child, and so on) whose name matches the parameter
 */
unsigned surgescript_object_find_child(const surgescript_object_t* object, const char* name)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);

    for(int i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        if(strcmp(name, child->name) == 0)
            return child->handle;
    }

    for(int i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        unsigned handle = surgescript_object_find_child(child, name);
        if(handle != null_handle)
            return handle;
    }

    return null_handle;
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
    ssfree(object->state_name);
    object->state_name = ssstrdup(state_name ? state_name : MAIN_STATE);
    object->state_time = clock();
}


/*
 * surgescript_object_elapsed_time()
 * elapsed time (in seconds) since last state change
 */
float surgescript_object_elapsed_time(const surgescript_object_t* object)
{
    clock_t elapsed_time = clock() - object->state_time;
    return (float)elapsed_time / CLOCKS_PER_SEC;
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


/* life-cycle */

/*
 * surgescript_object_init()
 * Initializes this object
 */
void surgescript_object_init(surgescript_object_t* object)
{
    static const char* CONSTRUCTOR_FUN = "__constructor"; /* regular constructor */
    static const char* PRE_CONSTRUCTOR_FUN = "__ssconstructor"; /* a constructor reserved for the VM */
    surgescript_programpool_t* program_pool = surgescript_renv_programpool(object->renv);

    if(surgescript_programpool_exists(program_pool, object->name, PRE_CONSTRUCTOR_FUN)) {
        surgescript_program_t* pre_constructor = surgescript_programpool_get(program_pool, object->name, PRE_CONSTRUCTOR_FUN);
        surgescript_program_run(pre_constructor, object->renv);
    }

    if(surgescript_programpool_exists(program_pool, object->name, CONSTRUCTOR_FUN)) {
        surgescript_program_t* constructor = surgescript_programpool_get(program_pool, object->name, CONSTRUCTOR_FUN);
        if(surgescript_program_arity(constructor) != 0)
            ssfatal("Runtime Error: Object \"%s\"'s %s() cannot receive parameters", object->name, CONSTRUCTOR_FUN);
        surgescript_program_run(constructor, object->renv);
    }
}

/*
 * surgescript_object_release()
 * Releases this object (program-wise)
 */
void surgescript_object_release(surgescript_object_t* object)
{
    static const char* DESTRUCTOR_FUN = "__destructor";
    surgescript_programpool_t* program_pool = surgescript_renv_programpool(object->renv);

    if(surgescript_programpool_exists(program_pool, object->name, DESTRUCTOR_FUN)) {
        surgescript_program_t* destructor = surgescript_programpool_get(program_pool, object->name, DESTRUCTOR_FUN);
        if(surgescript_program_arity(destructor) != 0)
            ssfatal("Runtime Error: Object \"%s\" %s() cannot receive parameters", object->name, DESTRUCTOR_FUN);
        surgescript_program_run(destructor, object->renv);
    }
}

/*
 * surgescript_object_update()
 * Updates this object; runs my programs and those of my children
 */
bool surgescript_object_update(surgescript_object_t* object)
{
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);

    /* check if I am destroyed */
    if(object->is_killed) {
        surgescript_objectmanager_delete(manager, object->handle); /* children gets deleted too */
        return false;
    }

    /* update myself */
    if(object->is_active)
        run_state(object, object->state_name);
    else
        return false; /* optimize; don't update my children */

    /* success! */
    return true;
}

/*
 * surgescript_object_traverse_tree()
 * Traverses the object tree, calling the callback function for each object
 * If the callback returns false, the traversal doesn't go to the children
 */
bool surgescript_object_traverse_tree(surgescript_object_t* object, bool (*callback)(surgescript_object_t*))
{
    if(!callback(object))
        return false;

    for(int i = 0; i < ssarray_length(object->child); i++) {
        surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(object->renv);
        surgescript_object_t* child = surgescript_objectmanager_get(manager, object->child[i]);
        surgescript_object_traverse_tree(child, callback);
    }

    return true;
}


/*
 * surgescript_object_call_function()
 * Call SurgeScript functions from C (you may pass NULL to return_value;
 * you may also pass NULL to param iff num_params is 0)
 */
void surgescript_object_call_function(surgescript_object_t* object, const char* fun_name, const surgescript_var_t* param[], int num_params, surgescript_var_t* return_value)
{
    int i;
    surgescript_stack_t* stack = surgescript_renv_stack(object->renv);
    surgescript_renv_t* runtime_environment;

    /* parameters are stacked left-to-right */
    surgescript_stack_push(stack, surgescript_var_set_objecthandle(surgescript_var_create(), object->handle));
    for(i = 0; i < num_params; i++)
        surgescript_stack_push(stack, surgescript_var_clone(param[i]));

    /* call the program */
    /*runtime_environment = surgescript_renv_create(object, stack, object->heap, surgescript_renv_programpool(obj->renv), surgescript_renv_objectmanager(obj->renv), NULL);*/ /* a clone of object->renv (so the original tmp's won't be messed up) -- do we need this? */
    runtime_environment = object->renv;
    surgescript_program_lowcall(runtime_environment, fun_name, num_params);
    if(return_value != NULL)
        surgescript_var_copy(return_value, *(surgescript_renv_tmp(runtime_environment) + 0)); /* the return value of the function (if any) */
    /*surgescript_renv_destroy(runtime_environment);*/

    /* pop stuff from the stack */
    surgescript_stack_popn(stack, 1 + ssmax(0, num_params));
}

/*
 * surgescript_object_call_state()
 * Runs the code of a state of the given object from C
 */
void surgescript_object_call_state(surgescript_object_t* object, const char* state_name)
{
    char* fun_name = state2fun(state_name);
    surgescript_object_call_function(object, fun_name, NULL, 0, NULL);
    ssfree(fun_name);
}


/*
 * surgescript_object_export_variable()
 * Associate a heap address to a (exported) variable name
 */
void surgescript_object_export_variable(surgescript_object_t* object, const char* var_name, surgescript_heapptr_t var_addr)
{
    if(!surgescript_object_exported_variable_exists(object, var_name)) {
        surgescript_object_exportedvar_t exported_var = { ssstrdup(var_name), var_addr };
        ssarray_push(object->exported_var, exported_var);
    }
}

/*
 * surgescript_object_exported_variable_address()
 * Returns the heap address of a given (exported) variable
 */
surgescript_heapptr_t surgescript_object_exported_variable_address(const surgescript_object_t* object, const char* var_name)
{
    for(int i = 0; i < ssarray_length(object->exported_var); i++) {
        if(0 == strcmp(object->exported_var[i].var_name, var_name))
            return object->exported_var[i].var_addr;
    }

    ssfatal("Runtime Error: can't find exported variable \"%s\" in object 0x%X (\"%s\").", var_name, object->handle, object->name);
    return 0;
}


/*
 * surgescript_object_exported_variable_name()
 * Returns the name of the i-th exported variable (i = 0, 1, 2 ...) or NULL if the index is invalid
 */
const char* surgescript_object_exported_variable_name(const surgescript_object_t* object, int index)
{
    if(index >= 0 && index < ssarray_length(object->exported_var))
        return object->exported_var[index].var_name;
    else
        return NULL;
}

/*
 * surgescript_object_exported_variable_count()
 * How many exported variables does this object have?
 */
int surgescript_object_exported_variable_count(const surgescript_object_t* object)
{
    return ssarray_length(object->exported_var);
}

/*
 * surgescript_object_exported_variable_exists()
 * Does a given var_name exists as a exported variable?
 */
bool surgescript_object_exported_variable_exists(const surgescript_object_t* object, const char* var_name)
{
    for(int i = 0; i < ssarray_length(object->exported_var); i++) {
        if(0 == strcmp(object->exported_var[i].var_name, var_name))
            return true;
    }
    
    return false;
}

/* private stuff */
char* state2fun(const char* state)
{
    /* fun = STATE2FUN + state */
    static const char prefix[] = "state:";
    char *fun_name = ssmalloc((strlen(state) + strlen(prefix) + 1) * sizeof(char));
    return strcat(strcpy(fun_name, prefix), state);
}

void run_state(surgescript_object_t* object, const char* state_name)
{
    char *fun_name = state2fun(state_name);
    surgescript_programpool_t* program_pool = surgescript_renv_programpool(object->renv);
    surgescript_program_t* program = surgescript_programpool_get(program_pool, object->name, fun_name);
    if(program)
        surgescript_program_run(program, object->renv);
    ssfree(fun_name);
}

bool object_exists(surgescript_programpool_t* program_pool, const char* object_name)
{
    return NULL != surgescript_programpool_get(program_pool, object_name, "state:" MAIN_STATE);
}