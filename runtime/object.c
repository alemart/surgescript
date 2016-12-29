/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/object.c
 * SurgeScript object
 */

#include <string.h>
#include "object.h"
#include "program_pool.h"
#include "object_manager.h"
#include "program.h"
#include "heap.h"
#include "stack.h"
#include "renv.h"
#include "../util/ssarray.h"
#include "../util/util.h"

/* object structure */
struct surgescript_object_t
{
    /* general properties */
    char* name; /* my name */
    surgescript_heap_t* heap; /* each object has its own heap */
    surgescript_objectmanager_t* pool; /* link to the object manager */
    const surgescript_renv_t* renv; /* runtime environment */

    /* object tree */
    unsigned handle; /* "this" pointer in the object manager */
    unsigned parent; /* handle to the parent in the object manager */
    SSARRAY(unsigned, child); /* handles to the children */

    /* state */
    bool is_killed; /* am i scheduled to be destroyed? */
    bool is_active; /* can i run programs? */
    char* state_name; /* current state name */
};


/* private stuff */
static const char* INITIAL_STATE = "main";
static const char* STATE2FUN_PREFIX = ":state ";
static void run_state(surgescript_object_t* object, const char* state_name);



/* -------------------------------
 * public methods
 * ------------------------------- */




/* object manager - related */




/*
 * surgescript_object_create()
 * Creates a new blank object
 */
surgescript_object_t* surgescript_object_create(const char* name, surgescript_objectmanager_t* object_manager, surgescript_programpool_t* program_pool, surgescript_stack_t* stack)
{
    surgescript_object_t* obj = ssmalloc(sizeof *obj);
    unsigned handle = surgescript_objectmanager_put(object_manager, obj);

    obj->name = surgescript_util_strdup(name);
    obj->handle = handle;
    obj->parent = handle;
    ssarray_init(obj->child);

    obj->pool = object_manager;
    obj->heap = surgescript_heap_create();
    obj->renv = surgescript_renv_create(obj, stack, obj->heap, program_pool, object_manager);

    obj->is_killed = false;
    obj->is_active = true;
    obj->state_name = surgescript_util_strdup(INITIAL_STATE);

    return obj;
}

/*
 * surgescript_object_destroy()
 * Destroys an existing object
 */
surgescript_object_t* surgescript_object_destroy(surgescript_object_t* obj)
{
    /* clear up the children */
    if(obj->parent != obj->handle) {
        /* i am not root */
        surgescript_object_t* parent = surgescript_objectmanager_get(obj->pool, obj->parent);
        surgescript_object_remove_child(parent, obj->handle);
        for(int i = 0; i < ssarray_length(obj->child); i++) {
            surgescript_object_t* child = surgescript_objectmanager_get(obj->pool, obj->child[i]);
            child->parent = obj->parent; /* modify the parent of the children */
            surgescript_object_add_child(parent, child->handle); /* modify the children of the parent */
        }
    }
    else {
        /* i am a root */
        sslog("Destroying the root object...");
        for(int i = 0; i < ssarray_length(obj->child); i++) {
            surgescript_object_t* child = surgescript_objectmanager_get(obj->pool, obj->child[i]);
            child->parent = child->handle; /* the child is a root now */
            surgescript_objectmanager_delete(obj->pool, child->handle); /* clear up everyone */
        }
        sslog("Done!");
    }
    ssarray_release(obj->child);

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
 * surgescript_object_child()
 * Gets the handle to the index-th child in the object manager
 */
unsigned surgescript_object_child(const surgescript_object_t* object, int index)
{
    if(index >= 0 && index < ssarray_length(object->child))
        return object->child[index];
    else if(index >= 0)
        ssfatal("Can't obtain child #%d of object 0x%X (\"%s\"): object has %d %s", index, object->handle, object->name, ssarray_length(object->child), ssarray_length(object->child) != 1 ? "children" : "child");
    else
        ssfatal("Can't obtain child #%d of object 0x%X (\"%s\"): negative index", index, object->handle, object->name);

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
 * surgescript_object_find_child()
 * find 1st child whose name matches the parameter
 */
unsigned surgescript_object_find_child(const surgescript_object_t* object, const char* name)
{
    for(int i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(object->pool, object->child[i]);
        if(strcmp(object->name, child->name) == 0)
            return child->handle;
    }

    ssfatal("Object 0x%X (\"%s\") has no child named \"%s\"", object->handle, object->name, name);
    return 0;
}

/*
 * surgescript_object_add_child()
 * Adds a child to this object
 */
void surgescript_object_add_child(surgescript_object_t* object, unsigned child_handle)
{
    surgescript_object_t* child;

    /* check if it doesn't exist already */
    for(int i = 0; i < ssarray_length(object->child); i++) {
        if(object->child[i] == child_handle)
            return;
    }

    /* check if the child does not belong to someone else */
    child = surgescript_objectmanager_get(object->pool, child_handle);
    if(child->parent != child->handle)
        ssfatal("Can't add child 0x%X (\"%s\") to object 0x%X (\"%s\"): child already registered", child->handle, child->name, object->handle, object->name);

    /* add it */
    ssarray_push(object->child, child->handle);
    child->parent = object->handle;
}

/*
 * surgescript_object_remove_child()
 * Removes a child having this handle from this object
 */
bool surgescript_object_remove_child(surgescript_object_t* object, unsigned child_handle)
{
    /* find the child */
    for(int i = 0; i < ssarray_length(object->child); i++) {
        if(object->child[i] == child_handle) {
            surgescript_object_t* child = surgescript_objectmanager_get(object->pool, child_handle);
            ssarray_remove(object->child, i);

            if(object->handle == object->parent) {
                /* i am root */
                child->parent = child->handle;
                sslog("Removing child of a root...");
                surgescript_objectmanager_delete(object->pool, child_handle); /* clear things up */
            }
            else {
                /* i am not root */
                child->parent = object->parent;
            }

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
    object->state_name = surgescript_util_strdup(state_name ? state_name : INITIAL_STATE);
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




/* life-cycle */

/*
 * surgescript_object_init()
 * Initializes this object
 */
void surgescript_object_init(surgescript_object_t* object)
{
    run_state(object, INITIAL_STATE);
}

/*
 * surgescript_object_update()
 * Updates this object; runs my programs
 */
void surgescript_object_update(surgescript_object_t* object)
{
    /* update myself */
    if(object->is_active)
        run_state(object, object->state_name);

    /* update my children */
    for(int i = 0; i < ssarray_length(object->child); i++) {
        surgescript_object_t* child = surgescript_objectmanager_get(object->pool, object->child[i]);
        surgescript_object_update(child);
    }

    /* check if I am destroyed */
    if(object->is_killed)
        surgescript_objectmanager_delete(object->pool, object->handle);
}

/*
 * surgescript_object_release()
 * Releases this object (program-wise)
 */
void surgescript_object_release(surgescript_object_t* object)
{
    ;
}






/* private stuff */

void run_state(surgescript_object_t* object, const char* state_name)
{
    /* fun_name = STATE2FUN_PREFIX + state_name */
    char *fun_name = ssmalloc((strlen(state_name) + strlen(STATE2FUN_PREFIX) + 1) * sizeof(char));
    surgescript_programpool_t* program_pool = surgescript_renv_programpool(object->renv);
    surgescript_program_t* program = surgescript_programpool_get(program_pool, object->name, strcat(strcpy(fun_name, STATE2FUN_PREFIX), state_name));

    surgescript_program_run(program, object->renv);

    ssfree(fun_name);
}