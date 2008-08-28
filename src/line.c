/* Line-oriented editing functions

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Reuben Thomas.
   Copyright (c) 2004 David A. Capello.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

#include "config.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"


/*
 * Circular doubly-linked lists
 */

/* Create an empty list, returning a pointer to the list */
Line *
line_new (void)
{
  Line *l = XZALLOC (Line);

  l->next = l->prev = l;
  l->text = NULL;

  return l;
}

/* Delete a list, freeing its nodes */
void
line_delete (Line *l)
{
  Line *p = l, *q;

  do
    {
      q = p;
      p = p->next;
      free (q);
    }
  while (p != l);
}

/* Insert an item into list before the given point, returning the new item */
Line *
line_insert (Line *l, astr i)
{
  Line *n = XZALLOC (Line);

  n->next = l->next;
  n->prev = l;
  n->text = i;
  l->next = l->next->prev = n;

  return n;
}

/* Remove the next item of a list, returning the item, or NULL if the
   list is empty */
astr
line_remove (Line *l)
{
  astr i;
  Line *p = l->next;

  if (p == l)
    return NULL;
  i = p->text;
  l->next = l->next->next;
  l->next->prev = l;
  free (p);

  return i;
}


static void
adjust_markers (Line * newlp, Line * oldlp, size_t pointo, int dir,
		int offset)
{
  Marker *pt = point_marker (), *marker;

  for (marker = cur_bp->markers; marker != NULL; marker = marker->next)
    if (marker->pt.p == oldlp &&
	(dir == -1 || marker->pt.o >= pointo + dir + (offset < 0)))
      {
	marker->pt.p = newlp;
	marker->pt.o -= pointo * dir - offset;
	marker->pt.n += dir;
      }
    else if (marker->pt.n > cur_bp->pt.n)
      marker->pt.n += dir;

  cur_bp->pt = pt->pt; /* This marker has been updated to new position */
  free_marker (pt);
}

/* Insert the character at the current position and move the text at its right
 * whatever the insert/overwrite mode is.
 * This function doesn't change the current position of the pointer.
 */
int
intercalate_char (int c)
{
  if (warn_if_readonly_buffer ())
    return false;

  undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 0, 1);
  astr_insert_char (cur_bp->pt.p->text, (ptrdiff_t) cur_bp->pt.o, c);
  cur_bp->flags |= BFLAG_MODIFIED;

  return true;
}

/*
 * Insert the character `c' at the current point position
 * into the current buffer.
 */
int
insert_char (int c)
{
  size_t t = tab_width (cur_bp);

  if (warn_if_readonly_buffer ())
    return false;

  if (cur_bp->flags & BFLAG_OVERWRITE)
    {
      /* Current character isn't the end of line
	 && isn't a \t
	 || tab width isn't correct
	 || current char is a \t && we are in the tab limit.  */
      if ((cur_bp->pt.o < astr_len (cur_bp->pt.p->text))
	  &&
	  ((*astr_char (cur_bp->pt.p->text, (ptrdiff_t) cur_bp->pt.o) != '\t')
	   ||
	   ((*astr_char (cur_bp->pt.p->text, (ptrdiff_t) cur_bp->pt.o) ==
	     '\t') && ((get_goalc () % t) == t))))
	{
	  /* Replace the character.  */
          undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 1, 1);
	  *astr_char (cur_bp->pt.p->text, (ptrdiff_t) cur_bp->pt.o) = c;
	  ++cur_bp->pt.o;

	  cur_bp->flags |= BFLAG_MODIFIED;

	  return true;
	}
      /*
       * Fall through the "insertion" mode of a character
       * at the end of the line, since it is totally
       * equivalent to "overwrite" mode.
       */
    }

  intercalate_char (c);
  adjust_markers (cur_bp->pt.p, cur_bp->pt.p, cur_bp->pt.o, 0, 1);

  return true;
}

/*
 * Insert a character at the current position in insert mode
 * whatever the current insert mode is.
 */
int
insert_char_in_insert_mode (int c)
{
  int old_overwrite = cur_bp->flags & BFLAG_OVERWRITE, ret;

  cur_bp->flags &= ~BFLAG_OVERWRITE;
  ret = insert_char (c);
  cur_bp->flags |= old_overwrite;

  return ret;
}

static void
insert_expanded_tab (int (*inschr) (int chr))
{
  int c = get_goalc ();
  int t = tab_width (cur_bp);

  undo_save (UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);

  for (c = t - c % t; c > 0; --c)
    (*inschr) (' ');

  undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
}

int
insert_tab (void)
{
  if (warn_if_readonly_buffer ())
    return false;

  if (get_variable_bool ("indent-tabs-mode"))
    insert_char ('\t');
  else
    insert_expanded_tab (insert_char_in_insert_mode);

  return true;
}

