/* Basic movement functions

   Copyright (c) 1997-2004, 2008-2009, 2011 Free Software Foundation, Inc.

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

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"

DEFUN ("beginning-of-line", beginning_of_line)
/*+
Move point to beginning of current line.
+*/
{
  goto_offset (get_buffer_line_o (cur_bp));
  set_buffer_goalc (cur_bp, 0);
}
END_DEFUN

DEFUN ("end-of-line", end_of_line)
/*+
Move point to end of current line.
+*/
{
  goto_offset (get_buffer_line_o (cur_bp) + get_buffer_line_len (cur_bp));
  set_buffer_goalc (cur_bp, SIZE_MAX);
}
END_DEFUN

/*
 * Get the goal column.  Take care of expanding tabulations.
 */
size_t
get_goalc_bp (Buffer * bp, Point pt)
{
  size_t col = 0, t = tab_width (bp);
  size_t end = MIN (pt.o, get_buffer_line_len (bp));

  for (size_t i = 0; i < end; i++, col++)
    if (astr_get (get_buffer_text (bp).as, get_buffer_line_o (bp) + i) == '\t')
      col |= t - 1;

  return col;
}

size_t
get_goalc (void)
{
  return get_goalc_bp (cur_bp, get_buffer_pt (cur_bp));
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
  ok = execute_with_uniarg (false, uniarg, previous_line, next_line);
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
  ok = execute_with_uniarg (false, uniarg, next_line, previous_line);
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
        const char *ms = astr_cstr (minibuf_read ("Goto char: ", ""));
        if (ms == NULL)
          {
            ok = FUNCALL (keyboard_quit);
            break;
          }
        n = strtoul (ms, NULL, 10);
        if (n == LONG_MAX)
          ding ();
      }
    while (n == LONG_MAX);

  if (ok == leT && n != LONG_MAX)
    {
      FUNCALL (beginning_of_buffer);
      for (long count = 1; count < n; count++)
        if (!forward_char ())
          break;
    }
}
END_DEFUN

DEFUN_ARGS ("goto-line", goto_line,
            INT_OR_UNIARG (n))
/*+
Goto line arg, counting from line 1 at beginning of buffer.
+*/
{
  INT_OR_UNIARG_INIT (n);
  if (noarg)
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

DEFUN ("beginning-of-buffer", beginning_of_buffer)
/*+
Move point to the beginning of the buffer; leave mark at previous position.
+*/
{
  goto_offset (0);
  thisflag |= FLAG_NEED_RESYNC;
}
END_DEFUN

DEFUN ("end-of-buffer", end_of_buffer)
/*+
Move point to the end of the buffer; leave mark at previous position.
+*/
{
  goto_offset (get_buffer_size (cur_bp));
  thisflag |= FLAG_NEED_RESYNC;
}
END_DEFUN

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
  ok = bool_to_lisp (move_char (-uniarg));
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
  ok = bool_to_lisp (move_char (uniarg));
  if (ok == leNIL)
    minibuf_error ("End of buffer");
}
END_DEFUN

static bool
scroll_down (void)
{
  if (!window_top_visible (cur_wp))
    return move_line (-get_window_eheight (cur_wp));

  minibuf_error ("Beginning of buffer");
  return false;
}

static bool
scroll_up (void)
{
  if (!window_bottom_visible (cur_wp))
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
