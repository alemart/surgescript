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
 * runtime/sslib/math.c
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
static surgescript_var_t* fun_getepsilon(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getpi(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getdeg2rad(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getrad2deg(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getinfinity(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getnan(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
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
static surgescript_var_t* fun_mod(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_sign(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_abs(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_min(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_max(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_clamp(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_approximately(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_lerp(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_smoothstep(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* constants */
static const double EPSILON = DBL_EPSILON;
static const double PI = 3.14159265358979323846;
static const double RAD2DEG = 57.29577951308232087684;
static const double DEG2RAD = 0.01745329251994329576;

/*
 * surgescript_sslib_register_math()
 * Register methods
 */
void surgescript_sslib_register_math(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Math", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Math", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Math", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Math", "get_epsilon", fun_getepsilon, 0);
    surgescript_vm_bind(vm, "Math", "get_pi", fun_getpi, 0);
    surgescript_vm_bind(vm, "Math", "get_deg2rad", fun_getdeg2rad, 0);
    surgescript_vm_bind(vm, "Math", "get_rad2deg", fun_getrad2deg, 0);
    surgescript_vm_bind(vm, "Math", "get_infinity", fun_getinfinity, 0);
    surgescript_vm_bind(vm, "Math", "get_NaN", fun_getnan, 0);
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
    surgescript_vm_bind(vm, "Math", "mod", fun_mod, 2);
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

/* constant: NaN (Not-a-Number) */
surgescript_var_t* fun_getnan(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), NAN);
}

/* random(): returns a random number between 0 (inclusive) and 1 (exclusive) */
surgescript_var_t* fun_random(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), rand() / ((double)RAND_MAX + 1.0));
}

/* sin(x): sine of x, x in radians */
surgescript_var_t* fun_sin(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), sin(surgescript_var_get_number(param[0])));
}

/* cos(x): cosine of x, x in radians */
surgescript_var_t* fun_cos(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), cos(surgescript_var_get_number(param[0])));
}

/* tan(x): tangent of x, x in radians */
surgescript_var_t* fun_tan(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), tan(surgescript_var_get_number(param[0])));
}

/* asin(x): arc sin of x, returned in radians */
surgescript_var_t* fun_asin(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), asin(surgescript_var_get_number(param[0])));
}

/* acos(x): arc cosine of x, returned in radians */
surgescript_var_t* fun_acos(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), acos(surgescript_var_get_number(param[0])));
}

/* atan(x): arc tangent of x, returned in radians */
surgescript_var_t* fun_atan(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), atan(surgescript_var_get_number(param[0])));
}

/* atan2(y,x): returns the angle, in radians, between the positive x-axis and the vector (x,y) */
surgescript_var_t* fun_atan2(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double y = surgescript_var_get_number(param[0]);
    double x = surgescript_var_get_number(param[1]);
    return surgescript_var_set_number(surgescript_var_create(), atan2(y, x));
}

/* pow(base, exponent): returns the value of base raised to the power exponent */
surgescript_var_t* fun_pow(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double base = surgescript_var_get_number(param[0]);
    double exponent = surgescript_var_get_number(param[1]);
    return surgescript_var_set_number(surgescript_var_create(), pow(base, exponent));
}

/* sqrt(x): square root of x */
surgescript_var_t* fun_sqrt(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), sqrt(surgescript_var_get_number(param[0])));
}

/* exp(x): e raised to the power of x */
surgescript_var_t* fun_exp(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), exp(surgescript_var_get_number(param[0])));
}

/* log(x): returns the natural logarithm of x */
surgescript_var_t* fun_log(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), log(surgescript_var_get_number(param[0])));
}

/* log10(x): the base-10 logarithm of x */
surgescript_var_t* fun_log10(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), log10(surgescript_var_get_number(param[0])));
}

/* floor(x): the floor of x */
surgescript_var_t* fun_floor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), floor(surgescript_var_get_number(param[0])));
}

