/*	$Id: window.c,v 1.5 2003/10/24 23:32:09 ssigala Exp $	*/

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

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

windowp new_window(void)
{
	windowp wp;

	wp = (windowp)zmalloc(sizeof *wp);
	memset(wp, 0, sizeof *wp);

	return wp;
}

/*
 * Free the window allocated memory.
 */
void free_window(windowp wp)
{
	free(wp);
}

/*
 * Free all the allocated windows (used at Zile exit).
 */
void free_windows(void)
{
	windowp wp, next;

	for (wp = head_wp; wp != NULL; wp = next) {
		next = wp->next;
		free_window(wp);
	}
}

DEFUN("split-window", split_window)
/*+
Split current window into two windows, one above the other.
Both windows display the same buffer now current.
+*/
{
	windowp newwp;

	/* Windows smaller than 4 lines cannot be split. */
	if (cur_wp->fheight < 4) {
		minibuf_error("Window height %d too small for splitting", cur_wp->fheight);
		return FALSE;
	}

	newwp = new_window();
	newwp->fwidth = cur_wp->fwidth;
	newwp->ewidth = cur_wp->ewidth;
	newwp->fheight = cur_wp->fheight / 2 + cur_wp->fheight % 2;
	newwp->eheight = newwp->fheight - 1;
	cur_wp->fheight = cur_wp->fheight / 2;
	cur_wp->eheight = cur_wp->fheight - 1;
	if (cur_wp->topdelta >= cur_wp->eheight)
		recenter(cur_wp);
	newwp->bp = cur_bp;
	assert(cur_bp == cur_wp->bp);
	++newwp->bp->num_windows;
	newwp->pointp = cur_wp->pointp;
	newwp->pointn = cur_wp->pointn;
	newwp->pointo = cur_wp->pointo;
	newwp->next = cur_wp->next;
	cur_wp->next = newwp;

	return TRUE;
}

DEFUN("delete-window", delete_window)
/*+
Remove the current window from the screen.
+*/
{
	windowp wp;

	if (cur_wp == head_wp && cur_wp->next == NULL) {
		minibuf_error("Attempt to delete sole ordinary window");
		return FALSE;
	}

	if (--cur_bp->num_windows == 0) {
		/*
		 * This is the only window that displays
		 * the buffer.
		 */
		cur_bp->save_pointp = cur_wp->pointp;
		cur_bp->save_pointn = cur_wp->pointn;
		cur_bp->save_pointo = cur_wp->pointo;
	}

	if (cur_wp == head_wp)
		wp = head_wp = head_wp->next;
	else
		for (wp = head_wp; wp != NULL; wp = wp->next)
			if (wp->next == cur_wp) {
				wp->next = wp->next->next;
				break;
			}

	wp->fheight += cur_wp->fheight;
	wp->eheight += cur_wp->eheight + 1;
	free_window(cur_wp);
	cur_wp = wp;
	cur_bp = cur_wp->bp;

	return TRUE;
}

DEFUN("enlarge-window", enlarge_window)
/*+
Make current window one line bigger.
+*/
{
	windowp wp;

	if (cur_wp == head_wp && cur_wp->next == NULL)
		return FALSE;

	if ((wp = cur_wp->next) == NULL || wp->fheight < 3)
		for (wp = head_wp; wp != NULL; wp = wp->next)
			if (wp->next == cur_wp) {
				if (wp->fheight < 3)
					return FALSE;
				break;
			}

	if (cur_wp == head_wp && cur_wp->next->fheight < 3)
		return FALSE;

	--wp->fheight;
	--wp->eheight;
	if (wp->topdelta >= wp->eheight)
		recenter(wp);
	++cur_wp->fheight;
	++cur_wp->eheight;

	return TRUE;
}

