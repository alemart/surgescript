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
 * compiler/parser.c
 * SurgeScript compiler: syntax analyzer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <ctype.h>
#include "parser.h"
#include "lexer.h"
#include "token.h"
#include "nodecontext.h"
#include "symtable.h"
#include "codegen.h"
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
};

/* helpers */
static void parse(surgescript_parser_t* parser);
static inline bool got_type(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
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
static surgescript_var_t* disable_object(surgescript_object_t* object, const surgescript_var_t* param[], int num_params);
static void create_getter_and_setter(surgescript_parser_t* parser, surgescript_nodecontext_t context, const char* identifier);
static void import_public_vars(surgescript_parser_t* parser, surgescript_nodecontext_t context, const char* object_name);
static void make_accessor(const char* fun_name, void* symtable);
static void init_plugins_list(surgescript_parser_t* parser);
static void add_to_plugins_list(surgescript_parser_t* parser, const char* plugin_name);
static void release_plugins_list(surgescript_parser_t* parser);
static surgescript_symtable_t* configure_base_table(surgescript_symtable_t* base_table);
static void read_annotations(surgescript_parser_t* parser, char*** annotations);
static void release_annotations(char** annotations);
static void process_annotations(surgescript_parser_t* parser, char** annotations, const char* object_name);

/* non-terminals */
static void importlist(surgescript_parser_t* parser);
static void objectlist(surgescript_parser_t* parser);
static void object(surgescript_parser_t* parser);
static void objectdecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void qualifierlist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void taglist(surgescript_parser_t* parser, surgescript_nodecontext_t context);

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
static void loopstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void jumpstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void retstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);

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
    init_plugins_list(parser);
    setlocale(LC_NUMERIC, "C"); /* use '.' as the decimal separator on atof() */
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
 * surgescript_parser_parsefile()
 * Parse a script file; returns false on error
 */
bool surgescript_parser_parsefile(surgescript_parser_t* parser, const char* absolute_path)
{
    FILE* fp = fopen(absolute_path, "rb"); /* use binary mode, so offsets don't get messed up */
    if(fp) {
        static size_t BUFSIZE = 1024;
        char* data = NULL;
        size_t read_chars = 0, data_size = 0;

        /* read file to data[] */
        sslog("Reading file %s...", absolute_path);
        do {
            data_size += BUFSIZE;
            data = ssrealloc(data, data_size + 1);
            read_chars += fread(data + read_chars, sizeof(char), BUFSIZE, fp);
            data[read_chars] = '\0';
        } while(read_chars == data_size);
        fclose(fp);

        /* parse it */
        ssfree(parser->filename);
        parser->filename = ssstrdup(surgescript_util_basename(absolute_path));
        surgescript_lexer_set(parser->lexer, data);
        parse(parser);

        /* done! */
        ssfree(data);
        return true;
    }
    else {
        ssfatal("Parse Error: can't read file \"%s\": %s", absolute_path, strerror(errno));
        return false;
    }
}

/*
 * surgescript_parser_parsemem()
 * Parse a script stored in memory
 */
bool surgescript_parser_parsemem(surgescript_parser_t* parser, const char* code_in_memory)
{
    ssfree(parser->filename);
    parser->filename = ssstrdup("<memory>");
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
        ssfatal("Can\'t unmatch symbol \"%s\" on %s:%d.", surgescript_tokentype_name(surgescript_token_type(parser->previous)), parser->filename, surgescript_token_linenumber(parser->previous));
    else
        ssfatal("Can\'t unmatch symbol on %s.", parser->filename);
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
            surgescript_program_t* cprogram = surgescript_cprogram_create(0, disable_object);
            surgescript_programpool_put(parser->program_pool, context.object_name, "state:main", cprogram);
            /*sslog("Object \"%s\" in \"%s\" has omitted its \"main\" state and will be disabled.", context.object_name, context.source_file);*/
        }
        else
            ssfatal("Compile Error: object \"%s\" in %s must have a \"main\" state.", context.object_name, parser->filename);
    }
}

