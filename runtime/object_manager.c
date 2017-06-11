/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016-2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/object_manager.c
 * SurgeScript object manager
 */

#include <string.h>
#include <stdint.h>
#include "object_manager.h"
#include "object.h"
#include "program_pool.h"
#include "stack.h"
#include "heap.h"
#include "variable.h"
#include "../util/ssarray.h"
#include "../util/uthash.h"
#include "../util/util.h"

typedef struct surgescript_tagtable_t surgescript_tagtable_t;
typedef struct surgescript_inversetagtable_t surgescript_inversetagtable_t;

/* types */
struct surgescript_objectmanager_t
{
    SSARRAY(surgescript_object_t*, data); /* object table */
    int count; /* how many objects are allocated at the moment */
    surgescript_objectmanager_handle_t handle_ptr; /* memory allocation */
    surgescript_objectmanager_handle_t app_handle; /* user's application */
    surgescript_programpool_t* program_pool; /* reference to the program pool */
    surgescript_stack_t* stack; /* reference to the stack */
    surgescript_tagtable_t* tag_table; /* tag table: object -> tags */
    surgescript_inversetagtable_t* inverse_tag_table; /* inverse tag table: tag -> objects */
    SSARRAY(surgescript_objectmanager_handle_t, objects_to_be_scanned); /* garbage collection */
    int first_object_to_be_scanned; /* an index of objects_to_be_scanned */
    int reachables_count; /* garbage-collector stuff */
};

/* fixed objects */
static const surgescript_objectmanager_handle_t NULL_HANDLE = 0; /* must always be zero */
static const surgescript_objectmanager_handle_t ROOT_HANDLE = 1;

/* names of important objects */
#define APPLICATION_OBJECT "Application"
static const char* ROOT_OBJECT = "System";
static const char* SYSTEM_OBJECTS[] = {
    "String", "Number", "Boolean", "Console",
    APPLICATION_OBJECT, NULL
}; /* this must be a NULL-terminated array, and APPLICATION_OBJECT should be the last element (spawning order) */

/* tag table: an object may hold an arbitrary number of tags */
typedef uint32_t surgescript_tag_t;
struct surgescript_tagtable_t
{
    char* object_name; /* key */
    SSARRAY(surgescript_tag_t, tag); /* values */
    UT_hash_handle hh;
};
struct surgescript_inversetagtable_t
{
    surgescript_tag_t tag; /* key */
    SSARRAY(char*, object_name); /* values */
    UT_hash_handle hh;
};

/* object methods acessible by me */
extern surgescript_object_t* surgescript_object_create(const char* name, unsigned handle, struct surgescript_objectmanager_t* object_manager, struct surgescript_programpool_t* program_pool, struct surgescript_stack_t* stack, void* user_data); /* creates a new blank object */
extern surgescript_object_t* surgescript_object_destroy(surgescript_object_t* object); /* destroys an object */

/* the life-cycle of the objects is handled by me */
extern void surgescript_object_init(surgescript_object_t* object); /* initializes the object (calls constructor, and so on) */
extern void surgescript_object_release(surgescript_object_t* object); /* releases the object (calls destructor, and so on) */

/* garbage collection is handled by me also */
extern bool surgescript_object_is_reachable(const surgescript_object_t* object); /* is this object reachable through some other? */
extern void surgescript_object_set_reachable(surgescript_object_t* object, bool reachable); /* sets whether this object is reachable or not */

/* garbage collector: private stuff */
static bool mark_as_reachable(unsigned handle, void* mgr);
static bool sweep_unreachables(surgescript_object_t* object);
static const int MIN_OBJECTS_FOR_DISPOSAL = 1; /* we need at least this amount to delete unreachable objects from memory */

