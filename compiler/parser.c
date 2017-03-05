/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * compiler/parser.c
 * SurgeScript compiler: syntax analyzer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "parser.h"
#include "lexer.h"
#include "token.h"
#include "nodecontext.h"
#include "symtable.h"
#include "codegen.h"
#include "../runtime/program_pool.h"
#include "../runtime/program.h"
#include "../util/util.h"

/* the parser */
struct surgescript_parser_t
{
    surgescript_token_t* lookahead;
    surgescript_token_t* previous;
    surgescript_lexer_t* lexer;
    char* filename;
    surgescript_programpool_t* program_pool;
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
static const char* ssbasename(const char* path);

/* non-terminals */
static void objectlist(surgescript_parser_t* parser);
static void object(surgescript_parser_t* parser);
static void objectdecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);

static void notelist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void notelist1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void note(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void signedconst(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void signednum(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void endnote(surgescript_parser_t* parser, surgescript_nodecontext_t context);

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
static void primaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void constant(surgescript_parser_t* parser, surgescript_nodecontext_t context);

static void stmtlist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void stmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void blockstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void exprstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void condstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void loopstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void jumpstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);


/* public api */


/*
 * surgescript_parser_create()
 * Creates a new parser
 */
surgescript_parser_t* surgescript_parser_create(surgescript_programpool_t* program_pool)
{
    surgescript_parser_t* parser = ssmalloc(sizeof *parser);
    parser->lookahead = parser->previous = NULL;
    parser->lexer = surgescript_lexer_create();
    parser->filename = ssstrdup("<unspecified>");
    parser->program_pool = program_pool;
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
        do {
            data_size += BUFSIZE;
            data = ssrealloc(data, data_size + 1);
            read_chars += fread(data + read_chars, sizeof(char), BUFSIZE, fp);
            data[read_chars] = '\0';
        }
        while(read_chars == data_size);
        fclose(fp);

        /* parse it */
        ssfree(parser->filename);
        parser->filename = ssstrdup(ssbasename(absolute_path));
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
 * surgescript_parser_parsefile()
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


/* privates & helpers */



/* parses a script */
void parse(surgescript_parser_t* parser)
{
    sslog("Parsing \"%s\"...", parser->filename);
    parser->lookahead = surgescript_lexer_scan(parser->lexer); /* grab first symbol */
    return objectlist(parser);
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

/* is there a token to be analyzed? */
bool has_token(surgescript_parser_t* parser)
{
    return parser->lookahead != NULL;
}

/* similar to basename(), but without the odd semantics. No strings are allocated. */
const char* ssbasename(const char* path)
{
    const char* p;

    if(!(p = strrchr(path, '/'))) {
        if(!(p = strrchr(path, '\\')))
            return path;
    }

    return p + 1;
}






/* non-terminals of the grammar */

void objectlist(surgescript_parser_t* parser)
{
    if(has_token(parser)) {
        object(parser);
        objectlist(parser);
    }
}

void object(surgescript_parser_t* parser)
{
    surgescript_nodecontext_t context;
    char* object_name;

    match(parser, SSTOK_OBJECT);
    expect(parser, SSTOK_STRING);
    context = nodecontext(
        (object_name = ssstrdup(
            surgescript_token_lexeme(parser->lookahead)
        )),
        surgescript_symtable_create(NULL), /* symbol table */
        surgescript_program_create(0) /* object constructor */
    );
    match(parser, SSTOK_STRING);
    match(parser, SSTOK_LCURLY);
    objectdecl(parser, context);
    match(parser, SSTOK_RCURLY);

    surgescript_programpool_put(parser->program_pool,  object_name, "__ssconstructor", context.program);
    surgescript_symtable_destroy(context.symtable);
    ssfree(object_name);
}

void objectdecl(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_program_label_t start = surgescript_program_new_label(context.program);
    surgescript_program_label_t end = surgescript_program_new_label(context.program);

    /* allocate variables */
    emit_object_header(context, start, end);

    /* read non-terminals */
    vardecllist(parser, context);

    /* tell the program how many variables should be allocated */
    emit_object_footer(context, start, end);
}

void vardecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    while(has_token(parser) && got_type(parser, SSTOK_IDENTIFIER))
        vardecl(parser, context);
}

void vardecl(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    char* id = ssstrdup(surgescript_token_lexeme(parser->lookahead));

    match(parser, SSTOK_IDENTIFIER);
    match_exactly(parser, SSTOK_ASSIGNOP, "=");
    expr(parser, context);
    match(parser, SSTOK_SEMICOLON);

    emit_vardecl(context, id);
    ssfree(id);
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
        match(parser, SSTOK_IDENTIFIER);

        if(got_type(parser, SSTOK_ASSIGNOP)) {
            char* assignop = ssstrdup(surgescript_token_lexeme(parser->lookahead));
            match(parser, SSTOK_ASSIGNOP);
            assignexpr(parser, context);
            emit_assignexpr(context, identifier, assignop);
            ssfree(assignop);
        }
        else {
            unmatch(parser);
            conditionalexpr(parser, context);
        }

        ssfree(identifier);
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
        emit_logicalorexpr2(context, done);
    }
}

void logicalandexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_program_label_t done = surgescript_program_new_label(context.program);

    equalityexpr(parser, context);
    while(optmatch(parser, SSTOK_LOGICALANDOP)) {
        emit_logicalandexpr1(context, done);
        equalityexpr(parser, context);
        emit_logicalandexpr2(context, done);
    }
}

void equalityexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    signedconst(parser, context);
}

/* notes */

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
            ssfatal("Parse Error: expected a constant value on %s near line %d.", parser->filename, surgescript_token_linenumber(token));
            break;
    }
}

void signednum(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_token_t* token;
    
    expect_something(parser);
    token = parser->lookahead;

    if(got_type(parser, SSTOK_ADDITIVEOP)) {
        float value = 0.0;
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


void vardecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void vardecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void statedecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void statedecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void fundecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void fundecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);

void expr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void assignexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void conditionalexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void logicalorexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void logicalorexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void logicalandexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void logicalandexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void equalityexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void equalityexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void relationalexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void relationalexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void additiveexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void additiveexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void multiplicativeexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void multiplicativeexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void unaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void primaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void constant(surgescript_parser_t* parser, surgescript_nodecontext_t context);

void stmtlist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void stmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void blockstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void exprstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void condstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void loopstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
void jumpstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);


