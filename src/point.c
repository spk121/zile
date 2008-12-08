/* Point facility functions

   Copyright (c) 2008 Free Software Foundation, Inc.
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

#include "zile.h"
#include "extern.h"

Point
make_point (size_t lineno, size_t offset)
{
  Point pt;
  pt.p = cur_bp->lines->next;
  pt.n = lineno;
  pt.o = offset;
  while (lineno > 0)
    {
      pt.p = pt.p->next;
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
    swap_point (&pt1, &pt2);

  for (lp = pt1.p;; lp = lp->next)
    {
      size += astr_len (lp->text);

      if (lp == pt1.p)
        size -= pt1.o;

      if (lp == pt2.p)
        {
          size -= astr_len (lp->text) - pt2.o;
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
  if (cmp_point (pt1, pt2) > 0)
    swap_point (&pt1, &pt2);

  return pt2.n - pt1.n;
}

void
swap_point (Point * pt1, Point * pt2)
{
  Point pt0 = *pt1;
  *pt1 = *pt2;
  *pt2 = pt0;
}

Point
point_min (void)
{
  Point pt;
  pt.p = cur_bp->lines->next;
  pt.n = 0;
  pt.o = 0;
  return pt;
}

Point
point_max (void)
{
  Point pt;
  pt.p = cur_bp->lines->prev;
  pt.n = cur_bp->last_line;
  pt.o = astr_len (cur_bp->lines->prev->text);
  return pt;
}

Point
line_beginning_position (int count)
{
  Point pt;

  /* Copy current point position without offset (beginning of
   * line). */
  pt = cur_bp->pt;
  pt.o = 0;

  count--;
  for (; count < 0 && pt.p->prev != cur_bp->lines; pt.n--, count++)
    pt.p = pt.p->prev;
  for (; count > 0 && pt.p->next != cur_bp->lines; pt.n++, count--)
    pt.p = pt.p->next;

  return pt;
}

Point
line_end_position (int count)
{
  Point pt = line_beginning_position (count);
  pt.o = astr_len (pt.p->text);
  return pt;
}

void
goto_point (Point pt)
{
  if (cur_bp->pt.n > pt.n)
    do
      FUNCALL (previous_line);
    while (cur_bp->pt.n > pt.n);
  else if (cur_bp->pt.n < pt.n)
    do
      FUNCALL (next_line);
    while (cur_bp->pt.n < pt.n);

  if (cur_bp->pt.o > pt.o)
    do
      FUNCALL (backward_char);
    while (cur_bp->pt.o > pt.o);
  else if (cur_bp->pt.o < pt.o)
    do
      FUNCALL (forward_char);
    while (cur_bp->pt.o < pt.o);
}
