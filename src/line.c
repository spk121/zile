/* Line-oriented editing functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
   Copyright (c) 2004 David A. Capello.
   All rights reserved.

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

/*	$Id: line.c,v 1.23 2004/02/17 23:20:33 rrt Exp $	*/

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"
#include "editfns.h"

static void adjust_markers_for_linep(Line *lp, Line *newlp);
static void adjust_markers_for_offset(Line *lp, int pointo, int offset);
static void adjust_markers_for_addline(Line *lp1, Line *lp2, int lp1len, int pt_insertion_type);
static void adjust_markers_for_delline(Line *lp1, Line *lp2, int lp1len);

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
Line *new_line(int maxsize)
{
	Line *lp;
	size_t size = sizeof *lp + maxsize - sizeof lp->text;

	lp = (Line *)zmalloc(size);
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
Line *resize_line(Line *lp, int maxsize)
{
	Line *newlp;

	newlp = (Line *)zrealloc(lp, sizeof *lp + maxsize - sizeof lp->text);
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
	 * Update all markers pointing to the modified line.
	 */
	adjust_markers_for_linep(lp, newlp);

	return newlp;
}

/*
 * Free the line contents.
 */
void free_line(Line *lp)
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
	if (cur_bp->pt.p->size + 1 >= cur_bp->pt.p->maxsize)
		resize_line(cur_bp->pt.p, cur_bp->pt.p->maxsize + 10);
	/*
	 * Move the line text one position forward after the
	 * point if required.
	 * This code assumes that memmove(d, s, 0) does nothing.
	 */
	memmove(cur_bp->pt.p->text + cur_bp->pt.o + 1,
		cur_bp->pt.p->text + cur_bp->pt.o,
		cur_bp->pt.p->size - cur_bp->pt.o);

	undo_save(UNDO_REMOVE_CHAR, cur_bp->pt, 0, 0);
	cur_bp->pt.p->text[cur_bp->pt.o] = c;

	++cur_bp->pt.p->size;

	cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
	if (cur_bp->flags & BFLAG_FONTLOCK)
		font_lock_reset_anchors(cur_bp, cur_bp->pt.p);
#endif

	return TRUE;
}

/*
 * Insert the character `c' at the current point position
 * into the current buffer.
 */
