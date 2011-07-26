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

#include "main.h"
#include "extern.h"

Point
offset_to_point (Buffer *bp, size_t offset)
{
  Point pt = {
    .n = 0,
    .o = 0
  };
  size_t o;
  for (o = 0; buffer_end_of_line (bp, o) < offset; o = buffer_next_line (bp, o))
    pt.n++;
  pt.o = offset - o;
  return pt;
}

void
goto_offset (size_t o)
{
  size_t old_n = get_buffer_pt (cur_bp).n;
  set_buffer_o (cur_bp, o);
  if (get_buffer_pt (cur_bp).n != old_n)
    {
      set_buffer_goalc (cur_bp, get_goalc ());
      thisflag |= FLAG_NEED_RESYNC;
    }
}
