/* Line-oriented editing functions

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.

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

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
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
  while (l->next != l)
    line_remove (l->next);

  free (l);
}

/* Insert a line into list after the given point, returning the new line */
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

/* Remove a line from a list, if not sole line in list */
void
line_remove (Line *l)
{
  if (l->next || l->prev)
    {
      astr as = l->text;
      l->prev->next = l->next;
      l->prev->next->prev = l->prev;
      free (l);
      astr_delete (as);
    }
}


/*
 * Adjust markers (including point) when line at point is split, or
 * next line is joined on, or where a line is edited.
 *   newlp is the line to which characters were moved, oldlp the line
 *    moved from (if dir == 0, newlp == oldlp)
 *   pointo is point at which oldlp was split (to make newlp) or
 *     joined to newlp
 *   dir is 1 for split, -1 for join or 0 for line edit (newlp == oldlp)
 *   if dir == 0, delta gives the number of characters inserted (>0) or
 *     deleted (<0)
 */
static void
adjust_markers (Line * newlp, Line * oldlp, size_t pointo, int dir, ptrdiff_t delta)
{
  Marker *pt = point_marker (), *m;

  assert (dir >= -1 && dir <= 1);

  for (m = cur_bp->markers; m != NULL; m = m->next)
    if (m->pt.p == oldlp && (dir == -1 || m->pt.o > pointo))
      {
        m->pt.p = newlp;
        m->pt.o += delta - (pointo * dir);
        m->pt.n += dir;
      }
    else if (m->pt.n > cur_bp->pt.n)
      m->pt.n += dir;

  cur_bp->pt = pt->pt; /* This marker has been updated to new position */
  free_marker (pt);
}

/* Insert the character at the current position and move the text at its right
 * whatever the insert/overwrite mode is.
 * This function doesn't change the current position of the pointer.
 */
static int
intercalate_char (int c)
{
  if (warn_if_readonly_buffer ())
    return false;

  undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 0, 1);
  astr_insert_char (cur_bp->pt.p->text, cur_bp->pt.o, c);
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
          ((astr_get (cur_bp->pt.p->text, cur_bp->pt.o) != '\t')
           ||
           ((astr_get (cur_bp->pt.p->text, cur_bp->pt.o) ==
             '\t') && ((get_goalc () % t) == t))))
        {
          /* Replace the character.  */
          char ch = (char) c;
          undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 1, 1);
          astr_nreplace_cstr (cur_bp->pt.p->text, cur_bp->pt.o, 1, &ch, 1);
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
  forward_char ();
  adjust_markers (cur_bp->pt.p, cur_bp->pt.p, cur_bp->pt.o, 0, 1);

  return true;
}

DEFUN_HIDDEN_ARGS ("insert-char", insert_char,
                   STR_ARG (c))
/*+
Insert CHARACTER.
+*/
{
  STR_INIT (c);
  if (strlen (c) > 0)
    ok = bool_to_lisp (insert_char (*c));
  else
    ok = leNIL;
  STR_FREE (c);
}
END_DEFUN

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

static int
insert_tab (void)
{
  if (warn_if_readonly_buffer ())
    return false;

  if (get_variable_bool ("indent-tabs-mode"))
    insert_char_in_insert_mode ('\t');
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
  ok = execute_with_uniarg (true, uniarg, insert_tab, NULL);
}
END_DEFUN

/*
 * Insert a newline at the current position without moving the cursor.
 * Update markers after point in the split line.
 */
static int
intercalate_newline (void)
{
  if (warn_if_readonly_buffer ())
    return false;

  undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 0, 1);

  /* Move the text after the point into a new line. */
  line_insert (cur_bp->pt.p,
               astr_substr (cur_bp->pt.p->text, cur_bp->pt.o,
                            astr_len (cur_bp->pt.p->text) - cur_bp->pt.o));
  ++cur_bp->last_line;
  astr_truncate (cur_bp->pt.p->text, cur_bp->pt.o);
  adjust_markers (cur_bp->pt.p->next, cur_bp->pt.p, cur_bp->pt.o, 1, 0);

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
 * Replace text in the line "lp" with "newtext". If "replace_case" is
 * true then the new characters will be the same case as the old.
 */
void
line_replace_text (Line ** lp, size_t offset, size_t oldlen,
                   char *newtext, int replace_case)
{
  int case_type = 0;
  size_t newlen = strlen (newtext);
  astr as;

  replace_case = replace_case && get_variable_bool ("case-replace");

  if (replace_case)
    {
      case_type = check_case (astr_cstr ((*lp)->text) + offset, oldlen);

      if (case_type != 0)
        {
          as = astr_new_cstr (newtext);
          astr_recase (as, case_type == 1 ? CAPITALIZED : UPPERCASE);
        }
    }

  cur_bp->flags |= BFLAG_MODIFIED;
  astr_replace_cstr ((*lp)->text, offset, oldlen, newtext);
  adjust_markers (*lp, *lp, offset, 0, (ptrdiff_t) (newlen - oldlen));

  if (case_type != 0)
    astr_delete (as);
}

