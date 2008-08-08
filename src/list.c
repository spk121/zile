/* Circular doubly-linked lists/queues

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Reuben Thomas.

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

#include <stdlib.h>
#include <assert.h>

#include "list.h"
#include "zile.h"
#include "extern.h"

/* Create an empty list, returning a pointer to the list */
list
list_new (void)
{
  list l = XZALLOC (struct list_s);

  l->next = l->prev = l;
  l->item = NULL;

  return l;
}

/* Delete a list, freeing its nodes */
void
list_delete (list l)
{
  list p = l, q;

  do
    {
      q = p;
      p = p->next;
      free (q);
    }
  while (p != l);
}

/* Add an item to the head of a list, returning the new list head */
list
list_prepend (list l, void *i)
{
  list n = XZALLOC (struct list_s);

  n->next = l->next;
  n->prev = l;
  n->item = i;
  l->next = l->next->prev = n;

  return n;
}

/* Add an item to the tail of a list, returning the new list tail */
list
list_append (list l, void *i)
{
  list n = XZALLOC (struct list_s);

  n->next = l;
  n->prev = l->prev;
  n->item = i;
  l->prev = l->prev->next = n;

  return n;
}

/* Remove the first item of a list, returning the item, or NULL if the
   list is empty */
void *
list_behead (list l)
{
  void *i;
  list p = l->next;

  if (p == l)
    return NULL;
  i = p->item;
  l->next = l->next->next;
  l->next->prev = l;
  free (p);

  return i;
}