/* an empty "main" state that disables the object for optimization purposes */
surgescript_var_t* disable_object(surgescript_object_t* object, const surgescript_var_t* param[], int num_params)
{
    surgescript_object_set_active(object, false);
    return NULL;
}

/* we'll create a getter and a setter for the variable named identifier */
void create_getter_and_setter(surgescript_parser_t* parser, surgescript_nodecontext_t context, const char* identifier)
{
    char* getter_name = surgescript_util_accessorfun("get", identifier);
    char* setter_name = surgescript_util_accessorfun("set", identifier);
    surgescript_program_t* getter = surgescript_program_create(0);
    surgescript_program_t* setter = surgescript_program_create(1);

    /* create a getter */
    emit_vargetter(nodecontext(
        context.source_file,
        context.object_name,
        context.symtable, /* reuse the same table; no locals */
        getter
    ), identifier);

    /* create a setter */
    emit_varsetter(nodecontext(
        context.source_file,
        context.object_name,
        context.symtable, /* reuse the same table; no locals */
        setter
    ), identifier);

    /* register things */
    surgescript_programpool_put(parser->program_pool, context.object_name, getter_name, getter);
    surgescript_programpool_put(parser->program_pool, context.object_name, setter_name, setter);

    /* done */
    ssfree(setter_name);
    ssfree(getter_name);
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
        surgescript_symtable_create(parser->base_table), /* symbol table */
        surgescript_program_create(0) /* object constructor */
    );

    /* validate */
    if(surgescript_programpool_exists(parser->program_pool, object_name, "__ssconstructor"))
        ssfatal("Compile Error: duplicate definition of object \"%s\" in %s:%d.", object_name, parser->filename, surgescript_token_linenumber(parser->lookahead));

    /* read the object */
    match(parser, SSTOK_STRING);
    qualifierlist(parser, context);
    match(parser, SSTOK_LCURLY);
    objectdecl(parser, context);
    match(parser, SSTOK_RCURLY);

    /* object configuration */
    process_annotations(parser, annotations, object_name);
    surgescript_programpool_put(parser->program_pool, object_name, "__ssconstructor", context.program);

    /* cleanup */
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
    taglist(parser, context);
    vardecllist(parser, context);
    statedecllist(parser, context);
    fundecllist(parser, context);

    /* check if the object is all right */
    validate_object(parser, context);

    /* tell the program how many variables should be allocated */
    emit_object_footer(context, start, end);
}

void qualifierlist(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    int love_counter = 0;
    while(got_type(parser, SSTOK_EMOTICON)) {
        const char* emoticon = surgescript_token_lexeme(parser->lookahead);
        surgescript_tagsystem_add_tag(parser->tag_system, context.object_name, emoticon);

        /* easter egg */
        if(strcmp(emoticon, "<3") == 0) {
            if(++love_counter == 3)
                sslog("Object \"%s\" is truly loved.", context.object_name);
        }
        else
            love_counter = 4;

        match(parser, SSTOK_EMOTICON);
    }
}