DEFUN("shrink-window", shrink_window)
/*+
Make current window one line smaller.
+*/
{
	windowp wp;

	if ((cur_wp == head_wp && cur_wp->next == NULL) || cur_wp->fheight < 3)
		return FALSE;

	if ((wp = cur_wp->next) == NULL) {
		for (wp = head_wp; wp != NULL; wp = wp->next) {
			if (wp->next == cur_wp)
				break;
		}
	}

	++wp->fheight;
	++wp->eheight;
	--cur_wp->fheight;
	--cur_wp->eheight;
	if (cur_wp->topdelta >= cur_wp->eheight)
		recenter(cur_wp);

	return TRUE;
}

windowp popup_window(void)
{
	if (head_wp->next == NULL) {
		/* There is only one window on the screen, so split it. */
		FUNCALL(split_window);
		return cur_wp->next;
	}

	/* Use the window after the current one. */
	if (cur_wp->next != NULL)
		return cur_wp->next;

	/* Use the first window. */
	return head_wp;
}

DEFUN("delete-other-windows", delete_other_windows)
/*+
Make the selected window fill the screen.
+*/
{
	windowp wp, nextwp;

	for (wp = head_wp; wp != NULL; wp = nextwp) {
		nextwp = wp->next;
		if (wp != cur_wp) {
			if (--wp->bp->num_windows == 0) {
				/*
				 * This is the only window that displays
				 * the buffer.
				 */
				wp->bp->save_pointp = wp->pointp;
				wp->bp->save_pointn = wp->pointn;
				wp->bp->save_pointo = wp->pointo;
			}
			free_window(wp);
		}
	}

	cur_wp->fwidth = cur_wp->ewidth = cur_tp->width;
	/* Save space for minibuffer. */
	cur_wp->fheight = cur_tp->height - 1;
	/* Save space for status line. */
	cur_wp->eheight = cur_wp->fheight - 1;
	cur_wp->next = NULL;
	head_wp = cur_wp;

	return TRUE;
}

DEFUN("other-window", other_window)
/*+
Select the first different window on the screen.
All windows are arranged in a cyclic order.
This command selects the window one step away in that order.
+*/
{
	if (cur_wp->next != NULL)
		cur_wp = cur_wp->next;
	else
		cur_wp = head_wp;
	cur_bp = cur_wp->bp;

	return TRUE;
}

#if 0
void new_window_buffer(bufferp bp)
{
	if (--cur_bp->num_windows == 0) {
		/*
		 * This is the only window that displays
		 * the buffer.
		 */
		cur_bp->pointp = cur_wp->pointp;
		cur_bp->pointn = cur_wp->pointn;
		cur_bp->pointo = cur_wp->pointo;
	}

	cur_wp->bp = cur_bp = bp;
	++bp->num_windows;
	cur_wp->pointp = bp->pointp;
	cur_wp->pointn = bp->pointn;
	cur_wp->pointo = bp->pointo;
}
#endif

/*
 * This function is called once in main(), for creating
 * the scratch buffer.
 */
void create_first_window(void)
{
	windowp wp;
	bufferp bp;

	/*
	 * Create the scratch buffer.
	 */
	bp = new_buffer();
	set_buffer_name(bp, "*scratch*");
	bp->flags = BFLAG_NOSAVE | BFLAG_NEEDNAME | BFLAG_TEMPORARY;
	cur_bp = head_bp = bp;
	if (lookup_bool_variable("text-mode-auto-fill"))
		FUNCALL(auto_fill_mode);

	wp = new_window();
	cur_wp = head_wp = wp;
	wp->fwidth = wp->ewidth = cur_tp->width;
	/* Save space for minibuffer. */
	wp->fheight = cur_tp->height - 1;
	/* Save space for status line. */
	wp->eheight = wp->fheight - 1;
	wp->bp = bp;
	++bp->num_windows;
	wp->pointp = bp->save_pointp;
	wp->pointn = bp->save_pointn;
	wp->pointo = bp->save_pointo;
}

windowp find_window(const char *name)
{
	windowp wp;

	for (wp = head_wp; wp != NULL; wp = wp->next)
		if (!strcmp(wp->bp->name, name))
			return wp;

	return NULL;
}
