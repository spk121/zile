/* Basic movement functions

   Copyright (c) 1997-2004, 2008-2009, 2011 Free Software Foundation, Inc.
   Copyright (c) 2011, 2012 Michael L. Gran

   This file is part of Michael Gran's unofficial fork of GNU Zile.

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
#include <libguile.h>

#include "main.h"
#include "extern.h"

SCM_DEFINE (G_beginning_of_line, "beginning-of-line", 0, 0, 0, (void), "\
Move point to beginning of current line.")
{
  goto_offset (get_buffer_line_o (cur_bp));
  set_buffer_goalc (cur_bp, 0);
  return SCM_BOOL_T;
}

SCM_DEFINE (G_end_of_line, "end-of-line", 0, 0, 0, (void), "\
Move point to end of current line.")
{
  goto_offset (get_buffer_line_o (cur_bp) + buffer_line_len (cur_bp, get_buffer_pt (cur_bp)));
  set_buffer_goalc (cur_bp, SIZE_MAX);
  return SCM_BOOL_T;
}


/*
 * Get the goal column.  Take care of expanding tabulations.
 */
size_t
get_goalc_bp (Buffer * bp, size_t o)
{
  size_t col = 0, t = tab_width (bp);
  size_t start = buffer_start_of_line (bp, o), end = o - start;

  for (size_t i = 0; i < end; i++, col++)
    if (get_buffer_char (bp, start + i) == '\t')
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

SCM_DEFINE (G_previous_line, "previous-line", 0, 1, 0, (SCM gn), "\
Move cursor vertically up one line.\n\
If there is no character in the target line exactly over the current column,\n\
the cursor is positioned after the character in that line which spans this\n\
column, or at the end of the line if it is not long enough.")
{
  long n = guile_to_long_or_error ("previous_line", SCM_ARG1, gn);

  return scm_from_bool (move_line (-n));
}

SCM_DEFINE (G_next_line, "next-line", 0, 1, 0,
	    (SCM gn), "\
Move cursor vertically down one line.\n\
If there is no character in the target line exactly under the current column,\n\
the cursor is positioned after the character in that line which spans this\n\
column, or at the end of the line if it is not long enough.")
{
  long n = guile_to_long_or_error ("next-line", SCM_ARG1, gn);

  return scm_from_bool (move_line (n));
}

SCM_DEFINE  (G_goto_char, "goto-char", 0, 1, 0, (SCM gn), "\
Set point to a position. The beginning of the buffer is position 1.")
{
  long n = 0;
  size_t buffer_size;

  if (SCM_UNBNDP (gn) && interactive)
    n = minibuf_read_number ("Goto char: ");
  else if (SCM_UNBNDP (gn) && !interactive)
    guile_wrong_number_of_arguments_error ("goto-char");
  else
    n = guile_to_long_or_error ("goto-char", SCM_ARG1, gn);

  /* minibuf return LONG_MAX or LONG_MAX-1 on error */
  if (n >= LONG_MAX - 1)
    return SCM_BOOL_F;

  buffer_size = get_buffer_size (cur_bp);
  goto_offset (MIN ((size_t) (MAX (n, 1) - 1), buffer_size));

  return scm_from_long (n);
}

SCM_DEFINE (G_goto_line, "goto-line", 0, 1, 0, (SCM gn), "\
Goto the given line number, counting from line 1 at beginning of buffer.")
{
  long n = 0;

  if (SCM_UNBNDP (gn) && interactive)
    n = minibuf_read_number ("Goto line: ");
  else if (SCM_UNBNDP (gn) && !interactive)
    guile_wrong_number_of_arguments_error ("goto-line");
  else
    n = guile_to_long_or_error ("goto-line", SCM_ARG1, gn);

  /* minibuf return LONG_MAX or LONG_MAX-1 on error */
  if (n >= LONG_MAX - 1)
    return SCM_BOOL_F;

  move_line ((MAX (n, 1) - 1) - offset_to_line (cur_bp, get_buffer_pt (cur_bp)));
  G_beginning_of_line ();
  return SCM_BOOL_T;
}

SCM_DEFINE (G_beginning_of_buffer, "beginning-of-buffer", 0, 0, 0, (void), "\
Move point to the beginning of the buffer; leave mark at previous position.")
{
  goto_offset (0);
  return SCM_BOOL_T;
}

SCM_DEFINE (G_end_of_buffer, "end-of-buffer", 0, 0, 0, (void), "\
Move point to the end of the buffer; leave mark at previous position.")
{
  goto_offset (get_buffer_size (cur_bp));
  return SCM_BOOL_T;
}

SCM_DEFINE (G_backward_char, "backward-char", 0, 1, 0,
	    (SCM gn), "\
Move point left N characters (right if N is negative).\n\
On attempt to pass beginning or end of buffer, stop and signal error.")
{
  long n;

  n = guile_to_long_or_error ("backward-char", SCM_ARG1, gn);
  if (!move_char (-n))
    {
      if (n <= 0)
	guile_end_of_buffer_error ("backward-char");
      else
	guile_beginning_of_buffer_error ("backward-char");
    }
  return SCM_BOOL_T;
}

SCM_DEFINE (G_forward_char, "forward-char", 0, 1, 0,
	    (SCM gn), "\
Move point right N characters (left if N is negative).\n\
On reaching end of buffer, stop and signal error.")
{
  long n;

  n = guile_to_long_or_error ("foward-char", SCM_ARG1, gn);
  if (!move_char (n))
    {
      if (n <= 0)
	guile_beginning_of_buffer_error ("forward-char");
      else
	guile_end_of_buffer_error ("forward-char");
    }
  return SCM_BOOL_T;
}

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

SCM_DEFINE (G_scroll_down, "scroll-down", 0, 1, 0, (SCM gn), "\
Scroll text of current window downward near full screen.")
{
  SCM ok;
  long n = guile_to_long_or_error ("scroll-down", SCM_ARG1, gn);

  ok = execute_with_uniarg (false, n, scroll_down, scroll_up);
  return ok;
}

SCM_DEFINE (G_scroll_up, "scroll-up", 0, 1, 0, (SCM gn), "\
Scroll text of current window upward near full screen.")
{
  SCM ok;
  long n = guile_to_long_or_error ("scroll-up", SCM_ARG1, gn);

  ok = execute_with_uniarg (false, n, scroll_up, scroll_down);
  return ok;
}


void
init_guile_basic_procedures (void)
{
#include "basic.x"
  guile_procedure_set_uniarg_integer ("previous-line");
  guile_procedure_set_uniarg_integer ("next-line");
  guile_procedure_set_uniarg_integer ("goto-char");
  guile_procedure_set_uniarg_integer ("goto-line");
  guile_procedure_set_uniarg_integer ("backward-char");
  guile_procedure_set_uniarg_integer ("forward-char");
  guile_procedure_set_uniarg_integer ("scroll-down");
  guile_procedure_set_uniarg_integer ("scroll-up");

  scm_c_export ("beginning-of-line",
		"end-of-line",
		"previous-line",
		"next-line",
		"goto-char",
		"goto-line",
		"beginning-of-buffer",
		"end-of-buffer",
		"forward-char",
		"backward-char",
		"scroll-down",
		"scroll-up", NULL);
}
