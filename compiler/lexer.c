/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * compiler/lexer.c
 * SurgeScript compiler: lexical analyzer
 */

#include <ctype.h>
#include "lexer.h"
#include "token.h"
#include "../util/util.h"
#include "../util/ssarray.h"

/* lexer */
struct surgescript_lexer_t
{
    char buf[1024]; /* auxiliary buffer */
    int bufptr; /* auxiliary buffer ptr */
    const char* p; /* auxiliary pointer */
    int line; /* current line */
    const char* lp; /* last position of the auxiliary pointer */
    int ll; /* line number of the last scan */
    //SSARRAY(surgescript_token_t*, tokens);
};

/* keywords */
static surgescript_tokentype_t keywords[] = { SSTOK_TRUE, SSTOK_FALSE, SSTOK_NULL, SSTOK_OBJECT, SSTOK_FUN, SSTOK_RETURN, SSTOK_IF, SSTOK_ELSE, SSTOK_WHILE, SSTOK_FOR, SSTOK_IN, SSTOK_BREAK, SSTOK_CONTINUE };
static int indexof_keyword(const char* identifier);
static inline bool is_unary_predecessor(surgescript_tokentype_t t);
static inline void bufadd(surgescript_lexer_t* lexer, char c);
static inline void bufclear(surgescript_lexer_t* lexer);

/* helpers */
#define isidchar(c)                 (isalnum(c) || (c) == '_' || (c) == '$')  /* is c an identifier-char? */
#define isnumchar(c)                (isdigit(c) || (c) == '.') /* is c a char belonging to a number? */
#define iswhitespace(c)             ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == '\f' || (c) == '\v')




/* public api */

/*
 * surgescript_lexer_create()
 * Creates a lexer
 */
surgescript_lexer_t* surgescript_lexer_create()
{
    surgescript_lexer_t* lexer = ssmalloc(sizeof *lexer);
    //ssarray_init(lexer->tokens);
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
    //ssarray_release(lexer->tokens);
    return ssfree(lexer);
}

/*
 * surgescript_lexer_tokens()
 * Returns the tokens read by the lexer so far
 */
/*surgescript_token_t* surgescript_lexer_tokens(surgescript_lexer_t* lexer, int* num_tokens)
{
    *num_tokens = ssarray_length(lexer->tokens);
    return lexer->tokens;
}*/

/*
 * surgescript_lexer_clear()
 * clears all tokens read by the lexer so far
 */
/*void surgescript_lexer_clear(surgescript_lexer_t* lexer)
{
    ssarray_release(lexer->tokens);
    ssarray_init(lexer->tokens);
}*/

/*
 * surgescript_lexer_feed()
 * Feeds code to the lexer
 */
