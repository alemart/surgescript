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
 * runtime/symtable.h
 * SurgeScript Compiler: symbol table
 */

#ifndef _SURGESCRIPT_COMPILER_SYMTABLE_H
#define _SURGESCRIPT_COMPILER_SYMTABLE_H

#include "../runtime/heap.h"
#include "../runtime/stack.h"

typedef struct surgescript_symtable_t surgescript_symtable_t;
struct surgescript_program_t;

/* create & destroy a symbol table */
surgescript_symtable_t* surgescript_symtable_create(surgescript_symtable_t* parent); /* parent symbolizes the parent scope and may be NULL */
surgescript_symtable_t* surgescript_symtable_destroy(surgescript_symtable_t* symtable);

/* put a new symbol on the table */
void surgescript_symtable_put_heap_symbol(surgescript_symtable_t* symtable, const char* symbol, surgescript_heapptr_t address);
void surgescript_symtable_put_stack_symbol(surgescript_symtable_t* symtable, const char* symbol, surgescript_stackptr_t address);
void surgescript_symtable_put_accessor_symbol(surgescript_symtable_t* symtable, const char* symbol);
void surgescript_symtable_put_plugin_symbol(surgescript_symtable_t* symtable, const char* path);
void surgescript_symtable_put_static_symbol(surgescript_symtable_t* symtable, const char* symbol);

/* emit surgescript program code so that t[k] is written to the address of the symbol */
void surgescript_symtable_emit_write(surgescript_symtable_t* symtable, const char* symbol, struct surgescript_program_t* program, unsigned k);

/* emit surgescript program code so that the content stored by the symbol is read to t[k] */
void surgescript_symtable_emit_read(surgescript_symtable_t* symtable, const char* symbol, struct surgescript_program_t* program, unsigned k);

/* does the table have a certain symbol? */
bool surgescript_symtable_has_symbol(surgescript_symtable_t* symtable, const char* symbol);
bool surgescript_symtable_has_local_symbol(surgescript_symtable_t* symtable, const char* symbol);

/* count the number of symbols */
int surgescript_symtable_local_count(surgescript_symtable_t* symtable); /* not including parents */
int surgescript_symtable_count(surgescript_symtable_t* symtable); /* including parents */

/* does this table have a parent? */
bool surgescript_symtable_has_parent(surgescript_symtable_t* symtable);

#endif