/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2018 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * util/object_manager.c
 * SurgeScript object manager
 */

#include <string.h>
#include "object_manager.h"
#include "object.h"
#include "program_pool.h"
#include "tag_system.h"
#include "stack.h"
#include "heap.h"
#include "variable.h"
#include "../util/ssarray.h"
#include "../util/util.h"

/* types */
typedef struct surgescript_vmargs_t surgescript_vmargs_t;

/* object manager */
struct surgescript_objectmanager_t
{
    int count; /* how many objects are allocated at the moment */
    surgescript_objecthandle_t handle_ptr; /* memory allocation */
    SSARRAY(surgescript_object_t*, data); /* object table */
    surgescript_programpool_t* program_pool; /* reference to the program pool */
    surgescript_stack_t* stack; /* reference to the stack */
    surgescript_tagsystem_t* tag_system; /* tag system */
    surgescript_vmargs_t* args; /* VM command-line arguments (NULL-terminated array) */
    SSARRAY(surgescript_objecthandle_t, objects_to_be_scanned); /* garbage collection */
    int first_object_to_be_scanned; /* an index of objects_to_be_scanned */
    int reachables_count; /* garbage-collector stuff */
    int garbage_count; /* last number of garbage-collected objects */
    SSARRAY(char*, plugin_list); /* plugin list */
};

/* fixed objects */
#define NULL_HANDLE                 0   /* must always be zero */
#define ROOT_HANDLE                 1

/* system objects are children of the root and
   their addresses must be known at compile-time */
#define SURGESCRIPT_SYSTEM_OBJECTS(F) \
    F( "String" )       \
    F( "Number" )       \
    F( "Boolean" )      \
    F( "__Temp" )       \
    F( "__GC" )         \
    F( "__TagSystem" )  \
    F( "Math" )         \
    F( "Time" )         \
    F( "Date" )         \
    F( "Console" )      \
    F( "SurgeScript" )  \
    F( "Plugin" )       /* Plugin must be the last element of the list, since it may spawn children */
#define PRINT_SYSTEM_OBJECT(x) x,

/* names of the builtin objects */
#define ROOT_OBJECT         "System"
#define APPLICATION_OBJECT  "Application"
static const char* SYSTEM_OBJECTS[] = {
    SURGESCRIPT_SYSTEM_OBJECTS(PRINT_SYSTEM_OBJECT)
    NULL
}; /* this must be a NULL-terminated array */

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

/* other */
#define is_power_of_two(x)                !((x) & ((x) - 1)) /* this assumes x > 0 */
static surgescript_objecthandle_t new_handle(surgescript_objectmanager_t* mgr);
static void add_to_plugin_list(surgescript_objectmanager_t* manager, const char* object_name);
static void release_plugin_list(surgescript_objectmanager_t* manager);
static char** compile_plugins_list(const surgescript_objectmanager_t* manager);
static inline surgescript_object_t* plugin_object(const surgescript_objectmanager_t* manager);

/* -------------------------------
 * public methods
 * ------------------------------- */

/*
 * surgescript_objectmanager_create()
 * Creates a new object manager
 */
surgescript_objectmanager_t* surgescript_objectmanager_create(surgescript_programpool_t* program_pool, surgescript_tagsystem_t* tag_system, surgescript_stack_t* stack, surgescript_vmargs_t* args)
{
    surgescript_objectmanager_t* manager = ssmalloc(sizeof *manager);

    ssarray_init(manager->data);
    ssarray_push(manager->data, NULL); /* NULL is *always* the first element */

    manager->count = 0;
    manager->program_pool = program_pool;
    manager->tag_system = tag_system;
    manager->stack = stack;
    manager->args = args;
    manager->handle_ptr = ROOT_HANDLE;

    ssarray_init(manager->objects_to_be_scanned);
    manager->first_object_to_be_scanned = 0;
    manager->reachables_count = 0;
    manager->garbage_count = 0;

    ssarray_init(manager->plugin_list);

    return manager;
}

/*
 * surgescript_objectmanager_destroy()
 * Destroys an object manager
 */
