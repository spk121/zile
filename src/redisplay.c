/*	$Id: redisplay.c,v 1.5 2004/02/08 04:39:26 dacap Exp $	*/

/*
 * Copyright (c) 1997-2003 Sandro Sigala.  All rights reserved.
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

/*
 * Terminal independent redisplay routines.
 */

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
