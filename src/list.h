/* Circular doubly-linked lists/queues
   Copyright (c) 1997-2005 Reuben Thomas.
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

/*	$Id: list.h,v 1.2 2005/01/10 00:24:03 rrt Exp $	*/

#ifndef LIST_H
#define LIST_H

typedef struct list_s *list;
struct list_s {
  list prev;
  list next;
  void *item;
};

list list_new(void);
void list_delete(list l);
unsigned long list_length(list l);
list list_prepend(list l, void *i);
list list_append(list l, void *i);
void *list_head(list l);
void *list_behead(list l);
void *list_betail(list l);
void *list_at(list l, unsigned int n);
void list_sort(list l, int (*cmp)(const void *p1, const void *p2));

#define list_first(l) ((l)->next)
#define list_last(l)  ((l)->prev)
#define list_next(l)  ((l)->next)
#define list_prev(l)  ((l)->prev)

#endif