surgescript_objectmanager_t* surgescript_objectmanager_destroy(surgescript_objectmanager_t* manager)
{
    surgescript_objecthandle_t handle = ssarray_length(manager->data);
    
    while(handle != 0)
        surgescript_objectmanager_delete(manager, --handle);

    ssarray_release(manager->data);
    ssarray_release(manager->objects_to_be_scanned);
    release_plugin_list(manager);

    return ssfree(manager);
}


/*
 * surgescript_objectmanager_spawn()
 * Spawns a new object and puts it in the internal pool (you can't spawn the root using this)
 */
surgescript_objecthandle_t surgescript_objectmanager_spawn(surgescript_objectmanager_t* manager, surgescript_objecthandle_t parent, const char* object_name, void* user_data)
{
    surgescript_objecthandle_t handle = new_handle(manager);
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

    /* this is important for garbage collection (will be cleared up later) */
    surgescript_object_set_reachable(object, true); /* assume the object is reachable at this frame */

    /* call constructor and so on */
    surgescript_object_init(object);

    /* done! */
    return handle;
}

/*
 * surgescript_objectmanager_spawn_root()
 * Spawns the root object
 */
surgescript_objecthandle_t surgescript_objectmanager_spawn_root(surgescript_objectmanager_t* manager)
{
    if(manager->handle_ptr == ROOT_HANDLE) {
        /* preparing the data */
        char** plugins = compile_plugins_list(manager);
        char** data[] = { (char**)SYSTEM_OBJECTS, plugins };

        /* spawn the root object */
        surgescript_object_t *object = surgescript_object_create(ROOT_OBJECT, ROOT_HANDLE, manager, manager->program_pool, manager->stack, data);
        ssarray_push(manager->data, object);
        manager->count++;

        /* initialize the root and call its constructor */
        surgescript_object_init(object);

        /* done! */
        ssfree(plugins);
    }
    else
        ssfatal("The root object should be the first one to be spawned.");

    return ROOT_HANDLE;
}

/*
 * surgescript_objectmanager_exists()
 * Does the specified handle points to a valid object?
 */
bool surgescript_objectmanager_exists(const surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle)
{
    return handle < ssarray_length(manager->data) && manager->data[handle] != NULL;
}

/*
 * surgescript_objectmanager_get()
 * Gets an object from the pool (returns NULL if not found)
 */
surgescript_object_t* surgescript_objectmanager_get(const surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle)
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
bool surgescript_objectmanager_delete(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle)
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
 * Warning: this may be called with a NULL manager parameter
 */
surgescript_objecthandle_t surgescript_objectmanager_null(const surgescript_objectmanager_t* manager)
{
    /* this must *always* be zero */
    return NULL_HANDLE;
}

/*
 * surgescript_objectmanager_root()
 * Returns a handle to the root object in the pool
 * The manager parameter may be set to NULL (useful when evaluating at compile-time)
 */
surgescript_objecthandle_t surgescript_objectmanager_root(const surgescript_objectmanager_t* manager)
{
    return ROOT_HANDLE;
}

/*
 * surgescript_objectmanager_application()
 * Returns a handle to the user's application
 * This can only be determined at runtime, hence the manager parameter must NOT be set to NULL
 */
surgescript_objecthandle_t surgescript_objectmanager_application(const surgescript_objectmanager_t* manager)
{
    surgescript_object_t* root = surgescript_objectmanager_get(manager, ROOT_HANDLE);
    return surgescript_object_child(root, APPLICATION_OBJECT);
}

/*
 * surgescript_objectmanager_system_object()
 * Returns a handle to a child of the system object
 * The manager parameter may be set to NULL (useful when evaluating at compile-time)
 */
surgescript_objecthandle_t surgescript_objectmanager_system_object(const surgescript_objectmanager_t* manager, const char* object_name)
{
    /* this must be determined at compile-time (for SurgeScript), hence the SYSTEM_OBJECTS array */
    for(const char** p = SYSTEM_OBJECTS; *p != NULL; p++) {
        if(strcmp(*p, object_name) == 0)
            return ROOT_HANDLE + (p - SYSTEM_OBJECTS + 1);
    }

    /* the root object is also a system object */
    if(strcmp(ROOT_OBJECT, object_name) == 0)
        return ROOT_HANDLE;
    
    /* not found */
    return NULL_HANDLE;
}

