/*	$Id: undo.c,v 1.8 2004/02/08 04:39:26 dacap Exp $	*/

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

/* This variable is set to TRUE when the `undo_save()' function should not
   register the undo information. */
int undo_nosave = FALSE;

/* This variable is set to TRUE when the undo is in execution. */
static int doing_undo = FALSE;

/*
 * Save a reverse delta for doing undo.
 */
void undo_save(int type, Point pt, int arg1, int arg2)
{
	Undo *up;

	if (cur_bp->flags & BFLAG_NOUNDO || undo_nosave)
		return;

	up = (Undo *)zmalloc(sizeof(Undo));

	up->type = type;

	up->pt = pt;

	switch (type) {
	case UNDO_INSERT_CHAR:
	case UNDO_REPLACE_CHAR:
	case UNDO_INTERCALATE_CHAR:
		up->delta.c = arg1;
		break;
	case UNDO_INSERT_BLOCK:
		up->delta.block.size = arg1;
		up->delta.block.text = copy_text_block(pt.n, pt.o, arg1);
		break;
	case UNDO_REPLACE_BLOCK:
		up->delta.block.osize = arg1;
		up->delta.block.size = arg2;
		up->delta.block.text = copy_text_block(pt.n, pt.o, arg1);
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
static Undo *revert_action(Undo *up)
{
	int i;

	doing_undo = TRUE;

	if (up->type == UNDO_END_SEQUENCE) {
		undo_save(UNDO_START_SEQUENCE, up->pt, 0, 0);
		up = up->next;
		while (up->type != UNDO_START_SEQUENCE) {
			up = revert_action(up);
		}
		undo_save(UNDO_END_SEQUENCE, up->pt, 0, 0);
		goto_point(up->pt);
		return up->next;
	}

	goto_point(up->pt);

	switch (up->type) {
	case UNDO_INSERT_CHAR:
		if (up->delta.c == '\n')
			insert_newline();
		else
			insert_char_in_insert_mode(up->delta.c);
		break;
	case UNDO_INTERCALATE_CHAR:
		if (up->delta.c == '\n')
			intercalate_newline();
		else
			intercalate_char(up->delta.c);
		break;
	case UNDO_INSERT_BLOCK:
		undo_save(UNDO_REMOVE_BLOCK, up->pt, up->delta.block.size, 0);
		undo_nosave = TRUE;
		for (i = 0; i < up->delta.block.size; ++i)
			if (up->delta.block.text[i] != '\n')
				insert_char(up->delta.block.text[i]);
			else
				insert_newline();
		undo_nosave = FALSE;
		break;
	case UNDO_REMOVE_CHAR:
		delete_char();
		break;
	case UNDO_REMOVE_BLOCK:
		undo_save(UNDO_INSERT_BLOCK, up->pt, up->delta.block.size, 0);
		undo_nosave = TRUE;
		for (i = 0; i < up->delta.block.size; ++i)
			delete_char();
		undo_nosave = FALSE;
		break;
	case UNDO_REPLACE_CHAR:
		undo_save(UNDO_REPLACE_CHAR, up->pt,
			  cur_bp->pt.p->text[up->pt.o], 0);
		cur_bp->pt.p->text[up->pt.o] = up->delta.c;
		cur_bp->flags |= BFLAG_MODIFIED;
#if ENABLE_NONTEXT_MODES
		if (cur_bp->flags & BFLAG_FONTLOCK)
			font_lock_reset_anchors(cur_bp, cur_bp->pt.p);
#endif
		break;
	case UNDO_REPLACE_BLOCK:
		undo_save(UNDO_REPLACE_BLOCK, up->pt,
			  up->delta.block.size, up->delta.block.osize);
		undo_nosave = TRUE;
		for (i = 0; i < up->delta.block.size; ++i)
			delete_char();
		for (i = 0; i < up->delta.block.osize; ++i)
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
