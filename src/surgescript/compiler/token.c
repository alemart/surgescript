/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2023 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * compiler/token.c
 * SurgeScript compiler: tokens
 */

#include "token.h"
#include "../util/util.h"

/* what's a token? */
struct surgescript_token_t
{
    surgescript_tokentype_t type;
    char* lexeme;
    int linenumber;
    const void* data;
};

/* the names of the tokens */
static const char* token_name[] = {
    #define TOKEN_NAME(x, y) y,
    SURGESCRIPT_TOKEN_TYPES(TOKEN_NAME)
};

/*
 * surgescript_token_create()
 * Creates a new token, given a type and a lexeme (plus user-defined data)
 */
surgescript_token_t* surgescript_token_create(surgescript_tokentype_t type, const char* lexeme, int linenumber, const void* data)
{
    surgescript_token_t* token = ssmalloc(sizeof *token);
    token->type = type;
    token->lexeme = ssstrdup(lexeme);
    token->linenumber = linenumber;
    token->data = data;
    return token;
}

/*
 * surgescript_token_destroy()
 * Destroys an existing token
 */
surgescript_token_t* surgescript_token_destroy(surgescript_token_t* token)
{
    ssfree(token->lexeme);
    return ssfree(token);
}

/*
 * surgescript_token_type()
 * What's the type of the token?
 */
surgescript_tokentype_t surgescript_token_type(const surgescript_token_t* token)
{
    return token ? token->type : SSTOK_UNKNOWN;
}

/*
 * surgescript_token_lexeme()
 * The lexeme (data) of the token
 */
const char* surgescript_token_lexeme(const surgescript_token_t* token)
{
    return token ? token->lexeme : "";
}

/*
 * surgescript_token_linenumber()
 * The number of the line in which this token has appeared
 */
int surgescript_token_linenumber(const surgescript_token_t* token)
{
    return token ? token->linenumber : 0;
}

/*
 * surgescript_token_data()
 * User-defined token data
 */
const void* surgescript_token_data(const surgescript_token_t* token)
{
    return token ? token->data : NULL;
}


/*
 * surgescript_tokentype_name()
 * The name of a token type
 */
const char* surgescript_tokentype_name(surgescript_tokentype_t type)
{
    return token_name[type];
}


/*
 * surgescript_token_clone()
 * Clones a token
 */
surgescript_token_t* surgescript_token_clone(surgescript_token_t* token)
{
    surgescript_token_t* clone = ssmalloc(sizeof *clone);
    clone->type = token->type;
    clone->lexeme = ssstrdup(token->lexeme);
    clone->linenumber = token->linenumber;
    clone->data = token->data;
    return clone;
}