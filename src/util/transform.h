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
 * util/transform.h
 * SurgeScript Transform Utility
 */

#ifndef _SURGESCRIPT_TRANSFORM_H
#define _SURGESCRIPT_TRANSFORM_H

/* transform type */
typedef struct surgescript_transform_t surgescript_transform_t;

/* A Transform holds position, rotation and scale */
struct surgescript_transform_t
{
    /* we'll make this transform struct public */
    struct {
        float x, y, z; /* local position in world units */
    } position;
    
    struct {
        float x, y, z; /* euler angles in degrees */
        float sx, cx, sy, cy, sz, cz; /* cached sin & cos of each component */
    } rotation; /* note: use the API to rotate */
    
    struct {
        float x, y, z; /* scale multipliers */
    } scale;
};

/* public API */
surgescript_transform_t* surgescript_transform_create(); /* creates a new identity transform */
surgescript_transform_t* surgescript_transform_destroy(surgescript_transform_t* transform); /* destroys a transform */

/* generic operations */
void surgescript_transform_reset(surgescript_transform_t* t); /* turns t into an identity transform */
void surgescript_transform_copy(surgescript_transform_t* dst, const surgescript_transform_t* src); /* copies src to dst */

/* 2D operations */
void surgescript_transform_setposition2d(surgescript_transform_t* t, float x, float y); /* set position */
void surgescript_transform_setrotation2d(surgescript_transform_t* t, float degrees); /* set rotation */
void surgescript_transform_setscale2d(surgescript_transform_t* t, float sx, float sy); /* set scale */
void surgescript_transform_translate2d(surgescript_transform_t* t, float x, float y); /* translate */
void surgescript_transform_rotate2d(surgescript_transform_t* t, float degrees); /* rotate */
void surgescript_transform_scale2d(surgescript_transform_t* t, float sx, float sy); /* scale */
void surgescript_transform_apply2d(const surgescript_transform_t* t, float* x, float* y); /* applies the transform to a 2D point */
void surgescript_transform_apply2dinverse(const surgescript_transform_t* t, float* x, float* y); /* applies the inverse transform to a 2D point */

/* 3D operations */
/* TODO */

#endif