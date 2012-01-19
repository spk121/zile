/* Line-oriented editing functions

   Copyright (c) 1997-2011 Free Software Foundation, Inc.

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

#include <config.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"


static void
insert_expanded_tab (void)
{
  size_t t = tab_width (cur_bp);
  bprintf ("%*s", t - get_goalc () % t);
}

static bool
insert_tab (void)
{
  if (warn_if_readonly_buffer ())
    return false;

  if (get_variable_bool ("indent-tabs-mode"))
    insert_char ('\t');
  else
    insert_expanded_tab ();

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
 */
bool
intercalate_newline (void)
{
  return insert_newline () && move_char (-1);
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
  size_t fillcol = get_variable_number ("fill-column");
  bool break_made = false;

  /* Only break if we're beyond fill-column. */
  if (get_goalc () > fillcol)
    {
      size_t break_col = 0;

      /* Save point. */
      Marker *m = point_marker ();

      /* Move cursor back to fill column */
      size_t old_col = get_buffer_pt (cur_bp) - get_buffer_line_o (cur_bp);
      while (get_goalc () > fillcol + 1)
        move_char (-1);

      /* Find break point moving left from fill-column. */
      for (size_t i = get_buffer_pt (cur_bp) - get_buffer_line_o (cur_bp); i > 0; i--)
        {
          if (isspace (get_buffer_char (cur_bp, get_buffer_line_o (cur_bp) + i - 1)))
            {
              break_col = i;
              break;
            }
        }

      /* If no break point moving left from fill-column, find first
         possible moving right. */
      if (break_col == 0)
        for (size_t i = get_buffer_pt (cur_bp) + 1;
             i < buffer_end_of_line (cur_bp, get_buffer_line_o (cur_bp));
             i++)
          if (isspace (get_buffer_char (cur_bp, i - 1)))
            {
              break_col = i - get_buffer_line_o (cur_bp);
              break;
            }

      if (break_col >= 1) /* Break line. */
        {
          goto_offset (get_buffer_line_o (cur_bp) + break_col);
          FUNCALL (delete_horizontal_space);
          insert_newline ();
          goto_offset (get_marker_o (m));
          break_made = true;
        }
      else /* Undo fiddling with point. */
        goto_offset (get_buffer_line_o (cur_bp) + old_col);

      unchain_marker (m);
    }

  return break_made;
}

static bool
newline (void)
{
  if (get_buffer_autofill (cur_bp) &&
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

bool
insert_newline (void)
{
  return insert_estr ((estr) {.as = astr_new_cstr ("\n"), .eol = coding_eol_lf});
}

DEFUN_NONINTERACTIVE_ARGS ("insert", insert,
                           STR_ARG (arg))
/*+
Insert the argument at point.
+*/
{
  STR_INIT (arg);
  bprintf ("%s", astr_cstr (arg));
}
END_DEFUN

void
bprintf (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  insert_estr ((estr) {.as = astr_vfmt (fmt, ap), .eol = coding_eol_lf});
  va_end (ap);
}

static bool
backward_delete_char (void)
{
  deactivate_mark ();

  if (!move_char (-1))
    {
      minibuf_error ("Beginning of buffer");
      return false;
    }

  delete_char ();
  return true;
}

DEFUN_ARGS ("delete-char", delete_char,
            INT_OR_UNIARG (n))
/*+
Delete the following @i{n} characters (previous if @i{n} is negative).
+*/
{
  INT_OR_UNIARG_INIT (n);
  ok = execute_with_uniarg (true, n, delete_char, backward_delete_char);
}
END_DEFUN

DEFUN_ARGS ("backward-delete-char", backward_delete_char,
            INT_OR_UNIARG (n))
/*+
Delete the previous @i{n} characters (following if @i{n} is negative).
+*/
{
  INT_OR_UNIARG_INIT (n);
  ok = execute_with_uniarg (true, n, backward_delete_char, delete_char);
}
END_DEFUN

DEFUN ("delete-horizontal-space", delete_horizontal_space)
/*+
Delete all spaces and tabs around point.
+*/
{
  undo_start_sequence ();

  while (!eolp () && isspace (following_char ()))
    delete_char ();

  while (!bolp () && isspace (preceding_char ()))
    backward_delete_char ();

  undo_end_sequence ();
}
END_DEFUN

DEFUN ("just-one-space", just_one_space)
/*+
Delete all spaces and tabs around point, leaving one space.
+*/
{
  undo_start_sequence ();
  FUNCALL (delete_horizontal_space);
  insert_char (' ');
  undo_end_sequence ();
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
    move_char (1);
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
  size_t t = tab_width (cur_bp);

  ok = leNIL;

  if (warn_if_readonly_buffer ())
    return leNIL;

  deactivate_mark ();

  /* If we're on the first line, set target to 0. */
  if (get_buffer_line_o (cur_bp) == 0)
    target_goalc = 0;
  else
    { /* Find goalc in previous non-blank line. */
      Marker *m = point_marker ();

      previous_nonblank_goalc ();

      /* Now find the next blank char. */
      if (!(preceding_char () == '\t' && get_goalc () > cur_goalc))
        while (!eolp () && (!isspace (following_char ())))
          move_char (1);

      /* Find next non-blank char. */
      while (!eolp () && (isspace (following_char ())))
        move_char (1);

      /* Target column. */
      if (!eolp ())
        target_goalc = get_goalc ();

      goto_offset (get_marker_o (m));
      unchain_marker (m);
    }

  /* Insert indentation.  */
  undo_start_sequence ();
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
                ok = bool_to_lisp (insert_char (' '));
            }
          while (ok == leT && (cur_goalc = get_goalc ()) < target_goalc);
        }
      else
        ok = bool_to_lisp (insert_tab ());
    }
  else
    ok = bool_to_lisp (insert_tab ());
  undo_end_sequence ();
}
END_DEFUN

static size_t
previous_line_indent (void)
{
  size_t cur_indent;
  Marker *m = point_marker ();

  FUNCALL (previous_line);
  FUNCALL (beginning_of_line);

  /* Find first non-blank char. */
  while (!eolp () && (isspace (following_char ())))
    move_char (1);

  cur_indent = get_goalc ();

  /* Restore point. */
  goto_offset (get_marker_o (m));
  unchain_marker (m);

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

  undo_start_sequence ();
  if (insert_newline ())
    {
      Marker *m = point_marker ();

      /* Check where last non-blank goalc is. */
      previous_nonblank_goalc ();
      size_t pos = get_goalc ();
      int indent = pos > 0 || (!eolp () && isspace (following_char ()));
      goto_offset (get_marker_o (m));
      unchain_marker (m);
      /* Only indent if we're in column > 0 or we're in column 0 and
         there is a space character there in the last non-blank line. */
      if (indent)
        FUNCALL (indent_for_tab_command);
      ok = leT;
    }
  undo_end_sequence ();
}
END_DEFUN
