/* Point facility functions

   Copyright (c) 2004, 2008, 2009 Free Software Foundation, Inc.

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

#include "main.h"
#include "extern.h"

Point
make_point (size_t lineno, size_t offset)
{
  Point pt;
  pt.p = get_line_next (get_buffer_lines (cur_bp));
  pt.n = lineno;
  pt.o = offset;
  while (lineno > 0)
    {
      pt.p = get_line_next (pt.p);
      lineno--;
    }
  return pt;
}

int
cmp_point (Point pt1, Point pt2)
{
  if (pt1.n < pt2.n)
    return -1;
  else if (pt1.n > pt2.n)
    return +1;
  else
    return ((pt1.o < pt2.o) ? -1 : (pt1.o > pt2.o) ? +1 : 0);
}

int
point_dist (Point pt1, Point pt2)
{
  int size = 0;
  Line *lp;

  if (cmp_point (pt1, pt2) > 0)
    {
      Point pt0 = pt1;
      pt1 = pt2;
      pt2 = pt0;
    }

  for (lp = pt1.p;; lp = get_line_next (lp))
    {
      size += astr_len (get_line_text (lp));

      if (lp == pt1.p)
        size -= pt1.o;

      if (lp == pt2.p)
        {
          size -= astr_len (get_line_text (lp)) - pt2.o;
          break;
        }
      else
        size++;
    }

  return size;
}

int
count_lines (Point pt1, Point pt2)
{
  return abs (pt2.n - pt1.n);
}

Point
point_min (void)
{
  Point pt;
  pt.p = get_line_next (get_buffer_lines (cur_bp));
  pt.n = 0;
  pt.o = 0;
  return pt;
}

Point
point_max (void)
{
  Point pt;
  pt.p = get_line_prev (get_buffer_lines (cur_bp));
  pt.n = get_buffer_last_line (cur_bp);
  pt.o = astr_len (get_line_text (get_line_prev (get_buffer_lines (cur_bp))));
  return pt;
}

Point
line_beginning_position (int count)
{
  Point pt;

  /* Copy current point position without offset (beginning of
   * line). */
  pt = get_buffer_pt (cur_bp);
  pt.o = 0;

  count--;
  for (; count < 0 && get_line_prev (pt.p) != get_buffer_lines (cur_bp); pt.n--, count++)
    pt.p = get_line_prev (pt.p);
  for (; count > 0 && get_line_next (pt.p) != get_buffer_lines (cur_bp); pt.n++, count--)
    pt.p = get_line_next (pt.p);

  return pt;
}

Point
line_end_position (int count)
{
  Point pt = line_beginning_position (count);
  pt.o = astr_len (get_line_text (pt.p));
  return pt;
}

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
