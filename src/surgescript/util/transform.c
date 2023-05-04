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
 * util/transform.c
 * SurgeScript Transform Utility
 */

#include <math.h>
#include <float.h>
#include <errno.h>
#include "util.h"
#include "transform.h"
#include "../runtime/object.h"
#include "../runtime/object_manager.h"

/* A Transform holds position, rotation and scale
   TODO: change this to a 4x4 matrix representation for more flexibility */
struct surgescript_transform_t
{
    /* we'll make this transform struct public */
    struct {
        float x, y, z; /* local position in world units */
    } position;
    
    struct {
        float x, y, z; /* euler angles in degrees */
    } rotation; /* note: use the API to rotate */
    
    struct {
        float x, y, z; /* scale multipliers */
    } scale;

    /* internal data */
    struct {
        float sx, cx, sy, cy, sz, cz; /* cached sin & cos of each component of the rotation */
    } _;
};

/* utilities */
static const float DEG2RAD = 0.01745329251f;
static const float RAD2DEG = 57.2957795131f;
static const surgescript_transform_t IDENTITY = {
    .position = { .x = 0.0f, .y = 0.0f, .z = 0.0f },
    .rotation = { .x = 0.0f, .y = 0.0f, .z = 0.0f },
    .scale    = { .x = 1.0f, .y = 1.0f, .z = 1.0f },
    ._ = {
        .sx = 0.0f, .sy = 0.0f, .sz = 0.0f,
        .cx = 1.0f, .cy = 1.0f, .cz = 1.0f
    }
};
static float y_axis = 1.0f;
static void world2local(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, float* x, float* y);

/*
 * surgescript_transform_create()
 * Creates a new identity transform
 */
surgescript_transform_t* surgescript_transform_create()
{
    surgescript_transform_t* t = ssmalloc(sizeof *t);
    surgescript_transform_reset(t);
    return t;
}

/*
 * surgescript_transform_destroy()
 * Destroys an existing transform
 */
surgescript_transform_t* surgescript_transform_destroy(surgescript_transform_t* transform)
{
    return ssfree(transform);
}

/*
 * surgescript_transform_reset()
 * Turns t into an identity transform
 */
void surgescript_transform_reset(surgescript_transform_t* t)
{
    *t = IDENTITY;
}

/*
 * surgescript_transform_copy()
 * Copies src to dst
 */
void surgescript_transform_copy(surgescript_transform_t* dst, const surgescript_transform_t* src)
{
    *dst = *src;
}

/*
 * surgescript_transform_setposition2d()
 * Set local position
 */
void surgescript_transform_setposition2d(surgescript_transform_t* t, float x, float y)
{
    t->position.x = x;
    t->position.y = y;
}

/*
 * surgescript_transform_getposition2d()
 * Get local position
 */
void surgescript_transform_getposition2d(const surgescript_transform_t* t, float* x, float* y)
{
    *x = t->position.x;
    *y = t->position.y;
}

/*
 * surgescript_transform_setrotation2d()
 * Set local rotation angle
 */
void surgescript_transform_setrotation2d(surgescript_transform_t* t, float degrees)
{
    t->rotation.z = fmodf(degrees, 360.0f);
    t->_.sz = sinf(t->rotation.z * DEG2RAD);
    t->_.cz = cosf(t->rotation.z * DEG2RAD);
}

/*
 * surgescript_transform_getrotation2d()
 * Get local rotation angle
 */
void surgescript_transform_getrotation2d(const surgescript_transform_t* t, float* degrees)
{
    *degrees = fmodf(t->rotation.z, 360.0f);
}

/*
 * surgescript_transform_setscale2d()
 * Set local scale
 */
void surgescript_transform_setscale2d(surgescript_transform_t* t, float sx, float sy)
{
    t->scale.x = sx;
    t->scale.y = sy;
}

/*
 * surgescript_transform_getscale2d()
 * Get local scale
 */
void surgescript_transform_getscale2d(const surgescript_transform_t* t, float* sx, float* sy)
{
    *sx = t->scale.x;
    *sy = t->scale.y;
}

/*
 * surgescript_transform_translate2d()
 * Translate by (x, y)
 */
void surgescript_transform_translate2d(surgescript_transform_t* t, float x, float y)
{
    t->position.x += x;
    t->position.y += y;
}

