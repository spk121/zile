/* Undo facility functions

   Copyright (c) 1997-2004, 2008-2011 Free Software Foundation, Inc.

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

/*
 * Undo action
 */
struct Undo
{
  Undo *next;      /* Next undo delta in list. */
  int type;        /* The type of undo delta. */
  size_t o;        /* Buffer offset of the undo delta. */
  bool unchanged;  /* Flag indicating that reverting this undo leaves
                      the buffer in an unchanged state. */
  estr text;       /* Old text. */
  size_t size;     /* Size of replacement text. */
};

/* Undo delta types. */
enum
{
  UNDO_REPLACE_BLOCK,		/* Replace a block of characters. */
  UNDO_START_SEQUENCE,		/* Start a multi operation sequence. */
  UNDO_END_SEQUENCE		/* End a multi operation sequence. */
};

/*
 * Save a reverse delta for doing undo.
 */
static void
undo_save (int type, size_t o, size_t osize, size_t size)
{
  if (get_buffer_noundo (cur_bp))
    return;

  Undo * up = (Undo *) XZALLOC (Undo);
  *up = (Undo) {
    .next = get_buffer_last_undop (cur_bp),
    .type = type,
  };

  up->o = o;

  if (type == UNDO_REPLACE_BLOCK)
    {
      up->size = size;
      up->text = get_buffer_region (cur_bp, region_new (o, o + osize));
      up->unchanged = !get_buffer_modified (cur_bp);
    }

  set_buffer_last_undop (cur_bp, up);
}

void
undo_start_sequence (void)
{
  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
}

void
undo_end_sequence (void)
{
  undo_save (UNDO_END_SEQUENCE, 0, 0, 0);
}

void
undo_save_block (size_t o, size_t osize, size_t size)
{
  undo_save (UNDO_REPLACE_BLOCK, o, osize, size);
}

/*
 * Revert an action.  Return the next undo entry.
 */
static Undo *
revert_action (Undo * up)
{
  if (up->type == UNDO_END_SEQUENCE)
    {
      undo_start_sequence ();
      up = up->next;
      while (up->type != UNDO_START_SEQUENCE)
        up = revert_action (up);
      undo_end_sequence ();
    }

  if (up->type != UNDO_END_SEQUENCE)
    goto_offset (up->o);
  if (up->type == UNDO_REPLACE_BLOCK)
    replace_estr (up->size, up->text);
  if (up->unchanged)
    set_buffer_modified (cur_bp, false);

  return up->next;
}

DEFUN ("undo", undo)
/*+
Undo some previous changes.
Repeat this command to undo more changes.
+*/
{
  if (get_buffer_noundo (cur_bp))
    {
      minibuf_error ("Undo disabled in this buffer");
      return leNIL;
    }

  if (warn_if_readonly_buffer ())
    return leNIL;

  if (get_buffer_next_undop (cur_bp) == NULL)
    {
      minibuf_error ("No further undo information");
      set_buffer_next_undop (cur_bp, get_buffer_last_undop (cur_bp));
      return leNIL;
    }

  set_buffer_next_undop (cur_bp, revert_action (get_buffer_next_undop (cur_bp)));
  minibuf_write ("Undo!");
}
END_DEFUN

/*
 * Set unchanged flags to false.
 */
void
undo_set_unchanged (Undo *up)
{
  for (; up; up = up->next)
    up->unchanged = false;
}
