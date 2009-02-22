/* Dynamically allocated strings

   Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Free Software Foundation, Inc.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

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
 * The opaque string type.
 */
typedef struct astr *astr;

/*
 * Allocate a new string with zero length.
 */
astr astr_new (void);

/*
 * Make a new string from a C null-terminated string.
 */
astr astr_new_cstr (const char *s);

/*
 * Deallocate the previously allocated string as.
 */
void astr_delete (astr as);

/*
 * Convert as into a C null-terminated string.
 * as[0] to as[astr_len (as) - 1] inclusive may be read.
 */
const char *astr_cstr (astr as);

/*
 * Return the length of the argument string as.
 */
size_t astr_len (astr as);

/*
 * Return the address of the pos'th character of as. If pos is >= 0,
 * than 0, count from the left; if less than zero count from the
 * right.
 */
char *astr_char (astr as, ptrdiff_t pos);

/*
 * Return a new astr consisting of size characters from string as
 * starting from position pos.
 */
astr astr_substr (astr as, ptrdiff_t pos, size_t size);

/*
 * Do strcmp on the contents of as1 and as2.
 */
int astr_cmp (astr as1, astr as2);

/*
 * Assign the contents of the argument string to the string as.
 */
astr astr_cpy (astr as, astr src);
astr astr_ncpy_cstr (astr as, const char *s, size_t len);
astr astr_cpy_cstr (astr as, const char *s);

/*
 * Append the contents of the argument string or character to as.
 */
astr astr_cat (astr as, astr src);
astr astr_cat_cstr (astr as, const char *s);
astr astr_ncat_cstr (astr as, const char *s, size_t len);
astr astr_cat_char (astr as, int c);

/*
 * Append the contents of src to as and free src.
 */
astr astr_cat_delete (astr as, astr src);

/*
 * Replace size characters of as, starting at pos, with the argument
 * string or character.
 */
astr astr_replace_cstr (astr as, ptrdiff_t pos, size_t size,
                               const char *s);
astr astr_replace_char (astr as, ptrdiff_t pos, int c);

/*
 * Insert the contents of the character in as.
 */
astr astr_insert_char (astr as, ptrdiff_t pos, int c);

/*
 * Remove size chars from as at position pos.
 */
astr astr_remove (astr as, ptrdiff_t pos, size_t size);

/*
 * Truncate as to given position.
 */
astr astr_truncate (astr as, ptrdiff_t pos);

/*
 * Read the stream fp into a string and return it.
 */
astr astr_fread (FILE * fp);

/*
 * Read a string from the stream fp and return it. The trailing
 * newline is removed from the string. If the stream is at EOF when
 * astr_fgets is called, it returns NULL.
 */
astr astr_fgets (FILE * fp);

/*
 * Append formatted text to the argument string.
 */
astr astr_vafmt (astr as, const char *fmt, va_list ap);
astr astr_afmt (astr as, const char *fmt, ...);


#endif /* !ASTR_H */
