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

/*	$Id: line.c,v 1.38 2005/01/03 00:45:27 rrt Exp $	*/

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

static void adjust_markers_for_offset(Line *lp, int pointo, int offset);
static void adjust_markers_for_addline(Line *lp1, Line *lp2, int lp1len, int pt_insertion_type);
static void adjust_markers_for_delline(Line *lp1, Line *lp2, int lp1len);

/*
 * This function creates a new line structure for holding a line
 * of buffer text.
 *
 * The '\n' is not stored in the text buffer since it is implicit.
 */
Line *new_line(void)
{
	Line *lp = (Line *)zmalloc(sizeof(Line));

	lp->text = astr_new();

	return lp;
}

/*
 * Free the line contents.
 */
void free_line(Line *lp)
{
        astr_delete(lp->text);
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

	undo_save(UNDO_REMOVE_CHAR, cur_bp->pt, 0, 0);

        astr_insert_char(cur_bp->pt.p->text, cur_bp->pt.o, c);

	cur_bp->flags |= BFLAG_MODIFIED;

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
		if ((cur_bp->pt.o < astr_len(cur_bp->pt.p->text))
		    && ((*astr_char(cur_bp->pt.p->text, cur_bp->pt.o) != '\t')
			|| (cur_bp->tab_width < 1)
			|| ((*astr_char(cur_bp->pt.p->text, cur_bp->pt.o) == '\t')
			    && ((get_goalc() % cur_bp->tab_width)
				== cur_bp->tab_width-1)))) {
			/* Replace the character.  */
			undo_save(UNDO_REPLACE_CHAR, cur_bp->pt,
				  *astr_char(cur_bp->pt.p->text, cur_bp->pt.o), 0);
			*astr_char(cur_bp->pt.p->text, cur_bp->pt.o) = c;
			++cur_bp->pt.o;

			cur_bp->flags |= BFLAG_MODIFIED;

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

	/* Calculate the two line lengths. */
	lp1len = cur_bp->pt.o;
	lp2len = astr_len(cur_bp->pt.p->text) - lp1len;

	lp1 = cur_bp->pt.p;
	lp2 = new_line();

	/* Move the text after the point into the new line. */
	astr_cpy(lp2->text, astr_substr(lp1->text, lp1len, lp2len));
        astr_truncate(lp1->text, lp1len);

	/* Update line linked list. */
	lp2->next = lp1->next;
	lp2->next->prev = lp2;
	lp2->prev = lp1;
	lp1->next = lp2;
	++cur_bp->num_lines;

	adjust_markers_for_addline(lp1, lp2, lp1len, move_pt);

	cur_bp->flags |= BFLAG_MODIFIED;

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

/* Recase s according to case of template. */
/* XXX At the moment this is not the same as Emacs (doesn't check that
 * replacement string contains no upper case) but is consistent with
 * search.  See TODO file. */
static void recase(char *s, char *template, int len)
{
        int i;

        for (i = 0; i < len; i++, s++) {
                if (isupper(*template++))
                        *s = toupper(*s);
        }
}

/* This routine replaces text in the line "lp" with "newtext".  If
 * "replace_case" is TRUE then the new characters will be the same
 * case as the old. */
void line_replace_text(Line **lp, int offset, int orgsize, const char *newtext,
		       int replace_case)
{
	int newsize = strlen(newtext);

	if (orgsize == 0)
		return;
	assert(orgsize > 0);

        if (replace_case) {
                newtext = strdup(newtext);
                recase((char *)newtext, astr_char((*lp)->text, offset), min(orgsize, newsize));
        }

	if (newsize != orgsize) {
                cur_bp->flags |= BFLAG_MODIFIED;
                astr_replace_cstr((*lp)->text, offset, orgsize, newtext);
		adjust_markers_for_offset(*lp, offset, newsize - orgsize);
	} else {
		if (memcmp(astr_char((*lp)->text, offset), newtext, newsize) != 0) {
                        memcpy(astr_char((*lp)->text, offset), newtext, newsize);
                        cur_bp->flags |= BFLAG_MODIFIED;
		}
	}
                
        if (replace_case)
                free((char *)newtext);
}

/*
 * Split the current line so that the text width is less
 * than the value specified by `fill-column'.
 */
void fill_break_line(void)
{
	int break_col, last_col, excess = 0;

        /* Move cursor back to fill_column */
        while (get_goalc() > cur_bp->fill_column + 1) {
                cur_bp->pt.o--;
                excess++;
        }

        /* Find break point moving left from fill-column. */
        for (break_col = cur_bp->pt.o; break_col > 0; --break_col) {
                int c = *astr_char(cur_bp->pt.p->text, break_col - 1);
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

DEFUN("newline", newline)
/*+
Insert a newline at the current point position into
the current buffer.
+*/
{
	int uni, ret = TRUE;

	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	for (uni = 0; uni < uniarg; ++uni) {
                if (cur_bp->flags & BFLAG_AUTOFILL &&
                    get_goalc() > cur_bp->fill_column)
                        fill_break_line();
		if (!insert_newline()) {
			ret = FALSE;
			break;
		}
        }
	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

	return ret;
}

DEFUN("open-line", open_line)
/*+
Insert a newline and leave point before it.
+*/
{
	int uni, ret = TRUE;

	undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
	for (uni = 0; uni < uniarg; ++uni)
		if (!intercalate_newline()) {
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

int self_insert_command(int c)
{
	deactivate_mark();

	if (c <= 255) {
                if (isspace(c) && cur_bp->flags & BFLAG_AUTOFILL &&
                    get_goalc() > cur_bp->fill_column)
                        fill_break_line();
		insert_char(c);
                return TRUE;
	} else {
		ding();
		return FALSE;
	}
}

DEFUN("self-insert-command", self_insert_command)
/*+
Insert the character you type.
+*/
{
	int uni, c, ret = TRUE;

	c = term_getkey();

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
	deactivate_mark();

	if (!eolp()) {
		if (warn_if_readonly_buffer())
			return FALSE;

		undo_save(UNDO_INTERCALATE_CHAR, cur_bp->pt,
			  *astr_char(cur_bp->pt.p->text, cur_bp->pt.o), 0);

		/*
		 * Move the text one position backward after the point,
		 * if required.
		 * This code assumes that memmove(d, s, 0) does nothing.
		 */
                astr_remove(cur_bp->pt.p->text, cur_bp->pt.o, 1);

		adjust_markers_for_offset(cur_bp->pt.p, cur_bp->pt.o, -1);

		cur_bp->flags |= BFLAG_MODIFIED;

		return TRUE;
	} else if (!eobp()) {
		Line *lp1, *lp2;
		int lp1len;

		if (warn_if_readonly_buffer())
			return FALSE;

		undo_save(UNDO_INTERCALATE_CHAR, cur_bp->pt, '\n', 0);

		lp1 = cur_bp->pt.p;
		lp2 = cur_bp->pt.p->next;
		lp1len = astr_len(lp1->text);

                /* Move the next line text into the current line. */
                astr_cat(lp1->text, lp2->text);

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
	deactivate_mark();

	if (backward_char()) {
		delete_char();
		return TRUE;
	}
	else {
		minibuf_error("Beginning of buffer");
		return FALSE;
	}
}

static int backward_delete_char_overwrite(void)
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

		return TRUE;
	} else
		return backward_delete_char();
}

DEFUN("backward-delete-char", backward_delete_char)
/*+
Delete the previous character.
Join lines if the character is a newline.
+*/
{
	 /* In overwrite-mode and isn't called by delete_char().  */
	int (*f)(void) = ((cur_bp->flags & BFLAG_OVERWRITE) &&
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

	deactivate_mark();
	old_point = point_marker();

	/* Find previous non-blank line.  */
	do {
		if (!FUNCALL_ARG(forward_line, -1)) {
			cur_bp->pt = old_point->pt;
			free_marker(old_point);

			return insert_tab();
		}
	} while (is_blank_line());

	/* Go to `cur_goalc' in that non-blank line. */
	while (!eolp() && get_goalc() < cur_goalc)
		forward_char();

	/* Now find the next blank char. */
        if (!(preceding_char() == '\t' && get_goalc() > cur_goalc))
                while (!eolp() && (!isspace(following_char())))
                        forward_char();

	/* Find next non-blank char. */
	while (!eolp() && (isspace(following_char())))
		forward_char();

	/* Target column. */
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
	deactivate_mark();
	pop_mark();

	undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

	return TRUE;
}

DEFUN("indent-command", indent_command)
/*+
Indent line in proper way for current major mode or insert a tab.
+*/
{
        return indent_relative();
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
