/*	$Id: history.c,v 1.1 2004/02/08 04:39:26 dacap Exp $	*/

/*
 * Copyright (c) 2004 David A. Capello.  All rights reserved.
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

#include "config.h"

#include <stdlib.h>

#include "zile.h"
#include "extern.h"

void free_history_elements(History *hp)
{
	if (hp->elements) {
		char *s;

		for (s=alist_first(hp->elements); s;
		     s=alist_next(hp->elements))
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
