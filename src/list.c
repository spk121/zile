/* Circular doubly-linked lists/queues
   Copyright (c) 1997-2005 Reuben Thomas.  All rights reserved.

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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#include <stdlib.h>
#include <assert.h>

#include "list.h"
#include "zile.h"
#include "extern.h"

/* Create an empty list, returning a pointer to the list */
list list_new(void)
{
  list l = zmalloc(sizeof(struct list_s));

  l->next = l->prev = l;
  l->item = NULL;

  return l;
}

/* Delete a list, freeing its nodes */
void list_delete(list l)
{
  list p = l, q;

  do {
    q = p;
    p = p->next;
    free(q);
  } while (p != l);
}

/* Return the length of a list */
size_t list_length(list l)
{
  list p;
  size_t length = 0;

  for (p = l->next; p != l; p = p->next)
    ++length;

  return length;
}

/* Add an item to the head of a list, returning the new list head */
list list_prepend(list l, void *i)
{
  list n = zmalloc(sizeof(struct list_s));

  n->next = l->next;
  n->prev = l;
  n->item = i;
  l->next = l->next->prev = n;

  return n;
}

/* Add an item to the tail of a list, returning the new list tail */
list list_append(list l, void *i)
{
  list n = zmalloc(sizeof(struct list_s));

  n->next = l;
  n->prev = l->prev;
  n->item = i;
  l->prev = l->prev->next = n;

  return n;
}

/* Return the first item of a list, or NULL if the list is empty */
void *list_head(list l)
{
  list p = l->next;

  if (p == l)
    return NULL;

  return p->item;
}

/* Remove the first item of a list, returning the item, or NULL if the
   list is empty */
void *list_behead(list l)
{
  void *i;
  list p = l->next;

  if (p == l)
    return NULL;
  i = p->item;
  l->next = l->next->next;
  l->next->prev = l;
  free(p);

  return i;
}

/* Remove the last item of a list, returning the item, or NULL if the
   list is empty */
void *list_betail(list l)
{
  void *i;
  list p = l->prev;

  if (p == l)
    return NULL;
  i = p->item;
  l->prev = l->prev->prev;
  l->prev->next = l;
  free(p);

  return i;
}

/* Return the nth item of l, or l->item (usually NULL) if that is out
   of range */
void *list_at(list l, size_t n)
{
  size_t i;
  list p;
        
  assert(l != NULL);

  for (p = list_first(l), i = 0; p != l && i < n; p = list_next(p), i++)
    ;
  
  return p->item;
}

/* Sort list l with qsort using comparison function cmp */
void list_sort(list l, int (*cmp)(const void *p1, const void *p2))
{
  list p;
  void **vec;
  size_t i, len = list_length(l);

  assert(l != NULL && cmp != NULL);

  vec = (void **)zmalloc(sizeof(void *) * len);

  for (p = list_first(l), i = 0; i < len; p = list_next(p), ++i)
    vec[i] = (void *)p->item;

  qsort(vec, len, sizeof(void *), cmp);

  for (p = list_first(l), i = 0; i < len; p = list_next(p), ++i)
    p->item = vec[i];

  free(vec);
}
