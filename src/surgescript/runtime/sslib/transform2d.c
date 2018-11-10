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
 * runtime/sslib/transform2d.c
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
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_translate(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_rotate(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_getxpos(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getypos(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getscalex(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getscaley(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setxpos(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setypos(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setscalex(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setscaley(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_getworldx(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getworldy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getworldangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setworldx(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setworldy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setworldangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_lookat(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_distanceto(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* misc */
static inline surgescript_object_t* target(const surgescript_object_t* object);
static inline surgescript_object_t* checked_target(const surgescript_object_t* object);
static void world2localposition(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, double *xpos, double *ypos);
static inline void worldposition2d(surgescript_object_t* object, double* world_x, double* world_y);
static inline void setworldposition2d(surgescript_object_t* object, double world_x, double world_y, int flags);
static inline double worldangle2d(surgescript_object_t* object);
static inline void setworldangle2d(surgescript_object_t* object, double angle);
static inline void notify_change(surgescript_object_t* object);
static const int T2SET_WORLDX = 0x1;
static const int T2SET_WORLDY = 0x2;
static const double RAD2DEG = 57.2957795131;
static const char* ONCHANGE = "onTransformChange"; /* fun onTransformChange(transform): [optional] listener on the parent object */



/*
 * surgescript_sslib_register_transform2d()
 * Register methods
 */
void surgescript_sslib_register_transform2d(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Transform2D", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Transform2D", "constructor", fun_constructor, 0);

    surgescript_vm_bind(vm, "Transform2D", "translate", fun_translate, 2);
    surgescript_vm_bind(vm, "Transform2D", "rotate", fun_rotate, 1);

    surgescript_vm_bind(vm, "Transform2D", "get_xpos", fun_getxpos, 0);
    surgescript_vm_bind(vm, "Transform2D", "set_xpos", fun_setxpos, 1);
    surgescript_vm_bind(vm, "Transform2D", "get_ypos", fun_getypos, 0);
    surgescript_vm_bind(vm, "Transform2D", "set_ypos", fun_setypos, 1);
    surgescript_vm_bind(vm, "Transform2D", "get_angle", fun_getangle, 0);
    surgescript_vm_bind(vm, "Transform2D", "set_angle", fun_setangle, 1);
    surgescript_vm_bind(vm, "Transform2D", "get_scaleX", fun_getscalex, 0);
    surgescript_vm_bind(vm, "Transform2D", "set_scaleX", fun_setscalex, 1);
    surgescript_vm_bind(vm, "Transform2D", "get_scaleY", fun_getscaley, 0);
    surgescript_vm_bind(vm, "Transform2D", "set_scaleY", fun_setscaley, 1);

    surgescript_vm_bind(vm, "Transform2D", "get_worldX", fun_getworldx, 0);
    surgescript_vm_bind(vm, "Transform2D", "set_worldX", fun_setworldx, 1);
    surgescript_vm_bind(vm, "Transform2D", "get_worldY", fun_getworldy, 0);
    surgescript_vm_bind(vm, "Transform2D", "set_worldY", fun_setworldy, 1);
    surgescript_vm_bind(vm, "Transform2D", "get_worldAngle", fun_getworldangle, 0);
    surgescript_vm_bind(vm, "Transform2D", "set_worldAngle", fun_setworldangle, 1);

    surgescript_vm_bind(vm, "Transform2D", "lookAt", fun_lookat, 1);
    surgescript_vm_bind(vm, "Transform2D", "distanceTo", fun_distanceto, 1);
}



/* my functions */



/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* register target object for change notifications */
    if(surgescript_object_has_function(target(object), ONCHANGE))
        surgescript_object_set_userdata(object, target(object));
    else
        surgescript_object_set_userdata(object, NULL);

    /* done */
    return NULL;
}

/* rigid operations */
surgescript_var_t* fun_translate(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    double tx = surgescript_var_get_number(param[0]);
    double ty = surgescript_var_get_number(param[1]);

    surgescript_transform_translate2d(transform, tx, ty);

    notify_change(object);
    return NULL;

}

surgescript_var_t* fun_rotate(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    double degrees = surgescript_var_get_number(param[0]);

    surgescript_transform_rotate2d(transform, degrees);

    notify_change(object);
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

surgescript_var_t* fun_getscalex(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    return surgescript_var_set_number(surgescript_var_create(), transform->scale.x);
}

surgescript_var_t* fun_getscaley(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    return surgescript_var_set_number(surgescript_var_create(), transform->scale.y);
}

surgescript_var_t* fun_setxpos(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    transform->position.x = surgescript_var_get_number(param[0]);
    notify_change(object);
    return NULL;
}

surgescript_var_t* fun_setypos(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    transform->position.y = surgescript_var_get_number(param[0]);
    notify_change(object);
    return NULL;
}

surgescript_var_t* fun_setangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    surgescript_transform_setrotation2d(transform, surgescript_var_get_number(param[0])); /* in degrees */
    notify_change(object);
    return NULL;
}

surgescript_var_t* fun_setscalex(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    transform->scale.x = surgescript_var_get_number(param[0]);
    notify_change(object);
    return NULL;
}

surgescript_var_t* fun_setscaley(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_transform_t* transform = surgescript_object_transform(target(object));
    transform->scale.y = surgescript_var_get_number(param[0]);
    notify_change(object);
    return NULL;
}



/* getters & setters: world transform values */
/* modifying world values may be a bit expensive; prefer modifying local transform values instead. */
surgescript_var_t* fun_getworldx(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double world_x = 0.0, world_y = 0.0;
    worldposition2d(target(object), &world_x, &world_y);
    return surgescript_var_set_number(surgescript_var_create(), world_x);
}

surgescript_var_t* fun_getworldy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double world_x = 0.0, world_y = 0.0;
    worldposition2d(target(object), &world_x, &world_y);
    return surgescript_var_set_number(surgescript_var_create(), world_y);
}

surgescript_var_t* fun_getworldangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double world_angle = worldangle2d(target(object)); /* in degrees */
    return surgescript_var_set_number(surgescript_var_create(), world_angle);
}

surgescript_var_t* fun_setworldx(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    setworldposition2d(target(object), surgescript_var_get_number(param[0]), 0.0, T2SET_WORLDX);
    notify_change(object);
    return NULL;
}

surgescript_var_t* fun_setworldy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    setworldposition2d(target(object), 0.0, surgescript_var_get_number(param[0]), T2SET_WORLDY);
    notify_change(object);
    return NULL;
}

surgescript_var_t* fun_setworldangle(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    setworldangle2d(target(object), surgescript_var_get_number(param[0])); /* in degrees */
    notify_change(object);
    return NULL;
}



/* utilities */

/* will look at an object */
surgescript_var_t* fun_lookat(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_object_t* looked = checked_target(surgescript_objectmanager_get(manager, surgescript_var_get_objecthandle(param[0])));

    if(looked != NULL) {
        surgescript_object_t* looker = target(object);
        double looked_x, looked_y, looker_x, looker_y, angle;

        worldposition2d(looker, &looker_x, &looker_y);
        worldposition2d(looked, &looked_x, &looked_y);

        errno = 0;
        angle = atan2(looked_y - looker_y, looked_x - looker_x);
        if(errno == 0)
            setworldangle2d(looker, angle * RAD2DEG);
    }

    return NULL;
}

/* computes the distance to an object */
surgescript_var_t* fun_distanceto(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_object_t* dst = checked_target(surgescript_objectmanager_get(manager, surgescript_var_get_objecthandle(param[0])));

    if(dst != NULL) {
        surgescript_object_t* src = target(object);
        double dst_x, dst_y, src_x, src_y, distance2;

        worldposition2d(src, &src_x, &src_y);
        worldposition2d(dst, &dst_x, &dst_y);
        distance2 = (dst_x - src_x) * (dst_x - src_x) + (dst_y - src_y) * (dst_y - src_y);

        return surgescript_var_set_number(surgescript_var_create(), sqrt(distance2));
    }

    return NULL;
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

/* will check if the given object is a Transform2D and return its target object */
surgescript_object_t* checked_target(const surgescript_object_t* object)
{
    const char* name = surgescript_object_name(object);

    if(strcmp(name, "Transform2D") != 0)
        return NULL;
        
    return target(object);
}

/* this will help compute the local position, recursively, of an object given its world position */
void world2localposition(surgescript_objectmanager_t* manager, surgescript_objecthandle_t handle, surgescript_objecthandle_t root, double* xpos, double* ypos)
{
    surgescript_object_t* object = surgescript_objectmanager_get(manager, handle);
    surgescript_transform_t transform;
    float x, y;

    if(handle != root)
        world2localposition(manager, surgescript_object_parent(object), root, xpos, ypos);

    x = *xpos;
    y = *ypos;
    surgescript_object_peek_transform(object, &transform);
    surgescript_transform_apply2dinverse(&transform, &x, &y);
    *xpos = x;
    *ypos = y;

    /* alternatively, one could compute the world position of the parent and return the difference */
}

/* Computes the 2D world position of the object */
void worldposition2d(surgescript_object_t* object, double* world_x, double* world_y)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    surgescript_transform_t transform;
    float x, y;

    /* get local position */
    surgescript_object_peek_transform(object, &transform);
    x = transform.position.x;
    y = transform.position.y;

    /* compute world position */
    while(handle != root) {
        handle = surgescript_object_parent(object);
        object = surgescript_objectmanager_get(manager, handle); /* object receives its parent */
        surgescript_object_peek_transform(object, &transform);
        surgescript_transform_apply2d(&transform, &x, &y);
    }

    /* done! */
    *world_x = x;
    *world_y = y;
}

/* Sets the 2D world position of the object (recomputes the local transform accordingly) */
void setworldposition2d(surgescript_object_t* object, double world_x, double world_y, int flags)
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

/* computes the world angle of the object, given its local angle */
double worldangle2d(surgescript_object_t* object)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root = surgescript_objectmanager_root(manager);
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    surgescript_transform_t transform;
    double world_angle;

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
    return fmod(world_angle, 360.0);
}

/* sets the world angle of the object by computing its corresponding local angle */
void setworldangle2d(surgescript_object_t* object, double angle)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t parent = surgescript_object_parent(object);
    surgescript_transform_t* transform = surgescript_object_transform(object);

    /* compute corresponding local angle */
    angle -= worldangle2d(surgescript_objectmanager_get(manager, parent));

    /* update local angle */
    surgescript_transform_setrotation2d(transform, angle);
}

/* notify the target object of a transform change */
void notify_change(surgescript_object_t* object)
{
    surgescript_object_t* notifiable_target = (surgescript_object_t*)surgescript_object_userdata(object);
    if(notifiable_target != NULL && notifiable_target == target(object)) { /* safety check */
        surgescript_var_t* transform_handle = surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
        const surgescript_var_t* p[] = { transform_handle };
        surgescript_object_call_function(notifiable_target, ONCHANGE, p, 1, NULL);
        surgescript_var_destroy(transform_handle);
    }
}
