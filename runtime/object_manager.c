/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016-2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/object_manager.c
 * SurgeScript object manager
 */

#include "object_manager.h"
#include "object.h"
#include "program_pool.h"
#include "stack.h"
#include "heap.h"
#include "../util/ssarray.h"
#include "../util/util.h"

/* types */
struct surgescript_objectmanager_t
{
    SSARRAY(surgescript_object_t*, data); /* object table */
    int count; /* how many objects are allocated at the moment */
    surgescript_programpool_t* program_pool; /* reference to the program pool */
    surgescript_stack_t* stack; /* reference to the stack */
    SSARRAY(surgescript_objectmanager_handle_t, objects_to_be_scanned); /* garbage collection */
    int first_object_to_be_scanned; /* an index of objects_to_be_scanned */
    int reachables_count; /* garbage-collector stuff */
};

/* object methods acessible by me */
extern surgescript_object_t* surgescript_object_create(const char* name, unsigned handle, struct surgescript_objectmanager_t* object_manager, struct surgescript_programpool_t* program_pool, struct surgescript_stack_t* stack, void* user_data, bool (*on_init)(surgescript_object_t*), bool (*on_release)(surgescript_object_t*)); /* creates a new blank object */
extern surgescript_object_t* surgescript_object_destroy(surgescript_object_t* object); /* destroys an object */

/* the life-cycle of the objects is handled by me */
extern void surgescript_object_init(surgescript_object_t* object); /* initializes the object (calls constructor, and so on) */
extern void surgescript_object_release(surgescript_object_t* object); /* releases the object (calls destructor, and so on) */

/* garbage collection is handled by me also */
extern bool surgescript_object_is_reachable(const surgescript_object_t* object); /* is this object reachable through some other? */
extern void surgescript_object_set_reachable(surgescript_object_t* object, bool reachable); /* sets whether this object is reachable or not */

/* garbage collector: private stuff */
static void mark_as_reachable(unsigned handle, void* mgr);
static bool sweep_unreachables(surgescript_object_t* object);
static const int MIN_OBJECTS_FOR_DISPOSAL = 1; /* we need at least this amount to delete unreachable objects from memory */

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

/*
 * surgescript_objectmanager_collectgarbage()
 * Run a cycle of the garbage collector (incremental mark-and-sweep algorithm)
 */
void surgescript_objectmanager_collectgarbage(surgescript_objectmanager_t* manager)
{

    /* if there are no objects to be scanned, scan the root */
    if(ssarray_length(manager->objects_to_be_scanned) == manager->first_object_to_be_scanned) {
        surgescript_objectmanager_handle_t root_handle = surgescript_objectmanager_root(manager);
        if(surgescript_objectmanager_exists(manager, root_handle)) {
            surgescript_object_t* root = surgescript_objectmanager_get(manager, root_handle);
        
            /* I have already scanned some objects */
            if(ssarray_length(manager->objects_to_be_scanned) > 0) {
                int unreachables = manager->count - manager->reachables_count;
sslog("gc: %d objects, unreachables: %d", ssarray_length(manager->objects_to_be_scanned), unreachables);

                /* clear the unreachable objects */
                if(unreachables >= MIN_OBJECTS_FOR_DISPOSAL) {
                    sslog("Garbage collector: disposing %d object%s", unreachables, unreachables > 1 ? "s" : "");
                    surgescript_object_traverse_tree(root, sweep_unreachables);
                }
            }
        }

        /* start a new cycle */
        manager->first_object_to_be_scanned = ssarray_length(manager->objects_to_be_scanned) = 0;
        manager->reachables_count = 0;
        //surgescript_object_set_reachable(root, true);
        //ssarray_push(manager->objects_to_be_scanned, root_handle);
        //manager->reachables_count++;
        mark_as_reachable(root_handle, manager);
        surgescript_stack_scan_objects(manager->stack, manager, mark_as_reachable);
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
void mark_as_reachable(unsigned handle, void* mgr)
{
    surgescript_objectmanager_t* manager = (surgescript_objectmanager_t*)mgr;
    if(surgescript_objectmanager_exists(manager, handle)) {
        surgescript_object_t* object = surgescript_objectmanager_get(manager, handle);
        if(!surgescript_object_is_reachable(object)) {
            surgescript_object_set_reachable(object, true);
            ssarray_push(manager->objects_to_be_scanned, handle);
            manager->reachables_count++;
        }
    }
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