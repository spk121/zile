/*	$Id: basic.c,v 1.2 2003/04/24 15:11:59 rrt Exp $	*/

/*
 * Copyright (c) 1997-2001 Sandro Sigala.  All rights reserved.
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

DEFUN("beginning-of-line", beginning_of_line)
/*+
Move point to beginning of current line.
+*/
{
	cur_wp->pointo = 0;

	thisflag |= FLAG_HIGHLIGHT_REGION_STAYS;

	return TRUE;
}

DEFUN("end-of-line", end_of_line)
/*+
Move point to end of current line.
+*/
{
	cur_wp->pointo = cur_wp->pointp->size;

	thisflag |= FLAG_HIGHLIGHT_REGION_STAYS;

	return TRUE;
}

/*
 * Get the current goal column.  Take care of expanding
 * tabulations.
 */
static int get_goalc(void)
{
	int col = 0, t = cur_bp->tab_width;
	char *sp = cur_wp->pointp->text, *p = sp;

	while (p < sp + cur_wp->pointo) {
		if (*p == '\t')
			col |= t - 1;
		++col, ++p;
	}

	return col;
}

/*
 * Go to the column `goalc'.  Take care of expanding
 * tabulations.
 */
static void goto_goalc(int goalc)
{
	int col = 0, t = cur_bp->tab_width, w;
	char *sp = cur_wp->pointp->text, *p = sp;

	while (p < sp + cur_wp->pointp->size) {
		if (col == goalc)
			break;
		else if (*p == '\t') {
			for (w = t - col % t; w > 0; w--)
				if (++col == goalc)
					break;
		} else
			++col;
		++p;
	}

	cur_wp->pointo = p - sp;
}

static int cur_goalc;

int previous_line(void)
{
	if (cur_wp->pointp->prev != cur_bp->limitp) {
		thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;

		if (!(lastflag & FLAG_DONE_CPCN))
			cur_goalc = get_goalc();

		cur_wp->pointp = cur_wp->pointp->prev;
		--cur_wp->pointn;

		goto_goalc(cur_goalc);

		return TRUE;
	}

	return FALSE;
}

DEFUN("previous-line", previous_line)
/*+
Move cursor vertically up one line.
If there is no character in the target line exactly over the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
+*/
{
	int uni;

	thisflag |= FLAG_HIGHLIGHT_REGION_STAYS;

	if (uniarg < 0)
		return FUNCALL_ARG(next_line, -uniarg);

	for (uni = 0; uni < uniarg; ++uni)
		if (!previous_line()) {
			minibuf_error("Beginning of buffer");
			return FALSE;
		}

	return TRUE;
}

int next_line(void)
{
	if (cur_wp->pointp->next != cur_bp->limitp) {
		thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;

		if (!(lastflag & FLAG_DONE_CPCN))
			cur_goalc = get_goalc();

		cur_wp->pointp = cur_wp->pointp->next;
		++cur_wp->pointn;

		goto_goalc(cur_goalc);

		return TRUE;
	}

	return FALSE;
}

DEFUN("next-line", next_line)
/*+
Move cursor vertically down one line.
If there is no character in the target line exactly under the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
+*/
{
	int uni;

	thisflag |= FLAG_HIGHLIGHT_REGION_STAYS;

	if (uniarg < 0)
		return FUNCALL_ARG(previous_line, -uniarg);

	for (uni = 0; uni < uniarg; ++uni)
		if (!next_line()) {
			minibuf_error("End of buffer");
			return FALSE;
		}

	return TRUE;
}

/*
 * Go to the line `to_line', counting from 0.  Point will end up in
 * "random" column.
 */
void goto_line(int to_line)
{
	if (cur_wp->pointn > to_line)
		ngotoup(cur_wp->pointn - to_line);
	else if (cur_wp->pointn < to_line)
		ngotodown(to_line - cur_wp->pointn);
}

DEFUN("goto-line", goto_line)
/*+
Move cursor to the beginning of the specified line.
+*/
{
	char *ms;
	int to_line;
  
	do {
		if ((ms = minibuf_read("Goto line: ", "")) == NULL)
			return cancel();
		if ((to_line = atoi(ms)) < 1)
			ding();
	} while (to_line < 1);

	goto_line(to_line - 1);
	cur_wp->pointo = 0;

	return TRUE;
}

/*
 * Move point to the beginning of the buffer; do not touch the mark.
 */
void gotobob(void)
{
	cur_wp->pointp = cur_bp->limitp->next;
	cur_wp->pointo = 0;
	cur_wp->pointn = 0;
	thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;
}

DEFUN("beginning-of-buffer", beginning_of_buffer)
/*+
Move point to the beginning of the buffer; leave mark at previous position.
+*/
{
	set_mark_command();
	gotobob();

	thisflag |= FLAG_HIGHLIGHT_REGION_STAYS;

	return TRUE;
}

