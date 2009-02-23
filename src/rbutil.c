/* String construction utilities.

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Free Software Foundation, Inc.

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

#include "config.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

#include "rblist.h"
#include "rbacc.h"
#include "rbutil.h"

/*
 * Formats a string a bit like printf, and returns the result as an
 * rblist. This function understands "%r" to mean an rblist. This
 * function does not undertand the full syntax for printf format
 * strings. The only conversion specifications it understands are:
 *
 *   %r - an rblist (non-standard!),
 *   %s - a C string,
 *   %c - a char,
 *   %d - an int,
 *   %o - an unsigned int, printed in octal,
 *   %x - an unsigned int, printed in hexadecimal,
 *   %% - a % character.
 *
 * Width and precision specifiers are not supported.
 */
rblist rblist_fmt(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  rbacc ret = rbacc_new();
  int x;
  size_t i;
  while (1) {
    /* Skip to next `%' or to end of string. */
    for (i = 0; format[i] && format[i] != '%'; i++);
    rbacc_add_array(ret, format, i);
    if (!format[i++])
      break;
    /* We've found a `%'. */
    switch (format[i++]) {
      case 'c':
        rbacc_add_char(ret, (char)va_arg(ap, int));
        break;
      case 'd': {
        x = va_arg(ap, int);
        if (x < 0) {
          rbacc_add_char(ret, '-');
          rbacc_add_number(ret, (unsigned)-x, 10);
        } else
          rbacc_add_number(ret, (unsigned)x, 10);
        break;
      }
      case 'o':
        ret = rbacc_add_number(ret, va_arg(ap, unsigned int), 8);
        break;
      case 'r':
        ret = rbacc_add_rblist(ret, va_arg(ap, rblist));
        break;
      case 's':
        ret = rbacc_add_string(ret, va_arg(ap, const char *));
        break;
      case 'x':
        ret = rbacc_add_number(ret, va_arg(ap, unsigned int), 16);
        break;
      case '%':
        ret = rbacc_add_char(ret, '%');
        break;
      default:
        assert(0);
    }
    format = &format[i];
  }
  va_end(ap);
  return rbacc_to_rblist(ret);
}

/*
 * Scans `rbl' and replaces each character with a string of one or
 * more printable characters. The returned string is suitable for
 * printing at screen column `col'; the screen column only matters
 * if `rbl' contains tab characters.
 *
 * Scanning stops when the screen column reaches or exceeds `goal',
 * or when `rbl' is exhausted. The number of input characters
 * scanned is returned in `*pos' if `pos' is non-NULL. If you don't
 * want a `goal', just pass `SIZE_MAX'.
 *
 * Characters that are already printable expand to themselves.
 * Characters from 0 to 26 are replaced with strings from `^@' to
 * `^Z'. Tab characters are replaced with enough spaces (but always
 * at least one) to reach a screen column that is a multiple of `tab'.
 * Newline characters must not occur in `rbl'. Other characters are
 * replaced with a backslash followed by their octal character code.
 */
/* FIXME: The length of the returned string can exceed size_t. */
rblist make_string_printable(rblist rbl, size_t col, size_t tab, size_t goal, size_t *pos)
{
  static const char ctrls[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  rbacc ret = rbacc_new();
  if (pos)
    *pos = 0;

  RBLIST_FOR(c, rbl)
    assert(c != '\n');

    size_t x = col + rbacc_length(ret);
    if (x >= goal)
      break;

    if (c == '\t') {
      for (size_t w = tab - (x % tab); w > 0; w--)
        rbacc_add_char(ret, ' ');
    } else if ((size_t)c < sizeof(ctrls)) {
      rbacc_add_char(ret, '^');
      rbacc_add_char(ret, ctrls[c]);
    } else if (isprint(c)) {
      rbacc_add_char(ret, c);
      /* FIXME: For double-width characters add a '\0' too so the
         length of 'ret' matches the display width. */
    } else {
      rbacc_add_char(ret, '\\');
      rbacc_add_number(ret, (unsigned)c, 8);
    }

    if (pos)
      (*pos)++;
  RBLIST_END

  return rbacc_to_rblist(ret);
}

/*
 * Returns a printable representation of a single character `c'
 * suitable for printing in column `col' with tab width `tab'. This is
 * sugar for:
 *   make_string_printable(rblist_from_char(c), col, tab, SIZE_MAX, NULL);
 */
rblist make_char_printable(int c, size_t col, size_t tab)
{
  return make_string_printable(rblist_from_char(c), col, tab, SIZE_MAX, NULL);
}
