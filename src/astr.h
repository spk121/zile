/* Dynamically allocated strings
   Copyright (c) 2001-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
   All rights reserved.

   This file is part of Zile.

   Zile is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Zile is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with Zile; see the file COPYING.  If not, write to the Free
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#ifndef ASTR_H
#define ASTR_H

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

/*
 * The astr library provides dynamically allocated null-terminated C
 * strings.
 *
 * The string type, astr, is a pointer type.
 *
 * String positions start at zero, as with ordinary C strings.
 * Negative values are also allowed, and count from the end of the
 * string. In particular, -1 refers to the last character of the
 * string.
 *
 * Where not otherwise specified, the functions return the first
 * argument string, usually named as in the function prototype.
 */

/*
 * The dynamic string type.
 */
typedef struct astr_s *astr;

/*
 * Allocate a new string with zero length.
 */
extern astr astr_new(void);

/*
 * Make a new string from a C null-terminated string
 */
#define astr_new_cstr(s)        (astr_cpy_cstr(astr_new(), (s)))

/*
 * Deallocate the previously allocated string as.
 */
extern void astr_delete(astr as);

/*
 * Convert as into a C null-terminated string.
 * as[0] to as[astr_size(as) - 1] inclusive may be read.
 */
#define astr_cstr(as)           ((const char *)(((astr)(as))->text))

/*
 * Return the length of the argument string as.
 */
#define astr_len(as)            ((const size_t)(((astr)(as))->len))

/*
 * Return the address of the pos'th character of as. If pos is >= 0,
 * than 0, count from the left; if less than zero count from the
 * right.
 */
extern char *astr_char(const astr as, ptrdiff_t pos);

/*
 * Return a new astr consisting of size characters from string as
 * starting from position pos.
 */
extern astr astr_substr(const astr as, ptrdiff_t pos, size_t size);

/*
 * Do strcmp on the contents of s1 and s2
 */
#define astr_cmp(as1, as2)      (strcmp(((astr)(as1))->text, ((astr)(as2))->text))
#define astr_cmp_cstr(as, s)    (strcmp(((astr)(as))->text, (s)))

/*
 * Assign the contents of the argument string to the string as.
 */
extern astr astr_cpy(astr as, const astr src);
extern astr astr_cpy_cstr(astr as, const char *s);

/*
 * Append the contents of the argument string or character to as.
 */
extern astr astr_cat(astr as, const astr src);
extern astr astr_cat_cstr(astr as, const char *s);
extern astr astr_ncat_cstr(astr as, const char *s, size_t len);
extern astr astr_cat_char(astr as, int c);

/*
 * Append the contents of src to as and free src.
 */
extern astr astr_cat_delete(astr as, const astr src);

/*
 * Replace size characters of as, starting at pos, with the argument
 * string or character.
 */
extern astr astr_replace(astr as, ptrdiff_t pos, size_t size, const astr src);
extern astr astr_replace_cstr(astr as, ptrdiff_t pos, size_t size, const char *s);
extern astr astr_replace_char(astr as, ptrdiff_t pos, size_t size, int c);

/*
 * Insert the contents of the argument string or character in as.
 */
extern astr astr_insert(astr as, ptrdiff_t pos, const astr src);
extern astr astr_insert_cstr(astr as, ptrdiff_t pos, const char *s);
extern astr astr_insert_char(astr as, ptrdiff_t pos, int c);

/*
 * Remove size chars from as at position pos.
 */
extern astr astr_remove(astr as, ptrdiff_t pos, size_t size);

/*
 * Truncate as to given position.
 */
extern astr astr_truncate(astr as, ptrdiff_t pos);

/*
 * Find the first occurrence of the argument string in as, returning
 * the position starting from the beginning of the string.
 */
extern int astr_find(const astr as, const astr src);
extern int astr_find_cstr(const astr as, const char *s);

/*
 * Find the last occurrence of the argument string in as, returning
 * the position starting from the end of the string.
 */
extern int astr_rfind(const astr as, const astr src);
extern int astr_rfind_cstr(const astr as, const char *s);

/*
 * Read a string from the stream f and return it. The trailing newline
 * is removed from the string. If the stream is at eof when astr_fgets
 * is called, it returns NULL.
 */
extern astr astr_fgets(FILE *f);

/*
 * Append formatted text to the argument string
 */
extern astr astr_vafmt(astr as, const char *fmt, va_list ap);
extern astr astr_afmt(astr as, const char *fmt, ...);


/*
 * Internal data structure
 *
 * Internally, each string has three fields: a buffer that contains
 * the C string, the buffer size and the size of the string. Each time
 * the string is enlarged beyond the current size of the buffer it is
 * reallocated with realloc.
 *
 * You should never directly access the struct fields.
 */
struct astr_s {
  char *  text;
  size_t  len;
  size_t  maxlen;
};


#endif /* !ASTR_H */
