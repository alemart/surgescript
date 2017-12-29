/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/math.c
 * SurgeScript standard library: Math routines
 */

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "../vm.h"
#include "../object.h"

/* private stuff */
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_gettransform(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getepsilon(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getpi(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getdeg2rad(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getrad2deg(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getinfinity(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_random(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_sin(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_cos(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tan(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_asin(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_acos(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_atan(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_atan2(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_pow(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_sqrt(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_exp(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_log(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_log10(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_floor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_ceil(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_round(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_fmod(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_sign(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_abs(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_min(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_max(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_clamp(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_approximately(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_lerp(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_smoothstep(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* constants */
static const float EPSILON = FLT_EPSILON; /*0.00001f;*/
static const float PI = 3.1415926535f;
static const float RAD2DEG = 57.2957795147f;
static const float DEG2RAD = 0.01745329251f;

/*
 * surgescript_sslib_register_math()
 * Register methods
 */
void surgescript_sslib_register_math(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Math", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Math", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Math", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Math", "getTransform", fun_gettransform, 0);
    surgescript_vm_bind(vm, "Math", "getEpsilon", fun_getepsilon, 0);
    surgescript_vm_bind(vm, "Math", "getPi", fun_getpi, 0);
    surgescript_vm_bind(vm, "Math", "getDeg2rad", fun_getdeg2rad, 0);
    surgescript_vm_bind(vm, "Math", "getRad2deg", fun_getrad2deg, 0);
    surgescript_vm_bind(vm, "Math", "getInfinity", fun_getinfinity, 0);
    surgescript_vm_bind(vm, "Math", "random", fun_random, 0);
    surgescript_vm_bind(vm, "Math", "sin", fun_sin, 1);
    surgescript_vm_bind(vm, "Math", "cos", fun_cos, 1);
    surgescript_vm_bind(vm, "Math", "tan", fun_tan, 1);
    surgescript_vm_bind(vm, "Math", "asin", fun_asin, 1);
    surgescript_vm_bind(vm, "Math", "acos", fun_acos, 1);
    surgescript_vm_bind(vm, "Math", "atan", fun_atan, 1);
    surgescript_vm_bind(vm, "Math", "atan2", fun_atan2, 2);
    surgescript_vm_bind(vm, "Math", "pow", fun_pow, 2);
    surgescript_vm_bind(vm, "Math", "sqrt", fun_sqrt, 1);
    surgescript_vm_bind(vm, "Math", "exp", fun_exp, 1);
    surgescript_vm_bind(vm, "Math", "log", fun_log, 1);
    surgescript_vm_bind(vm, "Math", "log10", fun_log10, 1);
    surgescript_vm_bind(vm, "Math", "floor", fun_floor, 1);
    surgescript_vm_bind(vm, "Math", "ceil", fun_ceil, 1);
    surgescript_vm_bind(vm, "Math", "round", fun_round, 1);
    surgescript_vm_bind(vm, "Math", "fmod", fun_fmod, 2);
    surgescript_vm_bind(vm, "Math", "sign", fun_sign, 1);
    surgescript_vm_bind(vm, "Math", "abs", fun_abs, 1);
    surgescript_vm_bind(vm, "Math", "min", fun_min, 2);
    surgescript_vm_bind(vm, "Math", "max", fun_max, 2);
    surgescript_vm_bind(vm, "Math", "clamp", fun_clamp, 3);
    surgescript_vm_bind(vm, "Math", "approximately", fun_approximately, 2);
    surgescript_vm_bind(vm, "Math", "lerp", fun_lerp, 3);
    surgescript_vm_bind(vm, "Math", "smoothstep", fun_smoothstep, 3);
}



/* my functions */

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_set_active(object, false); /* we don't need to spend time updating this object */
    return NULL;
}

/* destroy method */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing, as system objects cannot be destroyed */
    return NULL;
}

/* spawn */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing; you can't spawn children on this object */
    return NULL;
}

/* transform */
surgescript_var_t* fun_gettransform(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* this object has no transform */
    return NULL;
}

/* constant: value of epsilon */
surgescript_var_t* fun_getepsilon(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), EPSILON);
}

/* constant: value of pi */
surgescript_var_t* fun_getpi(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), PI);
}

/* constant: convert degrees to radians */
surgescript_var_t* fun_getdeg2rad(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), DEG2RAD);
}

/* constant: convert radians to degrees */
surgescript_var_t* fun_getrad2deg(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), RAD2DEG);
}

/* constant: a representation of +Infinity */
surgescript_var_t* fun_getinfinity(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), INFINITY);
}

/* random(): returns a random number between 0 (inclusive) and 1 (exclusive) */
surgescript_var_t* fun_random(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), rand() / ((double)RAND_MAX + 1.0));
}

/* sin(x): sine of x, x in degrees */
surgescript_var_t* fun_sin(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), sinf(surgescript_var_get_number(param[0]) * DEG2RAD));
}

/* cos(x): cosine of x, x in degrees */
surgescript_var_t* fun_cos(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), cosf(surgescript_var_get_number(param[0]) * DEG2RAD));
}

/* tan(x): tangent of x, x in degrees */
surgescript_var_t* fun_tan(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), tanf(surgescript_var_get_number(param[0]) * DEG2RAD));
}

/* asin(x): arc sin of x, returned in degrees */
surgescript_var_t* fun_asin(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), asinf(surgescript_var_get_number(param[0])) * RAD2DEG);
}

/* acos(x): arc cosine of x, returned in degrees */
surgescript_var_t* fun_acos(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), acosf(surgescript_var_get_number(param[0])) * RAD2DEG);
}

/* atan(x): arc tangent of x, returned in degrees */
surgescript_var_t* fun_atan(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), atanf(surgescript_var_get_number(param[0])) * RAD2DEG);
}

/* atan2(y,x): returns the angle, in degrees, between the positive x-axis and the vector (x,y) */
surgescript_var_t* fun_atan2(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float y = surgescript_var_get_number(param[0]);
    float x = surgescript_var_get_number(param[1]);
    return surgescript_var_set_number(surgescript_var_create(), atan2f(y, x) * RAD2DEG);
}

/* pow(base, exponent): returns the value of base raised to the power exponent */
surgescript_var_t* fun_pow(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float base = surgescript_var_get_number(param[0]);
    float exponent = surgescript_var_get_number(param[1]);
    return surgescript_var_set_number(surgescript_var_create(), powf(base, exponent));
}

/* sqrt(x): square root of x */
surgescript_var_t* fun_sqrt(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), sqrtf(surgescript_var_get_number(param[0])));
}

