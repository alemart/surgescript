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

/* utilities */
static const float DEG2RAD = 0.0174532925f;

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
    t->position.x = 0.0f;
    t->position.y = 0.0f;
    t->position.z = 0.0f;

    t->rotation.x = 0.0f;
    t->rotation.y = 0.0f;
    t->rotation.z = 0.0f;
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
 * surgescript_transform_setposition2d()
 * Set (local) position
 */
void surgescript_transform_setposition2d(surgescript_transform_t* t, float x, float y)
{
    t->position.x = x;
    t->position.y = y;
}

/*
 * surgescript_transform_setrotation2d()
 * Set (local) rotation angle
 */
void surgescript_transform_setrotation2d(surgescript_transform_t* t, float degrees)
{
    float radians = degrees * DEG2RAD;

    t->rotation.z = fmodf(degrees, 360.0f);
    t->rotation.sz = sinf(radians);
    t->rotation.cz = cosf(radians);
}

/*
 * surgescript_transform_setscale2d()
 * Set (local) scale
 */
void surgescript_transform_setscale2d(surgescript_transform_t* t, float sx, float sy)
{
    t->scale.x = sx;
    t->scale.y = sy;
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
    float a = degrees * DEG2RAD;
    float s = sinf(a), c = cosf(a);

    t->rotation.z = fmodf(t->rotation.z + degrees, 360.0f);
    t->rotation.sz = t->rotation.sz * c + t->rotation.cz * s;
    t->rotation.cz = t->rotation.cz * c - t->rotation.sz * s;
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
    float tx = t->position.x, ty = t->position.y, cz = t->rotation.cz, sz = t->rotation.sz;
    *x = isfinite(sx) ? sx * cz * (*x) + sx * sz * (*y) - sx * cz * tx - sx * sz * ty : 0.0f;
    *y = isfinite(sy) ? -sy * sz * (*x) + sy * cz * (*y) + sy * sz * tx - sy * cz * ty : 0.0f;
}