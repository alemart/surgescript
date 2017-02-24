/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * compiler/lexer.c
 * SurgeScript compiler: lexical analyzer
 */

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "token.h"
#include "../util/util.h"

/* size of the internal buffer */
#define BUFSIZE                     1024

/* lexer */
struct surgescript_lexer_t
{
    char buf[BUFSIZE]; /* auxiliary buffer */
    int bufptr; /* auxiliary buffer ptr */
    const char* p; /* auxiliary pointer */
    const char* lp; /* last position of the auxiliary pointer */
    int ll; /* line number of the last scan */
    int line; /* current line */
};

/* keywords */
static surgescript_tokentype_t keyword[] = { SSTOK_TRUE, SSTOK_FALSE, SSTOK_NULL, SSTOK_OBJECT, SSTOK_STATE, SSTOK_FUN, SSTOK_RETURN, SSTOK_THIS, SSTOK_IF, SSTOK_ELSE, SSTOK_WHILE, SSTOK_FOR, SSTOK_IN, SSTOK_BREAK, SSTOK_CONTINUE };
static int indexof_keyword(const char* identifier);
static inline void bufadd(surgescript_lexer_t* lexer, char c);
static inline void bufclear(surgescript_lexer_t* lexer);
static inline void skipspaces(surgescript_lexer_t* lexer);

/* helpers */
#define isidchar(c)                 (isalnum(c) || (c) == '_' || (c) == '$')  /* is c an identifier-char? */
#define isnumeric(c)                (isdigit(c) || (c) == '.') /* is c a char belonging to a number? */
#define iswhitespace(c)             ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == '\f' || (c) == '\v')



/* public api */

/*
 * surgescript_lexer_create()
 * Creates a lexer
 */
surgescript_lexer_t* surgescript_lexer_create()
{
    surgescript_lexer_t* lexer = ssmalloc(sizeof *lexer);
    lexer->buf[0] = 0;
    lexer->bufptr = 0;
    lexer->p = 0;
    lexer->line = 0;
    lexer->lp = 0;
    lexer->ll = 0;
    return lexer;
}

/*
 * surgescript_lexer_destroy()
 * Destroys a lexer
 */
surgescript_lexer_t* surgescript_lexer_destroy(surgescript_lexer_t* lexer)
{
    return ssfree(lexer);
}

/*
 * surgescript_lexer_set()
 * Sets the code to be read
 */
void surgescript_lexer_set(surgescript_lexer_t* lexer, const char* code)
{
    lexer->p = code;
    lexer->line = 1;
    lexer->lp = lexer->p;
    lexer->ll = lexer->line;
}


/*
 * surgescript_lexer_scan()
 * Scans the next token. Returns NULL if there are no more tokens
 */