/* ceil(x): the ceiling of x */
surgescript_var_t* fun_ceil(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), ceil(surgescript_var_get_number(param[0])));
}

/* round(x): round x to the nearest integer */
surgescript_var_t* fun_round(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double x = surgescript_var_get_number(param[0]); /* round x half away from zero */
    return surgescript_var_set_number(surgescript_var_create(), (x >= 0.0) ? floor(x + 0.5) : ceil(x - 0.5));
}

/* mod(x,y): the remainder of x/y */
surgescript_var_t* fun_mod(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double x = surgescript_var_get_number(param[0]);
    double y = surgescript_var_get_number(param[1]);
    return surgescript_var_set_number(surgescript_var_create(), fmod(x, y));
}

/* sign(x): returns +1 if x is non-negative, or -1 otherwise */
surgescript_var_t* fun_sign(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double x = surgescript_var_get_number(param[0]);
    return surgescript_var_set_number(surgescript_var_create(), (x >= 0.0) ? 1.0 : -1.0);
}

/* abs(x): the absolute value of x */
surgescript_var_t* fun_abs(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), fabs(surgescript_var_get_number(param[0])));
}

/* min(x,y): the minimum between x and y */
surgescript_var_t* fun_min(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double x = surgescript_var_get_number(param[0]);
    double y = surgescript_var_get_number(param[1]);
    return surgescript_var_set_number(surgescript_var_create(), (x < y) ? x : y);
}

/* max(x,y): the maximum between x and y */
surgescript_var_t* fun_max(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double x = surgescript_var_get_number(param[0]);
    double y = surgescript_var_get_number(param[1]);
    return surgescript_var_set_number(surgescript_var_create(), (x >= y) ? x : y);
}

/* clamp(x,min,max): clamps x between min and max */
surgescript_var_t* fun_clamp(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double x = surgescript_var_get_number(param[0]);
    double minval = surgescript_var_get_number(param[1]);
    double maxval = surgescript_var_get_number(param[2]);

    if(minval > maxval) {
        double tmp = minval;
        minval = maxval;
        maxval = tmp;
    }

    return surgescript_var_set_number(surgescript_var_create(), (x >= minval) ? (x <= maxval ? x : maxval) : minval);
}

/* approximately(a,b): returns true if floating points a and b are approximately equal */
surgescript_var_t* fun_approximately(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double a = surgescript_var_get_number(param[0]);
    double b = surgescript_var_get_number(param[1]);
    double fa = fabs(a), fb = fabs(b), fm = (fa >= fb ? fa : fb);
    double eps = EPSILON * (fm >= 1.0 ? fm : 1.0);

    return surgescript_var_set_bool(surgescript_var_create(), (a >= b - eps) && (a <= b + eps));
}

/* lerp(a,b,t): linear interpolation between a and b by t, where t is clamped to the range [0,1] */
surgescript_var_t* fun_lerp(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double a = surgescript_var_get_number(param[0]);
    double b = surgescript_var_get_number(param[1]);
    double t = surgescript_var_get_number(param[2]);

    /* t = clamp01(t) */
    t += (t > 1.0) * (1.0 - t) - (t < 0.0) * t;

    /* When t = 0, returns a. When t = 1, returns b. When t = 0.5, returns (a+b) / 2 */
    return surgescript_var_set_number(surgescript_var_create(), (b - a) * t + a);
}


/* smoothstep(a,b,t): interpolates smoothly between a and b by t, where t is clamped to [0,1]. Similar to lerp. Good for fading & animations. */
surgescript_var_t* fun_smoothstep(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double a = surgescript_var_get_number(param[0]);
    double b = surgescript_var_get_number(param[1]);
    double t = surgescript_var_get_number(param[2]);

    t += (t > 1.0) * (1.0 - t) - (t < 0.0) * t;
    t = (t * t) * (3.0 - 2.0 * t);

    return surgescript_var_set_number(surgescript_var_create(), (b - a) * t + a);
}