DEFUN ("tab-to-tab-stop", tab_to_tab_stop)
/*+
Insert a tabulation at the current point position into the current
buffer.
+*/
{
  return execute_with_uniarg (true, uniarg, insert_tab, NULL);
}
END_DEFUN

/*
 * Insert a newline at the current position without moving the cursor.
 * Update all other cursors if they point on the splitted line.
 */
int
intercalate_newline ()
{
  Line *lp1, *lp2;
  size_t lp1len, lp2len;
  astr as;

  if (warn_if_readonly_buffer ())
    return false;

  undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 0, 1);

  /* Calculate the two line lengths. */
  lp1len = cur_bp->pt.o;
  lp2len = astr_len (cur_bp->pt.p->text) - lp1len;

  lp1 = cur_bp->pt.p;

  /* Update line linked list. */
  lp2 = line_insert (lp1, astr_new ());
  ++cur_bp->num_lines;

  /* Move the text after the point into the new line. */
  as = astr_substr (lp1->text, (ptrdiff_t) lp1len, lp2len);
  astr_cpy (lp2->text, as);
  astr_delete (as);
  astr_truncate (lp1->text, (ptrdiff_t) lp1len);

  adjust_markers (lp2, lp1, lp1len, 1, 0);

  cur_bp->flags |= BFLAG_MODIFIED;

  thisflag |= FLAG_NEED_RESYNC;

  return true;
}

int
insert_newline (void)
{
  return intercalate_newline () && forward_char ();
}

/*
 * Check the case of a string.
 * Returns 2 if it is all upper case, 1 if just the first letter is,
 * and 0 otherwise.
 */
static int
check_case (const char *s, size_t len)
{
  size_t i;

  if (!isupper ((int) *s))
    return 0;

  for (i = 1; i < len; i++)
    if (!isupper ((int) s[i]))
      return 1;

  return 2;
}

/*
 * Recase str according to case of tmpl.
 */
static void
recase (char *str, size_t len, const char *tmpl, size_t tmpl_len)
{
  size_t i;
  int tmpl_case = check_case (tmpl, tmpl_len);

  if (tmpl_case >= 1)
    *str = toupper (*str);

  if (tmpl_case == 2)
    for (i = 1; i < len; i++)
      str[i] = toupper (str[i]);
}

/*
 * Replace text in the line "lp" with "newtext". If "replace_case" is
 * true then the new characters will be the same case as the old.
 */
void
line_replace_text (Line ** lp, size_t offset, size_t oldlen,
		   char *newtext, size_t newlen, int replace_case)
{
  if (oldlen == 0)
    return;

  if (replace_case && get_variable_bool ("case-replace"))
    {
      newtext = xstrdup (newtext);
      recase (newtext, newlen, astr_char ((*lp)->text, (ptrdiff_t) offset),
	      oldlen);
    }

  if (newlen != oldlen)
    {
      cur_bp->flags |= BFLAG_MODIFIED;
      astr_replace_cstr ((*lp)->text, (ptrdiff_t) offset, oldlen, newtext);
      adjust_markers (*lp, *lp, offset, 0, (int) (newlen - oldlen));
    }
  else if (memcmp (astr_char ((*lp)->text, (ptrdiff_t) offset),
		   newtext, newlen) != 0)
    {
      memcpy (astr_char ((*lp)->text, (ptrdiff_t) offset), newtext, newlen);
      cur_bp->flags |= BFLAG_MODIFIED;
    }

  if (replace_case)
    free ((char *) newtext);
}

/*
 * If point is greater than fill-column, then split the line at the
 * right-most space character at or before fill-column, if there is
 * one, or at the left-most at or after fill-column, if not. If the
 * line contains no spaces, no break is made.
 */
void
fill_break_line (void)
{
  size_t i, break_col = 0, excess = 0, old_col;
  size_t fillcol = get_variable_number ("fill-column");

  /* If we're not beyond fill-column, stop now. */
  if (get_goalc () <= fillcol)
    return;

  /* Move cursor back to fill column */
  old_col = cur_bp->pt.o;
  while (get_goalc () > fillcol + 1)
    {
      cur_bp->pt.o--;
      excess++;
    }

  /* Find break point moving left from fill-column. */
  for (i = cur_bp->pt.o; i > 0; i--)
    {
      int c = *astr_char (cur_bp->pt.p->text, (ptrdiff_t) (i - 1));
      if (isspace (c))
	{
	  break_col = i;
	  break;
	}
    }

  /* If no break point moving left from fill-column, find first
     possible moving right. */
  if (break_col == 0)
    {
      for (i = cur_bp->pt.o + 1; i < astr_len (cur_bp->pt.p->text); i++)
	{
	  int c = *astr_char (cur_bp->pt.p->text, (ptrdiff_t) (i - 1));
	  if (isspace (c))
	    {
	      break_col = i;
	      break;
	    }
	}
    }

  if (break_col >= 1)
    {
      /* Break line. */
      size_t last_col = cur_bp->pt.o - break_col;
      cur_bp->pt.o = break_col;
      FUNCALL (delete_horizontal_space);
      insert_newline ();
      cur_bp->pt.o = last_col + excess;
    }
  else
    /* Undo fiddling with point. */
    cur_bp->pt.o = old_col;
}

