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

/*	$Id: alist.c,v 1.7 2004/10/14 00:49:37 rrt Exp $	*/

#include <assert.h>
#include <stdlib.h>

#include "config.h"
#include "alist.h"

static aentry aentry_new(void *p)
{
	aentry ae;
	ae = (aentry)xmalloc(sizeof *ae);
	ae->p = p;
	ae->prev = ae->next = NULL;
	return ae;
}

static void aentry_delete(aentry ae)
{
	ae->p = NULL;			/* Useful for debugging. */
	ae->prev = ae->next = NULL;
	free(ae);
}

static aentry find_aentry(alist al, unsigned int i)
{
	aentry ae;
	unsigned int count;
	for (ae = al->head, count = 0; ae != NULL; ae = ae->next, ++count)
		if (count == i)
			return ae;
	return NULL;
}

alist alist_new(void)
{
	alist al;
	al = (alist)xmalloc(sizeof *al);
	al->head = al->tail = al->current = NULL;
	al->idx = 0;
	al->size = 0;
	return al;
}

void alist_delete(alist al)
{
	assert(al != NULL);
	alist_clear(al);
	free(al);
}

void alist_clear(alist al)
{
	aentry ae, next;
	assert(al != NULL);
	for (ae = al->head; ae != NULL; ae = next) {
		next = ae->next;
		aentry_delete(ae);
	}
	al->head = al->tail = al->current = NULL;
	al->idx = 0;
	al->size = 0;
}

void *alist_first(alist al)
{
	assert(al != NULL);
	al->current = al->head;
	al->idx = 0;
	if (al->current != NULL)
		return al->current->p;
	return NULL;
}

void *alist_last(alist al)
{
	assert(al != NULL);
	al->current = al->tail;
	if (al->current != NULL) {
		al->idx = al->size - 1;
		return al->current->p;
	}
	return NULL;
}

void *alist_prev(alist al)
{
	assert(al != NULL);
	if (al->current != NULL) {
		al->current = al->current->prev;
		if (al->current != NULL) {
			--al->idx;
			return al->current->p;
		} else
			return NULL;
	}
	return NULL;
}

void *alist_next(alist al)
{
	assert(al != NULL);
	if (al->current != NULL) {
		al->current = al->current->next;
		if (al->current != NULL) {
			++al->idx;
			return al->current->p;
		} else
			return NULL;
	}
	return NULL;
}

void alist_insert(alist al, unsigned int i, void *p)
{
        assert(i <= al->size);
	if (i == al->size) {
		i = al->size;
		alist_append(al, p);
	} else if (i == 0)
		alist_prepend(al, p);
	else {
		aentry oldae = find_aentry(al, i);
		aentry ae = aentry_new(p);
		ae->next = oldae;
		if (oldae->prev != NULL) {
			ae->prev = oldae->prev;
			ae->prev->next = ae;
		}
		oldae->prev = ae;
		al->current = ae;
		al->idx = i;
		++al->size;
	}
}

void alist_prepend(alist al, void *p)
{
	aentry ae;
	assert(al != NULL);
	ae = aentry_new(p);
	if (al->head != NULL) {
		al->head->prev = ae;
		ae->next = al->head;
		al->head = ae;
	} else
		al->head = al->tail = ae;
	al->current = ae;
	al->idx = 0;
	++al->size;
}

void alist_append(alist al, void *p)
{
	aentry ae;
	assert(al != NULL);
	ae = aentry_new(p);
	if (al->tail != NULL) {
		al->tail->next = ae;
		ae->prev = al->tail;
		al->tail = ae;
	} else
		al->head = al->tail = ae;
	al->current = ae;
	++al->size;
	al->idx = al->size - 1;
}

static void take_off(alist al, aentry ae)
{
	if (ae->prev != NULL)
		ae->prev->next = ae->next;
	if (ae->next != NULL)
		ae->next->prev = ae->prev;
	if (al->head == ae)
		al->head = ae->next;
	if (al->tail == ae)
		al->tail = ae->prev;
	al->current = NULL;
	al->idx = 0;
	--al->size;
}

