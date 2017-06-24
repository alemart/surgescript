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
 * Creates a new identity transform
 */
surgescript_transform_t* surgescript_transform_create()
{
    surgescript_transform_t* transform = ssmalloc(sizeof *transform);
    surgescript_transform_set_to_identity(transform);
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
 * surgescript_transform_set_to_identity()
 * Turns t into an identity transform
 */
void surgescript_transform_set_to_identity(surgescript_transform_t* t)
{
    t->position.x = 0.0f;
    t->position.y = 0.0f;
    t->position.z = 0.0f;

    t->rotation.sx = 0.0f;
    t->rotation.cx = 1.0f;
    t->rotation.sy = 0.0f;
    t->rotation.cy = 1.0f;
    t->rotation.sz = 0.0f;
    t->rotation.cz = 1.0f;

    t->scale.x = 1.0f;
    t->scale.y = 1.0f;
    t->scale.z = 1.0f;
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