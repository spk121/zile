/* Doubly-linked lists
   Copyright (c) 2001-2004 Sandro Sigala.  All rights reserved.

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

/*	$Id: alist.h,v 1.4 2004/02/17 20:21:18 ssigala Exp $	*/

#ifndef ALIST_H
#define ALIST_H

#include <stddef.h>

typedef struct alist_s *alist;

extern alist	alist_new(void);
extern alist	alist_copy(alist al);
extern void	alist_delete(alist al);
extern void	alist_clear(alist al);
extern int	alist_count(alist al);
extern int	alist_isempty(alist al);
extern void *	alist_first(alist al);
extern void *	alist_last(alist al);
extern void *	alist_prev(alist al);
extern void *	alist_next(alist al);
extern void *	alist_current(alist al);
extern int	alist_current_idx(alist al);
extern void	alist_insert(alist al, unsigned int i, void *p);
extern void	alist_prepend(alist al, void *p);
extern void	alist_append(alist al, void *p);
extern int	alist_remove(alist al);
extern int	alist_remove_ptr(alist al, void *p);
extern int	alist_remove_idx(alist al, unsigned int i);
extern void *	alist_take(alist al);
extern void *	alist_take_idx(alist al, unsigned int i);
extern void	alist_sort(alist al, int (*cmp)(const void *p1, const void *p2));
extern void *	alist_at(alist al, unsigned int i);
extern int	alist_find(alist al, void *p);

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

#ifndef ALIST_NO_MACRO_DEFS

/*
 * Fast macro definitions.
 */

#define alist_isempty(al)	(al->size == 0)
#define alist_count(al)		(al->size)
#define alist_first(al)		(al->idx = 0, (al->current = al->head) != NULL ? al->current->p : NULL)
#define alist_last(al)		((al->current = al->tail) != NULL ? (al->idx = al->size - 1, al->current->p) : NULL)
#define alist_prev(al)		((al->current != NULL) ? (al->current = al->current->prev, (al->current != NULL) ? --al->idx, al->current->p : NULL) : NULL)
#define alist_next(al)		((al->current != NULL) ? (al->current = al->current->next, (al->current != NULL) ? ++al->idx, al->current->p : NULL) : NULL)
#define alist_current(al)	((al->current != NULL) ? al->current->p : NULL)
#define alist_current_idx(al)	((al->current != NULL) ? al->idx : -1)

#endif /* !ALIST_NO_MACRO_DEFS */

#endif /* !ALIST_H */
