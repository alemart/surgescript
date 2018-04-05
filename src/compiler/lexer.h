/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * compiler/lexer.h
 * SurgeScript compiler: lexical analyzer
 */

#ifndef _SURGESCRIPT_COMPILER_LEXER_H
#define _SURGESCRIPT_COMPILER_LEXER_H

#include <stdbool.h>

typedef struct surgescript_lexer_t surgescript_lexer_t;
struct surgescript_token_t;

surgescript_lexer_t* surgescript_lexer_create();
surgescript_lexer_t* surgescript_lexer_destroy(surgescript_lexer_t* lexer);

void surgescript_lexer_set(surgescript_lexer_t* lexer, const char* code); /* sets the code to be read */
struct surgescript_token_t* surgescript_lexer_scan(surgescript_lexer_t* lexer); /* scans the next token */
bool surgescript_lexer_unscan(surgescript_lexer_t* lexer, struct surgescript_token_t* token); /* puts a token back into the lexer */

#endif