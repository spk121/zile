/* Randomly balanced lists.
   Copyright (c) 2006 Alistair Turnbull.
   Copyright (c) 2006-2007 Reuben Thomas.
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

#ifndef RBLIST_H
#define RBLIST_H

#include <stddef.h>

/*
 * The type of lists. This is a pointer type. The structure it points
 * to is opaque. You may assume the structure is immutable (const). No
 * operation in this library ever modifies an rblist in place.
 *
 * Takes memory O(n) where `n' is the length of the list. Typically
 * about twice as much memory will be used as would be used for an
 * array.
 */
typedef const union rblist *rblist;

/*
 * The type of list iterators (i.e. the type of the state of a loop
 * through a list). This is a pointer type, and NULL is means that the
 * loop is finished. The structure it points to is opaque and gets
 * modified in-place by `rblist_next'.
 *
 * An rblist_iterator can be thought of a bit like a pointer into the
 * list (but that's not what it is). rblist_iterator_value
 * dereferences an rblist_iterator to obtain an element of the list.
 * rblist_iterator_next increments an rblist_iterator to move on to
 * the next element. The most concise way to use these functions is
 * using the RBLIST_FOR macro.
 *
 * Takes memory O(log(n)) where `n' is the length of the list.
 */
typedef struct rblist_iterator *rblist_iterator;


/***************************/
// Primitive constructors.

/*
 * Make an rblist from an array. This can be achieved using
 * rblist_singleton and rblist_concat, but this method is more
 * efficient.
 *
 * Takes time O(n) where `n' is the length of the list.
 */
rblist rblist_from_array(const char *s, size_t length);

/*
 * The empty list. There is only one empty list, so this is a constant,
 * not a function.
 */
const rblist rblist_empty;

/*
 * Make a list of length 1. See also rblist_from_array.
 *
 * Takes time O(1).
 */
rblist rblist_from_char(int c);

/*
 * Concatenate two lists. The originals are not modified.
 *
 * Takes time O(log(n)) where `n' is the length of the result.
 */
rblist rblist_concat(rblist left, rblist right);


/*************************/
// Derived constructors.

/*
 * Makes an rblist from a 0-terminated string.
 *
 * Uses rblist_from_array, so takes time O(n).
 */
rblist rblist_from_string(const char *s);


/**************************/
// Primitive destructors.

/*
 * Read the length of an rblist.
 *
 * Takes time O(1).
 */
size_t rblist_length(rblist rbl);

/*
 * Count the newline characters in an rblist.
 *
 * Take time O(1).
 */
size_t rblist_nl_count(rblist rbl);

/*
 * Break an rblist into two at the specified position, and store the
 * two halves in *left and *right. The original is not modified.
 * `pos' is clipped to the range from 0 to rblist_length(rbl) before
 * use.
 *
 * Takes time O(log(n)) where `n' is the length of the list.
 */
void rblist_split(rblist rbl, size_t pos, rblist *left, rblist *right);

/*
 * Constructs an iterator over the specified rblist.
 *
 * Takes time O(log(n)) where `n' is the length of the list.
 */
rblist_iterator rblist_iterate(rblist rbl);

/*
 * Returns thelist element currently addressed by `it'.
 *
 * Takes time O(1).
 */
char rblist_iterator_value(rblist_iterator it);

/*
 * Advances the rblist_iterator one place down its list and returns
 * the result. The original is destroyed and should be discarded.
 * Returns NULL at the end of the list.
 *
 * Takes time O(1) amortized.
 */
rblist_iterator rblist_iterator_next(rblist_iterator it);

/*
 * Returns the specified element of the specified rblist. `pos' must be
 * in the range 0 to rblist_length(rbl) - 1.
 *
 * This is a slow way of extracting elements from a list, and should
 * be used only when accesses are scattered randomly throughout the
 * list. For sequential access, use rblist_iterate. The RBLIST_FOR macro
 * may be helpful.
 *
 * Takes time O(log(n)) where `n' is the length of the list.
 */
