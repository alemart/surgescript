/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/transform.c
 * SurgeScript Transform Utility
 */

#include <math.h>
#include "util.h"
#include "transform.h"

/*
 * surgescript_transform_create()
 * Creates an identity transform
 */
surgescript_transform_t* surgescript_transform_create()
{
    surgescript_transform_t* transform = ssmalloc(sizeof *transform);

    transform->position.x = 0.0f;
    transform->position.y = 0.0f;
    transform->position.z = 0.0f;

    transform->rotation.sx = 0.0f;
    transform->rotation.cx = 1.0f;
    transform->rotation.sy = 0.0f;
    transform->rotation.cy = 1.0f;
    transform->rotation.sz = 0.0f;
    transform->rotation.cz = 1.0f;

    transform->scale.x = 1.0f;
    transform->scale.y = 1.0f;
    transform->scale.z = 1.0f;

    return transform;
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
 * surgescript_transform_apply2d()
 * Applies the transform to a 2D point
 */
void surgescript_transform_apply2d(const surgescript_transform_t* t, float* x, float* y)
{
    *x = t->scale.x * t->rotation.cz * (*x) - t->scale.y * t->rotation.sz * (*y) + t->position.x;
    *y = t->scale.x * t->rotation.sz * (*x) + t->scale.y * t->rotation.cz * (*y) + t->position.y;
}

/*
 * surgescript_transform_apply2dinverse()
 * Applies the inverse transform to a 2D point
 */
void surgescript_transform_apply2dinverse(const surgescript_transform_t* t, float* x, float* y)
{
    float sx = (fpclassify(t->scale.x) != FP_ZERO) ? 1.0f / t->scale.x : INFINITY;
    float sy = (fpclassify(t->scale.y) != FP_ZERO) ? 1.0f / t->scale.y : INFINITY;
    *x = sx * t->rotation.cz * (*x) + sy * t->rotation.sz * (*y) - t->position.x;
    *y = -sx * t->rotation.sz * (*x) + sy * t->rotation.cz * (*y) - t->position.y;
}