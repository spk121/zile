/*	$Id: line.c,v 1.14 2004/01/29 03:50:50 dacap Exp $	*/

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
 * All the common line-oriented editing functions are declared in this file.
 */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

/*
 * This function creates a new line structure for holding a line
 * of buffer text.  If called with `maxsize == 0', it does not allocate
 * a text space at all (this trick is used for allocating the
 * buffer limit marker `bp->limitp').
 *
 * The '\n' is not stored in the text buffer since it is implicit.
 *
 * The tail of the line structure is used for storing the text buffer.
 */
linep new_line(int maxsize)
{
	linep lp;
	size_t size = sizeof *lp + maxsize - sizeof lp->text;

	lp = (linep)zmalloc(size);
	memset(lp, 0, size);

	lp->size = 0;
	lp->maxsize = maxsize;

	return lp;
}

/*
 * This function resizes the text buffer of the line by reallocating
 * the whole line structure.  If the pointer returned by realloc()
 * is different from the original one, scan all the windows looking
 * for the references to the old pointer and update their value.
 */
linep resize_line(windowp wp, linep lp, int maxsize)
{
	linep newlp;
	windowp wp1;

	newlp = (linep)zrealloc(lp, sizeof *lp + maxsize - sizeof lp->text);
	newlp->maxsize = maxsize;

	/*
	 * Little optimization: if the reallocated pointer
	 * is equal to the original, perform no other operations.
	 */
	if (newlp == lp)
		return newlp;

	newlp->prev->next = newlp;
	newlp->next->prev = newlp;

	/*
	 * Scan all the windows searching for points and marks
	 * pointing at the reallocated line.
	 */
	for (wp1 = head_wp; wp1 != NULL; wp1 = wp1->next)
		if (wp1->pointp == lp)
			wp1->pointp = newlp;

	if (wp->bp->markp == lp)
		wp->bp->markp = newlp;

	return newlp;
}

/*
 * Free the line contents.
 */
void free_line(linep lp)
{
	if (lp->anchors != NULL)
		free(lp->anchors);
	free(lp);
}

/* Insert the character at the current position and move the text at its right
 * whatever the insert/overwrite mode is.
 * This function doesn't change the current position of the pointer.
 */
int intercalate_char(int c)
{
	if (warn_if_readonly_buffer())
		return FALSE;

	/*
	 * Resize the line if required.
	 */
	if (cur_wp->pointp->size + 1 >= cur_wp->pointp->maxsize)
		resize_line(cur_wp, cur_wp->pointp,
			    cur_wp->pointp->maxsize + 10);
	/*
	 * Move the line text one position forward after the
	 * point if required.
	 * This code assumes that memmove(d, s, 0) does nothing.
	 */
	memmove(cur_wp->pointp->text + cur_wp->pointo + 1,
		cur_wp->pointp->text + cur_wp->pointo,
		cur_wp->pointp->size - cur_wp->pointo);

	undo_save(UNDO_REMOVE_CHAR, cur_wp->pointn, cur_wp->pointo, 0, 0);
	cur_wp->pointp->text[cur_wp->pointo] = c;

	++cur_wp->pointp->size;

	cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
	if (cur_bp->flags & BFLAG_FONTLOCK)
		font_lock_reset_anchors(cur_bp, cur_wp->pointp);
#endif

	return TRUE;
}

/*
 * Insert the character `c' at the current point position
 * into the current buffer.
 */
