/* Doubly-linked lists
   Copyright (c) 2001-2004 Sandro Sigala.
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

/*	$Id: alist.h,v 1.1 2004/11/15 00:47:12 rrt Exp $	*/

#ifndef ALIST_H
#define ALIST_H

#include <stddef.h>

/*
 * The alist library provides doubly-linked lists of arbitary objects
 * (represented by void * pointers).
 *
 * List items are indexed from zero.
 *
 * The list type, alist, is a pointer type.
 */

/*
 * The list type.
 */
typedef struct alist_s *alist;

/*
 * Allocate a new empty list.
 */
extern alist	alist_new(void);

/*
 * Deallocate al.
 */
extern void	alist_delete(alist al);

/*
 * Remove all the items from al.
 */
extern void	alist_clear(alist al);

/*
 * Return the number of items in al.
 */
#define alist_count(al)		((al)->size)

/*
 * Return the first item of al.
 */
extern void *	alist_first(alist al);

/*
 * Return the last item of al.
 */
extern void *	alist_last(alist al);

/*
 * Return the previous item of al.
 */
extern void *	alist_prev(alist al);

/*
 * Return the next item of al.
 */
extern void *	alist_next(alist al);

/*
 * Return the current item of al.
 */
#define alist_current(al)	(((al)->current != NULL) ? (al)->current->p : NULL)

/*
 * Return the index of the current item in al.
 */
#define alist_current_idx(al)	(((al)->current != NULL) ? (al)->idx : -1)

/*
 * Insert p before position i in al.
 */
extern void	alist_insert(alist al, unsigned int i, void *p);

/*
 * Append p to al.
 */
extern void	alist_append(alist al, void *p);

/*
 * Prepend p to al.
 */
extern void	alist_prepend(alist al, void *p);

/*
 * Remove the current item from al.
 */
extern void	alist_remove(alist al);

/*
 * Sort al using the comparison function cmp. The contents of the list
 * are sorted in ascending order according to cmp, which must return
 * an integer less than, equal to, or greater than zero if the first
 * argument is respectively less than, equal to, or greater than the
 * second. The sort is not stable (equal elements may end up in a
 * different order from in the original list).
 */
extern void	alist_sort(alist al, int (*cmp)(const void *p1, const void *p2));

/*
 * Return the item in al at index i.
 */
extern void *	alist_at(alist al, unsigned int i);


/*
 * Internal data structures
 *
 * You should never access directly to the following data.
 */
typedef struct aentry_s *aentry;

struct aentry_s {
	void *	p;
	aentry	prev;
	aentry	next;
};

struct alist_s {
	aentry	head;
	aentry	tail;
	aentry	current;
	int	idx;
	size_t	size;
};


#endif /* !ALIST_H */
