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
#include "../vm.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"
#include "../../util/transform.h"

/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* helper macros & local variables of the Transform */
#define T2VAR(varaddr)                  ssassert((varaddr) == surgescript_heap_malloc(heap))
#define T2SET(vartype, varaddr, value)  surgescript_var_set_##vartype(surgescript_heap_at(heap, (varaddr)), (value))
#define T2GET(vartype, varaddr)         surgescript_var_get_##vartype(surgescript_heap_at(heap, (varaddr)))
#define ADDR_ANGLE                      0  /* number - local rotation angle (in degrees) */
#define ADDR_WORLDX                     1  /* number - cached world x position */
#define ADDR_WORLDY                     2  /* number - cached world y position */
#define ADDR_WORLDANGLE                 3  /* number - cached world rotation angle */
#define ADDR_UPDATEDWORLDPOS            4  /* bool - control flag (need to update world position?) */
#define ADDR_UPDATEDWORLDANGLE          5  /* bool - control flag (need to update world angle?) */
#define UPDATE_WORLDPOS                 do { \
                                            if(!T2GET(bool, ADDR_UPDATEDWORLDPOS)) { \
                                                float world_x, world_y; \
                                                worldposition2d(object, &world_x, &world_y); \
                                                T2SET(number, ADDR_WORLDX, world_x); \
                                                T2SET(number, ADDR_WORLDY, world_y); \
                                                T2SET(bool, ADDR_UPDATEDWORLDPOS, true); \
                                            } \
                                        } while(0)
#define UPDATE_WORLDANGLE               do { \
                                            if(!T2GET(bool, ADDR_UPDATEDWORLDANGLE)) { \
                                                float world_angle; \
                                                worldangle2d(object, &world_angle); \
                                                T2SET(number, ADDR_WORLDANGLE, world_angle); \
                                                T2SET(bool, ADDR_UPDATEDWORLDANGLE, true); \
                                            } \
                                        } while(0)
/* TODOs: */
#define ADDR_POSITION                   9  /* object (Vector2) - stored local position */
#define ADDR_SCALE                      10 /* object (Vector2) - stored local scale */
#define ADDR_WORLDPOSITION              11 /* object (Vector2) - stored world position */

/* misc */
static void world2local(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, float *xpos, float *ypos);
static void worldposition2d(surgescript_object_t* object, float* world_x, float* world_y);
static void setworldposition2d(surgescript_object_t* object, float world_x, float world_y, int flags);
static void world2localangle(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, float *s, float *c);
static void worldangle2d(surgescript_object_t* object, float* world_angle);
static void setworldangle2d(surgescript_object_t* object, float world_angle);
#define T2SET_WORLDX                      1
#define T2SET_WORLDY                      2
static const float DEG2RAD = 0.0174532925f;



/*
 * surgescript_sslib_register_transform2d()
 * Register methods
 */
void surgescript_sslib_register_transform2d(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Transform2D", "__constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Transform2D", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Transform2D", "set", fun_set, 2);
    surgescript_vm_bind(vm, "Transform2D", "get", fun_get, 1);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);

    /* declare variables */
    T2VAR(ADDR_ANGLE);
    T2VAR(ADDR_WORLDX);
    T2VAR(ADDR_WORLDY);
    T2VAR(ADDR_WORLDANGLE);
    T2VAR(ADDR_UPDATEDWORLDPOS);
    T2VAR(ADDR_UPDATEDWORLDANGLE);

    /* initialize variables */
    T2SET(number, ADDR_ANGLE, 0.0f);
    T2SET(number, ADDR_WORLDX, 0.0f);
    T2SET(number, ADDR_WORLDY, 0.0f);
    T2SET(number, ADDR_WORLDANGLE, 0.0f);
    T2SET(bool, ADDR_UPDATEDWORLDPOS, false);
    T2SET(bool, ADDR_UPDATEDWORLDANGLE, false);

    /* done! */
    return NULL;
}


/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}

/* getter */
surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* key = surgescript_var_fast_get_string(param[0]);
    surgescript_transform_t* transform = surgescript_object_transform(object);

    /* use a jump table */
    switch(key[0]) {
        /* position */
        case 'x':
            if(!key[1])
                return surgescript_var_set_number(surgescript_var_create(), transform->position.x);
            break;

        case 'y':
            if(!key[1])
                return surgescript_var_set_number(surgescript_var_create(), transform->position.y);
            break;

        /* rotation */
        case 'a':
            if(strcmp(key, "angle") == 0) {
                surgescript_heap_t* heap = surgescript_object_heap(object);
                return surgescript_var_clone(surgescript_heap_at(heap, ADDR_ANGLE));
            }
            break;

        /* scale */
        case 's':
            if(strcmp(key, "scaleX") == 0)
                return surgescript_var_set_number(surgescript_var_create(), transform->scale.x);
            else if(strcmp(key, "scaleY") == 0)
                return surgescript_var_set_number(surgescript_var_create(), transform->scale.y);
            break;

        /* world values */
        case 'w': {
            surgescript_heap_t* heap = surgescript_object_heap(object);
            if(strcmp(key, "worldX") == 0) {
                UPDATE_WORLDPOS;
                return surgescript_var_clone(surgescript_heap_at(heap, ADDR_WORLDX));
            }
            else if(strcmp(key, "worldY") == 0) {
                UPDATE_WORLDPOS;
                return surgescript_var_clone(surgescript_heap_at(heap, ADDR_WORLDY));
            }
            else if(strcmp(key, "worldAngle") == 0) {
                UPDATE_WORLDANGLE;
                return surgescript_var_clone(surgescript_heap_at(heap, ADDR_WORLDANGLE));
            }
            break;
        }
    }

    /* done! */
    return NULL;
}

