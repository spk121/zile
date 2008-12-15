/* String construction utilities.
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
   Copyright (c) 2007 Alistair Turnbull.
   All rights reserved.

   This file is part of Zee.

   Zee is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Zee is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with Zee; see the file COPYING.  If not, write to the Free
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#include "config.h"

#include <stdbool.h>
#include <stdint.h>

#include "rblist.h"
#include "rbacc.h"

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
rblist rblist_fmt(const char *format, ...);

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
rblist make_string_printable(rblist rbl, size_t col, size_t tab, size_t goal, size_t *pos);

/*
 * Returns a printable representation of a single character `c'
 * suitable for printing in column `col' with tab width `tab'. This is
 * sugar for:
 *   make_string_printable(rblist_from_char(c), col, tab, SIZE_MAX, NULL);
 */
rblist make_char_printable(int c, size_t col, size_t tab);
