/* Point facility functions

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

#include <assert.h>

#include "main.h"
#include "extern.h"

Point
make_point (size_t lineno, size_t offset)
{
  return (Point) {.n = lineno, .o = offset};
}

Point
offset_to_point (Buffer *bp, size_t offset)
{
  Point pt = {
    .n = 0,
    .o = 0
  };
  size_t o;
  for (o = 0; estr_end_of_line (get_buffer_text (bp), o) < offset; o = estr_next_line (get_buffer_text (bp), o))
    pt.n++;
  pt.o = offset - o;
  return pt;
}

Point
point_min (void)
{
  return make_point (0, 0);
}

Point
point_max (void)
{
  return offset_to_point (cur_bp, get_buffer_size (cur_bp));
}

Point
line_beginning_position (int count)
{
  /* Copy current point position without offset (beginning of line). */
  size_t o = get_buffer_o (cur_bp);

  count--;
  for (; count < 0 && o > 0; count++)
    o = estr_prev_line (get_buffer_text (cur_bp), o);
  for (; count > 0 && o < SIZE_MAX; count--)
    o = estr_next_line (get_buffer_text (cur_bp), o);

  return offset_to_point (cur_bp, o);
}

Point
line_end_position (int count)
{
  Point pt = line_beginning_position (count);
  pt.o = get_buffer_line_len (cur_bp);
  return pt;
}

/* Go to coordinates described by pt. */
void
goto_point (Point pt)
{
  if (get_buffer_pt (cur_bp).n > pt.n)
    do
      FUNCALL (previous_line);
    while (get_buffer_pt (cur_bp).n > pt.n);
  else if (get_buffer_pt (cur_bp).n < pt.n)
    do
      FUNCALL (next_line);
    while (get_buffer_pt (cur_bp).n < pt.n);

  if (get_buffer_pt (cur_bp).o > pt.o)
    do
      FUNCALL (backward_char);
    while (get_buffer_pt (cur_bp).o > pt.o);
  else if (get_buffer_pt (cur_bp).o < pt.o)
    do
      FUNCALL (forward_char);
    while (get_buffer_pt (cur_bp).o < pt.o);
}