char rblist_get(rblist rbl, size_t pos);

/*
 * Converts a character position into a line number. More precisely,
 * Returns the number of newline characters before the specified
 * position. Requires 0 <= pos <= rblist_length(rbl).
 *
 * Equivalent to, but faster than, rblist_nl_count(rblist_sub(rbl, 0, pos));
 *
 * Takes time O(log(n)) where `n' is the length of the list.
 */
size_t rblist_pos_to_line(rblist rbl, size_t pos);

/*
 * Converts a line number to the character position of the start of
 * that line. More precisely, returns the smallest `pos' such that:
 *
 *   rblist_pos_to_line(rbl, pos) == line.
 *
 * Requires 0 <= line <= rblist_nl_count(rbl).
 *
 * Takes time O(log(n)) where `n' is the length of the list.
 */
size_t rblist_line_to_start_pos(rblist rbl, size_t line);

/*
 * Converts a line number to the character position of the end of that
 * line. More precisely, returns the largest `pos' such that:
 *
 *   rblist_pos_to_line(rbl, pos) == line.
 *
 * Requires 0 <= line <= rblist_nl_count(rbl).
 *
 * Takes time O(log(n)) where `n' is the length of the list.
 */
size_t rblist_line_to_end_pos(rblist rbl, size_t line);

 
/************************/
// Derived destructors.

/*
 * Calculates the length of line `line'.
 *
 * Requires 0 <= line <= rblist_nl_count(rbl).
 *
 * Takes time O(log(n)) where `n' is the length of the list.
 */
size_t rblist_line_length(rblist rbl, size_t line);

/*
 * Returns the line `line'.
 *
 * Requires 0 <= line <= rblist_nl_count(rbl).
 *
 * Takes time O(log(n)) where `n' is the length of the list.
 */
rblist rblist_line(rblist rbl, size_t line);

/*
 * Syntactic sugar for looping through the elements of an rblist.
 * RBLIST_FOR is much faster than calling rblist_get in a loop. It
 * should be used thus:
 *
 *   RBLIST_FOR(c, rblist)
 *     ... Do something with c ...
 *   RBLIST_END
 *
 * Note that RBLIST_FOR can be combined with rblist_sub to loop
 * through any part of an rblist.
 */
#define RBLIST_FOR(c, rbl) \
  for (rblist_iterator _it_##c = rblist_iterate(rbl); \
       _it_##c; \
       _it_##c = rblist_iterator_next(_it_##c)) { \
    int c = rblist_iterator_value(_it_##c);

#define RBLIST_END }

/*
 * Returns 'rbl' as a newly allocated 0-terminated C string.
 *
 * Takes time O(n) where `n' is the length of the list.
 */
char *rblist_to_string(rblist rbl);

/*
 * Returns the portion of `rbl' from `from' to `to'. If `to' is too
 * big it is clipped to the length of `rbl'. If from > to the result
 * is rblist_empty.
 *
 * To break a list into pieces, it is more efficient to use
 * rblist_split instead of this function. This function uses
 * rblist_split internally and then discards some of the pieces.
 *
 * Takes time O(log(n)) where `n' is the length of the list.
 */
rblist rblist_sub(rblist rbl, size_t from, size_t to);

/*
 * Compares the lists `left' and `right' lexicographically. Returns a
 * negative number for less, 0 for equal and a positive number for
 * more.
 *
 * Takes time O(n) where `n' is the length of the common prefix.
 */
int rblist_compare(rblist left, rblist right);

/*
 * Compares prefixes of the lists `left' and `right' lexographically.
 * 'n' is the length of the prefixes to compare. Equivalent to:
 *
 *   rblist_compare(rblist_sub(left, 0, n), rblist_sub(right, 0, n))
 */
int rblist_ncompare(rblist left, rblist right, size_t n);

#endif // !RBLIST_H