int insert_char(int c)
{
	if (warn_if_readonly_buffer())
		return FALSE;

	if (cur_bp->flags & BFLAG_OVERWRITE) {
		/* Current character isn't the end of line
		   && isn't a \t
		      || tab width isn't correct
		      || current char is a \t && we are in the tab limit.  */
		if ((cur_bp->pt.o < cur_bp->pt.p->size)
		    && ((cur_bp->pt.p->text[cur_bp->pt.o] != '\t')
			|| (cur_bp->tab_width < 1)
			|| ((cur_bp->pt.p->text[cur_bp->pt.o] == '\t')
			    && ((get_goalc() % cur_bp->tab_width)
				== cur_bp->tab_width-1)))) {
			/* Replace the charater.  */
			undo_save(UNDO_REPLACE_CHAR, cur_bp->pt,
				  cur_bp->pt.p->text[cur_bp->pt.o], 0);
			cur_bp->pt.p->text[cur_bp->pt.o] = c;
			++cur_bp->pt.o;

			cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
			if (cur_bp->flags & BFLAG_FONTLOCK)
				font_lock_reset_anchors(cur_bp, cur_bp->pt.p);
#endif

			return TRUE;
		}
		/*
		 * Fall through the "insertion" mode of a character
		 * at the end of the line, since it is totally
		 * equivalent to "overwrite" mode.
		 */
	}

	(void)intercalate_char(c);
	adjust_markers_for_offset(cur_bp->pt.p, cur_bp->pt.o, 1);

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

static void insert_expanded_tab(int (*inschr)(int chr))
{
	int c = get_goalc();
	int t = cur_bp->tab_width;

	for (c = t - c % t; c > 0; --c)
		(*inschr)(' ');
}

int insert_tab(void)
{
	if (warn_if_readonly_buffer())
		return FALSE;

	if (!lookup_bool_variable("expand-tabs"))
		insert_char_in_insert_mode('\t');
	else
		insert_expanded_tab(insert_char_in_insert_mode);

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

	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	for (uni = 0; uni < uniarg; ++uni)
		if (!insert_tab()) {
			ret = FALSE;
			break;
		}
	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

	return ret;
}

static int common_insert_newline(int move_pt)
{
	Line *lp1, *lp2;
	int lp1len, lp2len;

	if (warn_if_readonly_buffer())
		return FALSE;

	undo_save(UNDO_REMOVE_CHAR, cur_bp->pt, 0, 0);

	/*
	 * Calculate the two line lengths.
	 */
	lp1len = cur_bp->pt.o;
	lp2len = cur_bp->pt.p->size - lp1len;

	lp1 = cur_bp->pt.p;
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

	adjust_markers_for_addline(lp1, lp2, lp1len, move_pt);

	cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
	if (cur_bp->flags & BFLAG_FONTLOCK) {
		font_lock_reset_anchors(cur_bp, cur_bp->pt.p->prev);
		font_lock_reset_anchors(cur_bp, cur_bp->pt.p);
	}
#endif

	thisflag |= FLAG_NEED_RESYNC;

	return TRUE;
}

int insert_newline(void)
{
	return common_insert_newline(TRUE);
}

/* Insert a newline at the current position without moving the cursor.
 * Update all other cursors if they point on the splitted line.
 */
int intercalate_newline(void)
{
	return common_insert_newline(FALSE);
}

/* Replace a text string in the same case as the original */
/* XXX At the moment this is not the same as Emacs (doesn't check that
 * replacement string contains no upper case) but is consistent with
 * search. See TODO file */
static void replace_text_case(char *to, int tlen, char *from, int flen)
{
        int i;
        int overlap = min(tlen, flen);
        int tail = max(tlen, flen) - overlap;

        for (i = 0; i < overlap; i++) {
                if (isupper(*to))
                        *to++ = toupper(*from++);
                else
                        *to++ = *from++;
        }
        memcpy(to, from, tail);
}

/* This routine replaces text in the line "lp" with "newtext".  If
 * "replace_case" is TRUE it will replace only the case of characters
 * comparing with original text the "newtext".  */

void line_replace_text(Line **lp, int offset, int orgsize, char *newtext,
		       int replace_case)
{
	int modified = FALSE, newsize, newmaxsize, trailing_size;

	if (orgsize == 0)
		return;
	assert(orgsize >= 1);

	newsize = strlen(newtext);
	if (newsize != orgsize) { /* Reallocation of line may be required. */
		modified = TRUE;
		newmaxsize = (*lp)->size - orgsize + newsize;
		if (newmaxsize > (*lp)->maxsize)
			*lp = resize_line(*lp, newmaxsize + 1);
		trailing_size = (*lp)->size - (offset + orgsize);
		if (trailing_size > 0)
			memmove((*lp)->text + offset + newsize,
				(*lp)->text + offset + orgsize, trailing_size);

		if (!replace_case)
			memcpy((*lp)->text + offset, newtext, newsize);
		else
			replace_text_case((*lp)->text + offset, orgsize,
					  newtext, newsize);

		(*lp)->size = newmaxsize;

		adjust_markers_for_offset(*lp, offset, newsize-orgsize);
	} else { /* The new text size is equal to old text size. */
		if (memcmp((*lp)->text + offset, newtext, newsize) != 0) {
			if (!replace_case)
				memcpy((*lp)->text + offset, newtext, newsize);
			else
				replace_text_case((*lp)->text + offset,
						  orgsize, newtext, newsize);
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

	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	for (uni = 0; uni < uniarg; ++uni)
		if (!insert_newline()) {
			ret = FALSE;
			break;
		}
	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

	return ret;
}

void insert_string(const char *s)
{
	undo_save(UNDO_REMOVE_BLOCK, cur_bp->pt, strlen(s), 0);
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
	undo_save(UNDO_REMOVE_BLOCK, cur_bp->pt, size, 0);
	undo_nosave = TRUE;
	for (; 0 < size--; ++s)
		if (*s == '\n')
			insert_newline();
		else
			insert_char_in_insert_mode(*s);
	undo_nosave = FALSE;
}

/*
 * Split the current line so that the text width is less
 * than the value specified by `fill-column'.
 */
static void auto_fill_break_line()
{
	int break_col, last_col, excess = 0;

        /* Move cursor back to fill_column */
        while (get_goalc() > cur_bp->fill_column + 1) {
                cur_bp->pt.o--;
                excess++;
        }

        /* Find break point moving left from fill-column. */
        for (break_col = cur_bp->pt.o; break_col > 0; --break_col) {
                int c = cur_bp->pt.p->text[break_col - 1];
                if (isspace(c))
                        break;
        }

	if (break_col > 1) {
		/* Break line. */
		last_col = cur_bp->pt.o - break_col;
		cur_bp->pt.o = break_col;
                FUNCALL(delete_horizontal_space);
		insert_newline();
		cur_bp->pt.o = last_col + excess;
	}
}

int self_insert_command(int c)
{
	desactivate_mark();

	if (c == KBD_RET)
		insert_newline();
	else if (c == KBD_TAB)
		insert_tab();
	else if (c <= 255) {
		if (isspace(c) && cur_bp->flags & BFLAG_AUTOFILL &&
		    get_goalc() > cur_bp->fill_column)
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

	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	for (uni = 0; uni < uniarg; ++uni)
		if (!self_insert_command(c)) {
			ret = FALSE;
			break;
		}
	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

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
	desactivate_mark();

	if (!eolp()) {
		if (warn_if_readonly_buffer())
			return FALSE;

		undo_save(UNDO_INTERCALATE_CHAR, cur_bp->pt,
			  cur_bp->pt.p->text[cur_bp->pt.o], 0);

		/*
		 * Move the text one position backward after the point,
		 * if required.
		 * This code assumes that memmove(d, s, 0) does nothing.
		 */
		memmove(cur_bp->pt.p->text + cur_bp->pt.o,
			cur_bp->pt.p->text + cur_bp->pt.o + 1,
			cur_bp->pt.p->size - cur_bp->pt.o - 1);
		--cur_bp->pt.p->size;

		adjust_markers_for_offset(cur_bp->pt.p, cur_bp->pt.o, -1);

#if ENABLE_NONTEXT_MODES
		if (cur_bp->flags & BFLAG_FONTLOCK)
			font_lock_reset_anchors(cur_bp, cur_bp->pt.p);
#endif

		cur_bp->flags |= BFLAG_MODIFIED;

		return TRUE;
	} else if (!eobp()) {
		Line *lp1, *lp2;
		int lp1len, lp2len;

		if (warn_if_readonly_buffer())
			return FALSE;

		undo_save(UNDO_INTERCALATE_CHAR, cur_bp->pt, '\n', 0);

		lp1 = cur_bp->pt.p;
		lp2 = cur_bp->pt.p->next;
		lp1len = lp1->size;
		lp2len = lp2->size;

		/*
		 * Resize line if required.
		 */
		if (lp1len + lp2len + 1 >= lp1->maxsize)
			lp1 = resize_line(lp1, lp1len + lp2len + 1);

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

		adjust_markers_for_delline(lp1, lp2, lp1len);

		cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
		if (cur_bp->flags & BFLAG_FONTLOCK)
			font_lock_reset_anchors(cur_bp, cur_bp->pt.p);
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

	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	for (uni = 0; uni < uniarg; ++uni)
		if (!delete_char()) {
			ret = FALSE;
			break;
		}
	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

	return ret;
}

int backward_delete_char(void)
{
	desactivate_mark();

	if (backward_char()) {
		delete_char();
		return TRUE;
	}
	else {
		minibuf_error("Beginning of buffer");
		return FALSE;
	}
}

int backward_delete_char_overwrite(void)
{
	if (!bolp() && !eolp()) {
		if (warn_if_readonly_buffer())
			return FALSE;

		backward_char();
		if (following_char() == '\t') {
			/* In overwrite-mode.  */
			undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
			insert_expanded_tab(insert_char);
			undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
		}
		else {
			insert_char(' '); /* In overwrite-mode.  */
		}
		backward_char();

		cur_bp->flags |= BFLAG_MODIFIED;

#if ENABLE_NONTEXT_MODES
		if (cur_bp->flags & BFLAG_FONTLOCK)
			font_lock_reset_anchors(cur_bp, cur_bp->pt.p);
#endif

		return TRUE;
	} else {
		return backward_delete_char();
	}
}

DEFUN("backward-delete-char", backward_delete_char)
/*+
Delete the previous character.
Join lines if the character is a newline.
+*/
{
	 /* In overwrite-mode and isn't called by delete_char().  */
	int (*f)() = ((cur_bp->flags & BFLAG_OVERWRITE) &&
		      (last_uniarg > 0)) ?
		backward_delete_char_overwrite:
		backward_delete_char;
	int uni, ret = TRUE;

	if (uniarg < 0)
		return FUNCALL_ARG(delete_char, -uniarg);

	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	for (uni = 0; uni < uniarg; ++uni)
		if (!(*f)()) {
			ret = FALSE;
			break;
		}
	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

	return ret;
}

DEFUN("delete-horizontal-space", delete_horizontal_space)
/*+
Delete all spaces and tabs around point.
+*/
{
	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);

	while (!eolp() && isspace(following_char()))
		delete_char();

	while (!bolp() && isspace(preceding_char()))
		backward_delete_char();

	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
	return TRUE;
}

DEFUN("just-one-space", just_one_space)
/*+
Delete all spaces and tabs around point, leaving one space.
+*/
{
	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	FUNCALL(delete_horizontal_space);
	insert_char_in_insert_mode(' ');
	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
	return TRUE;
}

/***********************************************************************
			 Indentation command
 ***********************************************************************/

static int indent_relative(void)
{
	int target_goalc;
	int cur_goalc = get_goalc();
	Marker *old_point;

	if (warn_if_readonly_buffer())
		return FALSE;

	desactivate_mark();
	old_point = point_marker();

	/* Find previous non-blank line.  */
	do {
		if (!FUNCALL_ARG(forward_line, -1)) {
			cur_bp->pt = old_point->pt;
			free_marker(old_point);

			return insert_tab();
		}
	} while (is_blank_line());

	/* Go to `cur_goalc' in that non-blank line.  */
	while (!eolp() && (get_goalc() < cur_goalc))
		forward_char();

	/* Now find the first blank char.  */
	while (!eolp() && (!isspace(following_char())))
		forward_char();

	/* Find first non-blank char */
	while (!eolp() && (isspace(following_char())))
		forward_char();

	/* Target column.  */
	if (!eolp())
		target_goalc = get_goalc();
	else {
		cur_bp->pt = old_point->pt;
		free_marker(old_point);

		return insert_tab();
	}

	cur_bp->pt = old_point->pt;
	free_marker(old_point);

	/* Insert spaces.  */
	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	while (get_goalc() < target_goalc)
		insert_char_in_insert_mode(' ');

	/* Tabify.  */
	push_mark();
	set_mark();
	activate_mark();
	while (!bolp() && isspace(preceding_char()))
		backward_char();
	exchange_point_and_mark();
	FUNCALL(tabify);
	desactivate_mark();
	pop_mark();

	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

	return TRUE;
}

#if ENABLE_CLIKE_MODES
int c_indent_command(void)
{
#if 0				/* XXX finish it .-dacap */
	char *indent_string = get_variable("standard-indent");
	int c, ret, indent = !indent_string ? 4 : atoi(indent_string);

	if (indent < 0)
		indent = 0;

	if (indent) {
		save_excursion();

		while (!bolp()) {
			if (!isspace(preceding_char())) {
				indent = 0;
				break;
			}
			backward_char();
		}

		if (indent) {
			while (!bobp()) {
				if (!isspace(preceding_char())) {
					if (preceding_char() != '{')
						indent = 0;
					break;
				}
				backward_char();
			}
		}

		restore_excursion();
	}

	ret = indent_relative();
	if (ret) {
		for (c=0; c<indent; c++)
			insert_char_in_insert_mode(' ');
	}
#else
	return indent_relative();
#endif
}
#endif

DEFUN("indent-command", indent_command)
/*+
Indent line in proper way for current major mode or insert a tab.
+*/
{
	if (cur_bp->mode == BMODE_TEXT
#if ENABLE_MAIL_MODE
		|| cur_bp->mode == BMODE_MAIL
#endif
		) {
		return indent_relative();
	}
#if ENABLE_CLIKE_MODES
	else if (0
#if ENABLE_C_MODE
		 || cur_bp->mode == BMODE_C
#endif
#if ENABLE_CPP_MODE
		 || cur_bp->mode == BMODE_CPP
#endif
#if ENABLE_CSHARP_MODE
		 || cur_bp->mode == BMODE_CSHARP
#endif
#if ENABLE_JAVA_MODE
		 || cur_bp->mode == BMODE_JAVA
#endif
		) {
		return c_indent_command();
	}
#endif
	else {
		return insert_tab();
	}
}

DEFUN("newline-and-indent", newline_and_indent)
/*+
Insert a newline, then indent according to major mode.
Indentation is done using the `indent-command' function.
+*/
{
	int ret;

	if (warn_if_readonly_buffer())
		return FALSE;

	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	ret = insert_newline();
	if (ret)
		FUNCALL(indent_command);
	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
	return ret;
}

/***********************************************************************
		       Adjust markers routines
 ***********************************************************************/

#define CMP_MARKER_OFFSET(marker, offset, extra)			\
	(((!(marker)->type) && (marker)->pt.o > (offset))		\
	 || ((marker)->type && (marker)->pt.o >= (offset)+(extra)))

static void adjust_markers_for_linep(Line *lp, Line *newlp)
{
	Marker *pt, *marker;
	pt = point_marker();
	set_marker_insertion_type(pt, TRUE);

	/* Update the markers.  */
	for (marker=cur_bp->markers; marker; marker=marker->next) {
		if (marker->pt.p == lp)
			marker->pt.p = newlp;
	}

	cur_bp->pt = pt->pt;
	free_marker(pt);
}

static void adjust_markers_for_offset(Line *lp, int pointo, int offset)
{
	Marker *pt, *marker;
	pt = point_marker();
	set_marker_insertion_type(pt, TRUE);

	/* Update the markers.  */
	for (marker=cur_bp->markers; marker; marker=marker->next) {
		if (marker->pt.p == lp &&
		    CMP_MARKER_OFFSET(marker, pointo, ((offset < 0) ? 1: 0)))
			marker->pt.o += offset;
	}

	cur_bp->pt = pt->pt;
	free_marker(pt);
}

static void adjust_markers_for_addline(Line *lp1, Line *lp2, int lp1len, int pt_insertion_type)
{
	Marker *pt, *marker;
	pt = point_marker();
	set_marker_insertion_type(pt, pt_insertion_type);

	/* Update the markers.  */
	for (marker=cur_bp->markers; marker; marker=marker->next) {
		if (marker->pt.p == lp1 &&
		    CMP_MARKER_OFFSET(marker, lp1len, 0)) {
			marker->pt.p = lp2;
			marker->pt.o -= lp1len;
			marker->pt.n++;
		}
		else if (marker->pt.n > cur_bp->pt.n)
			marker->pt.n++;
	}

	cur_bp->pt = pt->pt;
	free_marker(pt);
}

static void adjust_markers_for_delline(Line *lp1, Line *lp2, int lp1len)
{
	Marker *pt, *marker;
	pt = point_marker();
	set_marker_insertion_type(pt, TRUE);

	/* Update the markers.  */
	for (marker=cur_bp->markers; marker; marker=marker->next) {
		if (marker->pt.p == lp2) {
			marker->pt.p = lp1;
			marker->pt.o += lp1len;
			marker->pt.n--;
		}
		else if (marker->pt.n > cur_bp->pt.n)
			marker->pt.n--;
	}

	cur_bp->pt = pt->pt;
	free_marker(pt);
}
