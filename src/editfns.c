/* Useful editing functions

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

#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "gl_linked_list.h"

#include "main.h"
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
    move_marker (cur_bp->mark, cur_bp, get_buffer_pt (cur_bp));
}

int
is_empty_line (void)
{
  Point pt = get_buffer_pt (cur_bp);
  return astr_len (pt.p->text) == 0;
}

int
is_blank_line (void)
{
  Point pt = get_buffer_pt (cur_bp);
  size_t c;
  for (c = 0; c < astr_len (pt.p->text); c++)
    if (!isspace ((int) astr_get (pt.p->text, c)))
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
    {
      Point pt = get_buffer_pt (cur_bp);
      return astr_get (pt.p->text, pt.o);
    }
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
    {
      Point pt = get_buffer_pt (cur_bp);
      return astr_get (pt.p->text, pt.o - 1);
    }
}

/* Return true if point is at the beginning of the buffer. */
int
bobp (void)
{
  Point pt = get_buffer_pt (cur_bp);
  return (pt.p->prev == get_buffer_lines (cur_bp) && pt.o == 0);
}

/* Return true if point is at the end of the buffer. */
int
eobp (void)
{
  Point pt = get_buffer_pt (cur_bp);
  return (pt.p->next == get_buffer_lines (cur_bp) &&
          pt.o == astr_len (pt.p->text));
}

/* Return true if point is at the beginning of a line. */
int
bolp (void)
{
  Point pt = get_buffer_pt (cur_bp);
  return pt.o == 0;
}

/* Return true if point is at the end of a line. */
int
eolp (void)
{
  Point pt = get_buffer_pt (cur_bp);
  return pt.o == astr_len (pt.p->text);
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
