/* Useful editing functions
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

/*	$Id: editfns.c,v 1.2 2004/02/17 20:21:18 ssigala Exp $	*/

#include "config.h"

#include <stdlib.h>
#include <ctype.h>

#include "zile.h"
#include "extern.h"
#include "editfns.h"

static alist mark_ring = NULL;	/* Mark-ring.  */

/* Push the current mark in the mark-ring.  */

void push_mark(void)
{
	if (!mark_ring)
		mark_ring = alist_new();

	/* Save the mark.  */
	if (cur_bp->mark)
		alist_append(mark_ring, copy_marker(cur_bp->mark));
	/* Save an invalid mark.  */
	else {
		Marker *m = point_min_marker();
		m->pt.p = NULL;
		alist_append(mark_ring, m);
	}
}

/* Pop a mark from the mark-ring a put it as current mark.  */

void pop_mark(void)
{
	Marker *m = alist_last(mark_ring);

	/* Replace the mark.  */
	if (m->bp->mark)
		free_marker(m->bp->mark);

	m->bp->mark = (m->pt.p) ? copy_marker(m): NULL;

	alist_remove(mark_ring);
	free_marker(m);

	/* Empty list?  */
	if (alist_isempty(mark_ring)) {
		alist_delete(mark_ring);
		mark_ring = NULL;
	}
}

/* Set the mark to the point position.  */

void set_mark(void)
{
	if (!cur_bp->mark)
		cur_bp->mark = point_marker();
	else
		move_marker(cur_bp->mark, cur_bp, cur_bp->pt);
}

int is_empty_line(void)
{
	return cur_bp->pt.p->size == 0;
}

int is_blank_line(void)
{
	int c;
	for (c=0; c<cur_bp->pt.p->size; c++)
		if (!isspace(cur_bp->pt.p->text[c]))
			return FALSE;
	return TRUE;
}

int char_after(Point *pt)
{
	/* if (eobp()) */
	if (pt->p->next == cur_bp->limitp && pt->o == pt->p->size)
		return 0;
	/* else if (eolp()) */
	else if (pt->o == pt->p->size)
		return '\n';
	else
		return pt->p->text[pt->o];
}

int char_before(Point *pt)
{
	/* if (bobp()) */
	if (pt->p->prev == cur_bp->limitp && pt->o == 0)
		return 0;
	/* else if (bolp()) */
	else if (pt->o == 0)
		return '\n';
	else
		return pt->p->text[pt->o-1];
}

/* This function returns the character following point in the current
   buffer.  */

int following_char(void)
{
	return char_after(&cur_bp->pt);
}

/* This function returns the character preceding point in the current
   buffer.  */

int preceding_char(void)
{
	return char_before(&cur_bp->pt);
}

/* This function returns TRUE if point is at the beginning of the
   buffer.  */

int bobp(void)
{
	return (cur_bp->pt.p->prev == cur_bp->limitp &&
		cur_bp->pt.o == 0);
}

/* This function returns TRUE if point is at the end of the
   buffer.  */

int eobp(void)
{
	return (cur_bp->pt.p->next == cur_bp->limitp &&
		cur_bp->pt.o == cur_bp->pt.p->size);
}

/* Returns TRUE if point is at the beginning of a line.  */

int bolp(void)
{
	return cur_bp->pt.o == 0;
}

/* Returns TRUE if point is at the end of a line.  */

int eolp(void)
{
	return cur_bp->pt.o == cur_bp->pt.p->size;
}
