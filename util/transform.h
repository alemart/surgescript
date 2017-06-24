/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/transform.h
 * SurgeScript Transform Utility
 */

#ifndef _SURGESCRIPT_TRANSFORM_H
#define _SURGESCRIPT_TRANSFORM_H

/* transform type */
typedef struct surgescript_transform_t surgescript_transform_t;

/* we'll make this struct public */
struct surgescript_transform_t
{
    struct {
        float x, y, z;
    } position;
    
    struct {
        float sx, cx, sy, cy, sz, cz; /* sin & cos of each component */
    } rotation;
    
    struct {
        float x, y, z;
    } scale;
};

/* public API */
surgescript_transform_t* surgescript_transform_create(); /* creates an identity transform */
surgescript_transform_t* surgescript_transform_destroy(surgescript_transform_t* transform); /* destroys a transform */
void surgescript_transform_apply2d(const surgescript_transform_t* t, float* x, float* y); /* applies the transform to a 2D point */
void surgescript_transform_apply2dinverse(const surgescript_transform_t* t, float* x, float* y); /* applies the inverse transform to a 2D point */

#endif