/*
 * surgescript_transform_rotate2d()
 * Rotate by degrees
 */
void surgescript_transform_rotate2d(surgescript_transform_t* t, float degrees)
{
    t->rotation.z = fmodf(t->rotation.z + degrees, 360.0f);
    t->_.sz = sinf(t->rotation.z * DEG2RAD);
    t->_.cz = cosf(t->rotation.z * DEG2RAD);
}

/*
 * surgescript_transform_scale2d()
 * Scale by (sx, sy)
 */
void surgescript_transform_scale2d(surgescript_transform_t* t, float sx, float sy)
{
    t->scale.x *= sx;
    t->scale.y *= sy;
}

/*
 * surgescript_transform_apply2d()
 * Applies the transform to a 2D point
 */
void surgescript_transform_apply2d(const surgescript_transform_t* t, float* x, float* y)
{
    float ox = *x, oy = *y; /* original values of (x,y) */
    float cz = t->_.cz, sz = t->_.sz * y_axis;

    *x = t->scale.x * cz * ox - t->scale.y * sz * oy + t->position.x;
    *y = t->scale.x * sz * ox + t->scale.y * cz * oy + t->position.y;
}

/*
 * surgescript_transform_apply2dinverse()
 * Applies the inverse transform to a 2D point
 */
void surgescript_transform_apply2dinverse(const surgescript_transform_t* t, float* x, float* y)
{
    float ox = *x, oy = *y; /* original values of (x,y) */
    float tx = t->position.x, ty = t->position.y;
    float cz = t->_.cz, sz = t->_.sz * y_axis;

    if(fpclassify(t->scale.x) != FP_ZERO)
        *x = (cz * ox + sz * oy - cz * tx - sz * ty) / t->scale.x;

    if(fpclassify(t->scale.y) != FP_ZERO)
        *y = (-sz * ox + cz * oy + sz * tx - cz * ty) / t->scale.y;
}

/*
 * surgescript_transform_util_worldposition2d()
 * Gets the 2D world position of an object
 */
void surgescript_transform_util_worldposition2d(const surgescript_object_t* object, float* x, float* y)
{
    /* this must be fast! */
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle;
    const surgescript_transform_t* transform;

    /* initialize (identity translation) */
    *x = *y = 0.0f;

    /* compute world position */
    do {
        if(surgescript_object_transform_changed(object)) {
            transform = surgescript_object_transform((surgescript_object_t*)object);
            surgescript_transform_apply2d(transform, x, y);
        }
        handle = surgescript_object_parent(object);
    } while(handle != root && (object = surgescript_objectmanager_get(manager, handle)));

    /* note: changing the transform of the root object is not supported (nor it is needed!) */
}

/*
 * surgescript_transform_util_setworldposition2d()
 * Sets the 2D world position of an object
 */
void surgescript_transform_util_setworldposition2d(surgescript_object_t* object, float x, float y)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    surgescript_transform_t* transform = surgescript_object_transform(object);

    /* compute local position */
    world2local(manager, surgescript_object_parent(object), root, &x, &y);

    /* set local position */
    transform->position.x = x;
    transform->position.y = y;
}

/*
 * surgescript_transform_util_worldangle2d()
 * Gets the 2D world angle of an object (in degrees)
 */
float surgescript_transform_util_worldangle2d(const surgescript_object_t* object)
{
    /* this must be fast! */
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle;
    const surgescript_transform_t* transform;
    float degrees = 0.0f; /* identity rotation */

    /* compute world angle */
    do {
        if(surgescript_object_transform_changed(object)) {
            transform = surgescript_object_transform((surgescript_object_t*)object);
            degrees += transform->rotation.z;
        }
        handle = surgescript_object_parent(object);
    } while(handle != root && (object = surgescript_objectmanager_get(manager, handle)));

    /* done! */
    return fmodf(degrees, 360.0f);
}

/*
 * surgescript_transform_util_setworldangle2d()
 * Sets the 2D world angle of an object (in degrees)
 */
