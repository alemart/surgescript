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
#include "../util/util.h"

/* the parser */
struct surgescript_parser_t
{
    surgescript_token_t* lookahead;
    surgescript_lexer_t* lexer;
    char* filename;
    // programa q estou examinando aki (usa pilha)
};

/* helpers */
static surgescript_parsetree_t* parse(surgescript_parser_t* parser);
static void match(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
static bool optmatch(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
static bool gottype(surgescript_parser_t* parser, surgescript_tokentype_t symbol);
static void check_if_eof(surgescript_parser_t* parser);
static const char* ssbasename(const char* path);
static surgescript_parsetree_t* emptytree();

/* non-terminals */
static surgescript_parsetree_t* objectlist(surgescript_parser_t* parser);
static surgescript_parsetree_t* objectdecl(surgescript_parser_t* parser);
static surgescript_parsetree_t* signedconst(surgescript_parser_t* parser);
static surgescript_parsetree_t* signednum(surgescript_parser_t* parser);
static surgescript_parsetree_t* signedval(surgescript_parser_t* parser);


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

/* privates */

/* parses a script */
surgescript_parsetree_t* parse(surgescript_parser_t* parser)
{
    parser->lookahead = surgescript_lexer_scan(parser->lexer); /* grab first symbol */
    return objectlist(parser);
}

surgescript_parsetree_t* objectlist(surgescript_parser_t* parser)
{
    if(parser->lookahead) {
        surgescript_parsetree_t* node;
        surgescript_parsetree_t* decl;
        surgescript_parsetree_t* list;
        surgescript_token_t* name;

        /* read the object */
        match(parser, SSTOK_OBJECT);
        name = surgescript_token_clone(parser->lookahead);
        match(parser, SSTOK_STRING);
        match(parser, SSTOK_LCURLY);
        decl = objectdecl(parser);
        match(parser, SSTOK_RCURLY);

        /* read other objects */
        list = objectlist(parser);
        node = surgescript_parsetree_create_objectlist(surgescript_token_lexeme(name), decl, list);
        surgescript_token_destroy(name);
        return node;
    }
    else
        return emptytree(); /* end of file */
}

surgescript_parsetree_t* objectdecl(surgescript_parser_t* parser)
{
    return signedconst(parser);
}

surgescript_parsetree_t* signedconst(surgescript_parser_t* parser)
{
    surgescript_token_t* token = parser->lookahead;
    surgescript_parsetree_t* node = NULL;
    
    check_if_eof(parser);

    switch(surgescript_token_type(token)) {
        case SSTOK_NULL:
            node = surgescript_parsetree_create_null(NULL);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_TRUE:
            node = surgescript_parsetree_create_bool(NULL, true);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_FALSE:
            node = surgescript_parsetree_create_bool(NULL, false);
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_STRING:
            node = surgescript_parsetree_create_string(NULL, surgescript_token_lexeme(token));
            match(parser, surgescript_token_type(token));
            break;

        case SSTOK_NUMBER:
        case SSTOK_ADDITIVEOP:
            node = signednum(parser);
            break;

        default:
            ssfatal("Parse Error: expected a signedconst value on %s near line %d.", parser->filename, surgescript_token_linenumber(token));
            break;
    }

    return node;
}

surgescript_parsetree_t* signednum(surgescript_parser_t* parser)
{
    surgescript_token_t* token = parser->lookahead;

    if(gottype(parser, SSTOK_ADDITIVEOP)) {
        float value = 0.0;
        bool plus = !strcmp(surgescript_token_lexeme(token), "+");

        match(parser, SSTOK_ADDITIVEOP);
        if(gottype(parser, SSTOK_NUMBER))
            value = atof(surgescript_token_lexeme(token));
        match(parser, SSTOK_NUMBER);

        return surgescript_parsetree_create_number(NULL, plus ? value : -value);
    }
    else if(gottype(parser, SSTOK_NUMBER)) {
        float value = atof(surgescript_token_lexeme(token));
        match(parser, SSTOK_NUMBER);
        return surgescript_parsetree_create_number(NULL, value);
    }

    match(parser, SSTOK_NUMBER); /* will throw an error */
    return surgescript_parsetree_create_number(NULL, 0.0f);
}

surgescript_parsetree_t* signedval(surgescript_parser_t* parser)
{
    /* TODO */
    return signednum(parser);
}


/* helpers */

/* match a symbol; throw a fatal error if the symbol is not matched */
void match(surgescript_parser_t* parser, surgescript_tokentype_t symbol)
{
    if(parser->lookahead && surgescript_token_type(parser->lookahead) == symbol) {
        surgescript_token_destroy(parser->lookahead);
        parser->lookahead = surgescript_lexer_scan(parser->lexer); /* grab next symbol */
    }
    else if(parser->lookahead) {
        ssfatal(
            "Parse Error: expected \"%s\" on %s near line %d.",
            surgescript_tokentype_name(symbol),
            parser->filename,
            surgescript_token_linenumber(parser->lookahead)
        );
    }
    else {
        ssfatal(
            "Parse Error: unexpected end of file on %s (did you forget a %s?)",
            parser->filename,
            surgescript_tokentype_name(symbol)
        );
    }
}

/* match the given symbol or the empty symbol */
bool optmatch(surgescript_parser_t* parser, surgescript_tokentype_t symbol)
{
    if(parser->lookahead && surgescript_token_type(parser->lookahead) == symbol) {
        match(parser, symbol);
        return true;
    }
    else
        return false;
}

/* does the lookahead symbol have the given type? */
bool gottype(surgescript_parser_t* parser, surgescript_tokentype_t symbol)
{
    return parser->lookahead && surgescript_token_type(parser->lookahead) == symbol;
}

/* check if we have reached the end of the file; throw an error if so */
void check_if_eof(surgescript_parser_t* parser)
{
    if(NULL == parser->lookahead) {
         ssfatal(
            "Parse Error: unexpected end of file on %s",
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