void taglist(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    while(optmatch(parser, SSTOK_TAG)) {
        const char* tag_name = surgescript_token_lexeme(parser->lookahead);
        expect(parser, SSTOK_STRING);
        surgescript_tagsystem_add_tag(parser->tag_system, context.object_name, tag_name);
        match(parser, SSTOK_STRING);
        match(parser, SSTOK_SEMICOLON);
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
            surgescript_symtable_put_plugin_symbol(parser->base_table, path);
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
    char* id = ssstrdup(surgescript_token_lexeme(parser->lookahead));

    match(parser, SSTOK_IDENTIFIER);
    match_exactly(parser, SSTOK_ASSIGNOP, "=");
    expr(parser, context);
    match(parser, SSTOK_SEMICOLON);

    emit_vardecl(context, id);
    if(public_var)
        create_getter_and_setter(parser, context, id);

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
    int fun_header = 0;
    static const char prefix[] = "state:";
    const char* state_name = surgescript_token_lexeme(parser->lookahead);
    char* program_name = ssmalloc((1 + strlen(prefix) + strlen(state_name)) * sizeof(*program_name));

    /* create context */
    context = nodecontext(
        context.source_file,
        context.object_name,
        surgescript_symtable_create(context.symtable), /* new symbol table for local variables */
        surgescript_program_create(0)
    );

    /* read state name & generate function name */
    strcat(strcpy(program_name, prefix), state_name);
    match(parser, SSTOK_STRING);

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
    do {
        assignexpr(parser, context);
    } while(optmatch(parser, SSTOK_COMMA));
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
        match_exactly(parser, SSTOK_ASSIGNOP, "=");
        assignexpr(parser, context);
        emit_setstate(context);
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
                /* obj.property++ <=> obj.setProperty(obj.getProperty() + 1) */
                const char* op = surgescript_token_lexeme(parser->lookahead);
                emit_setterincdec(context, identifier, op);
                match(parser, SSTOK_INCDECOP);
                ssfree(identifier);
                break;
            }
            else if(got_type(parser, SSTOK_ASSIGNOP)) {
                /* obj.property = value <=> obj.setProperty(value) */
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
                /* obj.property <=> obj.getProperty() */
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
            emit_number(context, atof(surgescript_token_lexeme(token)));
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
        } while(optmatch(parser, SSTOK_COMMA));
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
        } while(optmatch(parser, SSTOK_COMMA));
    }
    emit_dictdecl2(context);
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
    else if(got_type(parser, SSTOK_WHILE)) {
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
    else if(got_type(parser, SSTOK_BREAK) || got_type(parser, SSTOK_CONTINUE)) {
        jumpstmt(parser, context);
        return true;
    }
    else if(has_token(parser) && !got_type(parser, SSTOK_RCURLY)) {
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

void loopstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_program_label_t begin = surgescript_program_new_label(context.program);
    surgescript_program_label_t end = surgescript_program_new_label(context.program);

    /* save the loop context */
    context.loop_begin = begin;
    context.loop_end = end;

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
    else if(optmatch(parser, SSTOK_FOR)) {
        /* for loop */
        surgescript_program_label_t body = surgescript_program_new_label(context.program);
        surgescript_program_label_t increment = surgescript_program_new_label(context.program);

        /* emit code */
        match(parser, SSTOK_LPAREN);
        expr(parser, context); /* initialization */
        emit_for1(context, begin);
        match(parser, SSTOK_SEMICOLON);
        expr(parser, context); /* loop condition */
        match(parser, SSTOK_SEMICOLON);
        emit_forcheck(context, begin, body, increment, end);
        expr(parser, context); /* increment */
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

    if(optmatch(parser, SSTOK_BREAK))
        emit_break(context, line);
    else if(optmatch(parser, SSTOK_CONTINUE))
        emit_continue(context, line);
}

void retstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    match(parser, SSTOK_RETURN);
    if(!optmatch(parser, SSTOK_SEMICOLON)) {
        expr(parser, context);
        match(parser, SSTOK_SEMICOLON);
        emit_ret(context);
    }
    else {
        emit_null(context);
        emit_ret(context);
    }
}

/* misc */

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
            ssfatal("Parse Error: expected a constant on %s:%d.", context.source_file, surgescript_token_linenumber(token));
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
            value = atof(surgescript_token_lexeme(token));
            emit_number(context, plus ? value : -value);
        }
        match(parser, SSTOK_NUMBER);
    }
    else if(got_type(parser, SSTOK_NUMBER)) {
        emit_number(context, atof(surgescript_token_lexeme(token)));
        match(parser, SSTOK_NUMBER);
    }
    else
        expect(parser, SSTOK_NUMBER); /* will throw an error */
}

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
            if(strcmp(annotation, "@Plugin") == 0)
                add_to_plugins_list(parser, object_name);
            else
                ssfatal("Compile Error: unrecognized annotation \"%s\" around object \"%s\" in %s.", annotation, object_name, parser->filename);
        }
    }
}