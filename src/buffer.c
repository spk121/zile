/*	$Id: buffer.c,v 1.7 2004/02/08 04:39:26 dacap Exp $	*/

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

/*
 * Allocate a new buffer structure and set the default local
 * variable values.
 * The allocation of the first empty line is done here to simplify
 * a lot the code.
 */
static Buffer *new_buffer(void)
{
	Buffer *bp;
	char *s;

	bp = (Buffer *)zmalloc(sizeof(Buffer));
	memset(bp, 0, sizeof(Buffer));

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
	bp->pt.p = new_line(1);
	bp->pt.n = 0;
	bp->pt.o = 0;

	/* Allocate a null line for the limit marker. */
	bp->limitp = new_line(0);

	bp->limitp->prev = bp->limitp->next = bp->pt.p;
	bp->pt.p->prev = bp->pt.p->next = bp->limitp;

	/* Markers.  */
	bp->mark = bp->markers = NULL;

	return bp;
}

/*
 * Free the buffer allocated memory.
 */
void free_buffer(Buffer *bp)
{
	Line *lp, *next_lp;
	Undo *up, *next_up;

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
	 * Free markers.
	 */
	while (bp->markers)
		free_marker(bp->markers);

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
	Buffer *bp, *next;

	for (bp = head_bp; bp != NULL; bp = next) {
		next = bp->next;
		free_buffer(bp);
	}
}

/*
 * Allocate a new buffer and insert it into the buffer list.
 */
Buffer *create_buffer(const char *name)
{
	Buffer *bp;

	bp = new_buffer();
	set_buffer_name(bp, name);

	bp->next = head_bp;
	head_bp = bp;

	return bp;
}

/*
 * Set a new name for the buffer.
 */
void set_buffer_name(Buffer *bp, const char *name)
{
	if (bp->name != NULL)
		free(bp->name);
	bp->name = zstrdup(name);
}

/*
 * Set a new filename (and a name) for the buffer.
 */
void set_buffer_filename(Buffer *bp, const char *filename)
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
Buffer *find_buffer(const char *name, int cflag)
{
	Buffer *bp;

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
Buffer *get_next_buffer(void)
{
	if (cur_bp->next != NULL)
		return cur_bp->next;
	return head_bp;
}

/*
 * Create a buffer name using the file name.
 */
char *make_buffer_name(const char *filename)
{
	const char *p;
        char *name;
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

/* Move the selected buffer to head.  */

static void move_buffer_to_head(Buffer *bp)
{
        Buffer *it, *prev = NULL;

        for (it = head_bp; it; it = it->next) {
                if (bp == it) {
                        if (prev) {
                                prev->next = bp->next;
                                bp->next = head_bp;
                                head_bp = bp;
                        }
                        break;
                }
                prev = it;
        }
}

/*
 * Switch to the specified buffer.
 */
void switch_to_buffer(Buffer *bp)
{
	assert(cur_wp->bp == cur_bp);

	/* The buffer is the current buffer; return safely.  */
	if (cur_bp == bp)
		return;

	/* Set current buffer.  */
	cur_wp->bp = cur_bp = bp;

	/* Move the buffer to head.  */
	move_buffer_to_head(bp);

	thisflag |= FLAG_NEED_RESYNC;
}

/*
 * Remove the buffer contents and reset the mark and the flags.
 */
int zap_buffer_content(void)
{
	Window *wp;
	Line *new_lp, *old_lp, *next_lp;

	new_lp = new_line(1);
	new_lp->next = new_lp->prev = cur_bp->limitp;

	old_lp = cur_bp->limitp->next;
	cur_bp->limitp->next = cur_bp->limitp->prev = new_lp;
	cur_bp->pt.p = new_lp;
	cur_bp->pt.n = 0;
	cur_bp->pt.o = 0;
	cur_bp->flags = 0;
	cur_bp->num_lines = 0;

	/* 
	 * Free markers (remember that are windows pointing to some of
	 * this markers).
	 */
	while (cur_bp->markers)
		free_marker(cur_bp->markers);

	cur_bp->mark = cur_bp->markers = NULL;

	/*
	 * Free all the old lines.
	 */
	do {
		next_lp = old_lp->next;
		free_line(old_lp);
		old_lp = next_lp;
	} while (old_lp != cur_bp->limitp);

	/*
	 * Scan all the windows that have markers to this buffers.
	 */
	for (wp = head_wp; wp != NULL; wp = wp->next)
		if (wp->bp == cur_bp) {
			wp->topdelta = 0;
			wp->saved_pt = NULL; /* It was freed.  */
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
	if (!cur_bp->mark) {
		minibuf_error("The mark is not set now");
		return TRUE;
	}
	else if (transient_mark_mode() && !cur_bp->mark_active) {
		minibuf_error("The mark is not active now");
		return TRUE;
	}
	else
		return FALSE;
}

/*
 * Calculate the region size and set the region structure.
 */
int calculate_region(Region *rp)
{
	if (!is_mark_actived())
		return FALSE;

	/* The point is before the mark. */
	if (cmp_point(cur_bp->pt, cur_bp->mark->pt) < 0) {
		rp->start = cur_bp->pt;
		rp->end = cur_bp->mark->pt;
	}
	/* The mark is before the point. */
	else {
		rp->start = cur_bp->mark->pt;
		rp->end = cur_bp->pt;
	}

	rp->size = point_dist(rp->start, rp->end);
	rp->num_lines = count_lines(rp->start, rp->end);
	return TRUE;
}

/*
 * Set the specified buffer temporary flag and move the buffer
 * at the end of the buffer list.
 */
void set_temporary_buffer(Buffer *bp)
{
	Buffer *bp0;

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

int calculate_buffer_size(Buffer *bp)
{
	Line *lp = bp->limitp->next;
	int size = 0;

	if (lp == bp->limitp)
		return 0;

	for (;;) {
		size += lp->size;
		lp = lp->next;
		if (lp == bp->limitp)
			break;
		++size;
	}

	return size;
}

int transient_mark_mode(void)
{
	return lookup_bool_variable("transient-mark-mode");
}

void activate_mark(void)
{
	cur_bp->mark_active = TRUE;
}

void desactivate_mark(void)
{
	cur_bp->mark_active = FALSE;
}

int is_mark_actived(void)
{
	if (!cur_bp->mark)
		return FALSE;
	else if (transient_mark_mode())
		return (cur_bp->mark_active) ? TRUE: FALSE;
	else
		return TRUE;
}
