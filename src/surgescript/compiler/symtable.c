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
 * runtime/symtable.c
 * SurgeScript Compiler: symbol table
 */

#include <stdbool.h>
#include <string.h>
#include "symtable.h"
#include "../runtime/program.h"
#include "../runtime/object_manager.h"
#include "../runtime/object.h"
#include "../util/ssarray.h"
#include "../util/util.h"

/* utilities */
typedef struct surgescript_symtable_entry_t surgescript_symtable_entry_t;
typedef struct surgescript_symtable_entry_vtable_t surgescript_symtable_entry_vtable_t;
static int indexof_symbol(surgescript_symtable_t* symtable, const char* symbol);
static void read_from_heap(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
static void read_from_stack(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
static void write_to_heap(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
static void write_to_stack(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
static void read_from_getter(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
static void write_to_setter(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
static void read_plugin(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
static void write_plugin(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
static void read_static(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
static void write_static(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
static char* pack_plugin_path(const char* path);
static char* unpack_plugin_path(const char* symbol);
static const char* plugin_symbol(const char* path);

/* vtable for the entries of the symbol table */
struct surgescript_symtable_entry_vtable_t
{
    void (*read)(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
    void (*write)(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
};
static const surgescript_symtable_entry_vtable_t heapvt = { read_from_heap, write_to_heap };
static const surgescript_symtable_entry_vtable_t stackvt = { read_from_stack, write_to_stack };
static const surgescript_symtable_entry_vtable_t accvt = { read_from_getter, write_to_setter };
static const surgescript_symtable_entry_vtable_t pluginvt = { read_plugin, write_plugin };
static const surgescript_symtable_entry_vtable_t staticvt = { read_static, write_static };

/* a symbol table entry */
struct surgescript_symtable_entry_t
{
    char* symbol;
    union {
        surgescript_heapptr_t heapaddr;
        surgescript_stackptr_t stackaddr;
    };
    const surgescript_symtable_entry_vtable_t* vtable;
};

/* what's a symbol table? */
struct surgescript_symtable_t
{
    surgescript_symtable_t* parent; /* pointer to its parent (parent scope) */
    SSARRAY(surgescript_symtable_entry_t, entry); /* an entry of the symbol table */
};


/* public api */

/*
 * surgescript_symtable_create()
 * Creates a symbol table
 */
surgescript_symtable_t* surgescript_symtable_create(surgescript_symtable_t* parent)
{
    surgescript_symtable_t* symtable = ssmalloc(sizeof *symtable);
    symtable->parent = parent;
    ssarray_init(symtable->entry);
    return symtable;
}

/*
 * surgescript_symtable_destroy()
 * Destroys an existing symbol table
 */
surgescript_symtable_t* surgescript_symtable_destroy(surgescript_symtable_t* symtable)
{
    for(int i = 0; i < ssarray_length(symtable->entry); i++)
        ssfree(symtable->entry[i].symbol);

    ssarray_release(symtable->entry);
    return ssfree(symtable); /* don't mess with the parent */
}

/*
 * surgescript_symtable_has_symbol()
 * Does the symbol table have the informed symbol?
 */
bool surgescript_symtable_has_symbol(surgescript_symtable_t* symtable, const char* symbol)
{
    while(symtable) {
        if(indexof_symbol(symtable, symbol) >= 0)
            return true;
        symtable = symtable->parent;
    }
    return false;
}

/*
 * surgescript_symtable_has_local_symbol()
 * Does the symbol table have the informed symbol locally?
 * Perform a shallow search, i.e., don't search on the parent of this table
 */
bool surgescript_symtable_has_local_symbol(surgescript_symtable_t* symtable, const char* symbol)
{
    return indexof_symbol(symtable, symbol) >= 0;
}

/*
 * surgescript_symtable_put_heap_symbol()
 * Puts a symbol on the table that is stored on the heap
 */
void surgescript_symtable_put_heap_symbol(surgescript_symtable_t* symtable, const char* symbol, surgescript_heapptr_t address)
{
    if(indexof_symbol(symtable, symbol) < 0) {
        char* symname = ssstrdup(symbol);
        surgescript_symtable_entry_t entry = { .symbol = symname, .heapaddr = address, .vtable = &heapvt };
        ssarray_push(symtable->entry, entry);
    }
    else
        ssfatal("Compile Error: duplicate entry of symbol \"%s\".", symbol);
}

/*
 * surgescript_symtable_put_stack_symbol()
 * Puts a symbol on the table that is stored on the stack
 */
void surgescript_symtable_put_stack_symbol(surgescript_symtable_t* symtable, const char* symbol, surgescript_stackptr_t address)
{
    if(indexof_symbol(symtable, symbol) < 0) {
        char* symname = ssstrdup(symbol);
        surgescript_symtable_entry_t entry = { .symbol = symname, .stackaddr = address, .vtable = &stackvt };
        ssarray_push(symtable->entry, entry);
    }
    else
        ssfatal("Compile Error: duplicate entry of symbol \"%s\".", symbol);
}

/*
 * surgescript_symtable_put_accessor_symbol()
 * Puts a symbol on the table that is a syntax sugar to accessor function calls
 */
void surgescript_symtable_put_accessor_symbol(surgescript_symtable_t* symtable, const char* symbol)
{
    if(indexof_symbol(symtable, symbol) < 0) {
        char* symname = ssstrdup(symbol);
        surgescript_symtable_entry_t entry = { .symbol = symname, .vtable = &accvt };
        ssarray_push(symtable->entry, entry);
    }
    else
        ssfatal("Compile Error: duplicate entry of symbol \"%s\".", symbol);
}

/*
 * surgescript_symtable_put_plugin_symbol()
 * Puts a symbol that is a reference to a plugin object
 */
void surgescript_symtable_put_plugin_symbol(surgescript_symtable_t* symtable, const char* path)
{
    if(indexof_symbol(symtable, plugin_symbol(path)) < 0) {
        char* symname = pack_plugin_path(path);
        surgescript_symtable_entry_t entry = { .symbol = symname, .vtable = &pluginvt };
        ssarray_push(symtable->entry, entry);
    }
    else
        ssfatal("Compile Error: found duplicate symbol \"%s\" when importing \"%s\".", plugin_symbol(path), path);
}

/*
 * surgescript_symtable_put_static_symbol()
 * Puts a symbol that references a built-in object
 */
void surgescript_symtable_put_static_symbol(surgescript_symtable_t* symtable, const char* symbol)
{
    if(indexof_symbol(symtable, symbol) < 0) {
        char* symname = ssstrdup(symbol);
        surgescript_symtable_entry_t entry = { .symbol = symname, .vtable = &staticvt };
        ssarray_push(symtable->entry, entry);
    }
    else
        ssfatal("Compile Error: duplicate entry of symbol \"%s\".", symbol);
}

/*
 * surgescript_symtable_emit_write()
 * Emits SurgeScript program code so that the contents of t[k] are written to the memory location pointed by the symbol
 */
void surgescript_symtable_emit_write(surgescript_symtable_t* symtable, const char* symbol, surgescript_program_t* program, unsigned k)
{
    int j;

    if((j = indexof_symbol(symtable, symbol)) >= 0) {
        surgescript_symtable_entry_t* entry = &(symtable->entry[j]);
        entry->vtable->write(entry, program, k);
    }
    else if(symtable->parent)
        surgescript_symtable_emit_write(symtable->parent, symbol, program, k);
    else
        ssfatal("Compile Error: undefined symbol \"%s\".", symbol);
}

/*
 * surgescript_symtable_emit_read()
 * Emits SurgeScript program code so that the contents of the memory location pointed by the symbol are read to t[k]
 */
void surgescript_symtable_emit_read(surgescript_symtable_t* symtable, const char* symbol, surgescript_program_t* program, unsigned k)
{
    int j;

    if((j = indexof_symbol(symtable, symbol)) >= 0) {
        surgescript_symtable_entry_t* entry = &(symtable->entry[j]);
        entry->vtable->read(entry, program, k);
    }
    else if(symtable->parent)
        surgescript_symtable_emit_read(symtable->parent, symbol, program, k);
    else
        ssfatal("Compile Error: undefined symbol \"%s\".", symbol);
}

/*
 * surgescript_symtable_local_count()
 * How many symbols does this table have?
 */
int surgescript_symtable_local_count(surgescript_symtable_t* symtable)
{
    return ssarray_length(symtable->entry);
}

/*
 * surgescript_symtable_count()
 * How many symbols does this table have, including the parent (and its parent, and so on) ?
 */
int surgescript_symtable_count(surgescript_symtable_t* symtable)
{
    return symtable ? ssarray_length(symtable->entry) + surgescript_symtable_count(symtable->parent) : 0;
}


/*
 * surgescript_symtable_has_parent()
 * Does this table have a parent?
 */
bool surgescript_symtable_has_parent(surgescript_symtable_t* symtable)
{
    return symtable->parent != NULL;
}

/*
 * surgescript_symtable_push_addr()
 * Pushes an address (equivalent to symbol) to the stack
 */
void surgescript_symtable_push_addr(surgescript_symtable_t* symtable, const char* symbol, surgescript_program_t* program)
{
    int j;

    if((j = indexof_symbol(symtable, symbol)) >= 0) {
        surgescript_symtable_entry_t* entry = &(symtable->entry[j]);
        surgescript_heapptr_t address = entry->heapaddr; /* a heap address is pushed */
        surgescript_program_add_line(program, SSOP_MOVU, SSOPu(0), SSOPu(address));
        surgescript_program_add_line(program, SSOP_PUSH, SSOPu(0), SSOPu(0));
    }
    else if(symtable->parent)
        surgescript_symtable_push_addr(symtable->parent, symbol, program);
    else
        ssfatal("Compile Error: can't push address - undefined symbol \"%s\".", symbol);
}



/* private stuff */

/* returns i such that symtable->entry[i].symbol == symbol, or -1 if not found */
int indexof_symbol(surgescript_symtable_t* symtable, const char* symbol)
{
    for(int i = 0; i < ssarray_length(symtable->entry); i++) {
        if(strcmp(symtable->entry[i].symbol, symbol) == 0)
            return i;
    }

    return -1;
}

void read_from_heap(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k)
{
    surgescript_heapptr_t address = entry->heapaddr;
    surgescript_program_add_line(program, SSOP_PEEK, SSOPu(k), SSOPu(address));
}

void read_from_stack(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k)
{
    surgescript_stackptr_t address = entry->stackaddr;
    surgescript_program_add_line(program, SSOP_SPEEK, SSOPu(k), SSOPi(address));
}

void write_to_heap(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k)
{
    surgescript_heapptr_t address = entry->heapaddr;
    surgescript_program_add_line(program, SSOP_POKE, SSOPu(k), SSOPu(address));
}

void write_to_stack(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k)
{
    surgescript_stackptr_t address = entry->stackaddr;
    surgescript_program_add_line(program, SSOP_SPOKE, SSOPu(k), SSOPi(address));
}

void read_static(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k)
{
    const char* symbol = entry->symbol;
    surgescript_objecthandle_t addr = surgescript_objectmanager_system_object(NULL, symbol);
    if(addr == surgescript_objectmanager_null(NULL)) {
        /* no static address found; look for a direct child of the root */
        surgescript_objecthandle_t root = surgescript_objectmanager_root(NULL);
        surgescript_program_add_line(program, SSOP_MOVO, SSOPu(0), SSOPu(root));
        surgescript_program_add_line(program, SSOP_PUSH, SSOPu(0), SSOPu(0));
        surgescript_program_add_line(program, SSOP_MOVS, SSOPu(0), SSOPu(surgescript_program_add_text(program, symbol)));
        surgescript_program_add_line(program, SSOP_PUSH, SSOPu(0), SSOPu(0));
        surgescript_program_add_line(program, SSOP_CALL, SSOPu(surgescript_program_add_text(program, "child")), SSOPu(1));
        surgescript_program_add_line(program, SSOP_POPN, SSOPu(2), SSOPu(0));

        if(k != 0)
            surgescript_program_add_line(program, SSOP_MOV, SSOPu(k), SSOPu(0));
    }
    else {
        /* static address found at addr */
        surgescript_program_add_line(program, SSOP_MOVO, SSOPu(k), SSOPu(addr));
    }
}

void write_static(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k)
{
    ; /* do nothing; static objects can't have their references messed up */
}

void read_plugin(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k)
{
    surgescript_objecthandle_t plugin_object = surgescript_objectmanager_system_object(NULL, "Plugin");
    char* path = unpack_plugin_path(entry->symbol);
    char* next, *tok = path, *getter;

    /* generate the bytecode to access the plugin */
    surgescript_program_add_line(program, SSOP_MOVO, SSOPu(0), SSOPu(plugin_object));
    while(next = strchr(tok, '.')) {
        *next = 0;
        surgescript_program_add_line(program, SSOP_PUSH, SSOPu(0), SSOPu(0));
        surgescript_program_add_line(program, SSOP_CALL, SSOPu(
            surgescript_program_add_text(program, getter = surgescript_util_camelcaseprefix("get", tok))
        ), SSOPu(0));
        surgescript_program_add_line(program, SSOP_POPN, SSOPu(1), SSOPu(0));
        tok = next + 1;
        ssfree(getter);
    }
    surgescript_program_add_line(program, SSOP_PUSH, SSOPu(0), SSOPu(0));
    surgescript_program_add_line(program, SSOP_CALL, SSOPu(
        surgescript_program_add_text(program, getter = surgescript_util_camelcaseprefix("get", tok))
    ), SSOPu(0));
    surgescript_program_add_line(program, SSOP_POPN, SSOPu(1), SSOPu(0));

    /* set t[k] to the address of the plugin */
    if(k != 0)
        surgescript_program_add_line(program, SSOP_MOV, SSOPu(k), SSOPu(0));

    /* release */
    ssfree(getter);
    ssfree(path);
}

void write_plugin(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k)
{
    ; /* do nothing; plugin objects can't have their references changed */
}

void read_from_getter(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k)
{
    char* fun_name = surgescript_util_camelcaseprefix("get", entry->symbol);
    surgescript_program_add_line(program, SSOP_SELF, SSOPu(0), SSOPu(0));
    surgescript_program_add_line(program, SSOP_PUSH, SSOPu(0), SSOPu(0));
    surgescript_program_add_line(program, SSOP_CALL, SSOPu(surgescript_program_add_text(program, fun_name)), SSOPu(0));
    surgescript_program_add_line(program, SSOP_POPN, SSOPu(1), SSOPu(0));
    ssfree(fun_name);

    if(k != 0)
        surgescript_program_add_line(program, SSOP_MOV, SSOPu(k), SSOPu(0));
}

void write_to_setter(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k)
{
    char* fun_name = surgescript_util_camelcaseprefix("set", entry->symbol);
    unsigned w = (k + 1) % 2; /* w in {0, 1} is different than k */
    surgescript_program_add_line(program, SSOP_SELF, SSOPu(w), SSOPu(0));
    surgescript_program_add_line(program, SSOP_PUSH, SSOPu(w), SSOPu(0));
    surgescript_program_add_line(program, SSOP_PUSH, SSOPu(k), SSOPu(0));
    surgescript_program_add_line(program, SSOP_CALL, SSOPu(surgescript_program_add_text(program, fun_name)), SSOPu(1));
    surgescript_program_add_line(program, SSOP_POPN, SSOPu(2), SSOPu(0));
    ssfree(fun_name);
}

char* pack_plugin_path(const char* path)
{
    const char* symbol = plugin_symbol(path);
    size_t symbol_len = strlen(symbol);
    char* buf = ssmalloc((3 + symbol_len + strlen(path)) * sizeof(*buf));
    strcpy(buf, symbol);
    strcpy(buf + symbol_len + 1, path);
    return buf;
}

char* unpack_plugin_path(const char* symbol)
{
    size_t symbol_len = strlen(symbol);
    return ssstrdup(symbol + symbol_len + 1);
}

const char* plugin_symbol(const char* path)
{
    const char* symbol = strrchr(path, '.');
    return symbol ? symbol + 1 : path;
}