int insert_char(int c)
{
	windowp wp;
	int pointo;

	if (warn_if_readonly_buffer())
		return FALSE;

	if (cur_bp->flags & BFLAG_OVERWRITE) {
		/* Current character isn't the end of line
		   && isn't a \t
		      || tab width isn't correct
		      || current char is a \t && we are in the tab limit.  */
		if ((cur_wp->pointo < cur_wp->pointp->size)
		    && ((cur_wp->pointp->text[cur_wp->pointo] != '\t')
			|| (cur_bp->tab_width < 1)
			|| ((cur_wp->pointp->text[cur_wp->pointo] == '\t')
			    && ((get_text_goalc (cur_wp) % cur_bp->tab_width)
				== cur_bp->tab_width-1)))) {
			/* Replace the charater.  */
			undo_save(UNDO_REPLACE_CHAR,
				  cur_wp->pointn, cur_wp->pointo,
				  cur_wp->pointp->text[cur_wp->pointo], 0);
			cur_wp->pointp->text[cur_wp->pointo] = c;
			++cur_wp->pointo;

			cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
			if (cur_bp->flags & BFLAG_FONTLOCK)
				font_lock_reset_anchors(cur_bp, cur_wp->pointp);
#endif

			return TRUE;
		}
		/*
		 * Fall through the "insertion" mode of a character
		 * at the end of the line, since it is totally
		 * equivalent also in "overwrite" mode.
		 */
	}

	(void)intercalate_char(c);

	pointo = cur_wp->pointo;

	/*
	 * Scan all the windows searching for points
	 * pointing at the modified line.
	 */
	for (wp = head_wp; wp != NULL; wp = wp->next)
		if (wp->pointp == cur_wp->pointp && wp->pointo >= pointo)
			++wp->pointo;

	if (cur_bp->markp == cur_wp->pointp && cur_bp->marko >= pointo)
		++cur_bp->marko;

	return TRUE;
}

/* Insert a character at the current position in the insert mode
 * whetever the current insert mode is.
 */
int insert_char_in_insert_mode(int c)
{
	int overwrite_mode_save, ret_value;

	/* save current mode */
	overwrite_mode_save = cur_bp->flags & BFLAG_OVERWRITE;

	/* force insert mode */
	cur_bp->flags &= ~BFLAG_OVERWRITE;

	ret_value = insert_char(c);

	/* restore previous mode */
	cur_bp->flags |= overwrite_mode_save;

	return ret_value;
}

static void insert_expanded_tab(void)
{
	int col = 0, t = cur_bp->tab_width;
	char *sp = cur_wp->pointp->text, *p = sp;

	while (p < sp + cur_wp->pointo) {
		if (*p == '\t')
			col |= t - 1;
		++col, ++p;
	}

	for (col = t - col % t; col > 0; --col)
		insert_char(' ');
}

int insert_tab(void)
{
	if (warn_if_readonly_buffer())
		return FALSE;

	if (!lookup_bool_variable("expand-tabs"))
		insert_char('\t');
	else
		insert_expanded_tab();

	return TRUE;
}

DEFUN("tab-to-tab-stop", tab_to_tab_stop)
/*+
Insert a tabulation at the current point position into
the current buffer.  Convert the tabulation into spaces
if the `expand-tabs' variable is bound and set to true.
+*/
{
	int uni, ret = TRUE;

	undo_save(UNDO_START_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);
	for (uni = 0; uni < uniarg; ++uni)
		if (!insert_tab()) {
			ret = FALSE;
			break;
		}
	undo_save(UNDO_END_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);

	return ret;
}

static int common_insert_newline(int undo_mode)
{
	linep lp1, lp2;
	int lp1len, lp2len, cur_pointn;
	windowp wp;

	if (warn_if_readonly_buffer())
		return FALSE;

	undo_save(UNDO_REMOVE_CHAR, cur_wp->pointn, cur_wp->pointo, 0, 0);

	/*
	 * Calculate the two line lengths.
	 */
	lp1len = cur_wp->pointo;
	lp2len = cur_wp->pointp->size - lp1len;

	lp1 = cur_wp->pointp;
	lp2 = new_line(lp2len ? lp2len : 1);

	/*
	 * Copy the text after the point into the new line.
	 * This code assumes that memcpy(d, s, 0) does nothing.
	 */
	memcpy(lp2->text, lp1->text + lp1len, lp2len);

	lp1->size -= lp2len;
	lp2->size = lp2len;

	/*
	 * Update line linked list.
	 */
	lp2->next = lp1->next;
	lp2->next->prev = lp2;
	lp2->prev = lp1;
	lp1->next = lp2;
	++cur_bp->num_lines;

	/*
	 * Scan all the windows searching for points
	 * pointing at the changed line.
	 */
	cur_pointn = cur_wp->pointn;
	for (wp = head_wp; wp != NULL; wp = wp->next) {
		if (wp->bp != cur_bp)
			continue;
		if (wp->pointp == lp1 &&
		    ((undo_mode && wp->pointo > lp1len)
		     || (!undo_mode && wp->pointo >= lp1len))) {
			wp->pointp = lp2;
			wp->pointo -= lp1len;
			++wp->pointn;
		} else if (wp->pointn > cur_pointn)
			++wp->pointn;
	}

	if (cur_bp->markp == lp1 && cur_bp->marko >= lp1len) {
		cur_bp->markp = lp2;
		cur_bp->marko -= lp1len;
	}

	cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
	if (cur_bp->flags & BFLAG_FONTLOCK) {
		font_lock_reset_anchors(cur_bp, cur_wp->pointp->prev);
		font_lock_reset_anchors(cur_bp, cur_wp->pointp);
	}
#endif

	thisflag |= FLAG_NEED_RESYNC;

	return TRUE;
}