/* other */
#define is_power_of_two(x)                !((x) & ((x) - 1)) /* this assumes x > 0 */
static surgescript_objectmanager_handle_t new_handle(surgescript_objectmanager_t* mgr);

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
    ssarray_push(manager->data, NULL); /* NULL is *always* the first element */

    manager->count = 0;
    manager->program_pool = program_pool;
    manager->stack = stack;
    manager->handle_ptr = ROOT_HANDLE;
    manager->app_handle = NULL_HANDLE;

    ssarray_init(manager->objects_to_be_scanned);
    manager->first_object_to_be_scanned = 0;
    manager->reachables_count = 0;

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
    ssarray_release(manager->objects_to_be_scanned);
    return ssfree(manager);
}


/*
 * surgescript_objectmanager_spawn()
 * Spawns a new object and puts it in the internal pool (you can't spawn the root using this)
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_spawn(surgescript_objectmanager_t* manager, surgescript_objectmanager_handle_t parent, const char* object_name, void* user_data)
{
    surgescript_objectmanager_handle_t handle = new_handle(manager);
    surgescript_object_t *parent_object = surgescript_objectmanager_get(manager, parent);
    surgescript_object_t *object = surgescript_object_create(object_name, handle, manager, manager->program_pool, manager->stack, user_data);

    /* store the object */
    if(handle >= ssarray_length(manager->data) && handle > ROOT_HANDLE) {
        /* new slot */
        ssarray_push(manager->data, object);
        if(is_power_of_two(handle))
            manager->handle_ptr = ssmax(2, manager->handle_ptr / 2); /* handle_ptr must never be zero */
    }
    else if(handle > ROOT_HANDLE) {
        /* reuse unused slot */
        manager->data[handle] = object;
    }
    else
        ssfatal("Can't spawn the root object.");

    /* register the object */
    manager->count++;
    surgescript_object_add_child(parent_object, handle);

    /* call constructor and so on */
    surgescript_object_init(object);

    /* done! */
    return handle;
}

/*
 * surgescript_objectmanager_spawn_root()
 * Spawns the root object
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_spawn_root(surgescript_objectmanager_t* manager)
{
    if(manager->handle_ptr == ROOT_HANDLE) {
        /* spawn the root object */
        surgescript_object_t *object = surgescript_object_create(ROOT_OBJECT, ROOT_HANDLE, manager, manager->program_pool, manager->stack, SYSTEM_OBJECTS);
        ssarray_push(manager->data, object);
        manager->count++;

        /* initializes the root and call its constructor */
        surgescript_object_init(object);
    }
    else
        ssfatal("The root object should be the first one to be spawned.");

    return ROOT_HANDLE;
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
            manager->data[handle] = surgescript_object_destroy(manager->data[handle]);
            manager->count--;
            return true;
        }
    }

    return false;
}

/*
 * surgescript_objectmanager_null()
 * Returns a handle to a NULL pointer in the object manager
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_null(surgescript_objectmanager_t* manager)
{
    /* this must *always* be zero */
    return NULL_HANDLE;
}

/*
 * surgescript_objectmanager_root()
 * Returns a handle to the root object in the pool
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_root(surgescript_objectmanager_t* manager)
{
    return ROOT_HANDLE;
}

/*
 * surgescript_objectmanager_application()
 * Returns a handle to the user's application
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_application(surgescript_objectmanager_t* manager)
{
    if(manager->app_handle != NULL_HANDLE)
        return manager->app_handle;
    else
        return manager->app_handle = surgescript_objectmanager_system_object(manager, APPLICATION_OBJECT); /* cache the app handle */
}

/*
 * surgescript_objectmanager_system_object()
 * Returns a handle to a child of the system object
 * The manager parameter may be set to NULL (useful when evaluating at compile-time)
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_system_object(surgescript_objectmanager_t* manager, const char* object_name)
{
    /* this must be determined at compile-time (for SurgeScript), hence the SYSTEM_OBJECTS array */
    for(const char** p = SYSTEM_OBJECTS; *p != NULL; p++) {
        if(strcmp(*p, object_name) == 0)
            return ROOT_HANDLE + (p - SYSTEM_OBJECTS + 1);
    }
    return NULL_HANDLE;
}

/*
 * surgescript_objectmanager_count()
 * How many allocated objects there are?
 */
