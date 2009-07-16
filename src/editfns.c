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
  if (get_buffer_mark (cur_bp))
    gl_list_add_last (mark_ring, copy_marker (get_buffer_mark (cur_bp)));
  else
    { /* Save an invalid mark.  */
      Marker *m = point_min_marker ();
      Point pt = get_marker_pt (m);
      pt.p = NULL;
      set_marker_pt (m, pt);
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
  if (get_buffer_mark (get_marker_bp (m)))
    free_marker (get_buffer_mark (get_marker_bp (m)));

  set_buffer_mark (get_marker_bp (m), copy_marker (m));

  assert (gl_list_remove_at (mark_ring, gl_list_size (mark_ring) - 1));
  free_marker (m);
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

bool
is_empty_line (void)
{
  Point pt = get_buffer_pt (cur_bp);
  return astr_len (get_line_text (pt.p)) == 0;
}

bool
is_blank_line (void)
{
  Point pt = get_buffer_pt (cur_bp);
  size_t c;
  for (c = 0; c < astr_len (get_line_text (pt.p)); c++)
    if (!isspace ((int) astr_get (get_line_text (pt.p), c)))
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
      return astr_get (get_line_text (pt.p), pt.o);
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
      return astr_get (get_line_text (pt.p), pt.o - 1);
    }
}

/* Return true if point is at the beginning of the buffer. */
bool
bobp (void)
{
  Point pt = get_buffer_pt (cur_bp);
  return (get_line_prev (pt.p) == get_buffer_lines (cur_bp) && pt.o == 0);
}

/* Return true if point is at the end of the buffer. */
bool
eobp (void)
{
  Point pt = get_buffer_pt (cur_bp);
  return (get_line_next (pt.p) == get_buffer_lines (cur_bp) &&
          pt.o == astr_len (get_line_text (pt.p)));
}

/* Return true if point is at the beginning of a line. */
bool
bolp (void)
{
  Point pt = get_buffer_pt (cur_bp);
  return pt.o == 0;
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
