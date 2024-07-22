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
 * compiler/parser.c
 * SurgeScript compiler: syntax analyzer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include "parser.h"
#include "lexer.h"
#include "token.h"
#include "nodecontext.h"
#include "symtable.h"
#include "asm.h"
#include "../runtime/object.h"
#include "../runtime/object_manager.h"
#include "../runtime/tag_system.h"
#include "../runtime/program_pool.h"
#include "../runtime/program.h"
#include "../util/util.h"
#include "../util/ssarray.h"

/* the parser */
struct surgescript_parser_t
{
    surgescript_token_t* lookahead; /* scanned symbol */
    surgescript_token_t* previous; /* previous symbol */
    surgescript_lexer_t* lexer; /* lexer */
    char* filename; /* current filename */
    surgescript_programpool_t* program_pool; /* reference to the program pool */
    surgescript_tagsystem_t* tag_system; /* reference to the tag system */
    surgescript_symtable_t* base_table; /* valid symbols in the current file (code unit) */
    SSARRAY(char*, known_plugins); /* known plugins in all files (the names of the objects) */
    surgescript_parser_flags_t flags;
};

/* helpers */
static void parse(surgescript_parser_t* parser);
static inline bool got_type(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
static inline bool got_any_type(surgescript_parser_t* parser, const surgescript_tokentype_t symbol[], int number_of_symbols);
static inline bool has_token(surgescript_parser_t* parser);
static void match(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
static bool optmatch(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
static void match_exactly(surgescript_parser_t* parser, surgescript_tokentype_t symbol, const char* lexeme);
static void unmatch(surgescript_parser_t* parser);
static void expect(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
static void expect_something(surgescript_parser_t* parser);
static void expect_exactly(surgescript_parser_t* parser, surgescript_tokentype_t symbol, const char* lexeme);
static void unexpected_symbol(surgescript_parser_t* parser);
static void validate_object(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_var_t* empty_main(surgescript_object_t* object, const surgescript_var_t* param[], int num_params);
static void create_getter(surgescript_parser_t* parser, surgescript_nodecontext_t context, const char* identifier);
static void create_setter(surgescript_parser_t* parser, surgescript_nodecontext_t context, const char* identifier);
static void import_public_vars(surgescript_parser_t* parser, surgescript_nodecontext_t context, const char* object_name);
static void make_accessor(const char* fun_name, void* symtable);
static void init_plugins_list(surgescript_parser_t* parser);
static void add_to_plugins_list(surgescript_parser_t* parser, const char* plugin_name);
static void release_plugins_list(surgescript_parser_t* parser);
static surgescript_symtable_t* configure_base_table(surgescript_symtable_t* base_table);
static void read_annotations(surgescript_parser_t* parser, char*** annotations);
static void release_annotations(char** annotations);
static void process_annotations(surgescript_parser_t* parser, char** annotations, const char* object_name);
static surgescript_program_t* make_file_program(const char* source_file);
static void pick_non_natives(const char* program_name, void* data);
static void remove_object_definition(surgescript_programpool_t* pool, const char* object_name);
static bool forbid_duplicates(const surgescript_parser_t* parser, const char* object_name);
static bool is_state_context(surgescript_nodecontext_t context);
static char* randstr(char* buf, size_t size);
static bool is_large_name(const char* name);
static bool is_valid_name(const char* name);

/* non-terminals */
static void importlist(surgescript_parser_t* parser);
static void objectlist(surgescript_parser_t* parser);
static void object(surgescript_parser_t* parser);
static void objectdecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void qualifiers(surgescript_parser_t* parser, surgescript_nodecontext_t context);

static void vardecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void vardecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void statedecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void statedecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void fundecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void fundecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);

static void expr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void assignexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void conditionalexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void logicalorexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void logicalorexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void logicalandexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void logicalandexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void equalityexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void equalityexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void relationalexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void relationalexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void additiveexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void additiveexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void multiplicativeexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void multiplicativeexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void unaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void postfixexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void postfixexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void funcallexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context, const char* fun_name);
static void dictgetexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void lambdacall(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void primaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void constant(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void arrayexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void dictexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);

static void stmtlist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static bool stmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void blockstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void exprstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void condstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void switchstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void loopstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void jumpstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void retstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void miscstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);

static void constexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void signedconst(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void signednum(surgescript_parser_t* parser, surgescript_nodecontext_t context);




/* public api */


/*
 * surgescript_parser_create()
 * Creates a new parser
 */
surgescript_parser_t* surgescript_parser_create(surgescript_programpool_t* program_pool, surgescript_tagsystem_t* tag_system)
{
    surgescript_parser_t* parser = ssmalloc(sizeof *parser);
    parser->lookahead = parser->previous = NULL;
    parser->lexer = surgescript_lexer_create();
    parser->filename = ssstrdup("<unspecified>");
    parser->program_pool = program_pool;
    parser->tag_system = tag_system;
    parser->base_table = NULL;
    parser->flags = SSPARSER_DEFAULTS;
    init_plugins_list(parser);
    return parser;
}

/*
 * surgescript_parser_destroy()
 * Destroys an existing parser
 */
surgescript_parser_t* surgescript_parser_destroy(surgescript_parser_t* parser)
{
    ssfree(parser->filename);
    surgescript_lexer_destroy(parser->lexer);
    if(parser->lookahead)
        surgescript_token_destroy(parser->lookahead);
    if(parser->previous)
        surgescript_token_destroy(parser->previous);
    if(parser->base_table)
        surgescript_symtable_destroy(parser->base_table);
    release_plugins_list(parser);
    return ssfree(parser);
}


/*
 * surgescript_parser_parse()
 * Parse a script stored in memory
 * You may pass NULL to the (optional) filename if the script doesn't belong to a (possibly virtual) file
 */
bool surgescript_parser_parse(surgescript_parser_t* parser, const char* code_in_memory, const char* filename)
{
    ssfree(parser->filename);
    parser->filename = ssstrdup(filename != NULL ? filename : "<memory>");
    surgescript_lexer_set(parser->lexer, code_in_memory);
    parse(parser);
    return true;
}



/*
 * surgescript_parser_filename()
 * Returns the file being processed
 */
/*
const char* surgescript_parser_filename(surgescript_parser_t* parser)
{
    return parser->filename;
}
*/



/*
 * surgescript_parser_foreach_plugin()
 * Calls fun() for each plugin found in any parsed script
 */
void surgescript_parser_foreach_plugin(surgescript_parser_t* parser, void* data, void (*fun)(const char*,void*))
{
    for(int i = 0; i < ssarray_length(parser->known_plugins); i++)
        fun(parser->known_plugins[i], data);
}



/*
 * surgescript_parser_set_flags()
 * Set parser options (flags)
 */
void surgescript_parser_set_flags(surgescript_parser_t* parser, surgescript_parser_flags_t flags)
{
    parser->flags = flags;
}


/*
 * surgescript_parser_get_flags()
 * Get parser flags
 */
surgescript_parser_flags_t surgescript_parser_get_flags(surgescript_parser_t* parser)
{
    return parser->flags;
}


/* privates & helpers */



/* parses a script */
void parse(surgescript_parser_t* parser)
{
    parser->base_table = configure_base_table(surgescript_symtable_create(NULL));
    parser->lookahead = surgescript_lexer_scan(parser->lexer); /* grab first symbol */
    importlist(parser);
    objectlist(parser);
    parser->base_table = surgescript_symtable_destroy(parser->base_table);
}

/* does the lookahead symbol have the given type? */
bool got_type(surgescript_parser_t* parser, surgescript_tokentype_t symbol)
{
    return parser->lookahead && surgescript_token_type(parser->lookahead) == symbol;
}

/* does the lookahead symbol have any of the given types? */
bool got_any_type(surgescript_parser_t* parser, const surgescript_tokentype_t symbol[], int number_of_symbols)
{
    for(int i = 0; i < number_of_symbols; i++) {
        if(got_type(parser, symbol[i]))
            return true;
    }
    return false;
}

/* match a symbol; throw a fatal error if the symbol is not matched */
void match(surgescript_parser_t* parser, surgescript_tokentype_t symbol)
{
    if(got_type(parser, symbol)) {
        if(parser->previous)
            surgescript_token_destroy(parser->previous);
        parser->previous = parser->lookahead;
        parser->lookahead = surgescript_lexer_scan(parser->lexer); /* grab next symbol */
    }
    else
        expect(parser, symbol);
}

/* match the given symbol or the empty symbol */
bool optmatch(surgescript_parser_t* parser, surgescript_tokentype_t symbol)
{
    if(got_type(parser, symbol)) {
        match(parser, symbol);
        return true;
    }
    else
        return false;
}

/* match exactly the given symbol with the given lexeme */
void match_exactly(surgescript_parser_t* parser, surgescript_tokentype_t symbol, const char* lexeme)
{
    if(got_type(parser, symbol) && strcmp(surgescript_token_lexeme(parser->lookahead), lexeme) == 0)
        match(parser, symbol);
    else
        expect_exactly(parser, symbol, lexeme); /* error */
}

/* puts the last token back into the lexer */
void unmatch(surgescript_parser_t* parser)
{
    if(parser->previous && surgescript_lexer_unscan(parser->lexer, parser->previous)) {
        surgescript_token_destroy(parser->lookahead);
        parser->lookahead = surgescript_lexer_scan(parser->lexer);
    }
    else if(parser->previous)
        ssfatal("Parse Error: can\'t unmatch symbol \"%s\" on %s:%d.", surgescript_tokentype_name(surgescript_token_type(parser->previous)), parser->filename, surgescript_token_linenumber(parser->previous));
    else
        ssfatal("Parse Error: can\'t unmatch symbol on %s.", parser->filename);
}

/* throw an error if the lookahead is not of the expected type */
void expect(surgescript_parser_t* parser, surgescript_tokentype_t symbol)
{
    if(parser->lookahead && surgescript_token_type(parser->lookahead) != symbol) {
        ssfatal(
            "Parse Error: expected \"%s\" on %s:%d.",
            surgescript_tokentype_name(symbol),
            parser->filename,
            surgescript_token_linenumber(parser->lookahead)
        );
    }
    else if(NULL == parser->lookahead) {
         ssfatal(
            "Parse Error: unexpected end of the file on %s (did you forget a \"%s\"?)",
            parser->filename,
            surgescript_tokentype_name(symbol)
        );
    }
}

/* check if we have reached the end of the file; throw an error if so */
void expect_something(surgescript_parser_t* parser)
{
    if(NULL == parser->lookahead) {
         ssfatal(
            "Parse Error: unexpected end of file on %s.",
            parser->filename
        );
    }   
}

/* expect exactly the given token, with the given lexeme */
void expect_exactly(surgescript_parser_t* parser, surgescript_tokentype_t symbol, const char* lexeme)
{
    if(parser->lookahead && (surgescript_token_type(parser->lookahead) != symbol || strcmp(surgescript_token_lexeme(parser->lookahead), lexeme) != 0)) {
        ssfatal(
            "Parse Error: expected \"%s\" on %s:%d.",
            lexeme,
            parser->filename,
            surgescript_token_linenumber(parser->lookahead)
        );
    }
    else if(NULL == parser->lookahead) {
         ssfatal(
            "Parse Error: unexpected end of the file on %s (did you forget a \"%s\"?)",
            parser->filename,
            lexeme
        );
    }
}

/* throw an error: unexpected symbol */
void unexpected_symbol(surgescript_parser_t* parser)
{
     if(parser->lookahead) {
        ssfatal(
            "Parse Error: unexpected \"%s\" on %s:%d.",
            surgescript_token_lexeme(parser->lookahead),
            parser->filename,
            surgescript_token_linenumber(parser->lookahead)
        );
    }
    else
        expect_something(parser);
}

/* is there a token to be analyzed? */
bool has_token(surgescript_parser_t* parser)
{
    return parser->lookahead != NULL;
}

/* is the given object (in the given context) all right? */
void validate_object(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    /* found an invalid symbol? */
    if(!got_type(parser, SSTOK_RCURLY))
        unexpected_symbol(parser);

    /* do we have a "main" state? */
    if(!surgescript_programpool_exists(parser->program_pool, context.object_name, "state:main")) {
        if(strcmp(context.object_name, "Application") != 0) {
            surgescript_program_t* cprogram = surgescript_program_create_native(0, empty_main);
            surgescript_programpool_put(parser->program_pool, context.object_name, "state:main", cprogram);
            /*sslog("Object \"%s\" in \"%s\" has omitted its \"main\" state and will be disabled.", context.object_name, context.source_file);*/
        }
        else
            ssfatal("Compile Error: object \"%s\" in %s must have a \"main\" state.", context.object_name, parser->filename);
    }
}

/* an empty "main" state */
surgescript_var_t* empty_main(surgescript_object_t* object, const surgescript_var_t* param[], int num_params)
{
    /* disable the object for optimization purposes?
       no! what about the children? */
    /*surgescript_object_set_active(object, false);*/
    return NULL;
}

/* create a getter for the variable named identifier */
void create_getter(surgescript_parser_t* parser, surgescript_nodecontext_t context, const char* identifier)
{
    char* getter_name = surgescript_util_accessorfun("get", identifier);
    surgescript_program_t* getter = surgescript_program_create(0);

    /* create a getter */
    emit_vargetter(nodecontext(
        context.source_file,
        context.object_name,
        getter_name,
        context.symtable, /* reuse the same table; no locals */
        getter
    ), identifier);

    /* register things */
    surgescript_programpool_put(parser->program_pool, context.object_name, getter_name, getter);

    /* done */
    ssfree(getter_name);
}

/* create a setter for the variable named identifier */
void create_setter(surgescript_parser_t* parser, surgescript_nodecontext_t context, const char* identifier)
{
    char* setter_name = surgescript_util_accessorfun("set", identifier);
    surgescript_program_t* setter = surgescript_program_create(1);

    /* create a setter */
    emit_varsetter(nodecontext(
        context.source_file,
        context.object_name,
        setter_name,
        context.symtable, /* reuse the same table; no locals */
        setter
    ), identifier);

    /* register things */
    surgescript_programpool_put(parser->program_pool, context.object_name, setter_name, setter);

    /* done */
    ssfree(setter_name);
}

/* makes a program that returns source_file */
surgescript_program_t* make_file_program(const char* source_file)
{
    surgescript_program_t* program = surgescript_program_create(0);
    int text = surgescript_program_add_text(program, source_file);
    surgescript_program_add_line(program, SSOP_MOVS, SSOPu(0), SSOPi(text));
    surgescript_program_add_line(program, SSOP_RET, SSOPu(0), SSOPu(0));
    return program;
}

/* adds non-native programs to the programs[] vector */
void pick_non_natives(const char* program_name, void* data)
{
    surgescript_programpool_t* pool = (surgescript_programpool_t*)(((void**)data)[0]);
    const char* object_name = (const char*)(((void**)data)[1]);
    int* count = (int*)(((void**)data)[2]);
    char*** programs = (char***)(((void**)data)[3]);
    const surgescript_program_t* existing_program = surgescript_programpool_get(pool, object_name, program_name);

    if(existing_program != NULL && !surgescript_program_is_native(existing_program)) {
        *programs = ssrealloc(*programs, (++(*count)) * sizeof(char*));
        (*programs)[*count - 1] = ssstrdup(program_name);
    }
}

/* removes a previously defined object */
void remove_object_definition(surgescript_programpool_t* pool, const char* object_name)
{
    char** programs = NULL; int count = 0;
    void* data[] = { pool, (void*)object_name, &count, &programs };

    /* remove all programs (non-natives) */
    surgescript_programpool_foreach_ex(pool, object_name, data, pick_non_natives);
    if(programs != NULL) {
        for(int i = 0; i < count; i++) {
            surgescript_programpool_delete(pool, object_name, programs[i]);
            ssfree(programs[i]);
        }
        ssfree(programs);
    }
    
    /* FIXME: remove all tags of object_name (ps: how about tags added in C?) */
}

/* checks if duplicates of an object will be forbidden */
bool forbid_duplicates(const surgescript_parser_t* parser, const char* object_name)
{
    const char** builtins = surgescript_objectmanager_builtin_objects(NULL);

    for(; *builtins; builtins++) {
        if(strcmp(*builtins, object_name) == 0)
            return true;
    }

    /*for(int i = 0; i < ssarray_length(parser->known_plugins); i++) {
        if(strcmp(parser->known_plugins[i], object_name) == 0)
            return true;
    }*/

    return false;
}

/* checks if program_name is encoding the name of a state */
/* checks if the parsing context is of a state */
bool is_state_context(surgescript_nodecontext_t context)
{
    return context.program_name != NULL && strncmp(context.program_name, "state:", 6) == 0;
}

/* generates a random string, filling at most size bytes */
/* null character included. Returns buf */
char* randstr(char* buf, size_t size)
{
    char alphabet[] = "0123456789abcdef", *ret = buf;
    if(!size) return ret;
    
    while(size-- > 1)
        *(buf++) = alphabet[surgescript_util_random64() % 16];

    *buf = 0;
    return ret;
}

/* is the given [object|program|tag] name too large? */
bool is_large_name(const char* name)
{
    return strlen(name) > SS_NAMEMAX;
}

/* checks if the given string is a valid name for:
 * objects, states, tags... */
bool is_valid_name(const char* name)
{
    const char* p = name;

    /* check if it isn't blank */
    while(*p && isspace(*p))
        p++;
    if(*p == 0)
        return false;

    /* check if there are non-printable characters */
    for(; *p; p++) {
        if(!isprint(*p))
            return false;
    }

    /* now check the length */
    return (p - name) <= SS_NAMEMAX;
}


/* non-terminals of the grammar */

void objectlist(surgescript_parser_t* parser)
{
    while(has_token(parser))
        object(parser);
}

void object(surgescript_parser_t* parser)
{
    surgescript_nodecontext_t context;
    char** annotations;
    char* object_name;
    bool duplicate = false;

    /* object name */
    read_annotations(parser, &annotations);
    match(parser, SSTOK_OBJECT);
    expect(parser, SSTOK_STRING);

    /* create the parsing context */
    context = nodecontext(
        parser->filename,
        (object_name = ssstrdup(
            surgescript_token_lexeme(parser->lookahead)
        )),
        NULL,
        surgescript_symtable_create(parser->base_table), /* symbol table */
        surgescript_program_create(0) /* object constructor */
    );

    /* validate */
    if(is_large_name(object_name))
        ssfatal("Compile Error: object name \"%s\" is too large at %s:%d", object_name, parser->filename, surgescript_token_linenumber(parser->lookahead));
    else if(!is_valid_name(object_name))
        ssfatal("Compile Error: invalid object name \"%s\" in %s:%d.", object_name, parser->filename, surgescript_token_linenumber(parser->lookahead));
    else if((duplicate = surgescript_programpool_exists(parser->program_pool, object_name, "state:main"))) {
        if(parser->flags & SSPARSER_SKIP_DUPLICATES) {
            char buf[32] = { '.', 'd', 'u', 'p', '.' };
            sslog("Warning: skipping duplicate definition of object \"%s\" in %s:%d.", object_name, parser->filename, surgescript_token_linenumber(parser->lookahead));
            ssfree(object_name);
            object_name = ssstrdup(randstr(buf + 5, sizeof(buf) - 5) - 5);
            context.object_name = object_name;
        }
        else if((parser->flags & SSPARSER_ALLOW_DUPLICATES) && !forbid_duplicates(parser, object_name)) {
            sslog("Warning: reading duplicate definition of object \"%s\" in %s:%d.", object_name, parser->filename, surgescript_token_linenumber(parser->lookahead));
            remove_object_definition(parser->program_pool, object_name);
        }
        else
            ssfatal("Compile Error: duplicate definition of object \"%s\" in %s:%d.", object_name, parser->filename, surgescript_token_linenumber(parser->lookahead));
    }

    /* read the object */
    match(parser, SSTOK_STRING);
    qualifiers(parser, context);
    match(parser, SSTOK_LCURLY);
    objectdecl(parser, context);
    match(parser, SSTOK_RCURLY);

    /* object configuration */
    process_annotations(parser, annotations, object_name);
    surgescript_programpool_put(parser->program_pool, object_name, "__ssconstructor", context.program);
    if(!surgescript_programpool_shallowcheck(parser->program_pool, object_name, "get___file"))
        surgescript_programpool_put(parser->program_pool, object_name, "get___file", make_file_program(context.source_file));

    /* cleanup */
    if(duplicate && (parser->flags & SSPARSER_SKIP_DUPLICATES))
        remove_object_definition(parser->program_pool, object_name);
    surgescript_symtable_destroy(context.symtable);
    release_annotations(annotations);
    ssfree(object_name);
}

void objectdecl(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_program_label_t start = surgescript_program_new_label(context.program);
    surgescript_program_label_t end = surgescript_program_new_label(context.program);

    /* import properties */
    import_public_vars(parser, context, "Object");
    import_public_vars(parser, context, context.object_name);

    /* allocate variables */
    emit_object_header(context, start, end);

    /* read non-terminals */
    vardecllist(parser, context);
    statedecllist(parser, context);
    fundecllist(parser, context);

    /* check if the object is all right */
    validate_object(parser, context);

    /* tell the program how many variables should be allocated */
    emit_object_footer(context, start, end);
}

void qualifiers(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    if(optmatch(parser, SSTOK_IS)) {
        /* validate */
        if(!got_type(parser, SSTOK_STRING))
            unexpected_symbol(parser);

        /* read tags */
        while(got_type(parser, SSTOK_STRING)) {
            const char* tag_name = surgescript_token_lexeme(parser->lookahead);

            /* validate */
            if(is_large_name(tag_name))
                ssfatal("Compile Error: tag name \"%s\" of object \"%s\" is too large at %s:%d", tag_name, context.object_name, context.source_file, surgescript_token_linenumber(parser->lookahead));
            else if(!is_valid_name(tag_name))
                ssfatal("Compile Error: invalid tag name \"%s\" in object \"%s\" at %s:%d", tag_name, context.object_name, context.source_file, surgescript_token_linenumber(parser->lookahead));

            /* okay, add tag */
            surgescript_tagsystem_add_tag(parser->tag_system, context.object_name, tag_name);
            match(parser, SSTOK_STRING);
            if(optmatch(parser, SSTOK_COMMA))
                expect(parser, SSTOK_STRING);
            else
                break;
        }
    }
}

void importlist(surgescript_parser_t* parser)
{
    while(optmatch(parser, SSTOK_USING)) {
        SSARRAY(char, path);
        ssarray_init(path);

        /* read the import path */
        do {
            const char* subpath = surgescript_token_lexeme(parser->lookahead);
            expect(parser, SSTOK_IDENTIFIER);
            while(*subpath)
                ssarray_push(path, *subpath++);
            ssarray_push(path, '.');
            match(parser, SSTOK_IDENTIFIER);
        } while(optmatch(parser, SSTOK_DOT));
        match(parser, SSTOK_SEMICOLON);

        /* import the path into the symbol table */
        if(ssarray_length(path) > 0) {
            path[ssarray_length(path) - 1] = 0;
            surgescript_symtable_put_plugin_symbol(parser->base_table, path, parser->filename);
        }

        /* release the path */
        ssarray_release(path);
    }
}

void vardecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    while(got_type(parser, SSTOK_IDENTIFIER) || got_type(parser, SSTOK_PUBLIC))
        vardecl(parser, context);
}

void vardecl(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    bool public_var = optmatch(parser, SSTOK_PUBLIC);
    bool readonly_var = optmatch(parser, SSTOK_READONLY);
    char* id = ssstrdup(surgescript_token_lexeme(parser->lookahead));

    match(parser, SSTOK_IDENTIFIER);
    match_exactly(parser, SSTOK_ASSIGNOP, "=");
    conditionalexpr(parser, context);
    match(parser, SSTOK_SEMICOLON);

    emit_vardecl(context, id);
    if(public_var) {
        create_getter(parser, context, id);
        if(!readonly_var)
            create_setter(parser, context, id);
    }

    ssfree(id);
}

void statedecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    while(optmatch(parser, SSTOK_STATE)) {
        expect(parser, SSTOK_STRING);
        statedecl(parser, context);
    }
}

void statedecl(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    static const char prefix[] = "state:";
    const char* state_name = surgescript_token_lexeme(parser->lookahead);
    char* program_name;
    int fun_header = 0;

    /* read state name & generate function name */
    program_name = ssmalloc((1 + strlen(prefix) + strlen(state_name)) * sizeof(*program_name));
    strcat(strcpy(program_name, prefix), state_name);
    match(parser, SSTOK_STRING);

    /* validation */
    if(is_large_name(program_name))
        ssfatal("Compile Error: state name \"%s\" of object \"%s\" is too large at %s:%d", state_name, context.object_name, context.source_file, surgescript_token_linenumber(parser->lookahead));
    if(!is_valid_name(program_name))
        ssfatal("Compile Error: invalid state name \"%s\" in object \"%s\" at %s:%d", state_name, context.object_name, context.source_file, surgescript_token_linenumber(parser->lookahead));

    /* create context */
    context = nodecontext(
        context.source_file,
        context.object_name,
        program_name,
        surgescript_symtable_create(context.symtable), /* new symbol table for local variables */
        surgescript_program_create(0)
    );

    /* duplicate check */
    if(surgescript_programpool_shallowcheck(parser->program_pool, context.object_name, program_name))
        ssfatal("Compile Error: duplicate state \"%s\" in object \"%s\" at %s:%d", state_name, context.object_name, context.source_file, surgescript_token_linenumber(parser->lookahead));

    /* function body */
    match(parser, SSTOK_LCURLY);
    fun_header = emit_function_header(context);
    stmtlist(parser, context);
    emit_function_footer(context, surgescript_symtable_local_count(context.symtable), fun_header);
    match(parser, SSTOK_RCURLY);

    /* register the function and cleanup */
    surgescript_programpool_put(parser->program_pool, context.object_name, program_name, context.program);
    surgescript_symtable_destroy(context.symtable);
    ssfree(program_name);
}

void fundecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    while(optmatch(parser, SSTOK_FUN)) {
        expect(parser, SSTOK_IDENTIFIER);
        fundecl(parser, context);
    }
}

void fundecl(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    int i, fun_header = 0;
    int num_arguments = 0;
    char* program_name = ssstrdup(surgescript_token_lexeme(parser->lookahead));
    SSARRAY(surgescript_token_t*, arg);
    ssarray_init(arg);

    /* duplicate check */
    if(surgescript_programpool_shallowcheck(parser->program_pool, context.object_name, program_name))
        ssfatal("Compile Error: duplicate function \"%s\" in object \"%s\" at %s:%d", program_name, context.object_name, context.source_file, surgescript_token_linenumber(parser->lookahead));

    /* validity check */
    if(is_large_name(program_name))
        ssfatal("Compile Error: function name \"%s\" in object \"%s\" is too large at %s:%d", program_name, context.object_name, context.source_file, surgescript_token_linenumber(parser->lookahead));
    else if(!is_valid_name(program_name))
        ssfatal("Compile Error: invalid function name \"%s\" in object \"%s\" at %s:%d", program_name, context.object_name, context.source_file, surgescript_token_linenumber(parser->lookahead));

    /* read list of arguments */
    match(parser, SSTOK_IDENTIFIER);
    match(parser, SSTOK_LPAREN);
    if(!got_type(parser, SSTOK_RPAREN)) {
        do {
            expect(parser, SSTOK_IDENTIFIER);
            ssarray_push(arg, surgescript_token_clone(parser->lookahead));
            match(parser, SSTOK_IDENTIFIER);
        } while(optmatch(parser, SSTOK_COMMA));
    }
    match(parser, SSTOK_RPAREN);

    /* create context */
    num_arguments = ssarray_length(arg);
    context = nodecontext(
        context.source_file,
        context.object_name,
        program_name,
        surgescript_symtable_create(context.symtable), /* new symbol table for local variables */
        surgescript_program_create(num_arguments)
    );

    /* write list of arguments to the symbol table */
    for(i = 0; i < num_arguments; i++) {
        emit_function_argument(context, surgescript_token_lexeme(arg[i]), surgescript_token_linenumber(arg[i]), i, num_arguments);
        surgescript_token_destroy(arg[i]);
    }

    /* function body */
    match(parser, SSTOK_LCURLY);
    fun_header = emit_function_header(context);
    stmtlist(parser, context);
    emit_function_footer(context, surgescript_symtable_local_count(context.symtable) - num_arguments, fun_header);
    match(parser, SSTOK_RCURLY);

    /* register the function and cleanup */
    surgescript_programpool_put(parser->program_pool, context.object_name, program_name, context.program);
    surgescript_symtable_destroy(context.symtable);
    ssarray_release(arg);
    ssfree(program_name);
}


/* expressions (their return value is stored in t[0]) */
void expr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    const surgescript_tokentype_t acceptable_lookaheads[] = {
        SSTOK_TRUE, /* basic literals */
        SSTOK_FALSE,
        SSTOK_STRING,
        SSTOK_NUMBER,
        SSTOK_NULL,

        SSTOK_IDENTIFIER,
        SSTOK_THIS,
        SSTOK_CALLER,
        SSTOK_STATE,
        SSTOK_TYPEOF,
        SSTOK_TIMEOUT,

        SSTOK_LPAREN,
        SSTOK_LBRACKET, /* new array */
        SSTOK_LCURLY, /* new dictionary */

        SSTOK_ADDITIVEOP, /* unary */
        SSTOK_INCDECOP,
        SSTOK_LOGICALNOTOP
    };
    const int n = sizeof(acceptable_lookaheads) / sizeof(surgescript_tokentype_t);

    for(;;) {
        assignexpr(parser, context);

        /* comma operator: lowest precedence */
        if(!optmatch(parser, SSTOK_COMMA))
            break;

        /* do not accept trailing commas */
        if(!got_any_type(parser, acceptable_lookaheads, n))
            unexpected_symbol(parser);
    }
}

void assignexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    if(got_type(parser, SSTOK_IDENTIFIER)) {
        char* identifier = ssstrdup(surgescript_token_lexeme(parser->lookahead));
        int line = surgescript_token_linenumber(parser->lookahead);
        match(parser, SSTOK_IDENTIFIER);

        if(got_type(parser, SSTOK_ASSIGNOP)) {
            char* assignop = ssstrdup(surgescript_token_lexeme(parser->lookahead));

            match(parser, SSTOK_ASSIGNOP);
            assignexpr(parser, context);
            emit_assignexpr(context, assignop, identifier, line);

            ssfree(assignop);
        }
        /*else if(got_type(parser, SSTOK_LBRACKET)) {
            match(parser, SSTOK_LBRACKET);
            expr(parser, context);
            match(parser, SSTOK_RBRACKET);

            if(got_type(parser, SSTOK_ASSIGNOP)) {
                char* assignop = ssstrdup(surgescript_token_lexeme(parser->lookahead));
                
                match(parser, SSTOK_ASSIGNOP);
                emit_dictset1(context, assignop, identifier, line);
                assignexpr(parser, context);
                emit_dictset2(context, assignop, identifier, line);

                ssfree(assignop);
            }
            else
                emit_dictget(context, identifier, line);
        }*/
        else {
            unmatch(parser);
            conditionalexpr(parser, context);
        }

        ssfree(identifier);
    }
    else if(optmatch(parser, SSTOK_STATE)) {
        if(got_type(parser, SSTOK_ASSIGNOP)) {
            match_exactly(parser, SSTOK_ASSIGNOP, "=");
            assignexpr(parser, context);
            emit_setstate(context);
        }
        else {
            unmatch(parser);
            conditionalexpr(parser, context);
        }
    }
    else
        conditionalexpr(parser, context);
}

void conditionalexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    logicalorexpr(parser, context);
    if(optmatch(parser, SSTOK_CONDITIONALOP)) {
        surgescript_program_label_t nope = surgescript_program_new_label(context.program);
        surgescript_program_label_t done = surgescript_program_new_label(context.program);
        
        emit_conditionalexpr1(context, nope, done);
        expr(parser, context);
        match(parser, SSTOK_COLON);
        emit_conditionalexpr2(context, nope, done);
        conditionalexpr(parser, context);
        emit_conditionalexpr3(context, nope, done);
    }
}

void logicalorexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_program_label_t done = surgescript_program_new_label(context.program);

    logicalandexpr(parser, context);
    while(optmatch(parser, SSTOK_LOGICALOROP)) {
        emit_logicalorexpr1(context, done);
        logicalandexpr(parser, context);
    }
    emit_logicalorexpr2(context, done);
}

void logicalandexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_program_label_t done = surgescript_program_new_label(context.program);

    equalityexpr(parser, context);
    while(optmatch(parser, SSTOK_LOGICALANDOP)) {
        emit_logicalandexpr1(context, done);
        equalityexpr(parser, context);
    }
    emit_logicalandexpr2(context, done);
}

void equalityexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    relationalexpr(parser, context);
    while(got_type(parser, SSTOK_EQUALITYOP)) {
        char* op = ssstrdup(surgescript_token_lexeme(parser->lookahead));
        match(parser, SSTOK_EQUALITYOP);
        emit_equalityexpr1(context);
        relationalexpr(parser, context);
        emit_equalityexpr2(context, op);
        ssfree(op);
    }
}

void relationalexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    additiveexpr(parser, context);
    while(got_type(parser, SSTOK_RELATIONALOP)) {
        char* op = ssstrdup(surgescript_token_lexeme(parser->lookahead));
        match(parser, SSTOK_RELATIONALOP);
        emit_relationalexpr1(context);
        additiveexpr(parser, context);
        emit_relationalexpr2(context, op);
        ssfree(op);
    }
}

void additiveexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    multiplicativeexpr(parser, context);
    while(got_type(parser, SSTOK_ADDITIVEOP)) {
        char* op = ssstrdup(surgescript_token_lexeme(parser->lookahead));
        match(parser, SSTOK_ADDITIVEOP);
        emit_additiveexpr1(context);
        multiplicativeexpr(parser, context);
        emit_additiveexpr2(context, op);
        ssfree(op);
    }
}

void multiplicativeexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    unaryexpr(parser, context);
    while(got_type(parser, SSTOK_MULTIPLICATIVEOP)) {
        char* op = ssstrdup(surgescript_token_lexeme(parser->lookahead));
        match(parser, SSTOK_MULTIPLICATIVEOP);
        emit_multiplicativeexpr1(context);
        unaryexpr(parser, context);
        emit_multiplicativeexpr2(context, op);
        ssfree(op);
    }
}

void unaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    if(got_type(parser, SSTOK_ADDITIVEOP)) {
        char* op = ssstrdup(surgescript_token_lexeme(parser->lookahead));
        match(parser, SSTOK_ADDITIVEOP);
        unaryexpr(parser, context);
        emit_unarysign(context, op);
        ssfree(op);
    }
    else if(got_type(parser, SSTOK_INCDECOP)) {
        char* op = ssstrdup(surgescript_token_lexeme(parser->lookahead));
        match(parser, SSTOK_INCDECOP);
        if(got_type(parser, SSTOK_IDENTIFIER)) {
            const char* identifier = surgescript_token_lexeme(parser->lookahead);
            emit_unaryincdec(context, op, identifier, surgescript_token_linenumber(parser->lookahead));
            match(parser, SSTOK_IDENTIFIER);
        }
        else
            expect(parser, SSTOK_IDENTIFIER);
        ssfree(op);
    }
    else if(optmatch(parser, SSTOK_LOGICALNOTOP)) {
        unaryexpr(parser, context);
        emit_unarynot(context);
    }
    else if(optmatch(parser, SSTOK_TYPEOF)) {
        if(optmatch(parser, SSTOK_LPAREN)) {
            expr(parser, context);
            emit_unarytype(context);
            match(parser, SSTOK_RPAREN);
        }
        else {
            unaryexpr(parser, context);
            emit_unarytype(context);
        }
    }
    else if(optmatch(parser, SSTOK_TIMEOUT)) {
        if(!is_state_context(context))
            ssfatal("Compile Error: timeout can only be used inside a state (see %s:%d).", context.source_file, surgescript_token_linenumber(parser->previous));
        match(parser, SSTOK_LPAREN);
        assignexpr(parser, context);
        emit_timeout(context);
        match(parser, SSTOK_RPAREN);
    }
    else
        postfixexpr(parser, context);
}

void postfixexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    if(got_type(parser, SSTOK_IDENTIFIER)) {
        char* identifier = ssstrdup(surgescript_token_lexeme(parser->lookahead));
        int line = surgescript_token_linenumber(parser->lookahead);
        match(parser, SSTOK_IDENTIFIER);

        if(got_type(parser, SSTOK_INCDECOP)) {
            const char* op = surgescript_token_lexeme(parser->lookahead);
            emit_postincdec(context, op, identifier, line);
            match(parser, SSTOK_INCDECOP);
        }
        else if(got_type(parser, SSTOK_LPAREN)) { /* we have a function call here */
            if(!surgescript_symtable_has_symbol(context.symtable, identifier)) {
                /* regular function call */
                emit_this(context);
                funcallexpr(parser, context, identifier);
                postfixexpr1(parser, context);
            }
            else {
                /* call "call" method */
                surgescript_symtable_emit_read(context.symtable, identifier, context.program, 0);
                funcallexpr(parser, context, "call");
                postfixexpr1(parser, context);
            }
        }
        else {
            unmatch(parser);
            primaryexpr(parser, context);
            postfixexpr1(parser, context);
        }

        ssfree(identifier);
    }
    else {
        primaryexpr(parser, context);
        postfixexpr1(parser, context);
    }
}

void postfixexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    if(optmatch(parser, SSTOK_DOT)) {
        do {
            char* identifier = ssstrdup(surgescript_token_lexeme(parser->lookahead));
            match(parser, SSTOK_IDENTIFIER);
            if(got_type(parser, SSTOK_LPAREN)) {
                funcallexpr(parser, context, identifier);
                lambdacall(parser, context);
                dictgetexpr(parser, context);
                ssfree(identifier);
            }
            else if(got_type(parser, SSTOK_INCDECOP)) {
                /* obj.property++ <=> obj.set_property(obj.get_property() + 1) */
                const char* op = surgescript_token_lexeme(parser->lookahead);
                emit_setterincdec(context, identifier, op);
                match(parser, SSTOK_INCDECOP);
                ssfree(identifier);
                break;
            }
            else if(got_type(parser, SSTOK_ASSIGNOP)) {
                /* obj.property = value <=> obj.set_property(value) */
                char* op = ssstrdup(surgescript_token_lexeme(parser->lookahead));
                match(parser, SSTOK_ASSIGNOP);
                emit_setter1(context, identifier);
                assignexpr(parser, context);
                emit_setter2(context, identifier, op);
                ssfree(op);
                ssfree(identifier);
                break;
            }
            else {
                /* obj.property <=> obj.get_property() */
                emit_getter(context, identifier);
                lambdacall(parser, context);
                dictgetexpr(parser, context);
                ssfree(identifier);
            }
        } while(optmatch(parser, SSTOK_DOT));
    }
    else {
        lambdacall(parser, context);
        dictgetexpr(parser, context);
    }
}

