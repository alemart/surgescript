/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2024 Alexandre Martins <alemartf(at)gmail(dot)com>
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
void surgescript_sslib_register_arguments(struct surgescript_vm_t* vm);
void surgescript_sslib_register_application(struct surgescript_vm_t* vm);
void surgescript_sslib_register_system(struct surgescript_vm_t* vm);
void surgescript_sslib_register_boolean(struct surgescript_vm_t* vm);
void surgescript_sslib_register_number(struct surgescript_vm_t* vm);
void surgescript_sslib_register_string(struct surgescript_vm_t* vm);
void surgescript_sslib_register_console(struct surgescript_vm_t* vm);
void surgescript_sslib_register_math(struct surgescript_vm_t* vm);
void surgescript_sslib_register_dictionary(struct surgescript_vm_t* vm);
void surgescript_sslib_register_time(struct surgescript_vm_t* vm);
void surgescript_sslib_register_date(struct surgescript_vm_t* vm);
void surgescript_sslib_register_temp(struct surgescript_vm_t* vm);
void surgescript_sslib_register_gc(struct surgescript_vm_t* vm);
void surgescript_sslib_register_tagsystem(struct surgescript_vm_t* vm);
void surgescript_sslib_register_surgescript(struct surgescript_vm_t* vm);
void surgescript_sslib_register_plugin(struct surgescript_vm_t* vm);

#endif