int insert_newline(void)
{
	return common_insert_newline(0);
}

/* Insert a newline at the current position without moving the cursor.
 * Update all other cursors if they point on the splitted line.
 */
int intercalate_newline(void)
{
	return common_insert_newline(1);
}

void line_replace_text(linep *lp, int offset, int orgsize, char *newtext)
{
	int modified = FALSE, newsize, newmaxsize, trailing_size;
	windowp wp;

	if (orgsize == 0)
		return;
	assert(orgsize >= 1);

	newsize = strlen(newtext);
	if (newsize != orgsize) { /* Reallocation of line may be required. */
		modified = TRUE;
		newmaxsize = (*lp)->size - orgsize + newsize;
		if (newmaxsize > (*lp)->maxsize)
			*lp = resize_line(cur_wp, *lp, newmaxsize + 1);
		trailing_size = (*lp)->size - (offset + orgsize);
		if (trailing_size > 0)
			memmove((*lp)->text + offset + newsize,
				(*lp)->text + offset + orgsize, trailing_size);
		memcpy((*lp)->text + offset, newtext, newsize);
		(*lp)->size = newmaxsize;

		/*
		 * Scan all the windows searching for points
		 * pointing at the modified line.
		 */
		for (wp = head_wp; wp != NULL; wp = wp->next)
			if (wp->pointp == *lp && wp->pointo > offset)
				wp->pointo += newsize - orgsize;
	} else { /* The new text size is equal to old text size. */
		if (memcmp((*lp)->text + offset, newtext, newsize) != 0) {
			memcpy((*lp)->text + offset, newtext, newsize);
			modified = TRUE;
		}
	}

	if (modified) {
		cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
		if (cur_bp->flags & BFLAG_FONTLOCK)
			font_lock_reset_anchors(cur_bp, *lp);
#endif
	}
}

DEFUN("newline", newline)
/*+
Insert a newline at the current point position into
the current buffer.
+*/
{
	int uni, ret = TRUE;

	undo_save(UNDO_START_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);
	for (uni = 0; uni < uniarg; ++uni)
		if (!insert_newline()) {
			ret = FALSE;
			break;
		}
	undo_save(UNDO_END_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);

	return ret;
}

void insert_string(const char *s)
{
	undo_save(UNDO_REMOVE_BLOCK, cur_wp->pointn, cur_wp->pointo, strlen(s), 0);
	undo_nosave = TRUE;
	for (; *s != '\0'; ++s)
		if (*s == '\n')
			insert_newline();
		else
			insert_char(*s);
	undo_nosave = FALSE;
}

void insert_nstring(const char *s, size_t size)
{
	undo_save(UNDO_REMOVE_BLOCK, cur_wp->pointn, cur_wp->pointo, size, 0);
	undo_nosave = TRUE;
	for (; 0 < size--; ++s)
		if (*s == '\n')
			insert_newline();
		else
			insert_char(*s);
	undo_nosave = FALSE;
}

/*
 * Split the current line so that the text width is less
 * than the value specified by `fill-column'.
 */
