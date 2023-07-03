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
 * runtime/managed_string.c
 * Fast immutable strings
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "managed_string.h"
#include "../util/ssarray.h"
#include "../util/util.h"
#include "../third_party/utf8.h"

/* constants */
#define MAXLEN          63      /* the maximum length of a pooled string */
#define PAGE_CAPACITY   1024    /* the number of strings that a page holds */
#define WANT_POOLING    1       /* keep it enabled in production; for testing only */
#define WANT_VALIDATION 0       /* enable utf-8 validation? it takes extra cycles */

typedef struct surgescript_managedstringpool_t surgescript_managedstringpool_t;
typedef struct surgescript_managedstringpage_t surgescript_managedstringpage_t;

/* managed string */
struct surgescript_managedstring_t
{
    char* data; /* pointer to a C string; this must be the first field */
    bool in_use;
    surgescript_managedstring_t* next; /* free list */
};

/* a page of managed strings */
struct surgescript_managedstringpage_t
{
    /* each page holds PAGE_CAPACITY managed strings */
    surgescript_managedstring_t managed_string[PAGE_CAPACITY];
    char buffer[(1 + MAXLEN) * PAGE_CAPACITY];
};

/* a pool of managed strings */
struct surgescript_managedstringpool_t
{
    /* a pool holds pages of managed strings */
    SSARRAY(surgescript_managedstringpage_t*, page);

    /* the head of the free list */
    surgescript_managedstring_t* head;
};

/* private */
static inline char* convert_to_ascii(char* str);
static surgescript_managedstringpage_t* allocate_page();
static surgescript_managedstringpage_t* deallocate_page(surgescript_managedstringpage_t* page);
static surgescript_managedstringpool_t pool;




/*
 * public
 */


/*
 * surgescript_managedstring_create()
 * Quickly acquires a managed string from the pool
 */
surgescript_managedstring_t* surgescript_managedstring_create(const char* string)
{
    surgescript_managedstring_t* managed_string = NULL;
    size_t length = strlen(string);

#if WANT_POOLING
    /* strings of the pool are all small (up to MAXLEN characters) */
    if(length <= MAXLEN) {
#else
    if(false) {
#endif
        /* quickly prepare a managed string from the pool */
        ssassert(pool.head != NULL && !pool.head->in_use);
        managed_string = pool.head;
        managed_string->in_use = true;
        pool.head = managed_string->next;

        /* copy string */
        memcpy(managed_string->data, string, length + 1); /* we already know that length <= MAXLEN */

        /* let's allocate a new page if necessary */
        if(pool.head == NULL) {
            surgescript_managedstringpage_t* page = allocate_page();
            ssarray_push(pool.page, page);
            pool.head = managed_string->next = &page->managed_string[0];
        }

        /* now managed_string->next != NULL */
    }
    else {
       /* if the string is too long, we don't use the pool. We allocate
          memory. In usual circumstances, large strings are not expected to be
          released immediately after allocation, so no severe performance hit
          is expected here. */

#if 0
        /* impose a maximum length */
        ssassert(length < 1048576);
#endif

        managed_string = ssmalloc(sizeof *managed_string);
        managed_string->data = ssstrdup(string);
        managed_string->in_use = true;
        managed_string->next = NULL; /* the managed string is not in the pool */
    }

#if WANT_VALIDATION
    /* validate */
    if(!u8_isvalid(managed_string->data, length))
        convert_to_ascii(managed_string->data);
#else
    (void)convert_to_ascii;
#endif

    /* done! */
    return managed_string;
}

/*
 * surgescript_managedstring_destroy()
 * Quickly releases a managed string back into the pool
 */
surgescript_managedstring_t* surgescript_managedstring_destroy(surgescript_managedstring_t* managed_string)
{
    /* check if the managed string is NOT in the pool */
    if(managed_string->next == NULL) {
        ssfree(managed_string->data);
        return ssfree(managed_string);
    }

    /* the managed string is no longer in use */
    ssassert(managed_string->in_use);
    managed_string->in_use = false;

    /* quickly put the managed string back into the pool */
    ssassert(pool.head != NULL);
    managed_string->next = pool.head;
    pool.head = managed_string;

    /* done! */
    return NULL;
}

/*
 * surgescript_managedstring_clone()
 * Clone a managed string
 */
surgescript_managedstring_t* surgescript_managedstring_clone(const surgescript_managedstring_t* managed_string)
{
    ssassert(managed_string->in_use);
    return surgescript_managedstring_create(managed_string->data);
}



/*
 * surgescript_managedstring_init_pool()
 * Initializes a pool of managed strings
 */
void surgescript_managedstring_init_pool()
{
    surgescript_managedstringpage_t* page = allocate_page();

    ssarray_init(pool.page);
    ssarray_push(pool.page, page);
    pool.head = &page->managed_string[0];

    static_assert(MAXLEN >= SS_NAMEMAX, "");
}

/*
 * surgescript_managedstring_release_pool()
 * Releases a pool of managed strings
 */
void surgescript_managedstring_release_pool()
{
    for(int i = ssarray_length(pool.page) - 1; i >= 0; i--)
        deallocate_page(pool.page[i]);

    ssarray_release(pool.page);
    pool.head = NULL;
}




/*
 * private
 */

/* allocate a new page */
surgescript_managedstringpage_t* allocate_page()
{
    surgescript_managedstringpage_t* page = NULL;
    const int MAXSIZE = 1 + MAXLEN;

    sslog("Allocating a new page of strings...");

    /* initialize the page */
    page = ssmalloc(sizeof *page);
    for(int i = 0; i < PAGE_CAPACITY; i++) {
        page->managed_string[i].data = page->buffer + MAXSIZE * i;
        page->managed_string[i].in_use = false;
    }
    for(int i = 1; i < PAGE_CAPACITY; i++)
        page->managed_string[i-1].next = page->managed_string + i;
    page->managed_string[PAGE_CAPACITY-1].next = NULL;

    /* reset string data */
    memset(page->buffer, '\0', sizeof(page->buffer));

    /* done! */
    return page;
}

/* deallocate an existing page */
surgescript_managedstringpage_t* deallocate_page(surgescript_managedstringpage_t* page)
{
    return ssfree(page);
}

/* convert string to ascii */
char* convert_to_ascii(char* str)
{
    char *p, *q;

    for(q = p = str; *p; p++) {
        if(!(*p & 0x80))
            *(q++) = *p;
    }

    *q = '\0';
    return str;
}