static int
newline (void)
{
  if (cur_bp->flags & BFLAG_AUTOFILL &&
      get_goalc () > (size_t) get_variable_number ("fill-column"))
    fill_break_line ();
  return insert_newline ();
}

DEFUN ("newline", newline)
/*+
Insert a newline at the current point position into
the current buffer.
+*/
{
  return execute_with_uniarg (true, uniarg, newline, NULL);
}
END_DEFUN

DEFUN ("open-line", open_line)
/*+
Insert a newline and leave point before it.
+*/
{
  return execute_with_uniarg (true, uniarg, intercalate_newline, NULL);
}
END_DEFUN

void
insert_string (const char *s)
{
  undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 0, strlen (s));
  undo_nosave = true;
  for (; *s != '\0'; ++s)
    if (*s == '\n')
      insert_newline ();
    else
      insert_char (*s);
  undo_nosave = false;
}

void
insert_nstring (const char *s, size_t size)
{
  undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 0, size);
  undo_nosave = true;
  for (; 0 < size--; ++s)
    if (*s == '\n')
      insert_newline ();
    else
      insert_char_in_insert_mode (*s);
  undo_nosave = false;
}

void
bprintf (const char *fmt, ...)
{
  va_list ap;
  char *buf;
  va_start (ap, fmt);
  xvasprintf (&buf, fmt, ap);
  va_end (ap);
  insert_string (buf);
  free (buf);
}

int
delete_char (void)
{
  deactivate_mark ();

  if (!eolp ())
    {
      if (warn_if_readonly_buffer ())
	return false;

      undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 1, 0);
      astr_remove (cur_bp->pt.p->text, (ptrdiff_t) cur_bp->pt.o, 1);
      adjust_markers (cur_bp->pt.p, cur_bp->pt.p, cur_bp->pt.o, 0, -1);
      cur_bp->flags |= BFLAG_MODIFIED;

      return true;
    }
  else if (!eobp ())
    {
      Line *lp1, *lp2;
      size_t lp1len;

      if (warn_if_readonly_buffer ())
	return false;

      undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 1, 0);

      lp1 = cur_bp->pt.p;
      lp2 = lp1->next;
      lp1len = astr_len (lp1->text);

      /* Move the next line text into the current line. */
      lp2 = cur_bp->pt.p->next;
      astr_cat (lp1->text, cur_bp->pt.p->next->text);
      astr_delete (line_remove (lp1));
      --cur_bp->num_lines;

      adjust_markers (lp1, lp2, lp1len, -1, 0);

      cur_bp->flags |= BFLAG_MODIFIED;

      thisflag |= FLAG_NEED_RESYNC;

      return true;
    }

  minibuf_error ("End of buffer");

  return false;
}

static int
backward_delete_char (void)
{
  deactivate_mark ();

  if (backward_char ())
    {
      delete_char ();
      return true;
    }
  else
    {
      minibuf_error ("Beginning of buffer");
      return false;
    }
}

static int
backward_delete_char_overwrite (void)
{
  if (!bolp () && !eolp ())
    {
      deactivate_mark ();

      if (warn_if_readonly_buffer ())
	return false;

      backward_char ();
      if (following_char () == '\t')
	insert_expanded_tab (insert_char);
      else
	insert_char (' ');
      backward_char ();

      cur_bp->flags |= BFLAG_MODIFIED;

      return true;
    }
  else
    return backward_delete_char ();
}

DEFUN ("delete-char", delete_char)
/*+
Delete the following character.
Join lines if the character is a newline.
+*/
{
  return execute_with_uniarg (true, uniarg, delete_char, backward_delete_char);
}
END_DEFUN

DEFUN ("backward-delete-char", backward_delete_char)
/*+
Delete the previous character.
Join lines if the character is a newline.
+*/
{
  int (*forward) (void) = cur_bp->flags & BFLAG_OVERWRITE ?
    backward_delete_char_overwrite : backward_delete_char;
  return execute_with_uniarg (true, uniarg, forward, delete_char);
}
END_DEFUN

DEFUN ("delete-horizontal-space", delete_horizontal_space)
/*+
Delete all spaces and tabs around point.
+*/
{
  undo_save (UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);

  while (!eolp () && isspace (following_char ()))
    delete_char ();

  while (!bolp () && isspace (preceding_char ()))
    backward_delete_char ();

  undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
  return leT;
}
END_DEFUN