bool surgescript_lexer_feed(surgescript_lexer_t* lexer, const char* code)
{
    lexer->p = code;
    lexer->line = 1;
    lexer->lp = lexer->p;
    lexer->ll = lexer->line;
    return true;
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
    while(isspace(*lexer->p)) {
        if('\n' == *lexer->p)
            lexer->line++;
        lexer->p++;
    }

    /* read number */
    if(isdigit(*lexer->p) || ((*lexer->p == '.') && isdigit(*(lexer->p + 1)))) {
        bool dot = false;

        /* got a unary operator */
        /*if((*lexer->p == '+' || *lexer->p == '-') && is_unary_predecessor(lexer->last))
            bufadd(lexer, *(lexer->p++));*/

        /* read the number */
        while(isnumchar(*lexer->p)) {
            if(*lexer->p == '.') {
                if(dot) /* only one dot is allowed */
                    ssfatal("Unexpected '%c' around \"%s\" on line %d", , *lexer->p, lexer->buf, lexer->line);
                dot = true;
            }
            bufadd(lexer, *(lexer->p++)); /* add to buffer */
        }

        /* done! */
        return surgescript_token_create(SSTOK_NUMBER, lexer->buf, lexer->line);
    }

    /* read string */
    if(*lexer->p == '"') {
        lexer->p++; /* skip starting '"' */

        /* read string contents */
        while(*lexer->p != '"' && *lexer->p != 0) {
            /* read special character, with a backslash */
            if(*lexer->p == '\\') {
                switch(*(++lexer->p)) {
                    case 'n': bufadd(lexer, '\n'); break;
                    case 'r': bufadd(lexer, '\r'); break;
                    case 't': bufadd(lexer, '\t'); break;
                    case 'f': bufadd(lexer, '\f'); break;
                    case 'v': bufadd(lexer, '\v'); break;
                    case '"': bufadd(lexer, '"'); break;
                    case '\\': bufadd(lexer, '\\'); break;
                    default:
                        ssfatal("Invalid character '\\%c' around \"%s\" on line %d.", *lexer->p != 0 ? *lexer->p : '0', lexer->buf, lexer->line);
                        break;
                }
                lexer->p++;
                continue;
            }

            /* found a new line */
            if(*lexer->p == '\n') {
                ssfatal("Unexpected end of line around \"%s\" on line %d.", lexer->buf, lexer->line);
                lexer->line++;
            }

            /* add character to the buffer */
            bufadd(lexer, *(lexer->p++));
        }

        /* is everything ok? */
        if(*lexer->p != '"')
            ssfatal("Unexpected end of string around \"%s\" on line %d.", lexer->buf, lexer->line);
        else
            lexer->p++; /* skip ending '"' */

        /* done! */
        return surgescript_token_create(SSTOK_STRING, lexer->buf, lexer->line);
    }

    /* semicolon */
    if(*lexer->p == ';') {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_SEMICOLON, lexer->buf, lexer->line);       
    }

    /* comma */
    if(*lexer->p == ',') {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_COMMA, lexer->buf, lexer->line);       
    }

    /* question operator */
    if(*lexer->p == '?') {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_CONDITIONALOP, lexer->buf, lexer->line);       
    }

    /* colon operator */
    if(*lexer->p == ':') {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_COLON, lexer->buf, lexer->line);       
    }

    /* dot */
    if(*lexer->p == '.' && !isdigit(*(lexer->p + 1))) {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_DOT, lexer->buf, lexer->line);       
    }

    /* assignment operator */
    if(*lexer->p == '=' && *(lexer->p + 1) != '=') { /* just a simple '=', e.g. for constants */
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_ASSIGNOP, lexer->buf, lexer->line);
    }

    /* relational operators */
    if(*lexer->p == '=' && *(lexer->p + 1) == '=') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '!' && *(lexer->p + 1) == '=') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '>' && *(lexer->p + 1) != '=') {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '>' && *(lexer->p + 1) == '=') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '<' && *(lexer->p + 1) != '=') {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '<' && *(lexer->p + 1) == '=') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line);
    }

    /* arithmetic operators */
    if(*lexer->p == '+' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '+') {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_ARITHMETICOP, lexer->buf, lexer->line);       
    }
    else if(*lexer->p == '-' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '-') {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_ARITHMETICOP, lexer->buf, lexer->line);       
    }
    else if(*lexer->p == '*' && *(lexer->p + 1) != '=') {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_ARITHMETICOP, lexer->buf, lexer->line);       
    }
    else if(*lexer->p == '/' && *(lexer->p + 1) != '=') {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_ARITHMETICOP, lexer->buf, lexer->line);       
    }
    
    /* arithmetic-assignment operators */
    if(*lexer->p == '+' && *(lexer->p + 1) == '=') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_ARITHASSIGNOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '-' && *(lexer->p + 1) == '=') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_ARITHASSIGNOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '*' && *(lexer->p + 1) == '=') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_ARITHASSIGNOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '/' && *(lexer->p + 1) == '=') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_ARITHASSIGNOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '+' && *(lexer->p + 1) == '+') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_ARITHASSIGNOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '-' && *(lexer->p + 1) == '-') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_ARITHASSIGNOP, lexer->buf, lexer->line);
    }

    /* boolean operators */
    if(*lexer->p == '&' && *(lexer->p + 1) == '&') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_BOOLEANOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '|' && *(lexer->p + 1) == '|') {
        bufadd(lexer, lexer->p++);
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_BOOLEANOP, lexer->buf, lexer->line);
    }
    else if(*lexer->p == '!') {
        bufadd(lexer, lexer->p++);
        return surgescript_token_create(SSTOK_BOOLEANOP, lexer->buf, lexer->line);       
    }

    /* read identifiers */
        /* is this a keyword? */

    /* end of code */
    if(*lexer->p == 0) {
        return NULL;
    }
}


/* private stuff */

/* is the given keyword an identifier? returns its index in keywords[], or -1 if not found */
int indexof_keyword(const char* identifier)
{
    const int length = sizeof(keywords) / sizeof(surgescript_tokentype_t);

    for(int i = 0; i < length; i++) {
        if(strcmp(identifier, surgescript_tokentype_name(keywords[i])) == 0)
            return i;
    }

    return -1;
}

/* adds a character to the stringbuffer */
void bufadd(surgescript_lexer_t* lexer, char c)
{
    const int size = sizeof(lexer->buf) / sizeof(char) - 1;

    if(lexer->bufptr < size) {
        lexer->buf[lexer->bufptr++] = c;
        lexer->buf[lexer->bufptr] = 0;
    }
    else
        ssfatal("This token is too large! See '%s' around line %d.", lexer->buf, lexer->line);
}

/* clears the stringbuffer */
void bufclear(surgescript_lexer_t* lexer)
{
    lexer->buf[0] = 0;
    lexer->bufptr = 0;
}

/* is token t a predecessor of a unary operator? */
bool is_unary_predecessor(surgescript_tokentype_t t)
{
    switch(t) {
        case SSTOK_LPAREN:
        case SSTOK_LBRACKET:
        case SSTOK_LCURLY:
        case SSTOK_SEMICOLON:
        case SSTOK_COMMA:
        case SSTOK_RETURN:
        case SSTOK_ASSIGN:
        /*case SSTOK_BINARYOP:
        case SSTOK_UNARYOP:*/
            return true;
    }

    return false;
}