static void auto_fill_break_line()
{
	int break_col, last_col;
	linep lp;

	/* Find break point starting from fill column, or end of line,
         * whichever is smaller (if there are tabs, the end of line
         * may be smaller than the fill column). */
	for (break_col = min(cur_wp->bp->fill_column + 1, cur_wp->pointp->size);
             break_col > 0; --break_col) {
		int c = cur_wp->pointp->text[break_col - 1];
		if (isspace(c))
			break;
	}

	if (break_col < 2) {
		/* Find break point starting from current point. */
		for (break_col = cur_wp->pointo; break_col > 0; --break_col) {
			int c = cur_wp->pointp->text[break_col - 1];
			if (isspace(c))
				break;
		}
	}

	if (break_col > 1) {
		/* Break line. */
		last_col = cur_wp->pointo - break_col;
		cur_wp->pointo = break_col;
		insert_newline();
		cur_wp->pointo = last_col;

		/* Remove trailing whitespace in broken line. */
		lp = cur_wp->pointp->prev;
		while (lp->size > 0 && isspace(lp->text[lp->size - 1]))
			--lp->size;
	}
}

int self_insert_command(int c)
{
	if (c == KBD_RET)
		insert_newline();
	else if (c == KBD_TAB)
		insert_tab();
	else if (c <= 255) {
		if (isspace(c) && cur_wp->bp->flags & BFLAG_AUTOFILL &&
		    get_text_goalc(cur_wp) > cur_wp->bp->fill_column)
			auto_fill_break_line();
		insert_char(c);
	} else {
		ding();
		return FALSE;
	}

	return TRUE;
}

DEFUN("self-insert-command", self_insert_command)
/*+
Insert the character you type.
+*/
{
	int uni, c, ret = TRUE;

	c = cur_tp->getkey();

	undo_save(UNDO_START_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);
	for (uni = 0; uni < uniarg; ++uni)
		if (!self_insert_command(c)) {
			ret = FALSE;
			break;
		}
	undo_save(UNDO_END_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);

	return ret;
}

void bprintf(const char *fmt, ...)
{
	va_list ap;
	char *buf;
	va_start(ap, fmt);
	vasprintf(&buf, fmt, ap);
	va_end(ap);
	insert_string(buf);
	free(buf);
}

int delete_char(void)
{
	windowp wp;

	if (cur_wp->pointo < cur_wp->pointp->size) {
		if (warn_if_readonly_buffer())
			return FALSE;

		undo_save(UNDO_INTERCALATE_CHAR, cur_wp->pointn, cur_wp->pointo,
			  cur_wp->pointp->text[cur_wp->pointo], 0);

		/*
		 * Move the text one position backward after the point,
		 * if required.
		 * This code assumes that memcpy(d, s, 0) does nothing.
		 */
		memcpy(cur_wp->pointp->text + cur_wp->pointo,
		       cur_wp->pointp->text + cur_wp->pointo + 1,
		       cur_wp->pointp->size - cur_wp->pointo - 1);
		--cur_wp->pointp->size;

		/*
		 * Scan all the windows searching for points
		 * pointing at the modified line.
		 */
		for (wp = head_wp; wp != NULL; wp = wp->next)
			if (wp->pointp == cur_wp->pointp
			    && wp->pointo > cur_wp->pointo)
				--wp->pointo;

		if (cur_bp->markp == cur_wp->pointp
		    && cur_bp->marko > cur_wp->pointo)
			--cur_bp->marko;

#if ENABLE_NONTEXT_MODES
		if (cur_bp->flags & BFLAG_FONTLOCK)
			font_lock_reset_anchors(cur_bp, cur_wp->pointp);
#endif

		cur_bp->flags |= BFLAG_MODIFIED;

		return TRUE;
	} else if (cur_wp->pointp->next != cur_bp->limitp) {
		linep lp1, lp2;
		int lp1len, lp2len;

		if (warn_if_readonly_buffer())
			return FALSE;

		undo_save(UNDO_INTERCALATE_CHAR, cur_wp->pointn, cur_wp->pointo,
			  '\n', 0);

		lp1 = cur_wp->pointp;
		lp2 = cur_wp->pointp->next;
		lp1len = lp1->size;
		lp2len = lp2->size;

		/*
		 * Resize line if required.
		 */
		if (lp1len + lp2len + 1 >= lp1->maxsize)
			lp1 = resize_line(cur_wp, lp1, lp1len + lp2len + 1);

		if (lp2len > 0) {
			/*
			 * Move the next line text into the current line.
			 */
			memcpy(lp1->text + lp1len, lp2->text, lp2len);
			lp1->size += lp2len;
		}

		/*
		 * Update line linked list.
		 */
		lp1->next = lp2->next;
		lp1->next->prev = lp1;
		--cur_bp->num_lines;

		/*
		 * Free the unlinked line.
		 */
		free_line(lp2);

		/*
		 * Scan all the windows searching for points
		 * pointing at the deallocated line.
		 */
		for (wp = head_wp; wp != NULL; wp = wp->next) {
			if (wp->bp != cur_bp)
				continue;
			if (wp->pointp == lp2) {
				wp->pointp = lp1;
				wp->pointo += lp1len;
				--wp->pointn;
			} else if (wp->pointn > cur_wp->pointn)
				--wp->pointn;
		}

		if (cur_bp->markp == lp2) {
			cur_bp->markp = lp1;
			cur_bp->marko += lp1len;
		}

		cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
		if (cur_bp->flags & BFLAG_FONTLOCK)
			font_lock_reset_anchors(cur_bp, cur_wp->pointp);
#endif

		thisflag |= FLAG_NEED_RESYNC;

		return TRUE;
	}

	minibuf_error("End of buffer");

	return FALSE;
}

