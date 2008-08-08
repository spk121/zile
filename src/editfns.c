/* Useful editing functions

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

#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "gl_linked_list.h"

#include "zile.h"
#include "extern.h"

static gl_list_t mark_ring = NULL;	/* Mark ring. */

/* Push the current mark to the mark-ring. */
void
push_mark (void)
{
  if (!mark_ring)
    mark_ring = gl_list_create_empty (GL_LINKED_LIST,
                                      NULL, NULL, NULL, true);

  /* Save the mark.  */
  if (cur_bp->mark)
    gl_list_add_last (mark_ring, copy_marker (cur_bp->mark));
  /* Save an invalid mark.  */
  else
    {
      Marker *m = point_min_marker ();
      m->pt.p = NULL;
      gl_list_add_last (mark_ring, m);
    }
}

/* Pop a mark from the mark-ring and make it the current mark. */
void
pop_mark (void)
{
  Marker *m = (Marker *) gl_list_get_at (mark_ring,
                                         gl_list_size (mark_ring) - 1);

  /* Replace the mark. */
  if (m->bp->mark)
    free_marker (m->bp->mark);

  m->bp->mark = (m->pt.p) ? copy_marker (m) : NULL;

  assert (gl_list_remove_at (mark_ring, gl_list_size (mark_ring) - 1));
  free_marker (m);
}

/* Set the mark to point. */
void
set_mark (void)
{
  if (!cur_bp->mark)
    cur_bp->mark = point_marker ();
  else
    move_marker (cur_bp->mark, cur_bp, cur_bp->pt);
}

int
is_empty_line (void)
{
  return astr_len (cur_bp->pt.p->item) == 0;
}

int
is_blank_line (void)
{
  size_t c;
  for (c = 0; c < astr_len (cur_bp->pt.p->item); c++)
    if (!isspace ((int) *astr_char (cur_bp->pt.p->item, (ptrdiff_t) c)))
      return false;
  return true;
}

int
char_after (Point * pt)
{
  if (eobp ())
    return 0;
  else if (eolp ())
    return '\n';
  else
    return *astr_char (pt->p->item, (ptrdiff_t) pt->o);
}

int
char_before (Point * pt)
{
  if (bobp ())
    return 0;
  else if (bolp ())
    return '\n';
  else
    return *astr_char (pt->p->item, (ptrdiff_t) (pt->o - 1));
}

/* Returns the character following point in the current buffer. */
int
following_char (void)
{
  return char_after (&cur_bp->pt);
}

/* Return the character preceding point in the current buffer. */
int
preceding_char (void)
{
  return char_before (&cur_bp->pt);
}

/* Return true if point is at the beginning of the buffer. */
int
bobp (void)
{
  return (list_prev (cur_bp->pt.p) == cur_bp->lines && cur_bp->pt.o == 0);
}

/* Return true if point is at the end of the buffer. */
int
eobp (void)
{
  return (list_next (cur_bp->pt.p) == cur_bp->lines &&
	  cur_bp->pt.o == astr_len (cur_bp->pt.p->item));
}

/* Return true if point is at the beginning of a line. */
int
bolp (void)
{
  return cur_bp->pt.o == 0;
}

/* Return true if point is at the end of a line. */
int
eolp (void)
{
  return cur_bp->pt.o == astr_len (cur_bp->pt.p->item);
}