/* setter */
surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* key = surgescript_var_fast_get_string(param[0]);
    const surgescript_var_t* value = param[1];
    surgescript_transform_t* transform = surgescript_object_transform(object);
    surgescript_heap_t* heap = surgescript_object_heap(object);

    /* use a jump table */
    switch(key[0]) {
        /* position */
        case 'x':
            if(!key[1]) {
                transform->position.x = surgescript_var_get_number(value);
                T2SET(bool, ADDR_UPDATEDWORLDPOS, false); /* will only compute world position if needed */
            }
            break;

        case 'y':
            if(!key[1]) {
                transform->position.y = surgescript_var_get_number(value);
                T2SET(bool, ADDR_UPDATEDWORLDPOS, false);
            }
            break;

        /* rotation */
        case 'a':
            if(strcmp(key, "angle") == 0) {
                float degrees = fmodf(surgescript_var_get_number(value), 360.0f);
                surgescript_transform_setrotation2d(transform, degrees);
                T2SET(number, ADDR_ANGLE, degrees);
                T2SET(bool, ADDR_UPDATEDWORLDANGLE, false);
            }
            break;

        /* scale */
        case 's':
            if(strcmp(key, "scaleX") == 0)
                transform->scale.x = surgescript_var_get_number(value);
            else if(strcmp(key, "scaleY") == 0)
                transform->scale.y = surgescript_var_get_number(value);
            break;

        /* world values */
        case 'w':
            if(strcmp(key, "worldX") == 0) {
                float world_x = surgescript_var_get_number(value);
                setworldposition2d(object, world_x, 0.0f, T2SET_WORLDX);
                T2SET(number, ADDR_WORLDX, world_x);
            }
            else if(strcmp(key, "worldY") == 0) {
                float world_y = surgescript_var_get_number(value);
                setworldposition2d(object, 0.0f, world_y, T2SET_WORLDY);
                T2SET(number, ADDR_WORLDY, world_y);
            }
            else if(strcmp(key, "worldAngle") == 0) {
                float world_angle = fmodf(surgescript_var_get_number(value), 360.0f);
                setworldangle2d(object, world_angle);
                T2SET(number, ADDR_WORLDANGLE, world_angle);
                T2SET(number, ADDR_ANGLE, fmodf(atan2f(transform->rotation.sz, transform->rotation.cz) / DEG2RAD, 360.0f));
            }
            break;
    }

    /* done! */
    return NULL;
}


/* misc */

/* this will help compute the local position, recursively, of an object given its world position */
void world2local(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, float *xpos, float *ypos)
{
    surgescript_object_t* object = surgescript_objectmanager_get(manager, handle);
    surgescript_transform_t transform;

    if(handle != root)
        world2local(manager, surgescript_object_parent(object), root, xpos, ypos);

    surgescript_object_peek_transform(object, &transform);
    surgescript_transform_apply2dinverse(&transform, xpos, ypos);
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
        world2local(manager, surgescript_object_parent(object), root, &world_x, &world_y);

    /* set local transform */
    if(flags & T2SET_WORLDX)
        transform->position.x = world_x;
    if(flags & T2SET_WORLDY)
        transform->position.y = world_y;
}

/* helper function */
void world2localangle(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, float *s, float *c)
{
    surgescript_object_t* object = surgescript_objectmanager_get(manager, handle);
    surgescript_transform_t transform;
    float sa, sb, ca, cb;

    if(handle != root)
        world2localangle(manager, surgescript_object_parent(object), root, s, c);

    surgescript_object_peek_transform(object, &transform);
    sa = *s, ca = *c, sb = transform.rotation.sz, cb = transform.rotation.cz; /* swap? */
    *s = sa * cb - sb * ca;
    *c = ca * cb + sa * sb;
}

/* computes the world angle of the object, given its local angle */
void worldangle2d(surgescript_object_t* object, float* world_angle)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    surgescript_transform_t transform;
    float s, c, sa, ca, sb, cb;

    /* get local angle */
    surgescript_object_peek_transform(object, &transform);
    s = transform.rotation.sz;
    c = transform.rotation.cz;

    /* compute world angle */
    while(handle != root) {
        handle = surgescript_object_parent(object);
        object = surgescript_objectmanager_get(manager, handle); /* object receives its parent */
        surgescript_object_peek_transform(object, &transform);
        sa = s, ca = c, sb = transform.rotation.sz, cb = transform.rotation.cz;
        s = sa * cb + sb * ca, c = ca * cb - sa * sb;
    }

    /* done! */
    *world_angle = fmodf(atan2f(s, c) / DEG2RAD, 360.0f);
}

/* sets the world angle of the object by computing its corresponding local angle */
void setworldangle2d(surgescript_object_t* object, float world_angle)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    surgescript_transform_t* transform = surgescript_object_transform(object);
    float s = sinf(world_angle * DEG2RAD), c = cosf(world_angle * DEG2RAD);

    /* compute corresponding local angle */
    if(handle != root)
        world2localangle(manager, handle, root, &s, &c);

    /* update local angle */
    transform->rotation.sz = s;
    transform->rotation.cz = c;
}