DEFUN("delete-char", delete_char)
/*+
Delete the following character.
Join lines if the character is a newline.
+*/
{
	int uni, ret = TRUE;

	if (uniarg < 0)
		return FUNCALL_ARG(backward_delete_char, -uniarg);

	undo_save(UNDO_START_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);
	for (uni = 0; uni < uniarg; ++uni)
		if (!delete_char()) {
			ret = FALSE;
			break;
		}
	undo_save(UNDO_END_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);

	return ret;
}

int backward_delete_char(void)
{
	windowp wp;

	if (cur_wp->pointo > 0) {
		if (warn_if_readonly_buffer())
			return FALSE;

		undo_save(UNDO_INSERT_CHAR, cur_wp->pointn, cur_wp->pointo-1,
			  cur_wp->pointp->text[cur_wp->pointo - 1], 0);

		/*
		 * Move the text one position backward before the point,
		 * if required.
		 * This code assumes that memcpy(d, s, 0) does nothing.
		 */
		memcpy(cur_wp->pointp->text + cur_wp->pointo - 1,
		       cur_wp->pointp->text + cur_wp->pointo,
		       cur_wp->pointp->size - cur_wp->pointo);
		--cur_wp->pointp->size;

		/*
		 * Scan all the windows searching for points
		 * pointing at the modified line.
		 */
		for (wp = head_wp; wp != NULL; wp = wp->next)
			if (wp->pointp == cur_wp->pointp
			    && wp->pointo >= cur_wp->pointo)
				--wp->pointo;

		if (cur_bp->markp == cur_wp->pointp
		    && cur_bp->marko >= cur_wp->pointo)
			--cur_bp->marko;

		cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
		if (cur_bp->flags & BFLAG_FONTLOCK)
			font_lock_reset_anchors(cur_bp, cur_wp->pointp);
#endif

		return TRUE;
	} else if (cur_wp->pointp->prev != cur_bp->limitp) {
		linep lp1, lp2;
		int lp1len, lp2len;

		if (warn_if_readonly_buffer())
			return FALSE;

		undo_save(UNDO_INSERT_CHAR, cur_wp->pointn - 1,
			  cur_wp->pointp->prev->size, '\n', 0);

		lp1 = cur_wp->pointp->prev;
		lp2 = cur_wp->pointp;
		lp1len = lp1->size;
		lp2len = lp2->size;

		/*
		 * Resize line if required.
		 */
		if (lp1len + lp2len + 1 >= lp1->maxsize)
			lp1 = resize_line(cur_wp, lp1, lp1len + lp2len + 1);

		if (lp2len > 0) {
			/*
			 * Move the current line text into the previous line.
			 */
			memcpy(lp1->text + lp1len, lp2->text, lp2len);
			lp1->size += lp2len;
		}

		/*
		 * Update line linked list.
		 */
		lp1->next = lp2->next;
		lp1->next->prev = lp1;
		--cur_bp->num_lines;
		assert(cur_bp->limitp->next != lp2);
		assert(cur_bp->limitp->prev != lp2);

		/*
		 * Free the unlinked line.
		 */
		free_line(lp2);

		cur_wp->pointp = lp1;
		cur_wp->pointo += lp1len;
		--cur_wp->pointn;

		/*
		 * Scan all the windows searching for points
		 * pointing at the deallocated line.
		 */
		for (wp = head_wp; wp != NULL; wp = wp->next) {
			if (wp->bp != cur_bp)
				continue;
			if (wp->pointp == lp2) {
				wp->pointp = lp1;
				wp->pointo += lp1len;
				--wp->pointn;
			} else if (wp->pointn > cur_wp->pointn)
				--wp->pointn;
		}

		if (cur_bp->markp == lp2) {
			cur_bp->markp = lp1;
			cur_bp->marko += lp1len;
		}

		cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
		if (cur_bp->flags & BFLAG_FONTLOCK)
			font_lock_reset_anchors(cur_bp, cur_wp->pointp);
#endif

		thisflag |= FLAG_NEED_RESYNC;

		return TRUE;
	}

	minibuf_error("Beginning of buffer");

	return FALSE;
}