/*
 * Move point to the end of the buffer; do not touch the mark.
 */
void gotoeob(void)
{
	cur_wp->pointp = cur_bp->limitp->prev;
	cur_wp->pointo = cur_wp->pointp->size;
	cur_wp->pointn = cur_bp->num_lines;
	thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;
}

DEFUN("end-of-buffer", end_of_buffer)
/*+
Move point to the end of the buffer; leave mark at previous position.
+*/
{
	set_mark_command();
	gotoeob();

	thisflag |= FLAG_HIGHLIGHT_REGION_STAYS;

	return TRUE;
}

int backward_char(void)
{
	if (cur_wp->pointo > 0) {
		--cur_wp->pointo;

		return TRUE;
	} else if (cur_wp->pointp->prev != cur_bp->limitp) {
		thisflag |= FLAG_NEED_RESYNC;
		cur_wp->pointp = cur_wp->pointp->prev;
		--cur_wp->pointn;
		FUNCALL(end_of_line);

		return TRUE;
	}

	return FALSE;
}

DEFUN("backward-char", backward_char)
/*+
Move point left one character.
On attempt to pass beginning or end of buffer, stop and signal error.
+*/
{
	int uni;

	thisflag |= FLAG_HIGHLIGHT_REGION_STAYS;

	if (uniarg < 0)
		return FUNCALL_ARG(forward_char, -uniarg);

	for (uni = 0; uni < uniarg; ++uni)
		if (!backward_char()) {
			minibuf_error("Beginning of buffer");
			return FALSE;
		}

	return TRUE;
}

int forward_char(void)
{
	if (cur_wp->pointo < cur_wp->pointp->size) {
		++cur_wp->pointo;

		return TRUE;
	} else if (cur_wp->pointp->next != cur_bp->limitp) {
		thisflag |= FLAG_NEED_RESYNC;
		cur_wp->pointp = cur_wp->pointp->next;
		++cur_wp->pointn;
		FUNCALL(beginning_of_line);

		return TRUE;
	}

	return FALSE;
}

DEFUN("forward-char", forward_char)
/*+
Move point right one character.
On reaching end of buffer, stop and signal error.
+*/
{
	int uni;

	thisflag |= FLAG_HIGHLIGHT_REGION_STAYS;

	if (uniarg < 0)
		return FUNCALL_ARG(backward_char, -uniarg);

	for (uni = 0; uni < uniarg; ++uni)
		if (!forward_char()) {
			minibuf_error("End of buffer");
			return FALSE;
		}

	return TRUE;
}

int ngotoup(int n)
{
	for (; n > 0; n--)
		if (cur_wp->pointp->prev != cur_bp->limitp)
			FUNCALL(previous_line);
		else
			return FALSE;

	return TRUE;
}

int ngotodown(int n)
{
	for (; n > 0; n--)
		if (cur_wp->pointp->next != cur_bp->limitp)
			FUNCALL(next_line);
		else
			return FALSE;

	return TRUE;
}

/* The number of lines to scroll down or up. */
#define SCROLL_LINES	cur_wp->eheight

int scroll_down(void)
{
	if (cur_wp->pointn > 0) {
		if (ngotoup(SCROLL_LINES)) {
			/* XXX */
			return TRUE;
		} else
			return FALSE;
	} else if (cur_wp->pointo > 0)
		FUNCALL(beginning_of_line);
	else {
		minibuf_error("Beginning of buffer");
		return FALSE;
	}

	return TRUE;
}

DEFUN("scroll-down", scroll_down)
/*+
Scroll text of current window downward near full screen.
+*/
{
	int uni;

	thisflag |= FLAG_HIGHLIGHT_REGION_STAYS;

	if (uniarg < 0)
		return FUNCALL_ARG(scroll_up, -uniarg);

	for (uni = 0; uni < uniarg; ++uni)
		if (!scroll_down())
			return FALSE;

	return TRUE;
}

int scroll_up(void)
{
	if (cur_wp->pointn < cur_bp->num_lines) {
		if (ngotodown(SCROLL_LINES)) {
			/* XXX */
			return TRUE;
		} else
			return FALSE;
	} else if (cur_wp->pointo < cur_wp->pointp->size)
		FUNCALL(end_of_line);
	else {
		minibuf_error("End of buffer");
		return FALSE;
	}

	return TRUE;
}

DEFUN("scroll-up", scroll_up)
/*+
Scroll text of current window upward near full screen.
+*/
{
	int uni;

	thisflag |= FLAG_HIGHLIGHT_REGION_STAYS;

	if (uniarg < 0)
		return FUNCALL_ARG(scroll_down, -uniarg);

	for (uni = 0; uni < uniarg; ++uni)
		if (!scroll_up())
			return FALSE;

	return TRUE;
}
