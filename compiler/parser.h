/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * compiler/parser.h
 * SurgeScript compiler: syntax analyzer
 */

#ifndef _SURGESCRIPT_COMPILER_PARSER_H
#define _SURGESCRIPT_COMPILER_PARSER_H

/*
 * SurgeScript grammar:
 * (e is the empty symbol)
 *
 *
 *
 * <script> := <objectlist> // start here
 *
 * <objectlist> := <object> <objectlist> | e
 * <object> := object string { <objectdecl> }
 * <objectdecl> := <notelist> <vardecllist> <statedecllist> <fundecllist>
 *
 * <notelist> := <note> <notelist1> | e
 * <notelist1> := , <note> <notelist1> | <endnote>
 * <note> := string : <signedconst>
 * <signedconst> := <signednum> | string | true | false | null
 * <signednum> := + number | - number | number
 * <endnote> := emoticon | e
 *
 * <vardecllist> := <vardecl> <vardecllist> | e
 * <vardecl> := identifier = <conditionalexpr> ;
 *
 * <statedecllist> := <statedecl> <statedecllist> | e
 * <statedecl> := state string { <stmtlist> }
 *
 * <fundecllist> := <fundecl> <fundecllist> | e
 * <fundecl> := fun identifier ( ) { <stmtlist> }
 *           |  fun identifier ( <funargs> ) { <stmtlist> }
 * <funargs> := identifier <funargs1>
 * <funargs1> := , identifier <funargs1> | e
 *
 *
 *
 * <expr> := <assignexpr> , <expr> | <assignexpr>
 * <assignexpr> := <conditionalexpr> | identifier assignop <assignexpr>
 * <conditionalexpr> := <logicalorexpr> | <logicalorexpr> ? <expr> : <conditionalexpr>
 * <logicalorexpr> := <logicalandexpr> <logicalorexpr1>
 * <logicalorexpr1> := || <logicalandexpr> <logicalorexpr1> | e
 * <logicalandexpr> := <equalityexpr> <logicalandexpr1>
 * <logicalandexpr1> := && <equalityexpr> <logicalandexpr1> | e
 * <equalityexpr> := <relationalexpr> <equalityexpr1>
 * <equalityexpr1> := equalityop <relationalexpr> <equalityexpr1> | e
 * <relationalexpr> := <additiveexpr> <relationalexpr1>
 * <relationalexpr1> := relationalop <additiveexpr> <relationalexpr1> | e
 * <additiveexpr> := <multiplicativeexpr> <additiveexpr1>
 * <additiveexpr1> := additiveop <multiplicativeexpr> <additiveexpr1> | e
 * <multiplicativeexpr> := <unaryexpr> <multiplicativeexpr1>
 * <multiplicativeexpr1> := multiplicativeop <unaryexpr> <multiplicativeexpr1> | e
 * <unaryexpr> := + <unaryexpr> | - <unaryexpr>
 *             |  ! <unaryexpr>
 *             |  typeof <unaryexpr> | typeof ( <unaryexpr> )
 *             |  ++ identifier | -- identifier
 *             |  <postfixexpr>
 * <postfixexpr> := identifier . <postfixexpr>
 *               |  identifier ( ) <postfixexpr>
 *               |  identifier ( <funargsexprlist> ) <postfixexpr>
 *               |  identifier [ <expr> ] <postfixexpr>
 *               |  identifier ++ | identifier --
 *               |  <primaryexpr>
 * <primaryexpr> := identifier | this | <constant> | ( <expr> )
 * <constant> := number | string | true | false | null
 * <funargsexprlist> := <assignexpr> <funargsexprlist1>
 * <funargsexprlist1> := , <assignexpr> <funargsexprlist1> | e
 *
 *
 *
 * <stmtlist> := <stmt> <stmtlist> | e
 * <stmt> := <blockstmt>
 *        |  <exprstmt>
 *        |  <condstmt>
 *        |  <loopstmt>
 *        |  <jumpstmt>
 * <blockstmt> := { <stmtlist> }
 * <exprstmt> := ;
 *            |  <expr> ;
 * <condstmt> := if ( <expr> ) <stmt>
 *            |  if ( <expr> ) <stmt> else <stmt>
 * <loopstmt> := while ( <expr> ) <stmt>
 * <jumpstmt> := return <expr> ;
 *            |  return ;
 *            |  break ;
 *            |  continue ;
 *
 */

#include <stdbool.h>

typedef struct surgescript_parser_t surgescript_parser_t;
struct surgescript_programpool_t;

/* create & destroy */
surgescript_parser_t* surgescript_parser_create(struct surgescript_programpool_t* program_pool);
surgescript_parser_t* surgescript_parser_destroy(surgescript_parser_t* parser);

/* parse a script */
bool surgescript_parser_parsefile(surgescript_parser_t* parser, const char* absolute_path);
bool surgescript_parser_parsemem(surgescript_parser_t* parser, const char* code_in_memory);

#endif