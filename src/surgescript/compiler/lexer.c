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
 * compiler/lexer.c
 * SurgeScript compiler: lexical analyzer
 */

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "token.h"
#include "../util/util.h"
#include "../util/ssarray.h"


/* previous state */
typedef struct surgescript_lexer_prevstate_t
{
    const char* p;
    int line;
} surgescript_lexer_prevstate_t;
static const surgescript_lexer_prevstate_t* prev_state(surgescript_lexer_t* lexer, const char* prev_p, int prev_line);

/* lexer */
#define BUFSIZE                     1024 /* size of the internal buffer */
struct surgescript_lexer_t
{
    char buf[BUFSIZE]; /* auxiliary buffer */
    int bufptr; /* auxiliary buffer ptr */
    const char* p; /* auxiliary pointer */
    int line; /* current line */
    SSARRAY(surgescript_lexer_prevstate_t, prev); /* previous states */
};

/* keywords */
static surgescript_tokentype_t keyword[] = { SSTOK_TRUE, SSTOK_FALSE, SSTOK_NULL, SSTOK_OBJECT, SSTOK_STATE, SSTOK_FUN, SSTOK_RETURN, SSTOK_THIS, SSTOK_IF, SSTOK_ELSE, SSTOK_WHILE, SSTOK_FOR, SSTOK_FOREACH, SSTOK_IN, SSTOK_BREAK, SSTOK_CONTINUE, SSTOK_TYPEOF, SSTOK_PUBLIC, SSTOK_USING, SSTOK_DO, SSTOK_WAIT, SSTOK_OF, SSTOK_IS, SSTOK_CALLER };
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
    ssarray_init(lexer->prev);
    return lexer;
}

/*
 * surgescript_lexer_destroy()
 * Destroys a lexer
 */
surgescript_lexer_t* surgescript_lexer_destroy(surgescript_lexer_t* lexer)
{
    ssarray_release(lexer->prev);
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
}


/*
 * surgescript_lexer_scan()
 * Scans the next token. Returns NULL if there are no more tokens
 */