int surgescript_objectmanager_count(surgescript_objectmanager_t* manager)
{
    return manager->count;
}

/*
 * surgescript_objectmanager_programpool()
 * pointer to the program pool
 */
surgescript_programpool_t* surgescript_objectmanager_programpool(const surgescript_objectmanager_t* manager)
{
    return manager->program_pool;
}

/*
 * surgescript_objectmanager_collectgarbage()
 * Run a cycle of the garbage collector (incremental mark-and-sweep algorithm)
 */
void surgescript_objectmanager_collectgarbage(surgescript_objectmanager_t* manager)
{
    /* if there are no objects to be scanned, scan the root */
    if(ssarray_length(manager->objects_to_be_scanned) == manager->first_object_to_be_scanned) {
        if(surgescript_objectmanager_exists(manager, ROOT_HANDLE)) {
            /* I have already scanned some objects */
            if(ssarray_length(manager->objects_to_be_scanned) > 0) {
                int unreachables = manager->count - manager->reachables_count;

                /* clear the unreachable objects */
                if(unreachables >= MIN_OBJECTS_FOR_DISPOSAL) {
                    surgescript_object_t* root = surgescript_objectmanager_get(manager, ROOT_HANDLE);
                    sslog("Garbage collector: disposing %d of %d object%s.", unreachables, manager->count, unreachables > 1 ? "s" : "");
                    surgescript_object_traverse_tree(root, sweep_unreachables);
                }
                else { /* or, at least, unmark everyone (could be more efficient?) */
                    for(int i = 0; i < ssarray_length(manager->objects_to_be_scanned); i++) {
                        surgescript_objectmanager_handle_t handle = manager->objects_to_be_scanned[i];
                        if(manager->data[handle])
                            surgescript_object_set_reachable(manager->data[handle], false);
                    }
                }
            }

            /* start a new cycle */
            ssarray_reset(manager->objects_to_be_scanned);
            manager->first_object_to_be_scanned = 0;
            manager->reachables_count = 0;
            mark_as_reachable(ROOT_HANDLE, manager);
            surgescript_stack_scan_objects(manager->stack, manager, mark_as_reachable);
        }
    }

    /* for each object o to be scanned, check the ones that are reachable from o */
    int old_length = ssarray_length(manager->objects_to_be_scanned);
    for(int i = manager->first_object_to_be_scanned; i < old_length; i++) {
        surgescript_objectmanager_handle_t handle = manager->objects_to_be_scanned[i];
        if(manager->data[handle] != NULL) {
            surgescript_heap_t* heap = surgescript_object_heap(manager->data[handle]);
            surgescript_heap_scan_objects(heap, manager, mark_as_reachable);
        }
    }
    manager->first_object_to_be_scanned = old_length;
}







/* private stuff */

/* garbage collector */
bool mark_as_reachable(unsigned handle, void* mgr)
{
    surgescript_objectmanager_t* manager = (surgescript_objectmanager_t*)mgr;
    if(surgescript_objectmanager_exists(manager, handle)) {
        surgescript_object_t* object = surgescript_objectmanager_get(manager, handle);
        if(!surgescript_object_is_reachable(object)) {
            surgescript_object_set_reachable(object, true);
            ssarray_push(manager->objects_to_be_scanned, handle);
            manager->reachables_count++;
        }
        return true;
    }
    else
        return false; /* returns false if the handle is broken */
}

bool sweep_unreachables(surgescript_object_t* object)
{
    /* dispose the object */
    if(!surgescript_object_is_reachable(object))
        surgescript_object_kill(object);

    /* reset the mark */
    surgescript_object_set_reachable(object, false);

    /* done! */
    return true;
}

/* gets a handle at a unused space */
surgescript_objectmanager_handle_t new_handle(surgescript_objectmanager_t* mgr)
{
    while(mgr->handle_ptr < ssarray_length(mgr->data) && mgr->data[mgr->handle_ptr] != NULL)
        mgr->handle_ptr++;
    return mgr->handle_ptr;
}