void surgescript_transform_util_setworldangle2d(surgescript_object_t* object, float degrees)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t parent_handle = surgescript_object_parent(object);
    surgescript_object_t* parent = surgescript_objectmanager_get(manager, parent_handle);
    surgescript_transform_t* transform = surgescript_object_transform(object);

    /* compute the world angle of the parent */
    float parent_world_angle = surgescript_transform_util_worldangle2d(parent);

    /* update local angle (world_angle = parent_world_angle + local_angle) */
    surgescript_transform_setrotation2d(transform, degrees - parent_world_angle);
}

/*
 * surgescript_transform_util_lookat2d()
 * Rotates the right vector of the object transform so that is points at world position (x,y)
 */
void surgescript_transform_util_lookat2d(surgescript_object_t* object, float x, float y)
{
    float my_x, my_y, angle;

    surgescript_transform_util_worldposition2d(object, &my_x, &my_y);

    errno = 0;
    angle = atan2f((y - my_y) * y_axis, x - my_x);
    if(errno == 0)
        surgescript_transform_util_setworldangle2d(object, angle * RAD2DEG);
}

/*
 * surgescript_transform_util_right2d()
 * Gets the 2D right vector of the transform. Unlike the unit right vector,
 * this that takes into account the world angle of the object
 */
void surgescript_transform_util_right2d(const surgescript_object_t* object, float* x, float* y)
{
    float world_angle = surgescript_transform_util_worldangle2d(object) * DEG2RAD;

    /* the right vector of the transform is
       the unit right vector rotated by the
       world angle of the transform */
    *x = cosf(world_angle);
    *y = sinf(world_angle) * y_axis;
}

/*
 * surgescript_transform_util_up2d()
 * Gets the 2D up vector of the transform. Unlike the unit up vector,
 * this that takes into account the world angle of the object
 */
void surgescript_transform_util_up2d(const surgescript_object_t* object, float* x, float* y)
{
    float world_angle = surgescript_transform_util_worldangle2d(object) * DEG2RAD;

    /* the up vector of the transform is
       the unit up vector rotated by the
       world angle of the transform */
    *x = -sinf(world_angle);
    *y = cosf(world_angle) * y_axis;
}

/*
 * surgescript_transform_util_lossyscale2d()
 * Returns an approximation of the 2D world scale of the object.
 * While not very accurate (the parent transform may be rotated and scaled), it's useful
 */
void surgescript_transform_util_lossyscale2d(const surgescript_object_t* object, float* x, float* y)
{
    /* this must be fast! */
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle;
    const surgescript_transform_t* transform;
    const float upper_bound = 1.0f + FLT_EPSILON;
    const float lower_bound = 1.0f - FLT_EPSILON;

    /* initialize (identity) */
    *x = *y = 1.0f;

    /* compute lossy scale */
    do {
        if(surgescript_object_transform_changed(object)) {
            transform = surgescript_object_transform((surgescript_object_t*)object);
            if(transform->scale.x <= lower_bound || transform->scale.x >= upper_bound)
                *x *= transform->scale.x;
            if(transform->scale.y <= lower_bound || transform->scale.y >= upper_bound)
                *y *= transform->scale.y;
        }
        handle = surgescript_object_parent(object);
    } while(handle != root && (object = surgescript_objectmanager_get(manager, handle)));
}

/*
 * surgescript_transform_use_inverted_y()
 * Inverts the direction of the y-axis (i.e., set it to "down") if called with true,
 * or reverts it back to normal (i.e., "up") if called with false
 */
void surgescript_transform_use_inverted_y(bool inverted)
{
    y_axis = inverted ? -1.0f : 1.0f;
}

/*
 * surgescript_transform_is_using_inverted_y()
 * Checks if the y-axis has been inverted (defaults to false)
 */
bool surgescript_transform_is_using_inverted_y()
{
    return y_axis < 0.0f;
}



/* ----- private ----- */

/* auxiliary function to compute the inverse transform (world to local data)
   given n transforms T1, T2, ..., Tn,
   (T1 T2 ... Tn)^-1 (pos) = (Tn^-1 ... T2^-1 T1^-1) (pos) */
static void world2local(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, float* x, float* y)
{
    surgescript_object_t* object = surgescript_objectmanager_get(manager, handle);

    if(handle != root)
        world2local(manager, surgescript_object_parent(object), root, x, y);

    if(surgescript_object_transform_changed(object)) {
        const surgescript_transform_t* transform = surgescript_object_transform(object);
        surgescript_transform_apply2dinverse(transform, x, y);
    }
}