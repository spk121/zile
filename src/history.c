/* History facility functions
   Copyright (c) 2004 David A. Capello.  All rights reserved.

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

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

void free_history_elements(History *hp)
{
  if (hp->elements) {
    list l;

    for (l = list_first(hp->elements); l != hp->elements;
         l = list_next(l))
      free(l->item);

    list_delete(hp->elements);
    hp->elements = NULL;
    hp->sel = NULL;
  }
}

void add_history_element(History *hp, const char *string)
{
  const char *last;

  if (!hp->elements)
    hp->elements = list_new();

  last = list_last(hp->elements)->item;
  if (!last || strcmp(last, string) != 0)
    list_append(hp->elements, zstrdup(string));
}

void prepare_history(History *hp)
{
  hp->sel = NULL;
}

const char *previous_history_element(History *hp)
{
  const char *s = NULL;

  if (hp->elements) {
    /* First time that we use `previous-history-element'. */
    if (!hp->sel) {
      /* Select last element. */
      if (list_last(hp->elements) != hp->elements) {
        hp->sel = list_last(hp->elements);
        s = hp->sel->item;
      }
    }
    /* Is there another element? */
    else if (list_prev(hp->sel) != hp->elements) {
      /* Select it. */
      hp->sel = list_prev(hp->sel);
      s = hp->sel->item;
    }
  }

  return s;
}

const char *next_history_element(History *hp)
{
  const char *s = NULL;

  if (hp->elements && hp->sel) {
    /* Next element. */
    if (list_next(hp->sel) != hp->elements) {
      hp->sel = list_next(hp->sel);
      s = hp->sel->item;
    }
    /* No more elements (back to original status). */
    else
      hp->sel = NULL;
  }

  return s;
}
