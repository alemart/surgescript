/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/symtable.c
 * SurgeScript Compiler: symbol table
 */

#include <stdbool.h>
#include <string.h>
#include "symtable.h"
#include "../runtime/program.h"
#include "../runtime/object_manager.h"
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
static void read_system_object(const char* symbol, surgescript_program_t* program, unsigned k);
static void write_system_object(const char* symbol, surgescript_program_t* program, unsigned k);

/* vtable for the entries of the symbol table */
struct surgescript_symtable_entry_vtable_t
{
    void (*read)(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
    void (*write)(surgescript_symtable_entry_t* entry, surgescript_program_t* program, unsigned k);
};
static const surgescript_symtable_entry_vtable_t heapvt = { read_from_heap, write_to_heap };
static const surgescript_symtable_entry_vtable_t stackvt = { read_from_stack, write_to_stack };

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
    return (surgescript_objectmanager_system_object(NULL, symbol) != surgescript_objectmanager_null(NULL));
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
    else if(surgescript_objectmanager_system_object(NULL, symbol))
        write_system_object(symbol, program, k);
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
    else if(surgescript_objectmanager_system_object(NULL, symbol))
        read_system_object(symbol, program, k);
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
 * pushes an address (equivalent to symbol) to the stack
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

void read_system_object(const char* symbol, surgescript_program_t* program, unsigned k)
{
    surgescript_objecthandle_t addr = surgescript_objectmanager_system_object(NULL, symbol);
    surgescript_program_add_line(program, SSOP_MOVO, SSOPu(k), SSOPu(addr));
}

void write_system_object(const char* symbol, surgescript_program_t* program, unsigned k)
{
    ; /* do nothing; system objects are read-only */
}