void lambdacall(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    while(got_type(parser, SSTOK_LPAREN))
        funcallexpr(parser, context, "call");
}

void dictgetexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    while(optmatch(parser, SSTOK_LBRACKET)) {
        emit_dictptr(context);
        expr(parser, context);
        match(parser, SSTOK_RBRACKET);
        emit_dictkey(context);
        if(got_type(parser, SSTOK_ASSIGNOP)) {
            char* op = ssstrdup(surgescript_token_lexeme(parser->lookahead));
            match(parser, SSTOK_ASSIGNOP);
            assignexpr(parser, context);
            emit_dictset(context, op);
            ssfree(op);
            break;
        }
        else if(got_type(parser, SSTOK_INCDECOP)) {
            const char* op = surgescript_token_lexeme(parser->lookahead);
            emit_dictincdec(context, op);
            match(parser, SSTOK_INCDECOP);
            break;           
        }
        else {
            emit_dictget(context);
            if(got_type(parser, SSTOK_LBRACKET))
                continue;
        }
        postfixexpr1(parser, context);
        break;
    }
}

void funcallexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context, const char* fun_name)
{
    int num_params = 0;
    match(parser, SSTOK_LPAREN);

    /* quick validation */
    if(strcmp(fun_name, "constructor") == 0 || strcmp(fun_name, "destructor") == 0) {
        ssfatal(
            "Compile Error: the %s of \"%s\" can't be called directly in %s:%d.",
            fun_name,
            context.object_name,
            context.source_file,
            surgescript_token_linenumber(parser->lookahead)
        );
    }

    /* emit the function call code */
    emit_pushparam(context); /* push the object handle */
    if(!got_type(parser, SSTOK_RPAREN)) { /* read the parameters */
        do {
            ++num_params;
            assignexpr(parser, context);
            emit_pushparam(context); /* push the i-th param */
        } while(optmatch(parser, SSTOK_COMMA));
    }
    emit_funcall(context, fun_name, num_params);
    emit_popparams(context, 1 + num_params); /* pop the parameters and the object handle */

    match(parser, SSTOK_RPAREN);
}

void primaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    if(optmatch(parser, SSTOK_LPAREN)) {
        expr(parser, context);
        match(parser, SSTOK_RPAREN);
    }
    else if(optmatch(parser, SSTOK_LBRACKET)) {
        arrayexpr(parser, context);
        match(parser, SSTOK_RBRACKET);
    }
    else if(optmatch(parser, SSTOK_LCURLY)) {
        dictexpr(parser, context);
        match(parser, SSTOK_RCURLY);
    }
    else if(optmatch(parser, SSTOK_THIS)) {
        emit_this(context);
    }
    else if(optmatch(parser, SSTOK_STATE)) {
        emit_state(context);
    }
    else if(optmatch(parser, SSTOK_CALLER)) {
        emit_caller(context);
    }
    else if(got_type(parser, SSTOK_IDENTIFIER)) {
        const char* identifier = surgescript_token_lexeme(parser->lookahead);
        emit_identifier(context, identifier, surgescript_token_linenumber(parser->lookahead));
        match(parser, SSTOK_IDENTIFIER);
    }
    else
        constant(parser, context);
}

void constant(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_token_t* token = parser->lookahead;
    
    expect_something(parser);
    switch(surgescript_token_type(token)) {
        case SSTOK_NULL:
            emit_null(context);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_TRUE:
            emit_bool(context, true);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_FALSE:
            emit_bool(context, false);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_STRING:
            emit_string(context, surgescript_token_lexeme(token));
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_NUMBER:
            emit_number(context, ssatof(surgescript_token_lexeme(token)));
            match(parser, surgescript_token_type(token));
            break;

        default:
            ssfatal("Parse Error: expected a constant on %s:%d.", context.source_file, surgescript_token_linenumber(token));
            break;
    }
}

void arrayexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    emit_arrayexpr1(context);
    if(!got_type(parser, SSTOK_RBRACKET)) {
        do {
            assignexpr(parser, context);
            emit_arrayelement(context);
        } while(optmatch(parser, SSTOK_COMMA) && !got_type(parser, SSTOK_RBRACKET));
    }
    emit_arrayexpr2(context);
}

void dictexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    emit_dictdecl1(context);
    if(!got_type(parser, SSTOK_RCURLY)) {
        do {
            /* dict keys must be either strings or numbers (will be converted to strings at runtime) */
            if(!got_type(parser, SSTOK_STRING) && !got_type(parser, SSTOK_NUMBER))
                unexpected_symbol(parser);

            /* read key */
            constant(parser, context);
            emit_dictdeclkey(context);
            match(parser, SSTOK_COLON);

            /* read value */
            assignexpr(parser, context);
            emit_dictdeclvalue(context);
        } while(optmatch(parser, SSTOK_COMMA) && !got_type(parser, SSTOK_RCURLY));
    }
    emit_dictdecl2(context);
}

/* constant expressions */
void constexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    /*

    Constant expressions must be evaluated at compile-time
    (see the switch statement)

    */

    /* ( <constxpr> ) */
    if(optmatch(parser, SSTOK_LPAREN)) {
        constexpr(parser, context);
        match(parser, SSTOK_RPAREN);
        return;
    }

    /* TODO */
    signedconst(parser, context);
}

void signedconst(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_token_t* token;

    expect_something(parser);
    token = parser->lookahead;

    switch(surgescript_token_type(token)) {
        case SSTOK_NULL:
            emit_null(context);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_TRUE:
            emit_bool(context, true);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_FALSE:
            emit_bool(context, false);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_STRING:
            emit_string(context, surgescript_token_lexeme(token));
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_NUMBER:
        case SSTOK_ADDITIVEOP:
            signednum(parser, context);
            break;

        default:
            ssfatal("Parse Error: expected a literal on %s:%d.", context.source_file, surgescript_token_linenumber(token));
            break;
    }
}

void signednum(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_token_t* token;

    expect_something(parser);
    token = parser->lookahead;

    if(got_type(parser, SSTOK_ADDITIVEOP)) {
        double value = 0.0;
        bool plus = (strcmp(surgescript_token_lexeme(token), "+") == 0);

        match(parser, SSTOK_ADDITIVEOP);
        if(got_type(parser, SSTOK_NUMBER)) {
            token = parser->lookahead;
            value = ssatof(surgescript_token_lexeme(token));
            emit_number(context, plus ? value : -value);
        }
        match(parser, SSTOK_NUMBER);
    }
    else if(got_type(parser, SSTOK_NUMBER)) {
        emit_number(context, ssatof(surgescript_token_lexeme(token)));
        match(parser, SSTOK_NUMBER);
    }
    else
        expect(parser, SSTOK_NUMBER); /* will throw an error */
}

/* programming constructs */
void stmtlist(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    while(stmt(parser, context)) {
        ;
    }
}

bool stmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    if(got_type(parser, SSTOK_LCURLY)) {
        blockstmt(parser, context);
        return true;
    }
    else if(got_type(parser, SSTOK_IF)) {
        condstmt(parser, context);
        return true;
    }
    else if(got_type(parser, SSTOK_SWITCH)) {
        switchstmt(parser, context);
        return true;
    }
    else if(got_type(parser, SSTOK_WHILE)) {
        loopstmt(parser, context);
        return true;
    }
    else if(got_type(parser, SSTOK_DO)) {
        loopstmt(parser, context);
        return true;
    }
    else if(got_type(parser, SSTOK_FOR)) {
        loopstmt(parser, context);
        return true;
    }
    else if(got_type(parser, SSTOK_FOREACH)) {
        loopstmt(parser, context);
        return true;
    }
    else if(got_type(parser, SSTOK_RETURN)) {
        retstmt(parser, context);
        return true;
    }
    else if(got_type(parser, SSTOK_BREAK)) {
        jumpstmt(parser, context);
        return true;
    }
    else if(got_type(parser, SSTOK_CONTINUE)) {
        jumpstmt(parser, context);
        return true;
    }
    else if(got_type(parser, SSTOK_ASSERT)) {
        miscstmt(parser, context);
        return true;
    }
    else if(has_token(parser) && !got_type(parser, SSTOK_RCURLY) && !got_type(parser, SSTOK_CASE) && !got_type(parser, SSTOK_DEFAULT)) {
        exprstmt(parser, context);
        return true;
    }
    else {
        return false;
    }
}

void blockstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    match(parser, SSTOK_LCURLY);
    stmtlist(parser, context);
    match(parser, SSTOK_RCURLY);
}

void exprstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    if(!optmatch(parser, SSTOK_SEMICOLON)) {
        expr(parser, context);
        match(parser, SSTOK_SEMICOLON);
    }
}

void condstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_program_label_t nope = surgescript_program_new_label(context.program);

    match(parser, SSTOK_IF);
    match(parser, SSTOK_LPAREN);
    expr(parser, context);
    match(parser, SSTOK_RPAREN);

    /* evaluate the if-condition */
    emit_if(context, nope);
    if(!stmt(parser, context))
        unexpected_symbol(parser);

    /* is there an else block? match the inner-most if */
    if(optmatch(parser, SSTOK_ELSE)) {
        surgescript_program_label_t done = surgescript_program_new_label(context.program);
        emit_else(context, nope, done);
        if(!stmt(parser, context))
            unexpected_symbol(parser);
        emit_endif(context, done);
    }
    else
        emit_endif(context, nope);
}

void switchstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    /* in switch statements, the fallthrough behavior is a common source of
       programming errors, especially among beginners. We mitigate this problem
       in SurgeScript by requiring the termination of every non-empty case or
       default section by a break or by a return statement. */
    #define check_if_terminated() do { \
        surgescript_program_operator_t op; \
        surgescript_program_operand_t a; \
        \
        /* read the last emitted operation */ \
        int n = surgescript_program_count_lines(context.program); \
        if(!surgescript_program_read_line(context.program, n-1, &op, &a, NULL)) \
            break; \
        \
        /* checking the last emitted operation has a limitation: it's possible */ \
        /* to activate fallthrough behavior with a hack like if(false) break; */ \
        /* make sure we're not skipping line n-1, in which we expect a break */ \
        /* or a return statement */ \
        if(surgescript_program_find_label(context.program, n) == SURGESCRIPT_PROGRAM_UNDEFINED_LABEL) { \
            /* a break statement is emitted as JMP end */ \
            if(op == SSOP_JMP && a.u == end) \
                break; \
            \
            /* a return statement is emitted as a RET instruction */ \
            if(op == SSOP_RET) \
                break; \
        } \
        \
        /* error */ \
        ssfatal("Compile Error: found an unterminated section of a switch statement at %s:%d. Did you forget a break or a return statement?", context.source_file, surgescript_token_linenumber(parser->previous)); \
    } while(0)

    /* prevent repeated case constants */
    #define check_if_repeated() do { \
        /* read the last emitted operation */ \
        int n = surgescript_program_count_lines(context.program); \
        if(surgescript_program_read_line(context.program, n-1, &cc.op, NULL, &cc.b)){ \
            /* we should have emitted a constant */ \
            ssassert( \
                cc.op == SSOP_MOVF || cc.op == SSOP_MOVS || \
                cc.op == SSOP_MOVN || cc.op == SSOP_MOVB || \
                cc.op == SSOP_MOVX \
            ); \
            \
            /* no repetitions? */ \
            for(int j = ssarray_length(case_constant) - 1; j >= 0; j--) { \
                const struct caseconst_t* ref = &case_constant[j]; \
                if(ref->op == cc.op && ref->b.u64 == cc.b.u64) \
                    ssfatal("Compile Error: found a duplicate case constant in a switch statement at %s:%d", context.source_file, surgescript_token_linenumber(parser->previous)); \
            } \
            \
            /* store the constant */ \
            ssarray_push(case_constant, cc); \
        } \
    } while(0)

    surgescript_program_label_t next_test = surgescript_program_new_label(context.program);
    surgescript_program_label_t end = surgescript_program_new_label(context.program);
    surgescript_program_label_t def = SURGESCRIPT_PROGRAM_UNDEFINED_LABEL;

    /* auxiliary structure to prevent repeated case constants */
    struct caseconst_t { surgescript_program_operator_t op; surgescript_program_operand_t b; } cc;
    SSARRAY(struct caseconst_t, case_constant);
    ssarray_init(case_constant);

    /* save the switch context */
    context.loop_break = end; /* use the loop_break field, so that there is no ambiguity for break statements */

    /* switch statement */
    match(parser, SSTOK_SWITCH);
    match(parser, SSTOK_LPAREN);
    expr(parser, context);
    match(parser, SSTOK_RPAREN);

    emit_switch1(context, next_test);

    /* remark: in C, switch statements can be followed by *any* statement
       in SurgeScript, we require case / default labels (as in standard practice) */
    match(parser, SSTOK_LCURLY);
    for(;;) {
        if(optmatch(parser, SSTOK_CASE)) {
            /* we do not accept repeated case constants */
            surgescript_program_label_t skip = surgescript_program_new_label(context.program);
            surgescript_program_label_t test = next_test;
            next_test = surgescript_program_new_label(context.program);

            emit_case1(context, skip, test);
            constexpr(parser, context); /* evaluated at compile-time, so that */
            check_if_repeated(); /* we can check for repeated case constants */
            match(parser, SSTOK_COLON);
            emit_case2(context, skip, test, next_test);

            bool is_empty_section = (
                got_type(parser, SSTOK_CASE) ||
                got_type(parser, SSTOK_DEFAULT) ||
                false /*got_type(parser, SSTOK_RCURLY)*/ /* the last section can't be empty */
            );

            /* section body */
            stmtlist(parser, context);

            /* we allow fallthrough in empty sections only */
            if(!is_empty_section)
                check_if_terminated();
        }
        else if(optmatch(parser, SSTOK_DEFAULT)) {
            /* we accept at most one default label */
            surgescript_program_label_t test = next_test;
            next_test = surgescript_program_new_label(context.program);

            match(parser, SSTOK_COLON);

            bool is_empty_section = (
                got_type(parser, SSTOK_CASE) ||
                got_type(parser, SSTOK_DEFAULT) ||
                false /*got_type(parser, SSTOK_RCURLY)*/ /* the last section can't be empty */
            );

            if(def != SURGESCRIPT_PROGRAM_UNDEFINED_LABEL)
                ssfatal("Compile Error: found a duplicate default label in a switch statement at %s:%d", context.source_file, surgescript_token_linenumber(parser->previous));
            def = surgescript_program_new_label(context.program);
            emit_default(context, test, next_test, def);

            /* section body */
            stmtlist(parser, context);

            /* we allow fallthrough in empty sections only */
            if(!is_empty_section)
                check_if_terminated();
        }
        else if(optmatch(parser, SSTOK_RCURLY))
            break;
        else
            unexpected_symbol(parser);
    }

    emit_switch2(context, end, def, next_test);

    ssarray_release(case_constant);

    #undef check_if_repeated
    #undef check_if_terminated
}

void loopstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_program_label_t begin = surgescript_program_new_label(context.program);
    surgescript_program_label_t end = surgescript_program_new_label(context.program);

    /* save the loop context */
    context.loop_continue = begin;
    context.loop_break = end;

    /* what kind of loop do we have? */
    if(optmatch(parser, SSTOK_WHILE)) {
        /* while loops */
        emit_while1(context, begin);
        match(parser, SSTOK_LPAREN);
        expr(parser, context); /* loop condition */
        match(parser, SSTOK_RPAREN);
        emit_whilecheck(context, end);
        if(!stmt(parser, context)) /* loop body */
            unexpected_symbol(parser);
        emit_while2(context, begin, end);
    }
    else if(optmatch(parser, SSTOK_DO)) {
        /* do-while loops */
        surgescript_program_label_t condition = surgescript_program_new_label(context.program);
        context.loop_continue = condition;

        emit_dowhile1(context, begin);
        if(!stmt(parser, context)) /* loop body */
            unexpected_symbol(parser);
        match(parser, SSTOK_WHILE);
        match(parser, SSTOK_LPAREN);
        emit_dowhilecondition(context, condition);
        expr(parser, context); /* loop condition */
        match(parser, SSTOK_RPAREN);
        match(parser, SSTOK_SEMICOLON);
        emit_dowhile2(context, begin, end);
    }
    else if(optmatch(parser, SSTOK_FOR)) {
        /* for loop */
        surgescript_program_label_t body = surgescript_program_new_label(context.program);
        surgescript_program_label_t increment = surgescript_program_new_label(context.program);
        context.loop_continue = increment;

        /* emit code */
        match(parser, SSTOK_LPAREN);
        if(!got_type(parser, SSTOK_SEMICOLON))
            expr(parser, context); /* initialization */
        emit_for1(context, begin);
        match(parser, SSTOK_SEMICOLON);
        if(!got_type(parser, SSTOK_SEMICOLON))
            expr(parser, context); /* loop condition */
        else
            emit_bool(context, true); /* empty loop condition */
        match(parser, SSTOK_SEMICOLON);
        emit_forcheck(context, begin, body, increment, end);
        if(!got_type(parser, SSTOK_RPAREN))
            expr(parser, context); /* increment / update expression */
        match(parser, SSTOK_RPAREN);
        emit_for2(context, begin, body);
        if(!stmt(parser, context)) /* loop body */
            unexpected_symbol(parser);
        emit_for3(context, increment, end);
    }
    else if(optmatch(parser, SSTOK_FOREACH)) {
        /* foreach loop */
        char* identifier;

        match(parser, SSTOK_LPAREN);
        identifier = ssstrdup(surgescript_token_lexeme(parser->lookahead));
        match(parser, SSTOK_IDENTIFIER);
        match(parser, SSTOK_IN);
        expr(parser, context);
        match(parser, SSTOK_RPAREN);

        /* emit code */
        emit_foreach1(context, identifier, begin, end);
        if(!stmt(parser, context))
            unexpected_symbol(parser);
        emit_foreach2(context, identifier, begin, end);

        /* cleanup */
        ssfree(identifier);
    }
}

void jumpstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    int line = surgescript_token_linenumber(parser->lookahead);

    if(optmatch(parser, SSTOK_BREAK)) {
        match(parser, SSTOK_SEMICOLON);
        emit_break(context, line);
    }
    else if(optmatch(parser, SSTOK_CONTINUE)) {
        match(parser, SSTOK_SEMICOLON);
        emit_continue(context, line);
    }
}

void retstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    match(parser, SSTOK_RETURN);

    if(!optmatch(parser, SSTOK_SEMICOLON)) {
        if(!is_state_context(context)) {
            expr(parser, context);
            match(parser, SSTOK_SEMICOLON);
            emit_ret(context);
            return;
        }
        else
            ssfatal("Compile Error: found a non-empty return statement inside a state in %s:%d. Did you mean \"return;\"?", context.source_file, surgescript_token_linenumber(parser->previous));
    }

    emit_null(context);
    emit_ret(context);
}

void miscstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    if(optmatch(parser, SSTOK_ASSERT)) {
        int line = surgescript_token_linenumber(parser->lookahead);
        match(parser, SSTOK_LPAREN);
        assignexpr(parser, context);
        if(optmatch(parser, SSTOK_COMMA)) {
            const char* message = surgescript_token_lexeme(parser->lookahead);
            match(parser, SSTOK_STRING);
            emit_assert(context, line, message);
        }
        else
            emit_assert(context, line, NULL);
        match(parser, SSTOK_RPAREN);
        match(parser, SSTOK_SEMICOLON);
    }
}

/* utilities */
void make_accessor(const char* fun_name, void* symtable)
{
    /* run only if fun_name is an accessor */
    if((strncmp(fun_name, "get_", 4) == 0 || strncmp(fun_name, "set_", 4) == 0) && fun_name[4] != '\0') {
        /* fun_name is an accessor */
        const char* accessor = fun_name + 4;

        /* now that we have the accessor name, add it to the symbol table */
        if(!surgescript_symtable_has_symbol((surgescript_symtable_t*)symtable, accessor))
            surgescript_symtable_put_accessor_symbol((surgescript_symtable_t*)symtable, accessor);
    }
}

void import_public_vars(surgescript_parser_t* parser, surgescript_nodecontext_t context, const char* object_name)
{
    /* look for all accessors in object_name
       and add them to the symbol table */
    surgescript_programpool_foreach_ex(parser->program_pool, object_name, context.symtable, make_accessor);
}

void init_plugins_list(surgescript_parser_t* parser)
{
    ssarray_init(parser->known_plugins);
}

void release_plugins_list(surgescript_parser_t* parser)
{
    char* plugin = NULL;
    while(ssarray_length(parser->known_plugins) > 0) {
        ssarray_pop(parser->known_plugins, plugin);
        ssfree(plugin);
    }
    ssarray_release(parser->known_plugins);
}

void add_to_plugins_list(surgescript_parser_t* parser, const char* plugin_name)
{
    /* won't accept repeated elements */
    for(int i = 0; i < ssarray_length(parser->known_plugins); i++) {
        if(strcmp(parser->known_plugins[i], plugin_name) == 0)
            return;
    }

    /* add to the plugins list */
    ssarray_push(parser->known_plugins, ssstrdup(plugin_name));
}

surgescript_symtable_t* configure_base_table(surgescript_symtable_t* base_table)
{
    const char** builtins = surgescript_objectmanager_builtin_objects(NULL);
    while(*builtins) {
        if(**builtins != '_')
            surgescript_symtable_put_static_symbol(base_table, *builtins);
        builtins++;
    }
    return base_table;
}

void read_annotations(surgescript_parser_t* parser, char*** annotations)
{
    /* read the annotations into a NULL-terminated array of strings */
    if(got_type(parser, SSTOK_ANNOTATION)) {
        /* initialize & read */
        SSARRAY(char*, buf);
        ssarray_init(buf);
        while(got_type(parser, SSTOK_ANNOTATION)) {
            ssarray_push(buf, ssstrdup(surgescript_token_lexeme(parser->lookahead)));
            match(parser, SSTOK_ANNOTATION);
        }
        ssarray_push(buf, NULL);

        /* copy to the annotations output parameter */
        *annotations = memcpy(
            ssmalloc(ssarray_length(buf) * sizeof(char*)),
            buf, ssarray_length(buf) * sizeof(char*)
        );

        /* release */
        ssarray_release(buf);
    }
    else
        *annotations = NULL; /* no annotations found */
}

void release_annotations(char** annotations)
{
    if(annotations != NULL) {
        for(char** it = annotations; *it != NULL; it++)
            ssfree(*it);
        ssfree(annotations); /* ssarray_release not needed */
    }
}

void process_annotations(surgescript_parser_t* parser, char** annotations, const char* object_name)
{
    /* I have made the annotations system in such
       a way that it can be extended later */
    if(annotations != NULL) {
        while(*annotations != NULL) {
            const char* annotation = *annotations++;
            if(strcmp(annotation, "@Package") == 0 || strcmp(annotation, "@Plugin") == 0)
                add_to_plugins_list(parser, object_name);
            else
                ssfatal("Compile Error: unrecognized annotation \"%s\" around object \"%s\" in %s.", annotation, object_name, parser->filename);
        }
    }
}
