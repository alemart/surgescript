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
    F( SSTOK_ARROWOP, "=>" )                                                    \
    F( SSTOK_LOGICALNOTOP, "!" )                                                \
    F( SSTOK_LOGICALANDOP, "&&" )                                               \
    F( SSTOK_LOGICALOROP, "||" )                                                \
    F( SSTOK_ASSIGNOP, "assignment operator" )                                  \
    F( SSTOK_ADDITIVEOP, "additive operator" )                                  \
    F( SSTOK_MULTIPLICATIVEOP, "multiplicative operator" )                      \
    F( SSTOK_INCDECOP, "increment-decrement operator" )                         \
    F( SSTOK_RELATIONALOP, "relational operator" )                              \
    F( SSTOK_EQUALITYOP, "equality operator" )                                  \
    F( SSTOK_OBJECT, "object" )                                                 \
    F( SSTOK_STATE, "state" )                                                   \
    F( SSTOK_FUN, "fun" )                                                       \
    F( SSTOK_RETURN, "return" )                                                 \
    F( SSTOK_THIS, "this" )                                                     \
    F( SSTOK_IF, "if" )                                                         \
    F( SSTOK_ELSE, "else" )                                                     \
    F( SSTOK_WHILE, "while" )                                                   \
    F( SSTOK_FOR, "for" )                                                       \
    F( SSTOK_FOREACH, "foreach" )                                               \
    F( SSTOK_IN, "in" )                                                         \
    F( SSTOK_BREAK, "break" )                                                   \
    F( SSTOK_CONTINUE, "continue" )                                             \
    F( SSTOK_TYPEOF, "typeof" )                                                 \
    F( SSTOK_PUBLIC, "public" )                                                 \
    F( SSTOK_USING, "using" )                                                   \
    F( SSTOK_DO, "do" )                                                         \
    F( SSTOK_SWITCH, "switch" )                                                 \
    F( SSTOK_CASE, "case" )                                                     \
    F( SSTOK_DEFAULT, "default" )                                               \
    F( SSTOK_CONST, "const" )                                                   \
    F( SSTOK_ASSERT, "assert" )                                                 \
    F( SSTOK_WAIT, "wait" )                                                     \
    F( SSTOK_STATIC, "static" )                                                 \
    F( SSTOK_SUPER, "super" )                                                   \
    F( SSTOK_OF, "of" )                                                         \
    F( SSTOK_IS, "is" )                                                         \
    F( SSTOK_CALLER, "caller" )                                                 \
    F( SSTOK_ANNOTATION, "annotation")                                          \
    F( SSTOK_EMOTICON, "emoticon" )                                             \
    F( SSTOK_UNKNOWN, "<unknown>" )

typedef struct surgescript_token_t surgescript_token_t;
typedef enum surgescript_tokentype_t {
    #define TOKEN_CODE(x, y) x,
    SURGESCRIPT_TOKEN_TYPES(TOKEN_CODE)
} surgescript_tokentype_t;

surgescript_token_t* surgescript_token_create(surgescript_tokentype_t type, const char* lexeme, int linenumber, const void* data);
surgescript_token_t* surgescript_token_destroy(surgescript_token_t* token);
surgescript_tokentype_t surgescript_token_type(const surgescript_token_t* token);
const char* surgescript_token_lexeme(const surgescript_token_t* token);
int surgescript_token_linenumber(const surgescript_token_t* token);
const void* surgescript_token_data(const surgescript_token_t* token);
const char* surgescript_tokentype_name(surgescript_tokentype_t type);
surgescript_token_t* surgescript_token_clone(surgescript_token_t* token);

#endif
