/* Font Lock Mode
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
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

/*	$Id: fontlock.c,v 1.11 2004/03/14 14:36:05 rrt Exp $	*/

/*
 * In the future these routines may be rewritten using regex pattern
 * matching, simplifying a lot of the work.
 */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "zile.h"
#include "extern.h"


static void line_set_anchors(Buffer *bp, Line *lp, Anchor *lastanchor)
{
	if (lp->anchors) {
		free(lp->anchors);
		lp->anchors = NULL;
	}

	switch (bp->mode) {
		break;
	}
}

/*
 * Parse again the line for finding the anchors.  This is called when
 * the line get modified.
 */
void font_lock_reset_anchors(Buffer *bp, Line *lp)
{
	Anchor lastanchor;
	lastanchor = find_last_anchor(bp, lp->prev);
	line_set_anchors(bp, lp, &lastanchor);
}

/*
 * Find the last set anchor walking backward in the line list.
 */
Anchor find_last_anchor(Buffer *bp, Line *lp)
{
	Anchor *ap;

	for (; lp != bp->limitp; lp = lp->prev) {
		if (lp->anchors == NULL || lp->size == 0)
			continue;
		for (ap = lp->anchors + lp->size - 1; ap >= lp->anchors; --ap)
			if (*ap != ANCHOR_NULL) {
				return *ap;
			}
	}

	return ANCHOR_NULL;
}

/*
 * Go to the beginning of the buffer and parse for anchors.
 */
static void font_lock_set_anchors(Buffer *bp)
{
	Line *lp;
	Anchor lastanchor = ANCHOR_NULL;

	minibuf_write("Font Lock: setting anchors in `%s'...", bp->name);
	cur_tp->refresh();

	for (lp = bp->limitp->next; lp != bp->limitp; lp = lp->next)
		line_set_anchors(bp, lp, &lastanchor);

	minibuf_clear();
}

/*
 * Go to the beginning of the buffer and remove the anchors.
 */
static void font_lock_unset_anchors(Buffer *bp)
{
	Line *lp;

	minibuf_write("Font Lock: removing anchors from `%s'...", bp->name);
	cur_tp->refresh();

	for (lp = bp->limitp->next; lp != bp->limitp; lp = lp->next)
		if (lp->anchors != NULL) {
			free(lp->anchors);
			lp->anchors = NULL;
		}

	minibuf_clear();
}

DEFUN("font-lock-mode", font_lock_mode)
/*+
Toggle Font Lock mode.
When Font Lock mode is enabled, text is fontified as you type it.
+*/
{
	if (cur_bp->flags & BFLAG_FONTLOCK) {
		cur_bp->flags &= ~BFLAG_FONTLOCK;
		font_lock_unset_anchors(cur_bp);
	} else {
		cur_bp->flags |= BFLAG_FONTLOCK;
		font_lock_set_anchors(cur_bp);
	}

	return TRUE;
}

DEFUN("font-lock-refresh", font_lock_refresh)
/*+
Refresh the Font Lock mode internal structures.
This may be called when the fontification looks weird.
+*/
{
	if (cur_bp->flags & BFLAG_FONTLOCK) {
		font_lock_unset_anchors(cur_bp);
		font_lock_set_anchors(cur_bp);
	}

	return TRUE;
}
