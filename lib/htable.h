/* Hash table library
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004 Reuben Thomas.
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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id: htable.h,v 1.4 2004/03/13 20:07:00 rrt Exp $	*/

#ifndef HTABLE_H
#define HTABLE_H

#include <stdarg.h>

#include "alist.h"

/*
 * The hash table library provides string-keyed open non-extensible
 * hash tables.
 *
 * The hash table type, htable, is a pointer type.
 */

/*
 * The hash table type.
 */
typedef struct htable_s *htable;

/*
 * Allocate a new hash table with the given number of buckets.
 */
extern htable htable_new(unsigned long size);

/*
 * Deallocate ht.
 */
extern void   htable_delete(htable ht);

/*
 * Store a key-value pair into ht. The return value is -1 if the key
 * already existed and 0 otherwise.
 */
extern int    htable_store(htable ht, const char *key, void *val);

/*
 * Return the data associated with the key in ht. If the key does not
 * exist or no data was associated with the key, NULL will be
 * returned.
 */
extern void * htable_fetch(htable ht, const char *key);

/*
 * Remove the specified key from ht.
 */
extern int    htable_remove(htable ht, const char *key);

/*
 * The type of key-value pairs, used by htable_foreach iterators and
 * htable_list.
 */
typedef struct hpair_s {
	char *key;
	void *val;
} hpair;

/*
 * Iterate over every item of ht, calling f(&key, &val, ...) for each
 * item. The variadic arguments to f are those passed to
 * htable_foreach.
 */
typedef void (*hiterator)(hpair *pair, va_list ap);
htable htable_foreach(htable ht, hiterator f, ...);

/*
 * Build an (unsorted) list filled with all the pairs (of type hpair)
 * defined in the hash table. After use, the list should be freed (but
 * not the elements, because they belong to the hash table).
 */
extern alist  htable_list(htable ht);


#endif /* !HTABLE_H */
