/* Marker facility functions

   Copyright (c) 2004, 2008 Free Software Foundation, Inc.

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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "main.h"
#include "extern.h"

/*
 * Structure
 */

struct Marker
{
#define FIELD(ty, name) ty name;
#include "marker.h"
#undef FIELD
};

#define FIELD(ty, field)                         \
  GETTER (Marker, marker, ty, field)             \
  SETTER (Marker, marker, ty, field)

#include "marker.h"
#undef FIELD

Marker *
marker_new (void)
{
  return (Marker *) XZALLOC (Marker);
}

static void
unchain_marker (Marker * marker)
{
  Marker *m, *next, *prev = NULL;

  if (!marker->bp)
    return;

  for (m = get_buffer_markers (marker->bp); m; m = next)
    {
      next = m->next;
      if (m == marker)
        {
          if (prev)
            prev->next = next;
          else
            set_buffer_markers (m->bp, next);

          m->bp = NULL;
          break;
        }
      prev = m;
    }
}

void
free_marker (Marker * marker)
{
  unchain_marker (marker);
  free (marker);
}

void
move_marker (Marker * marker, Buffer * bp, Point pt)
{
  if (bp != marker->bp)
    {
      /* Unchain with the previous pointed buffer.  */
      unchain_marker (marker);

      /* Change the buffer.  */
      marker->bp = bp;

      /* Chain with the new buffer.  */
      marker->next = get_buffer_markers (bp);
      set_buffer_markers (bp, marker);
    }

  /* Change the point.  */
  marker->pt = pt;
}

Marker *
copy_marker (Marker * m)
{
  Marker *marker = NULL;
  if (m)
    {
      marker = marker_new ();
      move_marker (marker, m->bp, m->pt);
    }
  return marker;
}

Marker *
point_marker (void)
{
  Marker *marker = marker_new ();
  move_marker (marker, cur_bp, get_buffer_pt (cur_bp));
  return marker;
}

Marker *
point_min_marker (void)
{
  Marker *marker = marker_new ();
  move_marker (marker, cur_bp, point_min ());
  return marker;
}
