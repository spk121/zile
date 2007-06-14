/* Marker facility functions
   Copyright (c) 2004 David A. Capello.  All rights reserved.

   This file is part of Zile.

   Zile is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Zile is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with Zile; see the file COPYING.  If not, write to the Free
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#include "config.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

Marker *make_marker(void)
{
  Marker *marker;
  marker = (Marker *)zmalloc(sizeof(Marker));
  memset(marker, 0, sizeof(Marker));
  return marker;
}

static void unchain_marker(Marker *marker)
{
  Marker *m, *next, *prev = NULL;

  if (!marker->bp)
    return;

  for (m = marker->bp->markers; m; m = next) {
    next = m->next;
    if (m == marker) {
      if (prev)
        prev->next = next;
      else
        m->bp->markers = next;

      m->bp = NULL;
      break;
    }
    prev = m;
  }
}

void free_marker(Marker *marker)
{
  unchain_marker(marker);
  free(marker);
}

void move_marker(Marker *marker, Buffer *bp, Point pt)
{
  if (bp != marker->bp) {
    /* Unchain with the previous pointed buffer.  */
    unchain_marker(marker);

    /* Change the buffer.  */
    marker->bp = bp;

    /* Chain with the new buffer.  */
    marker->next = bp->markers;
    bp->markers = marker;
  }

  /* Change the point.  */
  marker->pt = pt;
}

Marker *copy_marker(Marker *m)
{
  Marker *marker = make_marker();
  move_marker(marker, m->bp, m->pt);
  return marker;
}

Marker *point_marker(void)
{
  Marker *marker = make_marker();
  move_marker(marker, cur_bp, cur_bp->pt);
  return marker;
}

Marker *point_min_marker(void)
{
  Marker *marker = make_marker();
  move_marker(marker, cur_bp, point_min());
  return marker;
}