/* exp(x): e raised to the power of x */
surgescript_var_t* fun_exp(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), expf(surgescript_var_get_number(param[0])));
}

/* log(x): returns the natural logarithm of x */
surgescript_var_t* fun_log(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), logf(surgescript_var_get_number(param[0])));
}

/* log10(x): the base-10 logarithm of x */
surgescript_var_t* fun_log10(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), log10f(surgescript_var_get_number(param[0])));
}

/* floor(x): the floor of x */
surgescript_var_t* fun_floor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), floorf(surgescript_var_get_number(param[0])));
}

/* ceil(x): the ceiling of x */
surgescript_var_t* fun_ceil(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), ceilf(surgescript_var_get_number(param[0])));
}

/* round(x): round x to the nearest integer */
surgescript_var_t* fun_round(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float x = surgescript_var_get_number(param[0]); /* round x half away from zero */
    return surgescript_var_set_number(surgescript_var_create(), (x >= 0.0f) ? floorf(x + 0.5f) : ceilf(x - 0.5f));
}

/* fmod(x,y): the remainder of x/y */
surgescript_var_t* fun_fmod(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float x = surgescript_var_get_number(param[0]);
    float y = surgescript_var_get_number(param[1]);
    return surgescript_var_set_number(surgescript_var_create(), fmodf(x, y));
}

/* sign(x): returns +1 if x is non-negative, or -1 otherwise */
surgescript_var_t* fun_sign(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float x = surgescript_var_get_number(param[0]);
    return surgescript_var_set_number(surgescript_var_create(), (x >= 0.0f) ? 1.0f : -1.0f);
}

/* abs(x): the absolute value of x */
surgescript_var_t* fun_abs(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), fabsf(surgescript_var_get_number(param[0])));
}

/* min(x,y): the minimum between x and y */
surgescript_var_t* fun_min(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float x = surgescript_var_get_number(param[0]);
    float y = surgescript_var_get_number(param[1]);
    return surgescript_var_set_number(surgescript_var_create(), (x < y) ? x : y);
}

/* max(x,y): the maximum between x and y */
surgescript_var_t* fun_max(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float x = surgescript_var_get_number(param[0]);
    float y = surgescript_var_get_number(param[1]);
    return surgescript_var_set_number(surgescript_var_create(), (x >= y) ? x : y);
}

/* clamp(x,min,max): clamps x between min and max */
surgescript_var_t* fun_clamp(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float x = surgescript_var_get_number(param[0]);
    float minval = surgescript_var_get_number(param[1]);
    float maxval = surgescript_var_get_number(param[2]);

    if(minval > maxval) {
        float tmp = minval;
        minval = maxval;
        maxval = tmp;
    }

    return surgescript_var_set_number(surgescript_var_create(), (x >= minval) ? (x <= maxval ? x : maxval) : minval);
}

/* approximately(a,b): returns true if floating points a and b are approximately equal */
surgescript_var_t* fun_approximately(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float a = surgescript_var_get_number(param[0]);
    float b = surgescript_var_get_number(param[1]);
    float fa = fabsf(a), fb = fabsf(b), fm = (fa >= fb ? fa : fb);
    float eps = EPSILON * (fm >= 1.0f ? fm : 1.0f);

    return surgescript_var_set_bool(surgescript_var_create(), (a >= b - eps) && (a <= b + eps));
}

/* lerp(a,b,t): linear interpolation between a and b by t, where t is clamped to the range [0,1] */
surgescript_var_t* fun_lerp(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float a = surgescript_var_get_number(param[0]);
    float b = surgescript_var_get_number(param[1]);
    float t = surgescript_var_get_number(param[2]);

    if(t < 0.0f)
        t = 0.0f;
    else if(t > 1.0f)
        t = 1.0f;

    /* When t = 0, returns a. When t = 1, returns b. When t = 0.5, returns (a+b) / 2 */
    return surgescript_var_set_number(surgescript_var_create(), (b - a) * t + a);
}


/* smoothstep(a,b,t): interpolates smoothly between a and b by t, where t is clamped to [0,1]. Similar to lerp. Good for fading & animations. */
surgescript_var_t* fun_smoothstep(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    float a = surgescript_var_get_number(param[0]);
    float b = surgescript_var_get_number(param[1]);
    float t = surgescript_var_get_number(param[2]);

    t = (t >= 0.0f) ? (t <= 1.0f ? t : 1.0f) : 0.0f;
    t = (t * t) * (3.0f - 2.0f * t);

    return surgescript_var_set_number(surgescript_var_create(), a * (1.0f - t) + b * t);
}