/*	$Id: alist.h,v 1.1 2003/04/24 15:11:59 rrt Exp $	*/

/*
 * Copyright (c) 2001 Sandro Sigala.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
#define alist_last(al)		((al->current = al->tail) != NULL ? (al->idx = al->size - 1, a->current->p) : NULL)
#define alist_prev(al)		((al->current != NULL) ? (al->current = al->current->prev, (al->current != NULL) ? --al->idx, al->current->p : NULL) : NULL)
#define alist_next(al)		((al->current != NULL) ? (al->current = al->current->next, (al->current != NULL) ? ++al->idx, al->current->p : NULL) : NULL)
#define alist_current(al)	((al->current != NULL) ? al->current->p : NULL)
#define alist_current_idx(al)	((al->current != NULL) ? al->idx : -1)

#endif /* !ALIST_NO_MACRO_DEFS */

#endif /* !ALIST_H */
