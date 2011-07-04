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

#include <stdlib.h>
#include <ctype.h>

#include "main.h"
#include "extern.h"

bool
is_empty_line (void)
{
  return astr_len (get_line_text (get_buffer_pt (cur_bp).p)) == 0;
}

bool
is_blank_line (void)
{
  for (size_t c = 0; c < astr_len (get_line_text (get_buffer_pt (cur_bp).p)); c++)
    if (!isspace ((int) astr_get (get_buffer_text (cur_bp).as, get_buffer_o (cur_bp) + c)))
      return false;
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
    return astr_get (get_buffer_text (cur_bp).as, get_buffer_o (cur_bp) + get_buffer_pt (cur_bp).o);
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
    return astr_get (get_buffer_text (cur_bp).as, get_buffer_o (cur_bp) + get_buffer_pt (cur_bp).o - 1);
}

/* Return true if point is at the beginning of the buffer. */
bool
bobp (void)
{
  return point_to_offset (get_buffer_pt (cur_bp)) == 0;
}

/* Return true if point is at the end of the buffer. */
bool
eobp (void)
{
  return point_to_offset (get_buffer_pt (cur_bp)) == get_buffer_size (cur_bp);
}

/* Return true if point is at the beginning of a line. */
bool
bolp (void)
{
  return get_buffer_pt (cur_bp).o == 0;
}

/* Return true if point is at the end of a line. */
bool
eolp (void)
{
  Point pt = get_buffer_pt (cur_bp);
  return pt.o == astr_len (get_line_text (pt.p));
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
