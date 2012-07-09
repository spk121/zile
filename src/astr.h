/* Dynamically allocated strings

   Copyright (c) 2001-2012 Free Software Foundation, Inc.

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
 *
 * Where not otherwise specified, the functions return the first
 * argument string, usually named as in the function prototype.
 */

/*
 * The opaque string type.
 */
typedef struct astr *astr;
typedef struct astr const *castr;

/*
 * Allocate a new string with zero length.
 */
astr astr_new (void);

/*
 * Make a new string from a C null-terminated string.
 */
astr astr_new_cstr (const char *s);

/*
 * Make a new constant string from a counted C string.
 */
castr castr_new_nstr (const char *s, size_t n);

/*
 * Convert as into a C null-terminated string.
 * as[0] to as[astr_len (as) - 1] inclusive may be read.
 */
_GL_ATTRIBUTE_PURE const char *astr_cstr (castr as);

/*
 * Return the length of the argument string `as'.
 */
_GL_ATTRIBUTE_PURE size_t astr_len (castr as);

/*
 * Return the `pos'th character of `as'.
 */
_GL_ATTRIBUTE_PURE char astr_get (castr as, size_t pos);

/*
 * Return a new astr consisting of `size' characters from string `as'
 * starting from position `pos'.
 */
astr astr_substr (castr as, size_t pos, size_t size);

/*
 * Assign the contents of the argument string to the string `as'.
 */
astr astr_cpy (astr as, castr src);
astr astr_cpy_cstr (astr as, const char *s);

/*
 * Append the contents of the argument string or character to `as'.
 */
astr astr_cat (astr as, castr src);
astr astr_cat_cstr (astr as, const char *s);
astr astr_cat_nstr (astr as, const char *s, size_t len);
astr astr_cat_char (astr as, int c);

/*
 * Overwrite `size' characters of `as', starting at `pos', with the
 * argument string.
 */
astr astr_replace_nstr (astr as, size_t pos, const char *s, size_t size);

/*
 * Remove `size' chars from `as' at position `pos'.
 */
astr astr_remove (astr as, size_t pos, size_t size);

/*
 * Insert gap of `size' characters in `as' at position `pos'.
 */
astr astr_insert (astr as, size_t pos, size_t size);

/*
 * Move `n' chars in `as' from position `from' to `to'.
 */
astr astr_move (astr as, size_t to, size_t from, size_t n);

/*
 * Set `n' chars in `as' at position `pos' to `c'.
 */
astr astr_set (astr as, size_t pos, int c, size_t n);

/*
 * Truncate `as' to position `pos'.
 */
astr astr_truncate (astr as, size_t pos);

/*
 * Read file contents into an astr.
 * Returns NULL if the file doesn't exist, or other error.
 */
astr astr_readf (const char *filename);

/*
 * Format text into a string and return it.
 */
astr astr_vfmt (const char *fmt, va_list ap);
astr astr_fmt (const char *fmt, ...);

/* Enumeration for casing. */
enum casing {
  case_upper = 1,
  case_lower,
  case_capitalized
};

/*
 * Recase as according to newcase.
 */
astr astr_recase (astr as, enum casing newcase);

#endif /* !ASTR_H */
