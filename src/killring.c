/*	$Id: killring.c,v 1.6 2004/02/08 04:39:26 dacap Exp $	*/

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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "zile.h"
#include "extern.h"
#include "editfns.h"

static char *kill_ring_text;
static int kill_ring_size;
static int kill_ring_maxsize;

static void flush_kill_ring(void)
{
	kill_ring_size = 0;
	if (kill_ring_maxsize > 1024) {
		/*
		 * Deallocate memory area, since is big enough.
		 */
		kill_ring_maxsize = 0;
		free(kill_ring_text);
		kill_ring_text = NULL;
	}
}

static void kill_ring_push(char c)
{
	if (kill_ring_text == NULL) {
		kill_ring_maxsize = 16;
		kill_ring_text = (char *)zmalloc(kill_ring_maxsize);
	} else if (kill_ring_size + 1 >= kill_ring_maxsize) {
		kill_ring_maxsize += 16;
		kill_ring_text = (char *)zrealloc(kill_ring_text, kill_ring_maxsize);
	}
	kill_ring_text[kill_ring_size++] = c;
}

static void kill_ring_push_in_beginning(char c)
{
	kill_ring_push(0);
	memmove(kill_ring_text+1, kill_ring_text, kill_ring_size-1);
	kill_ring_text[0] = c;
}

static void kill_ring_push_nstring(char *s, size_t size)
{
	if (kill_ring_text == NULL) {
		kill_ring_maxsize = size;
		kill_ring_text = (char *)zmalloc(size);
	} else if (kill_ring_size + (int)size >= kill_ring_maxsize) {
		kill_ring_maxsize += size;
		kill_ring_text = (char *)zrealloc(kill_ring_text, kill_ring_maxsize);
	}
	memcpy(kill_ring_text + kill_ring_size, s, size);
	kill_ring_size += size;
}

static int kill_line(int literally)
{
	if (!eolp()) {
		if (warn_if_readonly_buffer())
			return FALSE;

		undo_save(UNDO_INSERT_BLOCK, cur_bp->pt,
			  cur_bp->pt.p->size - cur_bp->pt.o, 0);
		undo_nosave = TRUE;
		while (!eolp()) {
			kill_ring_push(following_char());
			FUNCALL(delete_char);
		}
		undo_nosave = FALSE;

		thisflag |= FLAG_DONE_KILL;

		if (!literally)
		  return TRUE;
	}

	if (cur_bp->pt.p->next != cur_bp->limitp) {
		if (!FUNCALL(delete_char))
			return FALSE;

		kill_ring_push('\n');

		thisflag |= FLAG_DONE_KILL;

		return TRUE;
	}

	minibuf_error("End of buffer");

	return FALSE;
}

DEFUN("kill-line", kill_line)
/*+
Kill the rest of the current line; if no nonblanks there, kill thru newline.
+*/
{
	int uni, ret = TRUE;

	if (!(lastflag & FLAG_DONE_KILL))
		flush_kill_ring();

	if (uniarg == 1) {
		kill_line(FALSE);
	}
	else {
		undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
		for (uni = 0; uni < uniarg; ++uni)
			if (!kill_line(TRUE)) {
				ret = FALSE;
				break;
			}
		undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
	}

	desactivate_mark();
	return ret;
}

