/*	$Id: buffer.c,v 1.2 2003/04/24 15:11:59 rrt Exp $	*/

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

/*
 * Allocate a new buffer structure and set the default local
 * variable values.
 * The allocation of the first empty line is done here to simplify
 * a lot the code.
 */
bufferp new_buffer(void)
{
	bufferp bp;
	char *s;

	bp = (bufferp)zmalloc(sizeof *bp);
	memset(bp, 0, sizeof *bp);

	if ((s = get_variable("tab-width")) != NULL) {
		bp->tab_width = atoi(s);
		if (bp->tab_width < 1) {
			minibuf_error("Warning: wrong global tab-width value `@v%s@@'", s);
			waitkey(2 * 1000);
			bp->tab_width = 8;
		}
	} else
		bp->tab_width = 8;

	if ((s = get_variable("fill-column")) != NULL) {
		bp->fill_column = atoi(s);
		if (bp->fill_column < 2) {
			minibuf_error("warning: wrong global fill-column value `@v%s@@'", s);
			waitkey(2 * 1000);
			bp->fill_column = 70;
		}
	} else
		bp->fill_column = 70;

	/* Allocate a minimal space for a line. */
	bp->save_pointp = new_line(1);

	/* Allocate a null line for the limit marker. */
	bp->limitp = new_line(0);

	bp->limitp->prev = bp->limitp->next = bp->save_pointp;
	bp->save_pointp->prev = bp->save_pointp->next = bp->limitp;

	return bp;
}

/*
 * Free the buffer allocated memory.
 */
void free_buffer(bufferp bp)
{
	linep lp, next_lp;
	undop up, next_up;

	/*
	 * Free all the lines.
	 */
	lp = bp->limitp->next;
	while (lp != bp->limitp) {
		next_lp = lp->next;
		free_line(lp);
		lp = next_lp;
	}

	free(bp->limitp);

	/*
	 * Free all the undo operations.
	 */
	up = bp->last_undop;
	while (up != NULL) {
		next_up = up->next;
		if (up->type == UNDO_INSERT_BLOCK || up->type == UNDO_REPLACE_BLOCK)
			free(up->delta.block.text);
		free(up);
		up = next_up;
	}

	/*
	 * Free the name and the filename.
	 */
	free(bp->name);
	if (bp->filename != NULL)
		free(bp->filename);

	free(bp);
}

/*
 * Free all the allocated buffers (used at Zile exit).
 */
void free_buffers(void)
{
	bufferp bp, next;

	for (bp = head_bp; bp != NULL; bp = next) {
		next = bp->next;
		free_buffer(bp);
	}
}

/*
 * Allocate a new buffer and insert it into the buffer list.
 */
bufferp create_buffer(const char *name)
{
	bufferp bp;

	bp = new_buffer();
	set_buffer_name(bp, name);

	bp->next = head_bp;
        head_bp = bp;

	return bp;
}

/*
 * Set a new name for the buffer.
 */
void set_buffer_name(bufferp bp, const char *name)
{
	if (bp->name != NULL)
		free(bp->name);
	bp->name = zstrdup(name);
}

/*
 * Set a new filename (and a name) for the buffer.
 */
void set_buffer_filename(bufferp bp, char *filename)
{
	if (bp->filename != NULL)
		free(bp->filename);
	bp->filename = zstrdup(filename);

	if (bp->name != NULL)
		free(bp->name);
	bp->name = "";
	bp->name = make_buffer_name(filename);
}

/*
 * Search for a buffer named `name'.  If not buffer is found and
 * the `cflag' variable is set to `TRUE', create a new buffer.
 */
bufferp find_buffer(const char *name, int cflag)
{
	bufferp bp;

	for (bp = head_bp; bp != NULL; bp = bp->next)
		if (!strcmp(bp->name, name))
			return bp;

	/*
	 * Create flag not specified, return NULL.
	 */
	if (!cflag)
		return NULL;

	/*
	 * No buffer found with the specified name, then
	 * create one.
	 */
	bp = create_buffer(name);

	return bp;
}

