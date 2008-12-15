/* Character buffers optimised for repeated append.
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

#ifndef RBACC_H
#define RBACC_H


#include <stdio.h>
   
/*
 * The type of character buffers. This is a pointer type. The structure it
 * points to is opaque. The structure is modified in place when characters
 * are appended to the buffer, and it should therefore not be shared
 * without due care. If you want to share the data in an rbacc, your best
 * bet is to convert it to an rblist using rbacc_to_rblist.
 */
typedef struct rbacc *rbacc;


/**************************/
// Constructor.

/*
 * Makes a new, empty rbacc.
 */
rbacc rbacc_new(void);


/*************/
// Methods.

/*
 * Appends a character to `rba', returning `rba'.
 */
rbacc rbacc_add_char(rbacc rba, int c);

/*
 * Appends an rblist to `rba', returning `rba'.
 */
rbacc rbacc_add_rblist(rbacc rba, rblist rbl);

/*
 * Appends an array of characters to `rba', returning `rba'.
 */
rbacc rbacc_add_array(rbacc rba, const char *cs, size_t length);

/*
 * Appends a 0-terminated C string to `rba', returning `rba'.
 */
rbacc rbacc_add_string(rbacc rba, const char *s);

/*
 * Formats an unsigned number in any base up to 16.
 *
 * Takes time O(1) (numbers are fixed size).
 */
rbacc rbacc_add_number(rbacc rba, size_t x, unsigned base);

/*
 * Appends the contents of a file to `rba' and returns `rba'.
 */
rbacc rbacc_add_file(rbacc rba, FILE *fp);

/*
 * Returns the number of characters in `rba'.
 */
size_t rbacc_length(rbacc rba);

/*
 * Returns the contents of `rba' as an rblist.
 */
rblist rbacc_to_rblist(rbacc rba);

#endif // !RBACC_H
