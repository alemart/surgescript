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
#include <locale.h>
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
#include "../util/ssarray.h"

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
static void unexpected_symbol(surgescript_parser_t* parser);
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
static void postfixexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void funcallexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void primaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void constant(surgescript_parser_t* parser, surgescript_nodecontext_t context);

static void stmtlist(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static bool stmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void blockstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void exprstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void condstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void loopstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void jumpstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);
static void retstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context);


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
    while(has_token(parser))
        object(parser);
}

void object(surgescript_parser_t* parser)
{
    surgescript_nodecontext_t context;
    char* object_name;

    match(parser, SSTOK_OBJECT);
    expect(parser, SSTOK_STRING);

    /* create the parsing context */
    context = nodecontext(
        parser->filename,
        (object_name = ssstrdup(
            surgescript_token_lexeme(parser->lookahead)
        )),
        surgescript_symtable_create(NULL), /* symbol table */
        surgescript_program_create(0) /* object constructor */
    );
    if(surgescript_programpool_exists(parser->program_pool, object_name, "__ssconstructor"))
        ssfatal("Duplicate definition of object \"%s\" in %s:%d.", object_name, context.source_file, surgescript_token_linenumber(parser->lookahead));

    /* read the object */
    match(parser, SSTOK_STRING);
    match(parser, SSTOK_LCURLY);
    objectdecl(parser, context);
    match(parser, SSTOK_RCURLY);

    /* register the ssconstructor and cleanup */
    surgescript_programpool_put(parser->program_pool, object_name, "__ssconstructor", context.program);
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
    statedecllist(parser, context);
    fundecllist(parser, context);

    /* tell the program how many variables should be allocated */
    emit_object_footer(context, start, end);
}

void vardecllist(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    while(got_type(parser, SSTOK_IDENTIFIER))
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

    /* function body */
    match(parser, SSTOK_LCURLY);
    fun_header = emit_function_header(context);
    stmtlist(parser, context);
    emit_function_footer(context, surgescript_symtable_count(context.symtable), fun_header);
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
    emit_function_footer(context, surgescript_symtable_count(context.symtable) - num_arguments, fun_header);
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
        match(parser, SSTOK_IDENTIFIER);

        if(got_type(parser, SSTOK_ASSIGNOP)) {
            int line = surgescript_token_linenumber(parser->lookahead);
            char* assignop = ssstrdup(surgescript_token_lexeme(parser->lookahead));
            match(parser, SSTOK_ASSIGNOP);
            assignexpr(parser, context);
            emit_assignexpr(context, assignop, identifier, line);
            ssfree(assignop);
        }
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
        char* id = ssstrdup(surgescript_token_lexeme(parser->lookahead));
        match(parser, SSTOK_IDENTIFIER);
        if(got_type(parser, SSTOK_INCDECOP)) {
            const char* op = surgescript_token_lexeme(parser->lookahead);
            emit_postincdec(context, op, id, surgescript_token_linenumber(parser->lookahead));
            match(parser, SSTOK_INCDECOP);
        }
        else if(optmatch(parser, SSTOK_LBRACKET)) {
            unexpected_symbol(parser); /* TODO */
        }
        else if(got_type(parser, SSTOK_LPAREN)) { /* we have a function call here */
            unmatch(parser); /* put the identifier back */
            emit_this(context);
            do {
                funcallexpr(parser, context);
            } while(optmatch(parser, SSTOK_DOT));
        }
        else {
            unmatch(parser);
            primaryexpr(parser, context);
            if(optmatch(parser, SSTOK_DOT)) {
                do {
                    funcallexpr(parser, context);
                } while(optmatch(parser, SSTOK_DOT));
            }
        }
        ssfree(id);
    }
    else {
        primaryexpr(parser, context);
        if(optmatch(parser, SSTOK_DOT)) {
            do {
                funcallexpr(parser, context);
            } while(optmatch(parser, SSTOK_DOT));
        }
    }
}

void funcallexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    int i = 0;
    char* id = ssstrdup(surgescript_token_lexeme(parser->lookahead));
    match(parser, SSTOK_IDENTIFIER);
    match(parser, SSTOK_LPAREN);

    emit_pushparam(context); /* push the object handle */
    if(!got_type(parser, SSTOK_RPAREN)) { /* read the parameters */
        do {
            i++;
            assignexpr(parser, context);
            emit_pushparam(context); /* push the i-th param */
        } while(optmatch(parser, SSTOK_COMMA));
    }
    emit_funcall(context, id, i);
    emit_popparams(context, 1 + i); /* pop the parameters and the object handle */

    match(parser, SSTOK_RPAREN);
    ssfree(id);
}

void primaryexpr(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{
    if(optmatch(parser, SSTOK_LPAREN)) {
        expr(parser, context);
        match(parser, SSTOK_RPAREN);
    }
    else if(optmatch(parser, SSTOK_THIS)) {
        emit_this(context);
    }
    else if(optmatch(parser, SSTOK_STATE)) {
        emit_state(context);
    }
    else if(optmatch(parser, SSTOK_APP)) {
        emit_app(context);
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

}

void jumpstmt(surgescript_parser_t* parser, surgescript_nodecontext_t context)
{

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