/*
 * Find the next buffer in buffer list.
 */
bufferp get_next_buffer(void)
{
	if (cur_bp->next != NULL)
		return cur_bp->next;
	return head_bp;
}

/*
 * Create a buffer name using the file name.
 */
char *make_buffer_name(char *filename)
{
	char *p, *name;
	int i;

	if ((p = strrchr(filename, '/')) == NULL)
		p = filename;
	else
		++p;

	if (find_buffer(p, FALSE) == NULL)
		return zstrdup(p);

	/*
	 * The limit of 999 buffers with the same name should
	 * be enough; if it is too much restrictive for you,
	 * just change 999 to your preferred value :-)
	 */
	name = (char *)zmalloc(strlen(p) + 6);	/* name + "<nnn>\0" */
	for (i = 2; i <= 999; i++) {
		sprintf(name, "%s<%d>", p, i);
		if (find_buffer(name, FALSE) == NULL)
			return name;
	}

	/*
	 * This should never happen.
	 */
	assert(0);
	return NULL;
}

/*
 * Switch to the specified buffer.
 */
void switch_to_buffer(bufferp bp)
{
	windowp wp;

	assert(cur_wp->bp == cur_bp);

	/*
	 * The buffer is the current buffer; return safely.
	 */
	if (cur_bp == bp)
		return;

	thisflag |= FLAG_NEED_RESYNC;

	if (--cur_bp->num_windows == 0) {
		/*
		 * This is the only window that displays the buffer.
		 */
		cur_bp->save_pointp = cur_wp->pointp;
		cur_bp->save_pointn = cur_wp->pointn;
		cur_bp->save_pointo = cur_wp->pointo;
	}

	cur_wp->bp = cur_bp = bp;

	if (bp->num_windows++ == 0) {
		/*
		 * First use of buffer.
		 */
		cur_wp->pointp = bp->save_pointp;
		cur_wp->pointn = bp->save_pointn;
		cur_wp->pointo = bp->save_pointo;

		return;
	}

	/*
	 * The buffer is already displayed is another
	 * windows.  Search for the first one.
	 */
	for (wp = head_wp; wp != NULL; wp = wp->next)
		if (wp->bp == bp && wp != cur_wp) {
			cur_wp->pointp = wp->pointp;
			cur_wp->pointo = wp->pointo;
			break;
		}
}

/*
 * Remove the buffer contents and reset the mark and the flags.
 */
int zap_buffer_content(void)
{
	windowp wp;
	linep new_lp, old_lp, next_lp;

	new_lp = new_line(1);
	new_lp->next = new_lp->prev = cur_bp->limitp;

	old_lp = cur_bp->limitp->next;
	cur_bp->limitp->next = cur_bp->limitp->prev = new_lp;
	cur_bp->markp = NULL;
	cur_bp->marko = 0;
	cur_bp->flags = 0;
	cur_bp->num_lines = 0;

	/*
	 * Free all the old lines.
	 */
	do {
		next_lp = old_lp->next;
		free_line(old_lp);
		old_lp = next_lp;
	} while (old_lp != cur_bp->limitp);

	/*
	 * Scan all the windows for finding the ones that point at the old
	 * lines.
	 */
	for (wp = head_wp; wp != NULL; wp = wp->next)
		if (wp->bp == cur_bp) {
			wp->topdelta = 0;
			wp->pointp = new_lp;
			wp->pointo = 0;
			wp->pointn = 0;
		}

	return TRUE;
}

/*
 * Print an error message into the echo area and return TRUE
 * if the current buffer is readonly; otherwise return FALSE.
 */
int warn_if_readonly_buffer(void)
{
	if (cur_bp->flags & BFLAG_READONLY) {
		minibuf_error("Buffer is readonly: @b%s@@", cur_bp->name);
		return TRUE;
	}

	return FALSE;
}