DEFUN("kill-region", kill_region)
/*+
Kill between point and mark.
The text is deleted but saved in the kill ring.
The command C-y (yank) can retrieve it from there.
If the buffer is read-only, Zile will beep and refrain from deleting
the text, but put the text in the kill ring anyway.  This means that
you can use the killing commands to copy text from a read-only buffer.
If the previous command was also a kill command,
the text killed this time appends to the text killed last time
to make one entry in the kill ring.
+*/
{
	Region r;

	if (!(lastflag & FLAG_DONE_KILL))
		flush_kill_ring();

	if (warn_if_no_mark())
		return FALSE;

	calculate_region(&r);

	if (cur_bp->flags & BFLAG_READONLY) {
		/*
		 * The buffer is readonly; save only in the kill buffer
		 * and complain.
		 */
		char *p;

		p = copy_text_block(r.start.n, r.start.o, r.size);
		kill_ring_push_nstring(p, r.size);
		free(p);

		warn_if_readonly_buffer();
	} else {
		int size = r.size;

		if (cur_bp->pt.p != r.start.p || r.start.o != cur_bp->pt.o)
			FUNCALL(exchange_point_and_mark);
		undo_save(UNDO_INSERT_BLOCK, cur_bp->pt, size, 0);
		undo_nosave = TRUE;
		while (size--) {
			if (!eolp())
				kill_ring_push(following_char());
			else
				kill_ring_push('\n');
			FUNCALL(delete_char);
		}
		undo_nosave = FALSE;
	}

	thisflag |= FLAG_DONE_KILL;
	desactivate_mark();

	return TRUE;
}

DEFUN("copy-region-as-kill", copy_region_as_kill)
/*+
Save the region as if killed, but don't kill it.
+*/
{
	Region r;
	char *p;

	if (!(lastflag & FLAG_DONE_KILL))
		flush_kill_ring();

	if (warn_if_no_mark())
		return FALSE;

	calculate_region(&r);

	p = copy_text_block(r.start.n, r.start.o, r.size);
	kill_ring_push_nstring(p, r.size);
	free(p);

	thisflag |= FLAG_DONE_KILL;
	desactivate_mark();

	return TRUE;
}

DEFUN("kill-word", kill_word)
/*+
Kill characters forward until encountering the end of a word.
With argument, do this that many times.
+*/
{
	if (!(lastflag & FLAG_DONE_KILL))
		flush_kill_ring();

	if (warn_if_readonly_buffer())
		return FALSE;

	push_mark();
	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	FUNCALL_ARG(mark_word, uniarg);
	FUNCALL(kill_region);
	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
	pop_mark();

	thisflag |= FLAG_DONE_KILL;

	minibuf_write("");	/* Don't write "Set mark" message.  */

	return TRUE;
}

DEFUN("backward-kill-word", backward_kill_word)
/*+
Kill characters backward until encountering the end of a word.
With argument, do this that many times.
+*/
{
	return FUNCALL_ARG(kill_word, !uniarg? -1: -uniarg);
}

DEFUN("kill-sexp", kill_sexp)
/*+
Kill the sexp (balanced expression) following the cursor.
With ARG, kill that many sexps after the cursor.
Negative arg -N means kill N sexps before the cursor.
+*/
{
	if (!(lastflag & FLAG_DONE_KILL))
		flush_kill_ring();

	if (warn_if_readonly_buffer())
		return FALSE;

	push_mark();
	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	FUNCALL_ARG(mark_sexp, uniarg);
	FUNCALL(kill_region);
	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
	pop_mark();

	thisflag |= FLAG_DONE_KILL;

	minibuf_write("");	/* Don't write "Set mark" message.  */

	return TRUE;
}

DEFUN("yank", yank)
/*+
Reinsert the last stretch of killed text.
More precisely, reinsert the stretch of killed text most recently
killed OR yanked.  Put point at end, and set mark at beginning.
+*/
{
	if (kill_ring_size == 0) {
		minibuf_error("Kill ring is empty");
		return FALSE;
	}

	if (warn_if_readonly_buffer())
		return FALSE;

	set_mark_command();

	undo_save(UNDO_REMOVE_BLOCK, cur_bp->pt, kill_ring_size, 0);
	undo_nosave = TRUE;
	insert_nstring(kill_ring_text, kill_ring_size);
	undo_nosave = FALSE;

	desactivate_mark();

	return TRUE;
}

void free_kill_ring(void)
{
	if (kill_ring_text != NULL)
		free(kill_ring_text);
}
