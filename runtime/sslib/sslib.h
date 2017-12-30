/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/sslib/sslib.h
 * SurgeScript Standard Library
 */

#ifndef _SURGESCRIPT_RUNTIME_STDLIB_STDLIB_H
#define _SURGESCRIPT_RUNTIME_STDLIB_STDLIB_H

/* forward declarations */
struct surgescript_vm_t;

/* Register common methods to all objects */
void surgescript_sslib_register_object(struct surgescript_vm_t* vm);
void surgescript_sslib_register_array(struct surgescript_vm_t* vm);
void surgescript_sslib_register_application(struct surgescript_vm_t* vm);
void surgescript_sslib_register_system(struct surgescript_vm_t* vm);
void surgescript_sslib_register_boolean(struct surgescript_vm_t* vm);
void surgescript_sslib_register_number(struct surgescript_vm_t* vm);
void surgescript_sslib_register_string(struct surgescript_vm_t* vm);
void surgescript_sslib_register_console(struct surgescript_vm_t* vm);
void surgescript_sslib_register_transform2d(struct surgescript_vm_t* vm);
void surgescript_sslib_register_math(struct surgescript_vm_t* vm);
void surgescript_sslib_register_dictionary(struct surgescript_vm_t* vm);
void surgescript_sslib_register_time(struct surgescript_vm_t* vm);
void surgescript_sslib_register_temp(struct surgescript_vm_t* vm);
void surgescript_sslib_register_gc(struct surgescript_vm_t* vm);

#endif