surgescript_token_t* surgescript_lexer_scan(surgescript_lexer_t* lexer)
{
    /* save previous state */
    lexer->lp = lexer->p;
    lexer->ll = lexer->line;

    /* clear previous token (if any) */
    bufclear(lexer);

    /* skip spaces */
    skipspaces(lexer);

    /* skip comments */
    while(1) {
        if(*(lexer->p) == '/' && *(lexer->p + 1) == '/') { /* single-line comments */
            while(*(lexer->p) != '\n' && *(lexer->p) != 0)
                lexer->p++;
            skipspaces(lexer);
        }
        else if(*(lexer->p) == '/' && *(lexer->p + 1) == '*') { /* multiline comments */
            while(!(*(lexer->p) == '*' && *(lexer->p + 1) == '/') && *(lexer->p) != 0) {
                if('\n' == *(lexer->p++))
                    lexer->line++;
            }

            if(*(lexer->p) == 0)
                ssfatal("Lexical Error: Unexpected end of commentary block around line %d.", lexer->ll);
            else
                lexer->p += 2;

            skipspaces(lexer);
        }
        else
            break;
    }

    /* read number */
    if(isdigit(*(lexer->p)) || (*(lexer->p) == '.' && isdigit(*(lexer->p + 1)))) {
        bool dot = false;

        /* read the number */
        while(isnumeric(*(lexer->p))) {
            if(*(lexer->p) == '.') {
                if(dot) /* only one dot is allowed */
                    ssfatal("Lexical Error: Unexpected '%c' around \"%s\" on line %d", *(lexer->p), lexer->buf, lexer->line);
                dot = true;
            }
            bufadd(lexer, *(lexer->p++)); /* add to buffer */
        }

        /* done! */
        return surgescript_token_create(SSTOK_NUMBER, lexer->buf, lexer->line);
    }

    /* read string */
    if(*(lexer->p) == '"' || *(lexer->p) == '\'') {
        /* quote type */
        const char quo = *(lexer->p);

        /* skip starting quote mark */
        lexer->p++;

        /* read string contents */
        while(*(lexer->p) != quo && *(lexer->p) != 0) {
            /* read special character, with a backslash */
            if(*(lexer->p) == '\\') {
                switch(*(++lexer->p)) {
                    case '\\': bufadd(lexer, '\\'); break;
                    case '\'': bufadd(lexer, '\''); break;
                    case '"': bufadd(lexer, '"'); break;
                    case 'n': bufadd(lexer, '\n'); break;
                    case 'r': bufadd(lexer, '\r'); break;
                    case 't': bufadd(lexer, '\t'); break;
                    case 'f': bufadd(lexer, '\f'); break;
                    case 'v': bufadd(lexer, '\v'); break;
                    case 'b': bufadd(lexer, '\b'); break;
                    default:
                        ssfatal("Lexical Error: Invalid character '\\%c' around \"%s\" on line %d.", *(lexer->p) != 0 ? *(lexer->p) : '0', lexer->buf, lexer->line);
                        break;
                }
                lexer->p++;
                continue;
            }

            /* found a new line */
            if(*(lexer->p) == '\n') {
                ssfatal("Lexical Error: Unexpected end of line around \"%s\" on line %d.", lexer->buf, lexer->line);
                lexer->line++;
            }

            /* add character to the buffer */
            bufadd(lexer, *(lexer->p++));
        }

        /* is everything ok? */
        if(*(lexer->p) != quo)
            ssfatal("Lexical Error: Unexpected end of string around \"%s\" on line %d.", lexer->buf, lexer->line); /* found a NULL character */
        else
            lexer->p++; /* skip ending quotation mark */

        /* done! */
        return surgescript_token_create(SSTOK_STRING, lexer->buf, lexer->line);
    }

    /* semicolon */
    if(*(lexer->p) == ';') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_SEMICOLON, lexer->buf, lexer->line);       
    }

    /* comma */
    if(*(lexer->p) == ',') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_COMMA, lexer->buf, lexer->line);       
    }

    /* conditional operator */
    if(*(lexer->p) == '?') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_CONDITIONALOP, lexer->buf, lexer->line);       
    }

    /* colon operator */
    if(*(lexer->p) == ':') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_COLON, lexer->buf, lexer->line);       
    }

    /* dot */
    if(*(lexer->p) == '.' && !isdigit(*(lexer->p + 1))) {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_DOT, lexer->buf, lexer->line);       
    }

    /* arrow operator */
    if(*(lexer->p) == '=' && *(lexer->p + 1) == '>') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ARROWOP, lexer->buf, lexer->line);       
    }

    /* parenthesis */
    if(*(lexer->p) == '(') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_LPAREN, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == ')') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RPAREN, lexer->buf, lexer->line);
    }

    /* brackets */
    if(*(lexer->p) == '[') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_LBRACKET, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == ']') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RBRACKET, lexer->buf, lexer->line);
    }

    /* curly braces */
    if(*(lexer->p) == '{') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_LCURLY, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == '}') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RCURLY, lexer->buf, lexer->line);
    }

    /* not operator */
    if(*(lexer->p) == '!' && *(lexer->p + 1) != '=') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_NOTOP, lexer->buf, lexer->line);       
    }

    /* assignment operator */
    if(*(lexer->p) == '=' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '>') { /* just a simple '=' for attribution */
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ASSIGNOP, lexer->buf, lexer->line);
    }

    /* relational operators */
    if(*(lexer->p) == '=' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == '!' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == '>' && *(lexer->p + 1) != '=') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == '>' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == '<' && *(lexer->p + 1) != '=') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == '<' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }

    /* additive operators */
    if(*(lexer->p) == '+' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '+') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ADDITIVEOP, lexer->buf, lexer->line);       
    }
    else if(*(lexer->p) == '-' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '-') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ADDITIVEOP, lexer->buf, lexer->line);       
    }

    /* multiplicative operators */
    if(*(lexer->p) == '*' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '/') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_MULTIPLICATIVEOP, lexer->buf, lexer->line);       
    }
    else if(*(lexer->p) == '/' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '/' && *(lexer->p + 1) != '*') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_MULTIPLICATIVEOP, lexer->buf, lexer->line);       
    }
    
    /* additive-assignment operators */
    if(*(lexer->p) == '+' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ADDITIVEASSIGNOP, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == '-' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ADDITIVEASSIGNOP, lexer->buf, lexer->line);
    }

    /* multiplicative-assignment operators */
    if(*(lexer->p) == '*' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_MULTIPLICATIVEASSIGNOP, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == '/' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_MULTIPLICATIVEASSIGNOP, lexer->buf, lexer->line);
    }

    /* and-or boolean operators */
    if(*(lexer->p) == '&' && *(lexer->p + 1) == '&') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ANDOROP, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == '|' && *(lexer->p + 1) == '|') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ANDOROP, lexer->buf, lexer->line);
    }

    /* increment-decrement operators */
    if(*(lexer->p) == '+' && *(lexer->p + 1) == '+') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_INCDECOP, lexer->buf, lexer->line);
    }
    else if(*(lexer->p) == '-' && *(lexer->p + 1) == '-') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_INCDECOP, lexer->buf, lexer->line);
    }

    /* read an identifier */
    if(isidchar(*(lexer->p)) && !isdigit(*(lexer->p))) {
        int kw;

        /* read the whole thing */
        while(isidchar(*(lexer->p)))
            bufadd(lexer, *(lexer->p++));

        /* is this a keyword? */
        if((kw = indexof_keyword(lexer->buf)) >= 0)
            return surgescript_token_create(keyword[kw], lexer->buf, lexer->line);

        /* no, it's a regular identifier */
        return surgescript_token_create(SSTOK_IDENTIFIER, lexer->buf, lexer->line);
    }

    /* end of code */
    if(*(lexer->p) == 0)
        return NULL;

    /* well, we don't know what we've got */
    bufadd(lexer, *(lexer->p++));
    return surgescript_token_create(SSTOK_UNKNOWN, lexer->buf, lexer->line);
}

