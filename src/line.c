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

/*	$Id: line.c,v 1.61 2005/02/06 20:21:08 rrt Exp $	*/

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"


static void adjust_markers(Line *newlp, Line *oldlp, size_t pointo, int dir, int offset)
{
  Marker *pt = point_marker(), *marker;

  for (marker = cur_bp->markers; marker != NULL; marker = marker->next)
    if (marker->pt.p == oldlp &&
        (dir == -1 || marker->pt.o >= pointo + dir + (offset < 0))) {
      marker->pt.p = newlp;
      marker->pt.o -= pointo * dir - offset;
      marker->pt.n += dir;
    } else if (marker->pt.n > cur_bp->pt.n)
      marker->pt.n += dir;

  cur_bp->pt = pt->pt;
  free_marker(pt);
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
  astr_insert_char(cur_bp->pt.p->item, (ptrdiff_t)cur_bp->pt.o, c);
  cur_bp->flags |= BFLAG_MODIFIED;

  return TRUE;
}

/*
 * Insert the character `c' at the current point position
 * into the current buffer.
 */
int insert_char(int c)
{
  size_t t = tab_width(cur_bp);

  if (warn_if_readonly_buffer())
    return FALSE;

  if (cur_bp->flags & BFLAG_OVERWRITE) {
    /* Current character isn't the end of line
       && isn't a \t
       || tab width isn't correct
       || current char is a \t && we are in the tab limit.  */
    if ((cur_bp->pt.o < astr_len(cur_bp->pt.p->item))
        && ((*astr_char(cur_bp->pt.p->item, (ptrdiff_t)cur_bp->pt.o) != '\t')
            || ((*astr_char(cur_bp->pt.p->item,
                            (ptrdiff_t)cur_bp->pt.o) == '\t')
                && ((get_goalc() % t) == t)))) {
      /* Replace the character.  */
      undo_save(UNDO_REPLACE_CHAR, cur_bp->pt,
                (size_t)(*astr_char(cur_bp->pt.p->item,
                                      (ptrdiff_t)cur_bp->pt.o)), 0);
      *astr_char(cur_bp->pt.p->item, (ptrdiff_t)cur_bp->pt.o) = c;
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
  adjust_markers(cur_bp->pt.p, cur_bp->pt.p, cur_bp->pt.o, 0, 1);

  return TRUE;
}

/*
 * Insert a character at the current position in insert mode
 * whetever the current insert mode is.
 */
int insert_char_in_insert_mode(int c)
{
  int old_mode = cur_bp->flags, ret;

  cur_bp->flags &= ~BFLAG_OVERWRITE;
  ret = insert_char(c);
  cur_bp->flags = old_mode;

  return ret;
}

static void insert_expanded_tab(int (*inschr)(int chr))
{
  int c = get_goalc();
  int t = tab_width(cur_bp);

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

/*
 * Insert a newline at the current position without moving the cursor.
 * Update all other cursors if they point on the splitted line.
 */
int intercalate_newline()
{
  Line *lp1, *lp2;
  size_t lp1len, lp2len;

  if (warn_if_readonly_buffer())
    return FALSE;

  undo_save(UNDO_REMOVE_CHAR, cur_bp->pt, 0, 0);

  /* Calculate the two line lengths. */
  lp1len = cur_bp->pt.o;
  lp2len = astr_len(cur_bp->pt.p->item) - lp1len;

  lp1 = cur_bp->pt.p;

  /* Update line linked list. */
  lp2 = list_prepend(lp1, astr_new());
  ++cur_bp->num_lines;

  /* Move the text after the point into the new line. */
  astr_cpy(lp2->item,
           astr_substr(lp1->item, (ptrdiff_t)lp1len, lp2len));
  astr_truncate(lp1->item, (ptrdiff_t)lp1len);

  adjust_markers(lp2, lp1, lp1len, 1, 0);

  cur_bp->flags |= BFLAG_MODIFIED;

  thisflag |= FLAG_NEED_RESYNC;

  return TRUE;
}

int insert_newline(void)
{
  return intercalate_newline() && forward_char();
}

/*
 * Check the case of a string.
 * Returns 2 if it is all upper case, 1 if just the first letter is,
 * and 0 otherwise.
 */
static int check_case(const char *s, size_t len)
{
  size_t i;

  if (!isupper(*s))
    return 0;

  for (i = 1; i < len; i++)
    if (!isupper(s[i]))
      return 1;

  return 2;
}

/*
 * Recase str according to case of tmpl.
 */
static void recase(char *str, size_t len, const char *tmpl, size_t tmpl_len)
{
  size_t i;
  int tmpl_case = check_case(tmpl, tmpl_len);

  if (tmpl_case >= 1)
    *str = toupper(*str);

  if (tmpl_case == 2)
    for (i = 1; i < len; i++)
      str[i] = toupper(str[i]);
}

/*
 * Replace text in the line "lp" with "newtext". If "replace_case" is
 * TRUE then the new characters will be the same case as the old.
 */
void line_replace_text(Line **lp, size_t offset, size_t oldlen,
                       char *newtext, size_t newlen, int replace_case)
{
  if (oldlen == 0)
    return;

  if (replace_case && lookup_bool_variable("case-replace")) {
    newtext = zstrdup(newtext);
    recase(newtext, newlen, astr_char((*lp)->item, (ptrdiff_t)offset),
           oldlen);
  }

  if (newlen != oldlen) {
    cur_bp->flags |= BFLAG_MODIFIED;
    astr_replace_cstr((*lp)->item, (ptrdiff_t)offset, oldlen, newtext);
    adjust_markers(*lp, *lp, offset, 0, (int)(newlen - oldlen));
  } else if (memcmp(astr_char((*lp)->item, (ptrdiff_t)offset),
                    newtext, newlen) != 0) {
    memcpy(astr_char((*lp)->item, (ptrdiff_t)offset), newtext, newlen);
    cur_bp->flags |= BFLAG_MODIFIED;
  }

  if (replace_case)
    free((char *)newtext);
}

/*
 * If point is greater than fill-column, then split the line at the
 * right-most space character at or before fill-column, if there is
 * one, or at the left-most at or after fill-column, if not. If the
 * line contains no spaces, no break is made.
 */
void fill_break_line(void)
{
  size_t i, break_col = 0, excess = 0, old_col;
  size_t fillcol = get_variable_number("fill-column");

  /* If we're not beyond fill-column, stop now. */
  if (get_goalc() <= fillcol)
    return;

  /* Move cursor back to fill column */
  old_col = cur_bp->pt.o;
  while (get_goalc() > fillcol + 1) {
    cur_bp->pt.o--;
    excess++;
  }

  /* Find break point moving left from fill-column. */
  for (i = cur_bp->pt.o; i > 0; i--) {
    int c = *astr_char(cur_bp->pt.p->item, (ptrdiff_t)(i - 1));
    if (isspace(c)) {
      break_col = i;
      break;
    }
  }

  /* If no break point moving left from fill-column, find first
     possible moving right. */
  if (break_col == 0) {
    for (i = cur_bp->pt.o + 1; i < astr_len(cur_bp->pt.p->item); i++) {
      int c = *astr_char(cur_bp->pt.p->item, (ptrdiff_t)(i - 1));
      if (isspace(c)) {
        break_col = i;
        break;
      }
    }
  }

  if (break_col >= 1) {
    /* Break line. */
    size_t last_col = cur_bp->pt.o - break_col;
    cur_bp->pt.o = break_col;
    FUNCALL(delete_horizontal_space);
    insert_newline();
    cur_bp->pt.o = last_col + excess;
  } else
    /* Undo fiddling with point. */
    cur_bp->pt.o = old_col;
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
        get_goalc() > (size_t)get_variable_number("fill-column"))
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
        get_goalc() > (size_t)get_variable_number("fill-column"))
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

  c = getkey();

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
  zvasprintf(&buf, fmt, ap);
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
              (size_t)(*astr_char(cur_bp->pt.p->item,
                                    (ptrdiff_t)cur_bp->pt.o)), 0);

    /*
     * Move the text one position backward after the point,
     * if required.
     * This code assumes that memmove(d, s, 0) does nothing.
     */
    astr_remove(cur_bp->pt.p->item, (ptrdiff_t)cur_bp->pt.o, 1);

    adjust_markers(cur_bp->pt.p, cur_bp->pt.p, cur_bp->pt.o, 0, -1);

    cur_bp->flags |= BFLAG_MODIFIED;

    return TRUE;
  } else if (!eobp()) {
    Line *lp1, *lp2;
    size_t lp1len;

    if (warn_if_readonly_buffer())
      return FALSE;

    undo_save(UNDO_INTERCALATE_CHAR, cur_bp->pt, '\n', 0);

    lp1 = cur_bp->pt.p;
    lp2 = list_next(lp1);
    lp1len = astr_len(lp1->item);

    /* Move the next line text into the current line. */
    lp2 = list_next(cur_bp->pt.p);
    astr_cat(lp1->item, list_next(cur_bp->pt.p)->item);
    astr_delete(list_behead(lp1));
    --cur_bp->num_lines;

    adjust_markers(lp1, lp2, lp1len, -1, 0);

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
    backward_delete_char_overwrite : backward_delete_char;
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
  size_t target_goalc, cur_goalc = get_goalc();
  Marker *old_point;

  if (warn_if_readonly_buffer())
    return FALSE;

  deactivate_mark();
  old_point = point_marker();

  /* Find previous non-blank line. */
  do {
    int uniused = TRUE;
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
    Indent line or insert a tab.
    +*/
{
  return indent_relative();
}

DEFUN("newline-and-indent", newline_and_indent)
  /*+
    Insert a newline, then indent.
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
