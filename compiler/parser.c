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
#include "parsetree.h"
#include "../runtime/program.h"
#include "../util/util.h"

/* the parser */
struct surgescript_parser_t
{
    surgescript_token_t* lookahead;
    surgescript_lexer_t* lexer;
    char* filename;
};

/* helpers */
static surgescript_parsetree_t* parse(surgescript_parser_t* parser);
static inline bool gottype(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
static void match(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
static bool optmatch(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
static void expect(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
static void expect_something(surgescript_parser_t* parser);
static const char* ssbasename(const char* path);
static surgescript_parsetree_t* emptytree();

/* non-terminals */
static surgescript_parsetree_t* objectlist(surgescript_parser_t* parser);
static surgescript_parsetree_t* object(surgescript_parser_t* parser);
static surgescript_parsetree_t* objectdecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);

static surgescript_parsetree_t* notelist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* notelist1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* note(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* signedconst(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* signednum(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* endnote(surgescript_parser_t* parser, surgescript_nodecontext_t context);

static surgescript_parsetree_t* vardecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* vardecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* statedecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* statedecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* fundecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* fundecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);

static surgescript_parsetree_t* expr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* assignexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* conditionalexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* logicalorexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* logicalorexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* logicalandexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* logicalandexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* equalityexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* equalityexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* relationalexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* relationalexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* additiveexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* additiveexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* multiplicativeexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* multiplicativeexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* unaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* primaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* constant(surgescript_parser_t* parser, surgescript_nodecontext_t context);

static surgescript_parsetree_t* stmtlist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* stmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* blockstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* exprstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* condstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* loopstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static surgescript_parsetree_t* jumpstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);


/* public api */


/*
 * surgescript_parser_create()
 * Creates a new parser
 */
surgescript_parser_t* surgescript_parser_create()
{
    surgescript_parser_t* parser = ssmalloc(sizeof *parser);
    parser->lookahead = NULL;
    parser->lexer = surgescript_lexer_create();
    parser->filename = surgescript_util_strdup("<unspecified>");
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
    return ssfree(parser);
}

/*
 * surgescript_parser_parsefile()
 * Parse a script file
 */
surgescript_parsetree_t* surgescript_parser_parsefile(surgescript_parser_t* parser, const char* absolute_path)
{
    FILE* fp = fopen(absolute_path, "rb"); /* use binary mode, so offsets don't get messed up */
    if(fp) {
        static size_t BUFSIZE = 1024;
        char* data = NULL;
        size_t read_chars = 0, data_size = 0;
        surgescript_parsetree_t* tree;

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
        parser->filename = surgescript_util_strdup(ssbasename(absolute_path));
        surgescript_lexer_set(parser->lexer, data);
        tree = parse(parser);

        /* done! */
        ssfree(data);
        return tree;
    }
    else {
        ssfatal("Parse Error: can't read file \"%s\": %s", absolute_path, strerror(errno));
        return NULL;
    }
}

/*
 * surgescript_parser_parsefile()
 * Parse a script stored in memory
 */
surgescript_parsetree_t* surgescript_parser_parsemem(surgescript_parser_t* parser, const char* code_in_memory)
{
    ssfree(parser->filename);
    parser->filename = surgescript_util_strdup("<memory>");
    surgescript_lexer_set(parser->lexer, code_in_memory);
    return parse(parser);
}





/* privates & helpers */



/* parses a script */
surgescript_parsetree_t* parse(surgescript_parser_t* parser)
{
    parser->lookahead = surgescript_lexer_scan(parser->lexer); /* grab first symbol */
    return objectlist(parser);
}

/* does the lookahead symbol have the given type? */
bool gottype(surgescript_parser_t* parser, surgescript_tokentype_t symbol)
{
    return parser->lookahead && surgescript_token_type(parser->lookahead) == symbol;
}

/* match a symbol; throw a fatal error if the symbol is not matched */
void match(surgescript_parser_t* parser, surgescript_tokentype_t symbol)
{
    if(gottype(parser, symbol)) {
        surgescript_token_destroy(parser->lookahead);
        parser->lookahead = surgescript_lexer_scan(parser->lexer); /* grab next symbol */
    }
    else
        expect(parser, symbol);
}

/* match the given symbol or the empty symbol */
bool optmatch(surgescript_parser_t* parser, surgescript_tokentype_t symbol)
{
    if(gottype(parser, symbol)) {
        match(parser, symbol);
        return true;
    }
    else
        return false;
}

/* throw an error if the lookahead is not of the expected type */
void expect(surgescript_parser_t* parser, surgescript_tokentype_t symbol)
{
    if(parser->lookahead && surgescript_token_type(parser->lookahead) != symbol) {
        ssfatal(
            "Parse Error: expected \"%s\" on %s near line %d.",
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

/* returns an empty parse tree */
surgescript_parsetree_t* emptytree()
{
    return surgescript_parsetree_create();
}







/* non-terminals of the grammar */

surgescript_parsetree_t* objectlist(surgescript_parser_t* parser)
{
    if(parser->lookahead) {
        surgescript_parsetree_t* node;
        surgescript_parsetree_t* objectnode;
        surgescript_parsetree_t* objectlistnode;

        objectnode = object(parser);
        objectlistnode = objectlist(parser);

        node = surgescript_parsetree_create_objectlist(objectnode, objectlistnode);
        return node;
    }
    else
        return emptytree(); /* end of file */
}

surgescript_parsetree_t* object(surgescript_parser_t* parser)
{
    surgescript_parsetree_t* objectdeclnode;
    surgescript_nodecontext_t context;

    match(parser, SSTOK_OBJECT);
    expect(parser, SSTOK_STRING);
    context = nodecontext(
        surgescript_util_strdup(surgescript_token_lexeme(parser->lookahead)), /* object name */
        NULL, /* symbol table */
        surgescript_program_create(0, 0) /* object constructor */
    );
    match(parser, SSTOK_STRING);
    match(parser, SSTOK_LCURLY);
    objectdeclnode = objectdecl(parser, context);
    match(parser, SSTOK_RCURLY);

    return surgescript_parsetree_create_object(context, objectdeclnode);
}

surgescript_parsetree_t* objectdecl(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_parsetree_t* notelistnode;
    surgescript_parsetree_t* vardecllistnode;
    surgescript_parsetree_t* statedecllistnode;
    surgescript_parsetree_t* fundecllistnode;

    notelistnode = emptytree();
    vardecllistnode = emptytree();
    statedecllistnode = emptytree();
    fundecllistnode = emptytree();

    return surgescript_parsetree_create_objectdecl(context, notelistnode, vardecllistnode, statedecllistnode, fundecllistnode);
}




/* notes */

surgescript_parsetree_t* signedconst(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_token_t* token;
    surgescript_parsetree_t* node;
    
    expect_something(parser);
    token = parser->lookahead;

    switch(surgescript_token_type(token)) {
        case SSTOK_NULL:
            node = surgescript_parsetree_create_null(context);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_TRUE:
            node = surgescript_parsetree_create_bool(context, true);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_FALSE:
            node = surgescript_parsetree_create_bool(context, false);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_STRING:
            node = surgescript_parsetree_create_string(context, surgescript_token_lexeme(token));
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_NUMBER:
        case SSTOK_ADDITIVEOP:
            node = signednum(parser, context);
            break;

        default:
            ssfatal("Parse Error: expected a signedconst value on %s near line %d.", parser->filename, surgescript_token_linenumber(token));
            return emptytree();
    }

    return node;
}

surgescript_parsetree_t* signednum(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    surgescript_token_t* token;
    
    expect_something(parser);
    token = parser->lookahead;

    if(gottype(parser, SSTOK_ADDITIVEOP)) {
        float value = 0.0;
        bool plus = !strcmp(surgescript_token_lexeme(token), "+");

        match(parser, SSTOK_ADDITIVEOP);
        if(gottype(parser, SSTOK_NUMBER))
            value = atof(surgescript_token_lexeme(token));
        match(parser, SSTOK_NUMBER);

        return surgescript_parsetree_create_number(context, plus ? value : -value);
    }
    else if(gottype(parser, SSTOK_NUMBER)) {
        float value = atof(surgescript_token_lexeme(token));
        match(parser, SSTOK_NUMBER);
        return surgescript_parsetree_create_number(context, value);
    }

    expect(parser, SSTOK_NUMBER); /* will throw an error */
    return emptytree();
}


surgescript_parsetree_t* vardecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* vardecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* statedecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* statedecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* fundecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* fundecl(surgescript_parser_t* parser, surgescript_nodecontext_t context);

surgescript_parsetree_t* expr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* assignexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* conditionalexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* logicalorexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* logicalorexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* logicalandexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* logicalandexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* equalityexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* equalityexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* relationalexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* relationalexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* additiveexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* additiveexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* multiplicativeexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* multiplicativeexpr1(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* unaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* primaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* constant(surgescript_parser_t* parser, surgescript_nodecontext_t context);

surgescript_parsetree_t* stmtlist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* stmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* blockstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* exprstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* condstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* loopstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
surgescript_parsetree_t* jumpstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);