/*
 * surgescript_objectmanager_plugin_object()
 * Returns the handle of the specified plugin object
 * If plugin_name is NULL, returns a handle to Plugin (system object)
 * This can only be determined at runtime, hence the manager parameter must NOT be set to NULL
 */
surgescript_objecthandle_t surgescript_objectmanager_plugin_object(const surgescript_objectmanager_t* manager, const char* plugin_name)
{
    surgescript_object_t* plugin = plugin_object(manager);
    surgescript_objecthandle_t handle;

    if(plugin_name != NULL) {
        char* accessor_fun = surgescript_util_accessorfun("get", plugin_name);
        surgescript_var_t* ret = surgescript_var_create();

        surgescript_object_call_function(plugin, accessor_fun, NULL, 0, ret);
        handle = surgescript_var_get_objecthandle(ret);

        surgescript_var_destroy(ret);
        ssfree(accessor_fun);
    }
    else
        handle = surgescript_object_handle(plugin);

    return handle;
}

/*
 * surgescript_objectmanager_builtin_objects()
 * Returns a NULL-terminated list of the names of the built-in objects
 * The manager parameter may be set to NULL (useful when evaluating at compile-time)
 */
const char** surgescript_objectmanager_builtin_objects(const surgescript_objectmanager_t* manager)
{
    static const char* builtin_objects[] = {
        ROOT_OBJECT,
        APPLICATION_OBJECT,
        SURGESCRIPT_SYSTEM_OBJECTS(PRINT_SYSTEM_OBJECT)
        NULL
    };

    return builtin_objects;
}

/*
 * surgescript_objectmanager_count()
 * How many allocated objects there are?
 */
int surgescript_objectmanager_count(const surgescript_objectmanager_t* manager)
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
 * surgescript_objectmanager_tagsystem()
 * pointer to the tag manager
 */
surgescript_tagsystem_t* surgescript_objectmanager_tagsystem(const surgescript_objectmanager_t* manager)
{
    return manager->tag_system;
}

/*
 * surgescript_objectmanager_vmargs()
 * VM command-line arguments
 */
surgescript_vmargs_t* surgescript_objectmanager_vmargs(const surgescript_objectmanager_t* manager)
{
    return manager->args;
}

/*
 * surgescript_objectmanager_garbagecollect()
 * Runs the garbage collector (incremental mark-and-sweep algorithm)
 * Returns true if something has been disposed, false otherwise
 */
bool surgescript_objectmanager_garbagecollect(surgescript_objectmanager_t* manager)
{
    bool disposed = false;

    /* if there are no objects to be scanned, scan the root */
    if(ssarray_length(manager->objects_to_be_scanned) == manager->first_object_to_be_scanned) {
        if(surgescript_objectmanager_exists(manager, ROOT_HANDLE)) {
            /* I have already scanned some objects */
            if(ssarray_length(manager->objects_to_be_scanned) > 0) {
                /* clear the unreachable objects */
                surgescript_object_t* root = surgescript_objectmanager_get(manager, ROOT_HANDLE);
                manager->garbage_count = 0;
                surgescript_object_traverse_tree(root, sweep_unreachables);
                disposed = true;
            }

            /* start a new cycle */
            ssarray_reset(manager->objects_to_be_scanned);
            manager->first_object_to_be_scanned = 0;
            manager->reachables_count = 0;
            mark_as_reachable(ROOT_HANDLE, manager);
            surgescript_stack_scan_objects(manager->stack, manager, mark_as_reachable);
        }
    }

    /* done! */
    return disposed;
}

/*
 * surgescript_objectmanager_garbagechceck()
 * Incrementally looks for garbage in the system
 */
void surgescript_objectmanager_garbagecheck(surgescript_objectmanager_t* manager)
{
    /* for each object o to be scanned, check the ones that are reachable from o */
    int old_length = ssarray_length(manager->objects_to_be_scanned);
    for(int i = manager->first_object_to_be_scanned; i < old_length; i++) {
        surgescript_objecthandle_t handle = manager->objects_to_be_scanned[i];
        if(manager->data[handle] != NULL) {
            surgescript_heap_t* heap = surgescript_object_heap(manager->data[handle]);
            surgescript_heap_scan_objects(heap, manager, mark_as_reachable);
        }
    }
    manager->first_object_to_be_scanned = old_length;
}

/*
 * surgescript_objectmanager_garbagecount()
 * Last number of garbage-collected objects
 */