int backward_delete_char_overwrite(void)
{
	windowp wp;

	if ((cur_wp->pointo > 0) && (cur_wp->pointo < cur_wp->pointp->size)) {
		if (warn_if_readonly_buffer())
			return FALSE;

		cur_wp->pointo--;
		if (cur_wp->pointp->text[cur_wp->pointo] == '\t') {
			int c;

			undo_save(UNDO_START_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);
			for (c=0; c<cur_bp->tab_width; c++)
				insert_char (' ');
			undo_save(UNDO_END_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);
		}
		else {
			insert_char (' ');
		}
		cur_wp->pointo--;

		cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
		if (cur_bp->flags & BFLAG_FONTLOCK)
			font_lock_reset_anchors(cur_bp, cur_wp->pointp);
#endif

		return TRUE;
	} else {
		return backward_delete_char ();
	}
}

DEFUN("backward-delete-char", backward_delete_char)
/*+
Delete the previous character.
Join lines if the character is a newline.
+*/
{
	 /* In overwrite-mode and isn't called by delete_char().  */
	int (*f) () = (cur_bp->flags & BFLAG_OVERWRITE &&
		       (last_uniarg > 0)) ?
		backward_delete_char_overwrite:
		backward_delete_char;
	int uni, ret = TRUE;

	if (uniarg < 0)
		return FUNCALL_ARG(delete_char, -uniarg);

	undo_save(UNDO_START_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);
	for (uni = 0; uni < uniarg; ++uni)
		if (!(*f) ()) {
			ret = FALSE;
			break;
		}
	undo_save(UNDO_END_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);

	return ret;
}

DEFUN ("delete-horizontal-space", delete_horizontal_space)
{
	undo_save (UNDO_START_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);

	while ((cur_wp->pointo < cur_wp->pointp->size)
	       && isspace (cur_wp->pointp->text[cur_wp->pointo]))
		delete_char ();

	while ((cur_wp->pointo > 0) &&
	       isspace (cur_wp->pointp->text[cur_wp->pointo-1]))
		backward_delete_char ();

	undo_save (UNDO_END_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);
	return TRUE;
}

DEFUN ("just-one-space", just_one_space)
{
	undo_save (UNDO_START_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);
	FUNCALL (delete_horizontal_space);
	insert_char_in_insert_mode (' ');
	undo_save (UNDO_END_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0);
	return TRUE;
}