DEFUN ("just-one-space", just_one_space)
/*+
Delete all spaces and tabs around point, leaving one space.
+*/
{
  undo_save (UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  FUNCALL (delete_horizontal_space);
  insert_char_in_insert_mode (' ');
  undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
  return leT;
}
END_DEFUN

/***********************************************************************
			 Indentation command
***********************************************************************/
/*
 * Go to cur_goalc() in the previous non-blank line.
 */
static void
previous_nonblank_goalc (void)
{
  size_t cur_goalc = get_goalc ();

  /* Find previous non-blank line. */
  while (FUNCALL_ARG (forward_line, -1) && is_blank_line ());

  /* Go to `cur_goalc' in that non-blank line. */
  while (!eolp () && get_goalc () < cur_goalc)
    forward_char ();
}

DEFUN ("indent-relative", indent_relative)
/*+
Space out to under next indent point in previous nonblank line.
An indent point is a non-whitespace character following whitespace.
The following line shows the indentation points in this line.
    ^         ^    ^     ^   ^           ^      ^  ^    ^
If the previous nonblank line has no indent points beyond the
column point starts at, `tab-to-tab-stop' is done instead, unless
this command is invoked with a numeric argument, in which case it
does nothing.
+*/
{
  size_t target_goalc = 0, cur_goalc = get_goalc ();
  Marker *old_point;
  int ret = true;
  size_t t = tab_width (cur_bp);

  if (warn_if_readonly_buffer ())
    return leNIL;

  deactivate_mark ();

  /* If we're on first line, set target to 0. */
  if (cur_bp->pt.p->prev == cur_bp->lines)
    target_goalc = 0;
  else
    {				/* Find goalc in previous non-blank line. */
      old_point = point_marker ();

      previous_nonblank_goalc ();

      /* Now find the next blank char. */
      if (!(preceding_char () == '\t' && get_goalc () > cur_goalc))
	while (!eolp () && (!isspace (following_char ())))
	  forward_char ();

      /* Find next non-blank char. */
      while (!eolp () && (isspace (following_char ())))
	forward_char ();

      /* Target column. */
      if (!eolp ())
	target_goalc = get_goalc ();

      cur_bp->pt = old_point->pt;
      free_marker (old_point);
    }

  /* Insert indentation.  */
  undo_save (UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  if (target_goalc > 0)
    {
      /* If not at EOL on target line, insert spaces & tabs up to
	 target_goalc; if already at EOL on target line, insert a tab.
       */
      cur_goalc = get_goalc ();
      if (cur_goalc < target_goalc)
	{
	  do
	    {
	      if (cur_goalc % t == 0 && cur_goalc + t <= target_goalc)
		ret = insert_tab ();
	      else
		ret = insert_char (' ');
	    }
	  while (ret && (cur_goalc = get_goalc ()) < target_goalc);
	}
      else
	ret = insert_tab ();
    }
  else
    ret = insert_tab ();
  undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return bool_to_lisp (ret);
}
END_DEFUN

static size_t
previous_line_indent (void)
{
  size_t cur_indent;
  Marker *save_point = point_marker ();

  FUNCALL (previous_line);
  FUNCALL (beginning_of_line);

  /* Find first non-blank char. */
  while (!eolp () && (isspace (following_char ())))
    forward_char ();

  cur_indent = get_goalc ();

  /* Restore point. */
  cur_bp->pt = save_point->pt;
  free_marker (save_point);

  return cur_indent;
}

DEFUN ("indent-for-tab-command", indent_for_tab_command)
/*+
Indent line or insert a tab.
Depending on `tab-always-indent', either insert a tab or indent.
If initial point was within line's indentation, position after
the indentation.  Else stay at same point in text.
+*/
{
  if (get_variable_bool ("tab-always-indent"))
    return bool_to_lisp (insert_tab ());
  else if (get_goalc () < previous_line_indent ())
    return FUNCALL (indent_relative);
  return leT;
}
END_DEFUN

DEFUN ("newline-and-indent", newline_and_indent)
/*+
Insert a newline, then indent.
Indentation is done using the `indent-for-tab-command' function.
+*/
{
  int indent;
  size_t pos;
  Marker *old_point;

  if (warn_if_readonly_buffer ())
    return leNIL;

  deactivate_mark ();
  if (!insert_newline ())
    return leNIL;

  old_point = point_marker ();
  undo_save (UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);

  /* Check where last non-blank goalc is. */
  previous_nonblank_goalc ();
  pos = get_goalc ();
  indent = pos > 0 || (!eolp () && isspace (following_char ()));
  cur_bp->pt = old_point->pt;
  free_marker (old_point);
  /* Only indent if we're in column > 0 or we're in column 0 and
     there is a space character there in the last non-blank line. */
  if (indent)
    FUNCALL (indent_for_tab_command);

  undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return leT;
}
END_DEFUN