int surgescript_objectmanager_garbagecount(const surgescript_objectmanager_t* manager)
{
    return manager->garbage_count;
}

/*
 * surgescript_objectmanager_spawn_array()
 * Spawns an Array on __Temp and returns its handle
 */
surgescript_objecthandle_t surgescript_objectmanager_spawn_array(surgescript_objectmanager_t* manager)
{
    surgescript_objecthandle_t temp = surgescript_objectmanager_system_object(manager, "__Temp");
    return surgescript_objectmanager_spawn(manager, temp, "Array", NULL);
}

/*
 * surgescript_objectmanager_spawn_dictionary()
 * Spawns a Dictionary on __Temp and returns its handle
 */
surgescript_objecthandle_t surgescript_objectmanager_spawn_dictionary(surgescript_objectmanager_t* manager)
{
    surgescript_objecthandle_t temp = surgescript_objectmanager_system_object(manager, "__Temp");
    return surgescript_objectmanager_spawn(manager, temp, "Dictionary", NULL);
}

/*
 * surgescript_objectmanager_spawn_temp()
 * Spawns an object on __Temp and return its handle
 */
surgescript_objecthandle_t surgescript_objectmanager_spawn_temp(surgescript_objectmanager_t* manager, const char* object_name)
{
    surgescript_objecthandle_t temp = surgescript_objectmanager_system_object(manager, "__Temp");
    return surgescript_objectmanager_spawn(manager, temp, object_name, NULL);
}

/*
 * surgescript_objectmanager_install_plugin()
 * Installs a plugin. Call this before spawning the root object.
 */
void surgescript_objectmanager_install_plugin(surgescript_objectmanager_t* manager, const char* object_name)
{
    sslog("Installing plugin \"%s\"...", object_name);
    add_to_plugin_list(manager, object_name);
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
    if(!surgescript_object_is_reachable(object)) {
        /*sslog("Garbage Collector: disposing \"%s\"...", surgescript_object_name(object));*/
        surgescript_object_kill(object);
        surgescript_object_manager(object)->garbage_count++;
    }

    /* reset the mark */
    surgescript_object_set_reachable(object, false);

    /* done! */
    return true;
}

/* gets a handle at a unused space */
surgescript_objecthandle_t new_handle(surgescript_objectmanager_t* mgr)
{
    while(mgr->handle_ptr < ssarray_length(mgr->data) && mgr->data[mgr->handle_ptr] != NULL)
        mgr->handle_ptr++;
    return mgr->handle_ptr;
}

/* adds an object to the plugin list */
void add_to_plugin_list(surgescript_objectmanager_t* manager, const char* object_name)
{
    /* no repeated elements are allowed */
    for(int i = 0; i < ssarray_length(manager->plugin_list); i++) {
        if(strcmp(manager->plugin_list[i], object_name) == 0)
            return;
    }

    /* add the object */
    ssarray_push(manager->plugin_list, ssstrdup(object_name));
}

/* releases the plugin list */
void release_plugin_list(surgescript_objectmanager_t* manager)
{
    char* plugin = NULL;
    while(ssarray_length(manager->plugin_list) > 0) {
        ssarray_pop(manager->plugin_list, plugin);
        ssfree(plugin);
    }
    ssarray_release(manager->plugin_list);
}

/* instantiates a NULL-terminated array of strings with object names to be spawned as plugins */
/* you'll need to ssfree() this array */
char** compile_plugins_list(const surgescript_objectmanager_t* manager) {
    int i = 0, j = 0, count = 1 + ssarray_length(manager->plugin_list);
    char** buf = ssmalloc(count * sizeof(*buf));

    /* register plugins */
    while(i < ssarray_length(manager->plugin_list))
        buf[j++] = manager->plugin_list[i++];
    buf[j++] = NULL; /* end of list */

    /* done! */
    return buf;
}

/* returns the plugin object -- fast */
surgescript_object_t* plugin_object(const surgescript_objectmanager_t* manager)
{
    static surgescript_objecthandle_t handle = NULL_HANDLE;

    if(handle == NULL_HANDLE) /* cache the handle */
        handle = surgescript_objectmanager_system_object(NULL, "Plugin");

    return surgescript_objectmanager_get(manager, handle);
}
