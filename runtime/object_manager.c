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
#include "../util/ssarray.h"
#include "../util/util.h"

/* types */
struct surgescript_objectmanager_t
{
    SSARRAY(surgescript_object_t*, data);
};

/* -------------------------------
 * public methods
 * ------------------------------- */

/*
 * surgescript_objectmanager_create()
 * Creates a new object manager
 */
surgescript_objectmanager_t* surgescript_objectmanager_create(surgescript_object_t* root)
{
    surgescript_objectmanager_t* pool = ssmalloc(sizeof *pool);
    ssarray_init(pool->data);

    surgescript_objectmanager_put(pool, NULL);
    surgescript_objectmanager_put(pool, root);

    return pool;
}

/*
 * surgescript_objectmanager_destroy()
 * Destroys an object manager
 */
surgescript_objectmanager_t* surgescript_objectmanager_destroy(surgescript_objectmanager_t* pool)
{
    int i, len = ssarray_length(pool->data);

    for(i = len - 1; i >= 0; i--) {
        if(pool->data[i] != NULL)
            pool->data[i] = surgescript_object_destroy(pool->data[i]);
    }

    ssarray_release(pool->data);
    return ssfree(pool);
}


/*
 * surgescript_objectmanager_put()
 * Puts an object in the pool
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_put(surgescript_objectmanager_t* pool, surgescript_object_t* object)
{
    ssarray_push(pool->data, object);
    return ssarray_length(pool->data) - 1;
}

/*
 * surgescript_objectmanager_get()
 * Gets an object from the pool (returns NULL if not found)
 */
surgescript_object_t* surgescript_objectmanager_get(surgescript_objectmanager_t* pool, surgescript_objectmanager_handle_t handle)
{
    if(handle < ssarray_length(pool->data)) { /* handle is unsigned; therefore, not lower than zero */
        if(pool->data[handle] != NULL)
            return pool->data[handle];
    }

    ssfatal("Null pointer exception (can't find object @ 0x%X)", handle);
    return NULL;
}

/*
 * surgescript_objectmanager_delete()
 * Deletes an object from the pool
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_delete(surgescript_objectmanager_t* pool, surgescript_objectmanager_handle_t handle)
{
    if(handle < ssarray_length(pool->data)) {
        if(pool->data[handle] != NULL)
            pool->data[handle] = surgescript_object_destroy(pool->data[handle]);
    }

    return handle;
}

/*
 * surgescript_objectmanager_get_null()
 * Returns a handle to a NULL pointer in the object manager
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_get_null(surgescript_objectmanager_t* pool)
{
    return 0;
}

/*
 * surgescript_objectmanager_get_root()
 * Returns a handle to the root object in the pool
 */
surgescript_objectmanager_handle_t surgescript_objectmanager_get_root(surgescript_objectmanager_t* pool)
{
    return 1;
}
