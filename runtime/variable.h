/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/variable.h
 * SurgeScript variables
 */

#ifndef _SURGESCRIPT_RUNTIME_VARIABLE_H
#define _SURGESCRIPT_RUNTIME_VARIABLE_H

#include <stdlib.h>

/* some types */
typedef enum surgescript_vartype_t surgescript_vartype_t;
typedef struct surgescript_var_t surgescript_var_t;

/* the variable */
struct surgescript_var_t;

/* possible variable types */
enum surgescript_vartype_t {
    SSVAR_NULL,
    SSVAR_BOOL,
    SSVAR_NUMBER,
    SSVAR_STRING,
    SSVAR_OBJECTHANDLE
};

/* boolean values */
extern const int SSVAR_TRUE;
extern const int SSVAR_FALSE;





/* public methods */

/* create & destroy variables */
surgescript_var_t* surgescript_var_create();
surgescript_var_t* surgescript_var_destroy(surgescript_var_t* var);

/* retrieve the value stored in a variable */
int surgescript_var_get_bool(const surgescript_var_t* var);
float surgescript_var_get_number(const surgescript_var_t* var);
char* surgescript_var_get_string(const surgescript_var_t* var); /* warning: allocates a new buffer; you have to ssfree() this */
unsigned surgescript_var_get_objecthandle(const surgescript_var_t* var);

/* sets the value of a variable */
surgescript_var_t* surgescript_var_set_null(surgescript_var_t* var);
surgescript_var_t* surgescript_var_set_bool(surgescript_var_t* var, int boolean);
surgescript_var_t* surgescript_var_set_number(surgescript_var_t* var, float number);
surgescript_var_t* surgescript_var_set_string(surgescript_var_t* var, const char* string);
surgescript_var_t* surgescript_var_set_objecthandle(surgescript_var_t* var, unsigned handle);

/* misc */
surgescript_vartype_t surgescript_var_type(const surgescript_var_t* var);
surgescript_var_t* surgescript_var_copy(surgescript_var_t* dst, const surgescript_var_t* src); /* similar to strcpy */
surgescript_var_t* surgescript_var_clone(const surgescript_var_t* var); /* similar to strdup */
char* surgescript_var_to_string(const surgescript_var_t* var, char* buf, size_t bufsize); /* copies var to buf and returns buf, converting var to string if necessary (similar to itoa / strncpy) */
int surgescript_var_compare(const surgescript_var_t* a, const surgescript_var_t* b); /* similar to strcmp */

#endif
