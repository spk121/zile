/*	$Id: undo.c,v 1.2 2003/04/24 15:12:00 rrt Exp $	*/

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

/* This variable is set to TRUE when the `undo_save()' function should not
   register the undo information. */
int undo_nosave = FALSE;

/* This variable is set to TRUE when the undo is in execution. */
static int doing_undo = FALSE;

/*
 * Jump to the specified line number and offset.
 */
static void goto_point(int pointn, int pointo)
{
	if (cur_wp->pointn > pointn)
		do 
			FUNCALL(previous_line);
		while (cur_wp->pointn > pointn);
	else if (cur_wp->pointn < pointn)
		do
			FUNCALL(next_line);
		while (cur_wp->pointn < pointn);
	if (cur_wp->pointo > pointo)
		do
			FUNCALL(backward_char);
		while (cur_wp->pointo > pointo);
	else if (cur_wp->pointo < pointo)
		do
			FUNCALL(forward_char);
		while (cur_wp->pointo < pointo);
}

/*
 * Save a reverse delta for doing undo.
 */
void undo_save(int type, int startn, int starto, int arg1, int arg2)
{
	undop up;

	if (cur_bp->flags & BFLAG_NOUNDO || undo_nosave)
		return;

	up = (undop)zmalloc(sizeof(*up));

	up->type = type;

	up->pointn = startn;
	up->pointo = starto;

	switch (type) {
	case UNDO_INSERT_CHAR:
	case UNDO_REPLACE_CHAR:
		up->delta.c = arg1;
		break;
	case UNDO_INSERT_BLOCK:
		up->delta.block.size = arg1;
		up->delta.block.text = copy_text_block(startn, starto, arg1);
		break;
	case UNDO_REPLACE_BLOCK:
		up->delta.block.osize = arg1;
		up->delta.block.size = arg2;
		up->delta.block.text = copy_text_block(startn, starto, arg1);
		break;
	case UNDO_REMOVE_BLOCK:
		up->delta.block.size = arg1;
		break;
	case UNDO_START_SEQUENCE:
	case UNDO_END_SEQUENCE:
		break;
	}

	up->next = cur_bp->last_undop;
	cur_bp->last_undop = up;
	if (!doing_undo)
		cur_bp->next_undop = up;
}

/*
 * Revert an action.  Return the next undo entry.
 */
static undop revert_action(undop up)
{
	int i;

	doing_undo = TRUE;

	if (up->type == UNDO_END_SEQUENCE) {
		undo_save(UNDO_START_SEQUENCE, up->pointn, up->pointo, 0, 0);
		up = up->next;
		while (up->type != UNDO_START_SEQUENCE) {
			revert_action(up);
			up = up->next;
		}
		undo_save(UNDO_END_SEQUENCE, up->pointn, up->pointo, 0, 0);
		goto_point(up->pointn, up->pointo);
		return up->next;
	}

	goto_point(up->pointn, up->pointo);

	switch (up->type) {
	case UNDO_INSERT_CHAR:
		if (up->delta.c == '\n')
			insert_newline();
		else
			insert_char(up->delta.c);
		break;
	case UNDO_INSERT_BLOCK:
		undo_save(UNDO_REMOVE_BLOCK, up->pointn, up->pointo, up->delta.block.size, 0);
		undo_nosave = TRUE;
		for (i = 0; i < up->delta.block.size; i++)
			if (up->delta.block.text[i] != '\n')
				insert_char(up->delta.block.text[i]);
			else
				insert_newline();
		undo_nosave = FALSE;
		break;
	case UNDO_REMOVE_CHAR:
		FUNCALL(delete_char);
		break;
	case UNDO_REMOVE_BLOCK:
		undo_save(UNDO_INSERT_BLOCK, up->pointn, up->pointo, up->delta.block.size, 0);
		undo_nosave = TRUE;
		for (i = 0; i < up->delta.block.size; i++)
			FUNCALL(delete_char);
		undo_nosave = FALSE;
		break;
	case UNDO_REPLACE_CHAR:
		undo_save(UNDO_REPLACE_CHAR, up->pointn, up->pointo,
			  cur_wp->pointp->text[up->pointo], 0);
		cur_wp->pointp->text[up->pointo] = up->delta.c;
		cur_bp->flags |= BFLAG_MODIFIED;
		if (cur_bp->flags & BFLAG_FONTLOCK)
			font_lock_reset_anchors(cur_bp, cur_wp->pointp);
		break;
	case UNDO_REPLACE_BLOCK:
		undo_save(UNDO_REPLACE_BLOCK, up->pointn, up->pointo,
			  up->delta.block.size, up->delta.block.osize);
		undo_nosave = TRUE;
		for (i = 0; i < up->delta.block.size; i++)
			FUNCALL(delete_char);
		for (i = 0; i < up->delta.block.osize; i++)
			if (up->delta.block.text[i] != '\n')
				insert_char(up->delta.block.text[i]);
			else
				insert_newline();
		undo_nosave = FALSE;
		break;
	}

	doing_undo = FALSE;

	return up->next;
}

DEFUN("undo", undo)
/*+
Undo some previous changes.
Repeat this command to undo more changes.
+*/
{
	if (cur_bp->flags & BFLAG_NOUNDO) {
		minibuf_error("Undo disabled in this buffer");
		return FALSE;
	}

	if (warn_if_readonly_buffer())
		return FALSE;

	if (cur_bp->next_undop == NULL) {
		minibuf_error("No further undo information");
		cur_bp->next_undop = cur_bp->last_undop;
		return FALSE;
	}

	/*
	 * Switch to next undo entry; unset the BFLAG_MODIFIED flag
	 * if all the undo deltas were applied.
	 */
	cur_bp->next_undop = revert_action(cur_bp->next_undop);
	if (cur_bp->next_undop == NULL)
		cur_bp->flags &= ~BFLAG_MODIFIED;

	minibuf_write("Undo!");

	return TRUE;
}
