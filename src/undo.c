/* Undo facility functions

   Copyright (c) 1997-2004, 2008-2010, 2011 Free Software Foundation, Inc.

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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"

/*
 * Undo action
 */
struct Undo
{
  /* Next undo delta in list. */
  Undo *next;

  /* The type of undo delta. */
  int type;

  /* Location of the undo delta. Stored as a numeric position because
     line pointers can change. */
  size_t n, o;

  /* Flag indicating that reverting this undo leaves the buffer
     in an unchanged state. */
  bool unchanged;

  /* The block to insert. */
  struct
  {
    astr text;
    size_t osize;		/* Original size. */
    size_t size;		/* New block size. */
  } block;
};

/* Setting this variable to true stops undo_save saving the given
   information. */
int undo_nosave = false;

/* This variable is set to true when an undo is in execution. */
static int doing_undo = false;

/*
 * Save a reverse delta for doing undo.
 */
void
undo_save (int type, Point pt, size_t osize, size_t size)
{
  if (get_buffer_noundo (cur_bp) || undo_nosave)
    return;

  Undo * up = (Undo *) XZALLOC (Undo);
  *up = (Undo) {.type = type, .n = pt.n, .o = pt.o};
  if (!get_buffer_modified (cur_bp))
    up->unchanged = true;

  if (type == UNDO_REPLACE_BLOCK)
    {
      Line * lp = get_buffer_pt (cur_bp).p;
      size_t n = get_buffer_pt (cur_bp).n;

      if (n > pt.n)
        do
          lp = get_line_prev (lp);
        while (--n > pt.n);
      else if (n < pt.n)
        do
          lp = get_line_next (lp);
        while (++n < pt.n);

      pt.p = lp;
      up->block.osize = osize;
      up->block.size = size;
      up->block.text = copy_text_block (pt, osize);
    }

  up->next = get_buffer_last_undop (cur_bp);
  set_buffer_last_undop (cur_bp, up);

  if (!doing_undo)
    set_buffer_next_undop (cur_bp, up);
}

/*
 * Revert an action.  Return the next undo entry.
 */
static Undo *
revert_action (Undo * up)
{
  Point pt = {.n = up->n, .o = up->o};

  doing_undo = true;

  if (up->type == UNDO_END_SEQUENCE)
    {
      undo_save (UNDO_START_SEQUENCE, pt, 0, 0);
      up = up->next;
      while (up->type != UNDO_START_SEQUENCE)
        up = revert_action (up);
      pt = (Point) {.n = up->n, .o = up->o};
      undo_save (UNDO_END_SEQUENCE, pt, 0, 0);
      goto_point (pt);
      return up->next;
    }

  goto_point (pt);

  if (up->type == UNDO_REPLACE_BLOCK)
    {
      undo_save (UNDO_REPLACE_BLOCK, pt,
                 up->block.size, up->block.osize);
      undo_nosave = true;
      for (size_t i = 0; i < up->block.size; ++i)
        delete_char ();
      insert_nstring (astr_cstr (up->block.text), up->block.osize);
      undo_nosave = false;
    }

  doing_undo = false;

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