int warn_if_no_mark(void)
{
	if (cur_bp->markp == NULL) {
		minibuf_error("The region is not active now");
		return TRUE;
	}

	return FALSE;
}

/*
 * Calculate the region size and set the region structure.
 */
int calculate_region(regionp rp)
{
	linep lpforw, lpback;
	int forwsize = 0, forwlines = 0, backsize = 0, backlines = 0;

	if (cur_bp->markp == NULL)
		return FALSE;

	if (cur_wp->pointp == cur_bp->markp) {
		/*
		 * The point and the mark are on the same line.
		 */
		if (cur_wp->pointo < cur_bp->marko) {
			/*
			 * The point is before the mark.
			 */
			rp->startp = cur_wp->pointp;
			rp->startn = cur_wp->pointn;
			rp->starto = cur_wp->pointo;
			rp->endp = cur_bp->markp;
			rp->endn = cur_wp->pointn;
			rp->endo = cur_bp->marko;
			rp->size = cur_bp->marko - cur_wp->pointo;
			rp->num_lines = 0;
		} else {
			/*
			 * The mark is before the point.
			 */
			rp->startp = cur_bp->markp;
			rp->startn = cur_wp->pointn;
			rp->starto = cur_bp->marko;
			rp->endp = cur_wp->pointp;
			rp->endn = cur_wp->pointn;
			rp->endo = cur_wp->pointo;
			rp->size = cur_wp->pointo - cur_bp->marko;
			rp->num_lines = 0;
		}

		return TRUE;
	}

	/*
	 * The point and the mark are on different lines.
	 * Search forward and backward the point for the mark.
	 */
	lpforw = lpback = cur_wp->pointp;
	while (lpforw != cur_bp->markp && lpback != cur_bp->markp) {
		if (lpforw != cur_bp->limitp) {
			if (lpforw != cur_wp->pointp)
				forwsize += lpforw->size + 1;
			else
				forwsize += lpforw->size - cur_wp->pointo + 1;
			lpforw = lpforw->next;
			++forwlines;
		}
		if (lpback != cur_bp->limitp) {
			if (lpback != cur_wp->pointp)
				backsize += lpback->size + 1;
			else
				backsize += cur_wp->pointo + 1;
			lpback = lpback->prev;
			++backlines;
		}
	}

	if (lpforw == cur_bp->markp) {
		/*
		 * The point is before the mark.
		 */
		rp->startp = cur_wp->pointp;
		rp->startn = cur_wp->pointn;
		rp->starto = cur_wp->pointo;
		rp->endp = cur_bp->markp;
		rp->endn = cur_wp->pointn + forwlines;
		rp->endo = cur_bp->marko;
		rp->size = forwsize + cur_bp->marko;
		rp->num_lines = forwlines;
	} else {
		/*
		 * The mark is before the point.
		 */
		rp->startp = cur_bp->markp;
		rp->startn = cur_wp->pointn - backlines;
		rp->starto = cur_bp->marko;
		rp->endp = cur_wp->pointp;
		rp->endn = cur_wp->pointn;
		rp->endo = cur_wp->pointo;
		rp->size = backsize + cur_bp->markp->size - cur_bp->marko;
		rp->num_lines = backlines;
	}

	return TRUE;
}

/*
 * Set the specified buffer temporary flag and move the buffer
 * at the end of the buffer list.
 */
void set_temporary_buffer(bufferp bp)
{
	bufferp bp0;

	bp->flags |= BFLAG_TEMPORARY;

	if (bp == head_bp) {
		if (head_bp->next == NULL)
			return;
		head_bp = head_bp->next;
	} else if (bp->next == NULL)
		return;

	for (bp0 = head_bp; bp0 != NULL; bp0 = bp0->next)
		if (bp0->next == bp) {
			bp0->next = bp0->next->next;
			break;
		}

	for (bp0 = head_bp; bp0->next != NULL; bp0 = bp0->next)
		;

	bp0->next = bp;
	bp->next = NULL;
}
