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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id: history.c,v 1.5 2005/01/08 22:41:34 rrt Exp $	*/

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

void free_history_elements(History *hp)
{
	if (hp->elements) {
		char *s;

		for (s = alist_first(hp->elements); s != NULL;
		     s = alist_next(hp->elements))
			free(s);

		alist_delete(hp->elements);
		hp->elements = NULL;
		hp->sel = NULL;
	}
}

void add_history_element(History *hp, const char *string)
{
	const char *last;

	if (!hp->elements)
		hp->elements = alist_new();

	last = alist_last(hp->elements);
	if (last && strcmp(last, string) == 0)
		return;

	alist_append(hp->elements, zstrdup(string));
}

void prepare_history(History *hp)
{
	hp->sel = NULL;
}

const char *previous_history_element(History *hp)
{
	const char *s = NULL;

	if (hp->elements) {
		/* First time that we use `previous-history-element'.  */
		if (!hp->sel) {
			/* Select last element.  */
			if (hp->elements->tail) {
				hp->sel = hp->elements->tail;
				s = hp->sel->p;
			}
		}
		/* There are another element? */
		else if (hp->sel->prev) {
			/* Select it.  */
			hp->sel = hp->sel->prev;
			s = hp->sel->p;
		}
	}

	return s;
}

const char *next_history_element(History *hp)
{
	const char *s = NULL;

	if (hp->elements && hp->sel) {
		/* Next element.  */
		if (hp->sel->next) {
			hp->sel = hp->sel->next;
			s = hp->sel->p;
		}
		/* No more element (back to original status).  */
		else {
			hp->sel = NULL;
		}
	}

	return s;
}
