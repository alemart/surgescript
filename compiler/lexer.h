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

typedef struct surgescript_lexer_t surgescript_lexer_t;
typedef struct surgescript_token_t surgescript_token_t;

surgescript_lexer_t* surgescript_lexer_create();
surgescript_lexer_t* surgescript_lexer_destroy(surgescript_lexer_t* lexer);

bool surgescript_lexer_feed(surgescript_lexer_t* lexer, const char* code); /* feeds some code to the lexer */
surgescript_token_t* surgescript_lexer_scan(surgescript_lexer_t* lexer); /* scans the next token */

#endif