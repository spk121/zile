/* Basic movement functions

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2008, 2009 Free Software Foundation, Inc.

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

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

DEFUN ("beginning-of-line", beginning_of_line)
/*+
Move point to beginning of current line.
+*/
{
  set_buffer_pt (cur_bp, line_beginning_position (uniarg));
  set_buffer_goalc (cur_bp, 0);
}
END_DEFUN

DEFUN ("end-of-line", end_of_line)
/*+
Move point to end of current line.
+*/
{
  set_buffer_pt (cur_bp, line_end_position (uniarg));
  set_buffer_goalc (cur_bp, SIZE_MAX);
}
END_DEFUN

/*
 * Get the goal column.  Take care of expanding tabulations.
 */
size_t
get_goalc_bp (Buffer * bp, Point pt)
{
  size_t col = 0, t = tab_width (bp), i;
  const char *sp = astr_cstr (get_line_text (pt.p));
  size_t end = MIN (pt.o, astr_len (get_line_text (pt.p)));

  for (i = 0; i < end; i++)
    {
      if (sp[i] == '\t')
        col |= t - 1;
      ++col;
    }

  return col;
}

size_t
get_goalc (void)
{
  return get_goalc_bp (cur_bp, get_buffer_pt (cur_bp));
}

/*
 * Go to the column `goalc'.  Take care of expanding
 * tabulations.
 */
static void
goto_goalc (void)
{
  Point pt = get_buffer_pt (cur_bp);
  size_t i, col = 0, t = tab_width (cur_bp);

  for (i = 0; i < astr_len (get_line_text (pt.p)); i++)
    {
      if (col == get_buffer_goalc (cur_bp))
        break;
      else if (astr_get (get_line_text (pt.p), i) == '\t')
        {
          size_t w;
          for (w = t - col % t; w > 0; w--)
            if (++col == get_buffer_goalc (cur_bp))
              break;
        }
      else
        ++col;
    }

  pt.o = i;
  set_buffer_pt (cur_bp, pt);
}

static bool
move_line (int n)
{
  bool ok = true;
  int dir;
  Point pt = get_buffer_pt (cur_bp);

  if (n == 0)
    return false;
  else if (n > 0)
    {
      dir = 1;
      if ((size_t) n > get_buffer_last_line (cur_bp) - pt.n)
        {
          ok = false;
          n = get_buffer_last_line (cur_bp) - pt.n;
        }
    }
  else
    {
      dir = -1;
      n = -n;
      if ((size_t) n > pt.n)
        {
          ok = false;
          n = pt.n;
        }
    }

  for (; n > 0; n--)
    {
      Point pt = get_buffer_pt (cur_bp);
      pt.p = (dir > 0 ? get_line_next : get_line_prev) (pt.p);
      pt.n += dir;
      set_buffer_pt (cur_bp, pt);
    }

  if (last_command () != F_next_line && last_command () != F_previous_line)
    set_buffer_goalc (cur_bp, get_goalc ());
  goto_goalc ();

  thisflag |= FLAG_NEED_RESYNC;

  return ok;
}

bool
previous_line (void)
{
  return move_line (-1);
}

bool
next_line (void)
{
  return move_line (1);
}

DEFUN ("previous-line", previous_line)
/*+
Move cursor vertically up one line.
If there is no character in the target line exactly over the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
+*/
{
  if (uniarg < 0 || !bobp ())
    ok = execute_with_uniarg (false, uniarg, previous_line, next_line);
  if (ok == leNIL)
    FUNCALL (beginning_of_line);
}
END_DEFUN

DEFUN ("next-line", next_line)
/*+
Move cursor vertically down one line.
If there is no character in the target line exactly under the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
+*/
{
  if (uniarg < 0 || !eobp ())
    ok = execute_with_uniarg (false, uniarg, next_line, previous_line);
  if (ok == leNIL)
    FUNCALL (end_of_line);
}
END_DEFUN

DEFUN_ARGS ("goto-char", goto_char,
            INT_ARG (n))
/*+
Read a number N and move the cursor to character number N.
Position 1 is the beginning of the buffer.
+*/
{
  INT_INIT (n)
  else
    do
      {
        char *ms = minibuf_read ("Goto char: ", "");
        if (ms == NULL)
          {
            ok = FUNCALL (keyboard_quit);
            break;
          }
        n = strtoul (ms, NULL, 10);
        if (n == LONG_MAX)
          ding ();
        free (ms);
      }
    while (n == LONG_MAX);

  if (ok == leT && n != LONG_MAX)
    {
      long count;

      gotobob ();
      for (count = 1; count < n; count++)
        if (!forward_char ())
          break;
    }
}
END_DEFUN