surgescript_token_t* surgescript_lexer_scan(surgescript_lexer_t* lexer)
{
    /* previous state */
    const surgescript_lexer_prevstate_t* prev = prev_state(lexer, lexer->p, lexer->line);

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
                ssfatal("Lexical Error: Unexpected end of commentary block around line %d.", prev->line);
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
                else if(!isdigit(*(lexer->p + 1))) /* there must be a digit after the dot */
                    break;
                dot = true;
            }
            bufadd(lexer, *(lexer->p++)); /* add to buffer */
        }

        /* done! */
        return surgescript_token_create(SSTOK_NUMBER, lexer->buf, lexer->line, prev);
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
        return surgescript_token_create(SSTOK_STRING, lexer->buf, lexer->line, prev);
    }

    /* semicolon */
    if(*(lexer->p) == ';') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_SEMICOLON, lexer->buf, lexer->line, prev);
    }

    /* comma */
    if(*(lexer->p) == ',') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_COMMA, lexer->buf, lexer->line, prev);
    }

    /* conditional operator */
    if(*(lexer->p) == '?') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_CONDITIONALOP, lexer->buf, lexer->line, prev);
    }

    /* colon operator */
    if(*(lexer->p) == ':' && (*(lexer->p + 1) != ')' && *(lexer->p + 1) != '(' && *(lexer->p + 1) != 'P')) {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_COLON, lexer->buf, lexer->line, prev);
    }

    /* dot */
    if(*(lexer->p) == '.' && !isdigit(*(lexer->p + 1))) {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_DOT, lexer->buf, lexer->line, prev);
    }

    /* arrow operator */
    if(*(lexer->p) == '=' && *(lexer->p + 1) == '>') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ARROWOP, lexer->buf, lexer->line, prev);
    }

    /* parenthesis */
    if(*(lexer->p) == '(') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_LPAREN, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == ')') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RPAREN, lexer->buf, lexer->line, prev);
    }

    /* brackets */
    if(*(lexer->p) == '[') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_LBRACKET, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == ']') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RBRACKET, lexer->buf, lexer->line, prev);
    }

    /* curly braces */
    if(*(lexer->p) == '{') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_LCURLY, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '}') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RCURLY, lexer->buf, lexer->line, prev);
    }

    /* logical not operator */
    if(*(lexer->p) == '!' && *(lexer->p + 1) != '=') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_LOGICALNOTOP, lexer->buf, lexer->line, prev);
    }

    /* assignment operators */
    if(*(lexer->p) == '=' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '>') { /* just a simple '=' for attribution */
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ASSIGNOP, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '+' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ASSIGNOP, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '-' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ASSIGNOP, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '*' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ASSIGNOP, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '/' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ASSIGNOP, lexer->buf, lexer->line, prev);
    }


    /* equality operators */
    if(*(lexer->p) == '=' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        if(*(lexer->p) == '=')
            bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_EQUALITYOP, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '!' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        if(*(lexer->p) == '=')
            bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_EQUALITYOP, lexer->buf, lexer->line, prev);
    }

    /* relational operators */
    if(*(lexer->p) == '>' && *(lexer->p + 1) != '=') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '>' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '<' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '3') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '<' && *(lexer->p + 1) == '=') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_RELATIONALOP, lexer->buf, lexer->line, prev);
    }

    /* additive operators */
    if(*(lexer->p) == '+' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '+') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ADDITIVEOP, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '-' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '-') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ADDITIVEOP, lexer->buf, lexer->line, prev);
    }

    /* multiplicative operators */
    if(*(lexer->p) == '*' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '/') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_MULTIPLICATIVEOP, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '/' && *(lexer->p + 1) != '=' && *(lexer->p + 1) != '/' && *(lexer->p + 1) != '*') {
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_MULTIPLICATIVEOP, lexer->buf, lexer->line, prev);
    }
    
    /* logical and operator */
    if(*(lexer->p) == '&' && *(lexer->p + 1) == '&') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_LOGICALANDOP, lexer->buf, lexer->line, prev);
    }

    /* logical or operator */
    if(*(lexer->p) == '|' && *(lexer->p + 1) == '|') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_LOGICALOROP, lexer->buf, lexer->line, prev);
    }

    /* increment-decrement operators */
    if(*(lexer->p) == '+' && *(lexer->p + 1) == '+') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_INCDECOP, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '-' && *(lexer->p + 1) == '-') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_INCDECOP, lexer->buf, lexer->line, prev);
    }

    /* read emoticon */
    if(*(lexer->p) == '$' && 0 == strncmp(lexer->p, "$_$", 3)) {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_EMOTICON, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == '<' && *(lexer->p + 1) == '3') {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_EMOTICON, lexer->buf, lexer->line, prev);
    }
    else if(*(lexer->p) == ':' && (*(lexer->p + 1) == ')' || *(lexer->p + 1) == '(' || *(lexer->p + 1) == 'P')) {
        bufadd(lexer, *(lexer->p++));
        bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_EMOTICON, lexer->buf, lexer->line, prev);
    }

    /* read an annotation */
    if(*(lexer->p) == '@' && (isalpha(*(lexer->p + 1)) || *(lexer->p + 1) == '_')) {
        bufadd(lexer, *(lexer->p++));
        while(isalnum(*(lexer->p)) || *(lexer->p) == '_')
            bufadd(lexer, *(lexer->p++));
        return surgescript_token_create(SSTOK_ANNOTATION, lexer->buf, lexer->line, prev);
    }

    /* read an identifier */
    if(isidchar(*(lexer->p)) && !isdigit(*(lexer->p))) {
        int kw;

        /* read the whole thing */
        while(isidchar(*(lexer->p)))
            bufadd(lexer, *(lexer->p++));

        /* is this a keyword? */
        if((kw = indexof_keyword(lexer->buf)) >= 0)
            return surgescript_token_create(keyword[kw], lexer->buf, lexer->line, prev);

        /* no, it's a regular identifier */
        return surgescript_token_create(SSTOK_IDENTIFIER, lexer->buf, lexer->line, prev);
    }

    /* end of code */
    if(*(lexer->p) == 0)
        return NULL;

    /* well, we don't know what we've got */
    bufadd(lexer, *(lexer->p++));
    return surgescript_token_create(SSTOK_UNKNOWN, lexer->buf, lexer->line, prev);
}

/*
 * surgescript_lexer_unscan()
 * Rewinds the scanning, putting the given token back into the lexer
 */
bool surgescript_lexer_unscan(surgescript_lexer_t* lexer, surgescript_token_t* token)
{
    const surgescript_lexer_prevstate_t* prev = surgescript_token_data(token);
    bool val = (lexer->p != prev->p);
    lexer->p = prev->p;
    lexer->line = prev->line;
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
    static const int size = BUFSIZE - 1;

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

/* creates a new prev_state struct */
const surgescript_lexer_prevstate_t* prev_state(surgescript_lexer_t* lexer, const char* prev_p, int prev_line)
{
    surgescript_lexer_prevstate_t prev = { prev_p, prev_line };
    int last = ssarray_length(lexer->prev);
    ssarray_push(lexer->prev, prev);
    return &(lexer->prev[last]);
}