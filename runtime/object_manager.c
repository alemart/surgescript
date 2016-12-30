/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/object_manager.c
 * SurgeScript object manager
 */

#include "object_manager.h"
#include "object.h"
#include "program_pool.h"
#include "stack.h"
#include "../util/ssarray.h"
#include "../util/util.h"

/* types */
struct surgescript_objectmanager_t
{
    SSARRAY(surgescript_object_t*, data); /* object table */
    int count; /* how many objects are allocated at the moment */
    surgescript_programpool_t* program_pool; /* reference to the program pool */
    surgescript_stack_t* stack; /* reference to the stack */
};

/* object methods acessible by me */
extern surgescript_object_t* surgescript_object_create(const char* name, unsigned handle, struct surgescript_objectmanager_t* object_manager, struct surgescript_programpool_t* program_pool, struct surgescript_stack_t* stack, void* user_data, bool (*on_init)(surgescript_object_t*), bool (*on_release)(surgescript_object_t*)); /* creates a new blank object */
extern surgescript_object_t* surgescript_object_destroy(surgescript_object_t* object); /* destroys an object */

/* the life-cycle of the objects is handled by me */
extern void surgescript_object_init(surgescript_object_t* object);
extern void surgescript_object_release(surgescript_object_t* object);

/* -------------------------------
 * public methods
 * ------------------------------- */

/*
 * surgescript_objectmanager_create()
 * Creates a new object manager
 */
surgescript_objectmanager_t* surgescript_objectmanager_create(surgescript_programpool_t* program_pool, surgescript_stack_t* stack)
{
    surgescript_objectmanager_t* manager = ssmalloc(sizeof *manager);

    ssarray_init(manager->data);
    ssarray_push(manager->data, NULL); /* NULL is the first element */

    manager->count = 0;
    manager->program_pool = program_pool;
    manager->stack = stack;

    return manager;
}

/*
 * surgescript_objectmanager_destroy()
 * Destroys an object manager
 */
surgescript_objectmanager_t* surgescript_objectmanager_destroy(surgescript_objectmanager_t* manager)
{
    surgescript_objectmanager_handle_t handle = ssarray_length(manager->data);

    while(handle != 0)
        surgescript_objectmanager_delete(manager, --handle);

    ssarray_release(manager->data);
    return ssfree(manager);
}


/*
 * surgescript_objectmanager_spawn()
 * Spawns a new object and puts it in the internal pool
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_spawn(surgescript_objectmanager_t* manager, const char* object_name, void* user_data, bool (*on_init)(surgescript_object_t*), bool (*on_release)(surgescript_object_t*))
{
    surgescript_objectmanager_handle_t handle = ssarray_length(manager->data); /* TODO: malloc-like routine (grab unused spaces) */
    surgescript_object_t *object = surgescript_object_create(object_name, handle, manager, manager->program_pool, manager->stack, user_data, on_init, on_release);

    manager->count++;
    ssarray_push(manager->data, object);
    surgescript_object_init(object);

    return handle;
}

/*
 * surgescript_objectmanager_exists()
 * Does the specified handle points to a valid object?
 */
bool surgescript_objectmanager_exists(surgescript_objectmanager_t* manager, surgescript_objectmanager_handle_t handle)
{
    return handle < ssarray_length(manager->data) && manager->data[handle] != NULL;
}

/*
 * surgescript_objectmanager_get()
 * Gets an object from the pool (returns NULL if not found)
 */
surgescript_object_t* surgescript_objectmanager_get(surgescript_objectmanager_t* manager, surgescript_objectmanager_handle_t handle)
{
    if(handle < ssarray_length(manager->data)) { /* handle is unsigned; therefore, not lower than zero */
        if(manager->data[handle] != NULL)
            return manager->data[handle];
    }

    ssfatal("Runtime Error: null pointer exception (can't find object 0x%X)", handle);
    return NULL;
}

/*
 * surgescript_objectmanager_delete()
 * Deletes an object from the pool
 */
bool surgescript_objectmanager_delete(surgescript_objectmanager_t* manager, surgescript_objectmanager_handle_t handle)
{
    if(handle < ssarray_length(manager->data)) {
        if(manager->data[handle] != NULL) {
            surgescript_object_release(manager->data[handle]);
            manager->data[handle] = surgescript_object_destroy(manager->data[handle]);
            manager->count--;
            return true;
        }
    }

    return false;
}

/*
 * surgescript_objectmanager_get_null()
 * Returns a handle to a NULL pointer in the object manager
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_get_null(surgescript_objectmanager_t* manager)
{
    return 0;
}

/*
 * surgescript_objectmanager_root()
 * Returns a handle to the root object in the pool
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_root(surgescript_objectmanager_t* manager)
{
    if(ssarray_length(manager->data) <= 1)
        ssfatal("Runtime Error: can't find the root object");

    return 1;
}

/*
 * surgescript_objectmanager_count()
 * How many allocated objects there are?
 */
int surgescript_objectmanager_count(surgescript_objectmanager_t* manager)
{
    return manager->count;
}