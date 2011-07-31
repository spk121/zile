/* Useful editing functions

   Copyright (c) 2004, 2008-2011 Free Software Foundation, Inc.

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

#include "main.h"
#include "extern.h"

bool
is_empty_line (void)
{
  return buffer_line_len (cur_bp, get_buffer_pt (cur_bp)) == 0;
}

bool
is_blank_line (void)
{
  for (size_t i = 0; i < buffer_line_len (cur_bp, get_buffer_pt (cur_bp)); i++)
    {
      char c = get_buffer_char (cur_bp, get_buffer_line_o (cur_bp) + i);
      if (c != ' ' && c != '\t')
        return false;
    }
  return true;
}

/* Returns the character following point in the current buffer. */
int
following_char (void)
{
  if (eobp ())
    return 0;
  else if (eolp ())
    return '\n';
  else
    return get_buffer_char (cur_bp, get_buffer_pt (cur_bp));
}

/* Return the character preceding point in the current buffer. */
int
preceding_char (void)
{
  if (bobp ())
    return 0;
  else if (bolp ())
    return '\n';
  else
    return get_buffer_char (cur_bp, get_buffer_pt (cur_bp) - 1);
}

/* Return true if point is at the beginning of the buffer. */
bool
bobp (void)
{
  return get_buffer_pt (cur_bp) == 0;
}

/* Return true if point is at the end of the buffer. */
bool
eobp (void)
{
  return get_buffer_pt (cur_bp) == get_buffer_size (cur_bp);
}

/* Return true if point is at the beginning of a line. */
bool
bolp (void)
{
  return get_buffer_pt (cur_bp) == get_buffer_line_o (cur_bp);
}

/* Return true if point is at the end of a line. */
bool
eolp (void)
{
  return get_buffer_pt (cur_bp) - get_buffer_line_o (cur_bp) ==
    buffer_line_len (cur_bp, get_buffer_pt (cur_bp));
}

/* Signal an error, and abort any ongoing macro definition. */
void
ding (void)
{
  if (thisflag & FLAG_DEFINING_MACRO)
    cancel_kbd_macro ();

  if (get_variable_bool ("ring-bell") && cur_wp != NULL)
    term_beep ();
}
