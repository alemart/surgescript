/*
 * UTF-8 Utilities
 */
#include <stdint.h>
#include <stdarg.h>
#include "util.h"



/*
 * Flexible and Economical UTF-8 Decoder
 * by Bjoern Hoehrmann (MIT License)
 */

// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

static const uint8_t utf8d[] = {
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,
   0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
  12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
  12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
  12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
  12,36,12,12,12,12,12,12,12,12,12,12, 
};

static inline uint32_t
decodeUTF8(uint32_t* state, uint32_t* codep, uint32_t byte) {
  uint32_t type = utf8d[byte];

  *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = utf8d[256 + *state + type];
  return *state;
}

static inline int
countCodePoints(uint8_t* s, size_t* count) {
  uint32_t codepoint;
  uint32_t state = 0;

  for (*count = 0; *s; ++s)
    if (!decodeUTF8(&state, &codepoint, *s))
      *count += 1;

  return state != UTF8_ACCEPT;
}

static inline int
IsUTF8(uint8_t* s) {
  uint32_t codepoint, state = 0;

  while (*s)
    decodeUTF8(&state, &codepoint, *s++);

  return state == UTF8_ACCEPT;
}


/* the routine below is written by Alexandre */
static inline uint32_t codePointAt(uint8_t* s, uint32_t index)
{
    uint32_t codepoint, state = 0;

    for(; *s; ++s) {
        if(!decodeUTF8(&state, &codepoint, *s) && !(index--))
            return codepoint;
    }

    return 0; /* either the index or the string is not valid */
}


/*
 * Basic UTF-8 manipulation routines
 * by Jeff Bezanson (Public Domain)
 */

/* is c the start of a utf8 sequence? */
#define isutf(c) (((c)&0xC0)!=0x80)

/* convert UTF-8 data to wide character */
int u8_toucs(uint32_t *dest, int sz, char *src, int srcsz);

/* the opposite conversion */
int u8_toutf8(char *dest, int sz, uint32_t *src, int srcsz);

/* single character to UTF-8 */
int u8_wc_toutf8(char *dest, uint32_t ch);

/* character number to byte offset */
int u8_offset(char *str, int charnum);

/* byte offset to character number */
int u8_charnum(char *s, int offset);

/* return next character, updating an index variable */
uint32_t u8_nextchar(char *s, int *i);

/* move to next character */
void u8_inc(char *s, int *i);

/* move to previous character */
void u8_dec(char *s, int *i);

/* returns length of next utf-8 sequence */
int u8_seqlen(char *s);

/* assuming src points to the character after a backslash, read an
   escape sequence, storing the result in dest and returning the number of
   input characters processed */
int u8_read_escape_sequence(char *src, uint32_t *dest);

/* given a wide character, convert it to an ASCII escape sequence stored in
   buf, where buf is "sz" bytes. returns the number of characters output. */
int u8_escape_wchar(char *buf, int sz, uint32_t ch);

/* convert a string "src" containing escape sequences to UTF-8 */
int u8_unescape(char *buf, int sz, char *src);

/* convert UTF-8 "src" to ASCII with escape sequences.
   if escape_quotes is nonzero, quote characters will be preceded by
   backslashes as well. */
int u8_escape(char *buf, int sz, char *src, int escape_quotes);

/* utility predicates used by the above */
int octal_digit(char c);
int hex_digit(char c);

/* return a pointer to the first occurrence of ch in s, or NULL if not
   found. character index of found character returned in *charn. */
char *u8_strchr(char *s, uint32_t ch, int *charn);

/* same as the above, but searches a buffer of a given size instead of
   a NUL-terminated string. */
char *u8_memchr(char *s, uint32_t ch, size_t sz, int *charn);

/* count the number of characters in a UTF-8 string */
int u8_strlen(char *s);

int u8_is_locale_utf8(char *locale);

/* printf where the format string and arguments may be in UTF-8.
   you can avoid this function and just use ordinary printf() if the current
   locale is UTF-8. */
int u8_vprintf(char *fmt, va_list ap);
int u8_printf(char *fmt, ...);

/* the routine below is based on Jeff Bezanson's; you'll need to ssfree() the return value */
static inline char* str2utf8(const char* str)
{
    ssfatal("Can't convert string to UTF-8: %s", str); /* FIXME */
    /* is this correct? */
    char* u8s; /* a new string is malloc'd() every time */
    int len = mbstowcs(NULL, str, 0) + 1; /* FIXME: race condition? */
    wchar_t* wcs = ssmalloc(len * sizeof(uint32_t));
    mbstowcs(wcs, str, len);
    u8s = ssmalloc(len * sizeof(uint32_t));
    u8_toutf8(u8s, len * sizeof(uint32_t), (uint32_t*)wcs, len);
    ssfree(wcs);
    return u8s;
}