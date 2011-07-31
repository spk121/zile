/* Marker facility functions

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
#include <stdlib.h>
#include <ctype.h>
#include "gl_linked_list.h"

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

void
unchain_marker (const Marker * marker)
{
  if (!marker->bp)
    return;

  Marker *prev = NULL;
  for (Marker *m = get_buffer_markers (marker->bp); m; m = m->next)
    {
      if (m == marker)
        {
          if (prev)
            prev->next = m->next;
          else
            set_buffer_markers (m->bp, m->next);

          m->bp = NULL;
          break;
        }
      prev = m;
    }
}

void
move_marker (Marker * marker, Buffer * bp, size_t o)
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
  marker->o = o;
}

Marker *
copy_marker (const Marker * m)
{
  Marker *marker = NULL;
  if (m)
    {
      marker = marker_new ();
      move_marker (marker, m->bp, m->o);
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


/*
 * Mark ring
 */

static gl_list_t mark_ring = NULL;	/* Mark ring. */

/* Push the current mark to the mark-ring. */
void
push_mark (void)
{
  if (!mark_ring)
    mark_ring = gl_list_create_empty (GL_LINKED_LIST,
                                      NULL, NULL, NULL, true);

  /* Save the mark.  */
  if (get_buffer_mark (cur_bp))
    gl_list_add_last (mark_ring, copy_marker (get_buffer_mark (cur_bp)));
  else
    { /* Save an invalid mark.  */
      Marker *m = marker_new ();
      move_marker (m, cur_bp, 0);
      gl_list_add_last (mark_ring, m);
    }

  FUNCALL (set_mark);
}

/* Pop a mark from the mark-ring and make it the current mark. */
void
pop_mark (void)
{
  const Marker *m = (const Marker *) gl_list_get_at (mark_ring,
                                                     gl_list_size (mark_ring) - 1);

  /* Replace the mark. */
  if (get_buffer_mark (m->bp))
    unchain_marker (get_buffer_mark (m->bp));

  set_buffer_mark (m->bp, copy_marker (m));

  unchain_marker (m);
  assert (gl_list_remove_at (mark_ring, gl_list_size (mark_ring) - 1));
}

/* Set the mark to point. */
void
set_mark (void)
{
  if (!get_buffer_mark (cur_bp))
    set_buffer_mark (cur_bp, point_marker ());
  else
    move_marker (get_buffer_mark (cur_bp), cur_bp, get_buffer_pt (cur_bp));
}
