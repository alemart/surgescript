/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * compiler/token.h
 * SurgeScript compiler: tokens
 */

#ifndef _SURGESCRIPT_COMPILER_TOKEN_H
#define _SURGESCRIPT_COMPILER_TOKEN_H

#define SURGESCRIPT_TOKEN_TYPES(F)                                              \
    F( SSTOK_IDENTIFIER, "identifier" )                                         \
    F( SSTOK_NUMBER, "number" )                                                 \
    F( SSTOK_STRING, "string" )                                                 \
    F( SSTOK_TRUE, "true" )                                                     \
    F( SSTOK_FALSE, "false" )                                                   \
    F( SSTOK_NULL, "null" )                                                     \
    F( SSTOK_SEMICOLON, ";" )                                                   \
    F( SSTOK_COMMA, "," )                                                       \
    F( SSTOK_DOT, "." )                                                         \
    F( SSTOK_CONDITIONALOP, "?" )                                               \
    F( SSTOK_COLON, ":" )                                                       \
    F( SSTOK_LPAREN, "(" )                                                      \
    F( SSTOK_RPAREN, ")" )                                                      \
    F( SSTOK_LBRACKET, "[" )                                                    \
    F( SSTOK_RBRACKET, "]" )                                                    \
    F( SSTOK_LCURLY, "{" )                                                      \
    F( SSTOK_RCURLY, "}" )                                                      \
    F( SSTOK_NOTOP, "!" )                                                       \
    F( SSTOK_ARROWOP, "=>" )                                                    \
    F( SSTOK_ASSIGNOP, "=" )                                                    \
    F( SSTOK_ADDITIVEOP, "additive operator" )                                  \
    F( SSTOK_MULTIPLICATIVEOP, "multiplicative operator" )                      \
    F( SSTOK_ADDITIVEASSIGNOP, "additive-assignment operator" )                 \
    F( SSTOK_MULTIPLICATIVEASSIGNOP, "multiplicative-assignment operator" )     \
    F( SSTOK_INCDECOP, "increment-decrement operator" )                         \
    F( SSTOK_RELATIONALOP, "relational operator" )                              \
    F( SSTOK_ANDOROP, "and-or boolean operator" )                               \
    F( SSTOK_OBJECT, "object" )                                                 \
    F( SSTOK_STATE, "state" )                                                   \
    F( SSTOK_FUN, "fun" )                                                       \
    F( SSTOK_RETURN, "return" )                                                 \
    F( SSTOK_THIS, "this" )                                                     \
    F( SSTOK_IF, "if" )                                                         \
    F( SSTOK_ELSE, "else" )                                                     \
    F( SSTOK_WHILE, "while" )                                                   \
    F( SSTOK_FOR, "for" )                                                       \
    F( SSTOK_IN, "in" )                                                         \
    F( SSTOK_BREAK, "break" )                                                   \
    F( SSTOK_CONTINUE, "continue" )                                             \
    F( SSTOK_NOTES, "notes" )                                                   \
    F( SSTOK_UNKNOWN, "<unknown>" )

typedef struct surgescript_token_t surgescript_token_t;
typedef enum surgescript_tokentype_t {
    #define TOKEN_CODE(x, y) x,
    SURGESCRIPT_TOKEN_TYPES(TOKEN_CODE)
} surgescript_tokentype_t;

surgescript_token_t* surgescript_token_create(surgescript_tokentype_t type, const char* lexeme, int linenumber);
surgescript_token_t* surgescript_token_destroy(surgescript_token_t* token);
surgescript_tokentype_t surgescript_token_type(const surgescript_token_t* token);
const char* surgescript_token_lexeme(const surgescript_token_t* token);
int surgescript_token_linenumber(const surgescript_token_t* token);
const char* surgescript_tokentype_name(surgescript_tokentype_t type);
surgescript_token_t* surgescript_token_clone(surgescript_token_t* token);

#endif