/*
 * If point is greater than fill-column, then split the line at the
 * right-most space character at or before fill-column, if there is
 * one, or at the left-most at or after fill-column, if not. If the
 * line contains no spaces, no break is made.
 *
 * Return flag indicating whether break was made.
 */
bool
fill_break_line (void)
{
  size_t i, break_col = 0, excess = 0, old_col;
  size_t fillcol = get_variable_number ("fill-column");

  /* If we're not beyond fill-column, stop now. */
  if (get_goalc () <= fillcol)
    return false;

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
      int c = astr_get (cur_bp->pt.p->text, i - 1);
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
          int c = astr_get (cur_bp->pt.p->text, i - 1);
          if (isspace (c))
            {
              break_col = i;
              break;
            }
        }
    }

  if (break_col >= 1) /* Break line. */
    {
      size_t last_col = cur_bp->pt.o - break_col;
      cur_bp->pt.o = break_col;
      FUNCALL (delete_horizontal_space);
      insert_newline ();
      cur_bp->pt.o = last_col + excess;
      return true;
    }
  else /* Undo fiddling with point. */
    {
      cur_bp->pt.o = old_col;
      return false;
    }
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
  ok = execute_with_uniarg (true, uniarg, newline, NULL);
}
END_DEFUN

DEFUN ("open-line", open_line)
/*+
Insert a newline and leave point before it.
+*/
{
  ok = execute_with_uniarg (true, uniarg, intercalate_newline, NULL);
}
END_DEFUN

void
insert_nstring (const char *s, size_t len)
{
  size_t i;
  undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 0, len);
  undo_nosave = true;
  for (i = 0; i < len; i++)
    {
      if (s[i] == '\n')
        insert_newline ();
      else
        insert_char_in_insert_mode (s[i]);
    }
  undo_nosave = false;
}

void
insert_astr (astr as)
{
  insert_nstring (astr_cstr (as), astr_len (as));
}

void
bprintf (const char *fmt, ...)
{
  va_list ap;
  char *buf;

  va_start (ap, fmt);
  xvasprintf (&buf, fmt, ap);
  va_end (ap);
  insert_nstring (buf, strlen (buf));
  free (buf);
}

int
delete_char (void)
{
  deactivate_mark ();

  if (eobp ())
    {
      minibuf_error ("End of buffer");
      return false;
    }

  if (warn_if_readonly_buffer ())
    return false;

  undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, 1, 0);

  if (eolp ())
    {
      size_t oldlen = astr_len (cur_bp->pt.p->text);
      Line *oldlp = cur_bp->pt.p->next;

      /* Join the lines. */
      astr_cat (cur_bp->pt.p->text, oldlp->text);
      line_remove (oldlp);

      adjust_markers (cur_bp->pt.p, oldlp, oldlen, -1, 0);
      --cur_bp->last_line;
      thisflag |= FLAG_NEED_RESYNC;
    }
  else
    {
      astr_remove (cur_bp->pt.p->text, cur_bp->pt.o, 1);
      adjust_markers (cur_bp->pt.p, cur_bp->pt.p, cur_bp->pt.o, 0, -1);
    }

  cur_bp->flags |= BFLAG_MODIFIED;

  return true;
}

static int
backward_delete_char (void)
{
  deactivate_mark ();

  if (!backward_char ())
    {
      minibuf_error ("Beginning of buffer");
      return false;
    }

  delete_char ();
  return true;
}

static int
backward_delete_char_overwrite (void)
{
  if (bolp () || eolp ())
    return backward_delete_char ();

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

DEFUN ("delete-char", delete_char)
/*+
Delete the following character.
Join lines if the character is a newline.
+*/
{
  ok = execute_with_uniarg (true, uniarg, delete_char, backward_delete_char);
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
  ok = execute_with_uniarg (true, uniarg, forward, delete_char);
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
}
END_DEFUN

/***********************************************************************
                         Indentation command
***********************************************************************/
/*
 * Go to cur_goalc () in the previous non-blank line.
 */
static void
previous_nonblank_goalc (void)
{
  size_t cur_goalc = get_goalc ();

  /* Find previous non-blank line. */
  while (FUNCALL_ARG (forward_line, -1) == leT && is_blank_line ());

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
  size_t t = tab_width (cur_bp);

  ok = leNIL;

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
                ok = bool_to_lisp (insert_tab ());
              else
                ok = bool_to_lisp (insert_char_in_insert_mode (' '));
            }
          while (ok == leT && (cur_goalc = get_goalc ()) < target_goalc);
        }
      else
        ok = bool_to_lisp (insert_tab ());
    }
  else
    ok = bool_to_lisp (insert_tab ());
  undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
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
}
END_DEFUN

DEFUN ("newline-and-indent", newline_and_indent)
/*+
Insert a newline, then indent.
Indentation is done using the `indent-for-tab-command' function.
+*/
{
  ok = leNIL;

  if (warn_if_readonly_buffer ())
    return leNIL;

  deactivate_mark ();

  undo_save (UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  if (insert_newline ())
    {
      Marker *old_point = point_marker ();
      int indent;
      size_t pos;

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
      ok = leT;
    }
  undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
}
END_DEFUN
