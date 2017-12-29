/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/transform2d.c
 * SurgeScript standard library: Transform2D
 */

#include <string.h>
#include <math.h>
#include <errno.h>
#include "../vm.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"
#include "../../util/transform.h"

/* private stuff */
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_gettransform(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_translate(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_rotate(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_scale(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_getxpos(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getypos(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getwidth(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getheight(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setxpos(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setypos(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setwidth(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setheight(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_getworldx(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getworldy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getworldangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setworldx(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setworldy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setworldangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_lookat(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_lookto(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_distanceto(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* misc */
static inline surgescript_object_t* target(const surgescript_object_t* object);
static void world2localposition(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, float *xpos, float *ypos);
static void worldposition2d(surgescript_object_t* object, float* world_x, float* world_y);
static void setworldposition2d(surgescript_object_t* object, float world_x, float world_y, int flags);
static void world2localangle(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, float *angle);
static float worldangle2d(surgescript_object_t* object);
static void setworldangle2d(surgescript_object_t* object, float angle);
static const int T2SET_WORLDX = 0x1;
static const int T2SET_WORLDY = 0x2;
static const float DEG2RAD = 0.0174532925f;



/*
 * surgescript_sslib_register_transform2d()
 * Register methods
 */
void surgescript_sslib_register_transform2d(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Transform2D", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Transform2D", "getTransform", fun_gettransform, 0);

    surgescript_vm_bind(vm, "Transform2D", "translate", fun_translate, 2);
    surgescript_vm_bind(vm, "Transform2D", "rotate", fun_rotate, 1);
    surgescript_vm_bind(vm, "Transform2D", "scale", fun_scale, 2);

    surgescript_vm_bind(vm, "Transform2D", "getXpos", fun_getxpos, 0);
    surgescript_vm_bind(vm, "Transform2D", "setXpos", fun_setxpos, 1);
    surgescript_vm_bind(vm, "Transform2D", "getYpos", fun_getypos, 0);
    surgescript_vm_bind(vm, "Transform2D", "setYpos", fun_setypos, 1);
    surgescript_vm_bind(vm, "Transform2D", "getAngle", fun_getangle, 0);
    surgescript_vm_bind(vm, "Transform2D", "setAngle", fun_setangle, 1);
    surgescript_vm_bind(vm, "Transform2D", "getWidth", fun_getwidth, 0);
    surgescript_vm_bind(vm, "Transform2D", "setWidth", fun_setwidth, 1);
    surgescript_vm_bind(vm, "Transform2D", "getHeight", fun_getheight, 0);
    surgescript_vm_bind(vm, "Transform2D", "setHeight", fun_setheight, 1);

    surgescript_vm_bind(vm, "Transform2D", "getWorldX", fun_getworldx, 0);
    surgescript_vm_bind(vm, "Transform2D", "setWorldX", fun_setworldx, 1);
    surgescript_vm_bind(vm, "Transform2D", "getWorldY", fun_getworldy, 0);
    surgescript_vm_bind(vm, "Transform2D", "setWorldY", fun_setworldy, 1);
    surgescript_vm_bind(vm, "Transform2D", "getWorldAngle", fun_getworldangle, 0);
    surgescript_vm_bind(vm, "Transform2D", "setWorldAngle", fun_setworldangle, 1);

    surgescript_vm_bind(vm, "Transform2D", "lookAt", fun_lookat, 2);
    surgescript_vm_bind(vm, "Transform2D", "lookTo", fun_lookto, 1);
    surgescript_vm_bind(vm, "Transform2D", "distanceTo", fun_distanceto, 1);
}



/* my functions */



/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}

/* transform */
surgescript_var_t* fun_gettransform(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* this object has no transform */
    return NULL;
}



/* rigid operations */
surgescript_var_t* fun_translate(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    float tx = surgescript_var_get_number(param[0]);
    float ty = surgescript_var_get_number(param[1]);

    surgescript_transform_translate2d(transform, tx, ty);

    return NULL;

}
surgescript_var_t* fun_rotate(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    float degrees = surgescript_var_get_number(param[0]);

    surgescript_transform_rotate2d(transform, degrees);

    return NULL;
}

surgescript_var_t* fun_scale(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    float sx = surgescript_var_get_number(param[0]);
    float sy = surgescript_var_get_number(param[1]);

    surgescript_transform_scale2d(transform, sx, sy);

    return NULL;
}



/* getters & setters: local transform values */
surgescript_var_t* fun_getxpos(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    return surgescript_var_set_number(surgescript_var_create(), transform->position.x);
}

surgescript_var_t* fun_getypos(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    return surgescript_var_set_number(surgescript_var_create(), transform->position.y);
}

surgescript_var_t* fun_getangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    return surgescript_var_set_number(surgescript_var_create(), transform->rotation.z);
}

surgescript_var_t* fun_getwidth(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    return surgescript_var_set_number(surgescript_var_create(), transform->scale.x);
}

surgescript_var_t* fun_getheight(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    return surgescript_var_set_number(surgescript_var_create(), transform->scale.y);
}

surgescript_var_t* fun_setxpos(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    transform->position.x = surgescript_var_get_number(param[0]);
    return NULL;
}

surgescript_var_t* fun_setypos(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    transform->position.y = surgescript_var_get_number(param[0]);
    return NULL;
}

surgescript_var_t* fun_setangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    surgescript_transform_setrotation2d(transform, surgescript_var_get_number(param[0])); /* in degrees */
    return NULL;
}

surgescript_var_t* fun_setwidth(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    transform->scale.x = surgescript_var_get_number(param[0]);
    return NULL;
}

surgescript_var_t* fun_setheight(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    transform->scale.y = surgescript_var_get_number(param[0]);
    return NULL;
}



/* getters & setters: world transform values */
/* modifying world values may be a bit expensive; prefer modifying local transform values instead. */
surgescript_var_t* fun_getworldx(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float world_x = 0.0f, world_y = 0.0f;
    worldposition2d(target(object), &world_x, &world_y);
    return surgescript_var_set_number(surgescript_var_create(), world_x);
}

surgescript_var_t* fun_getworldy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float world_x = 0.0f, world_y = 0.0f;
    worldposition2d(target(object), &world_x, &world_y);
    return surgescript_var_set_number(surgescript_var_create(), world_y);
}

surgescript_var_t* fun_getworldangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float world_angle = worldangle2d(target(object)); /* in degrees */
    return surgescript_var_set_number(surgescript_var_create(), world_angle);
}

surgescript_var_t* fun_setworldx(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    setworldposition2d(target(object), surgescript_var_get_number(param[0]), 0.0f, T2SET_WORLDX);
    return NULL;
}

surgescript_var_t* fun_setworldy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    setworldposition2d(target(object), 0.0f, surgescript_var_get_number(param[0]), T2SET_WORLDY);
    return NULL;
}

surgescript_var_t* fun_setworldangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    setworldangle2d(target(object), surgescript_var_get_number(param[0])); /* in degrees */
    return NULL;
}



/* utilities */

/* will look at a given (world_x, world_y) position */
surgescript_var_t* fun_lookat(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_t* target_object = target(object);
    float target_world_x = surgescript_var_get_number(param[0]);
    float target_world_y = surgescript_var_get_number(param[1]);
    float angle = 0.0f, eye_world_x = 0.0f, eye_world_y = 0.0f;

    worldposition2d(target_object, &eye_world_x, &eye_world_y);

    errno = 0;
    angle = atan2f(target_world_y - eye_world_y, target_world_x - eye_world_x);
    if(errno == 0)
        setworldangle2d(target_object, angle / DEG2RAD);

    return NULL;
}

/* will look to an object */
surgescript_var_t* fun_lookto(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_object_t* looker = target(object);
    surgescript_object_t* looked = surgescript_objectmanager_get(manager, surgescript_var_get_objecthandle(param[0]));
    float looked_x, looked_y, looker_x, looker_y, angle;

    worldposition2d(looker, &looker_x, &looker_y);
    worldposition2d(looked, &looked_x, &looked_y);

    errno = 0;
    angle = atan2f(looked_y - looker_y, looked_x - looker_x);
    if(errno == 0)
        setworldangle2d(looker, angle / DEG2RAD);

    return NULL;
}

/* computes the distance to an object */
surgescript_var_t* fun_distanceto(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_object_t* src = target(object);
    surgescript_object_t* dst = surgescript_objectmanager_get(manager, surgescript_var_get_objecthandle(param[0]));
    float dst_x, dst_y, src_x, src_y, distance2;

    worldposition2d(src, &src_x, &src_y);
    worldposition2d(dst, &dst_x, &dst_y);
    distance2 = (dst_x - src_x) * (dst_x - src_x) + (dst_y - src_y) * (dst_y - src_y);

    return surgescript_var_set_number(surgescript_var_create(), sqrt(distance2));
}


/* misc */

/* will return the target object of the given transform object */
surgescript_object_t* target(const surgescript_object_t* object)
{
    /* the target object is the parent of the transform */
    return surgescript_objectmanager_get(
        surgescript_object_manager(object),
        surgescript_object_parent(object)
    );
}

/* this will help compute the local position, recursively, of an object given its world position */
void world2localposition(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, float *xpos, float *ypos)
{
    surgescript_object_t* object = surgescript_objectmanager_get(manager, handle);
    surgescript_transform_t transform;

    if(handle != root)
        world2localposition(manager, surgescript_object_parent(object), root, xpos, ypos);

    surgescript_object_peek_transform(object, &transform);
    surgescript_transform_apply2dinverse(&transform, xpos, ypos);

    /* alternatively, one could compute the world position of the parent and return the difference */
}

/* Computes the 2D world position of the object */
void worldposition2d(surgescript_object_t* object, float* world_x, float *world_y)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    surgescript_transform_t transform;

    /* get local position */
    surgescript_object_peek_transform(object, &transform);
    *world_x = transform.position.x;
    *world_y = transform.position.y;

    /* compute world position */
    while(handle != root) {
        handle = surgescript_object_parent(object);
        object = surgescript_objectmanager_get(manager, handle); /* object receives its parent */
        surgescript_object_peek_transform(object, &transform);
        surgescript_transform_apply2d(&transform, world_x, world_y);
    }
}

/* Sets the 2D world position of the object (recomputes the local transform accordingly) */
void setworldposition2d(surgescript_object_t* object, float world_x, float world_y, int flags)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    surgescript_transform_t* transform = surgescript_object_transform(object);

    /* compute local transform */
    if(handle != root)
        world2localposition(manager, surgescript_object_parent(object), root, &world_x, &world_y);

    /* set local transform */
    if(flags & T2SET_WORLDX)
        transform->position.x = world_x;
    if(flags & T2SET_WORLDY)
        transform->position.y = world_y;
}

/* helper function */
void world2localangle(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, float *angle)
{
    surgescript_object_t* object = surgescript_objectmanager_get(manager, handle);
    surgescript_transform_t transform;

    if(handle != root)
        world2localangle(manager, surgescript_object_parent(object), root, angle);

    surgescript_object_peek_transform(object, &transform);
    *angle -= transform.rotation.z; /* in degrees */
}

/* computes the world angle of the object, given its local angle */
float worldangle2d(surgescript_object_t* object)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    surgescript_transform_t transform;
    float world_angle;

    /* get local angle */
    surgescript_object_peek_transform(object, &transform);
    world_angle = transform.rotation.z;

    /* compute world angle */
    while(handle != root) {
        handle = surgescript_object_parent(object);
        object = surgescript_objectmanager_get(manager, handle); /* object receives its parent */
        surgescript_object_peek_transform(object, &transform);
        world_angle += transform.rotation.z;
    }

    /* done! */
    return fmodf(world_angle, 360.0f);
}

/* sets the world angle of the object by computing its corresponding local angle */
void setworldangle2d(surgescript_object_t* object, float angle)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    surgescript_transform_t* transform = surgescript_object_transform(object);

    /* compute corresponding local angle */
    if(handle != root)
        world2localangle(manager, handle, root, &angle);

    /* update local angle */
    surgescript_transform_setrotation2d(transform, angle);
}