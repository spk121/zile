/* Terminal independent redisplay routines
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

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

/*	$Id: redisplay.c,v 1.6 2004/02/17 20:21:18 ssigala Exp $	*/

#include <stdarg.h>

#include "config.h"
#include "zile.h"
#include "extern.h"

void resync_redisplay(void)
{
#if 1
	/* Normal Emacs-like resyncing calculation. */
	int delta = cur_bp->pt.n - cur_wp->lastpointn;

	if (delta > 0) {
		if (cur_wp->topdelta + delta < cur_wp->eheight)
			cur_wp->topdelta += delta;
		else if (cur_bp->pt.n > cur_wp->eheight / 2)
			cur_wp->topdelta = cur_wp->eheight / 2;
		else
			cur_wp->topdelta = cur_bp->pt.n;
	} else if (delta < 0) {
		if (cur_wp->topdelta + delta >= 0)
			cur_wp->topdelta += delta;
		else if (cur_bp->pt.n > cur_wp->eheight / 2)
			cur_wp->topdelta = cur_wp->eheight / 2;
		else
			cur_wp->topdelta = cur_bp->pt.n;
	}
	cur_wp->lastpointn = cur_bp->pt.n;
#else
	/* Alternative classic resyncing calculation. */
	if (cur_bp->pt.n < cur_wp->eheight)
		cur_wp->topdelta = cur_bp->pt.n;
	else
		cur_wp->topdelta = cur_bp->pt.n % cur_wp->eheight;
#endif
}

void recenter(Window *wp)
{
	Point pt = window_pt(wp);

	if (pt.n > wp->eheight / 2)
		wp->topdelta = wp->eheight / 2;
	else
		wp->topdelta = pt.n;
}

DEFUN("recenter", recenter)
/*+
Center point in window and redisplay screen.
The desired position of point is always relative to the current window.
+*/
{
	recenter(cur_wp);
	if (cur_bp->flags & BFLAG_FONTLOCK && lookup_bool_variable("auto-font-lock-refresh"))
		FUNCALL(font_lock_refresh);
	cur_tp->full_redisplay();
	return TRUE;
}