DEFUN_ARGS ("goto-line", goto_line,
            INT_ARG (n))
/*+
Goto line arg, counting from line 1 at beginning of buffer.
+*/
{
  INT_INIT (n)
  else
    {
      n = minibuf_read_number ("Goto line: ");
      if (n == LONG_MAX - 1)
        minibuf_error ("End of file during parsing");
    }

  if (n >= LONG_MAX - 1)
    ok = leNIL;
  else
    {
      Point pt = get_buffer_pt (cur_bp);
      move_line ((size_t) (MAX (n, 1) - 1) - pt.n);
      FUNCALL (beginning_of_line);
    }
}
END_DEFUN

/*
 * Move point to the beginning of the buffer; do not touch the mark.
 */
void
gotobob (void)
{
  set_buffer_pt (cur_bp, point_min ());
  thisflag |= FLAG_NEED_RESYNC;
}

DEFUN ("beginning-of-buffer", beginning_of_buffer)
/*+
Move point to the beginning of the buffer; leave mark at previous position.
+*/
{
  set_mark_interactive ();
  gotobob ();
}
END_DEFUN

/*
 * Move point to the end of the buffer; do not touch the mark.
 */
void
gotoeob (void)
{
  set_buffer_pt (cur_bp, point_max ());
  thisflag |= FLAG_NEED_RESYNC;
}

DEFUN ("end-of-buffer", end_of_buffer)
/*+
Move point to the end of the buffer; leave mark at previous position.
+*/
{
  set_mark_interactive ();
  gotoeob ();
}
END_DEFUN

static bool
move_char (int dir)
{
  if (dir > 0 ? !eolp () : !bolp ())
    {
      Point pt = get_buffer_pt (cur_bp);
      pt.o += dir;
      set_buffer_pt (cur_bp, pt);
      return true;
    }
  else if (dir > 0 ? !eobp () : !bobp ())
    {
      Point pt = get_buffer_pt (cur_bp);
      thisflag |= FLAG_NEED_RESYNC;
      pt.p = (dir > 0 ? get_line_next : get_line_prev) (pt.p);
      pt.n += dir;
      set_buffer_pt (cur_bp, pt);
      if (dir > 0)
        FUNCALL (beginning_of_line);
      else
        FUNCALL (end_of_line);
      return true;
    }

  return false;
}

bool
backward_char (void)
{
  return move_char (-1);
}

bool
forward_char (void)
{
  return move_char (1);
}

DEFUN ("backward-char", backward_char)
/*+
Move point left N characters (right if N is negative).
On attempt to pass beginning or end of buffer, stop and signal error.
+*/
{
  ok = execute_with_uniarg (false, uniarg, backward_char, forward_char);
  if (ok == leNIL)
    minibuf_error ("Beginning of buffer");
}
END_DEFUN

DEFUN ("forward-char", forward_char)
/*+
Move point right N characters (left if N is negative).
On reaching end of buffer, stop and signal error.
+*/
{
  ok = execute_with_uniarg (false, uniarg, forward_char, backward_char);
  if (ok == leNIL)
    minibuf_error ("End of buffer");
}
END_DEFUN

static bool
scroll_down (void)
{
  if (get_buffer_pt (cur_bp).n > get_window_eheight (cur_wp))
    return move_line (-get_window_eheight (cur_wp));

  minibuf_error ("Beginning of buffer");
  return false;
}

static bool
scroll_up (void)
{
  if (get_buffer_pt (cur_bp).n <
      get_buffer_last_line (cur_bp) - get_window_eheight (cur_wp))
    return move_line (get_window_eheight (cur_wp));

  minibuf_error ("End of buffer");
  return false;
}

DEFUN ("scroll-down", scroll_down)
/*+
Scroll text of current window downward near full screen.
+*/
{
  ok = execute_with_uniarg (false, uniarg, scroll_down, scroll_up);
}
END_DEFUN

DEFUN ("scroll-up", scroll_up)
/*+
Scroll text of current window upward near full screen.
+*/
{
  ok = execute_with_uniarg (false, uniarg, scroll_up, scroll_down);
}
END_DEFUN