void alist_remove(alist al)
{
	aentry ae;
	assert(al != NULL);
	if ((ae = al->current) == NULL)
		return;
	take_off(al, ae);
	aentry_delete(ae);
}

void alist_sort(alist al, int (*cmp)(const void *p1, const void *p2))
{
	void **vec;
	aentry ae;
	int i;
	assert(al != NULL && cmp != NULL);
	if (al->size == 0)
		return;
	vec = (void **)xmalloc(sizeof(void *) * al->size);
	for (ae = al->head, i = 0; ae != NULL; ae = ae->next, ++i)
		vec[i] = ae->p;
	qsort(vec, al->size, sizeof(void *), cmp);
	for (ae = al->head, i = 0; ae != NULL; ae = ae->next, ++i)
		ae->p = vec[i];
	free(vec);
}

void *alist_at(alist al, unsigned int i)
{
	aentry ae;
	assert(al != NULL);
	if (i >= al->size)
		return NULL;
	ae = find_aentry(al, i);
	return ae->p;
}

#ifdef TEST

#include <stdio.h>
#include <string.h>

static int sorter(const void *p1, const void *p2)
{
	return strcmp(*(char **)p1, *(char **)p2);
}

int main(void)
{
	alist al;
	char *t1 = "def", *t2 = "abc", *t3 = "xyz";
	char *s;

	al = alist_new();
	assert(alist_count(al) == 0);
	assert(alist_current(al) == NULL);
	assert(alist_current_idx(al) == -1);

	alist_append(al, t1);
	assert(alist_count(al) == 1);
	assert(alist_current(al) == t1);
	assert(alist_current_idx(al) == 0);

	alist_append(al, t2);
	assert(alist_count(al) == 2);
	assert(alist_current(al) == t2);
	assert(alist_current_idx(al) == 1);

	s = alist_first(al);
	assert(s == t1);
	assert(alist_current(al) == t1);
	assert(alist_current_idx(al) == 0);

	s = alist_next(al);
	assert(s == t2);
	assert(alist_current(al) == t2);
	assert(alist_current_idx(al) == 1);

	s = alist_next(al);
	assert(s == NULL);
	assert(alist_current(al) == NULL);
	assert(alist_current_idx(al) == -1);

	alist_prepend(al, t3);
	assert(alist_count(al) == 3);
	assert(alist_current(al) == t3);
	assert(alist_current_idx(al) == 0);

	printf("elements:\n");
	for (s = alist_first(al); s != NULL; s = alist_next(al))
		printf("element %d: %s\n", alist_current_idx(al), s);

	alist_sort(al, sorter);
	printf("sorted elements:\n");
	for (s = alist_first(al); s != NULL; s = alist_next(al))
		printf("element %d: %s\n", alist_current_idx(al), s);
        assert(alist_at(al, 0) == t2);
	assert(alist_at(al, 1) == t1);
	assert(alist_at(al, 2) == t3);

	alist_clear(al);
	assert(alist_count(al) == 0);
	assert(alist_current(al) == NULL);
	assert(alist_current_idx(al) == -1);

	alist_insert(al, 0, t1);
	assert(alist_count(al) == 1);
	assert(alist_current(al) == t1);
	assert(alist_current_idx(al) == 0);

	alist_insert(al, 0, t2);
	assert(alist_count(al) == 2);
	assert(alist_current(al) == t2);
	assert(alist_current_idx(al) == 0);

	alist_insert(al, 1, t3);
	assert(alist_count(al) == 3);
	assert(alist_at(al, 0) == t2);
	assert(alist_at(al, 1) == t3);
	assert(alist_at(al, 2) == t1);
	assert(alist_current(al) == t3);
	assert(alist_current_idx(al) == 1);

	alist_delete(al);
	printf("alist test successful.\n");

	return 0;
}

#endif /* TEST */