/*
 * surgescript_lexer_rewind()
 * Rewinds the scanning on one token
 */
bool surgescript_lexer_rewind(surgescript_lexer_t* lexer)
{
    bool val = (lexer->p != lexer->lp);

    lexer->p = lexer->lp;
    lexer->line = lexer->ll;
    
    return val;
}


/* private stuff */

/* is the given keyword an identifier? returns its index in keyword[], or -1 if not found */
int indexof_keyword(const char* identifier)
{
    const int length = sizeof(keyword) / sizeof(surgescript_tokentype_t);

    for(int i = 0; i < length; i++) {
        if(strcmp(identifier, surgescript_tokentype_name(keyword[i])) == 0)
            return i;
    }

    return -1;
}

/* adds a character to the stringbuffer */
void bufadd(surgescript_lexer_t* lexer, char c)
{
    const int size = BUFSIZE - 1;

    if(lexer->bufptr < size) {
        lexer->buf[lexer->bufptr++] = c;
        lexer->buf[lexer->bufptr] = 0;
    }
    else
        ssfatal("Lexical Error: This token is too large! See \"%s\" around line %d.", lexer->buf, lexer->line);
}

/* clears the stringbuffer */
void bufclear(surgescript_lexer_t* lexer)
{
    lexer->buf[0] = 0;
    lexer->bufptr = 0;
}

/* skips white spaces */
void skipspaces(surgescript_lexer_t* lexer)
{
    while(isspace(*(lexer->p))) {
        if('\n' == *(lexer->p))
            lexer->line++;
        lexer->p++;
    }
}