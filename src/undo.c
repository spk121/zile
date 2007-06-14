/* Undo facility functions
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

/* This variable is set to TRUE when the `undo_save()' function should not
   register the undo information. */
int undo_nosave = FALSE;

/* This variable is set to TRUE when the undo is in execution. */
static int doing_undo = FALSE;

/*
 * Save a reverse delta for doing undo.
 */
void undo_save(int type, Point pt, size_t arg1, size_t arg2)
{
  Undo *up;

  if (cur_bp->flags & BFLAG_NOUNDO || undo_nosave)
    return;

  up = (Undo *)zmalloc(sizeof(Undo));

  up->type = type;
  up->pt = pt;

  /* If the buffer is currently unchanged, record the fact. */
  if (!(cur_bp->flags & BFLAG_MODIFIED))
    up->unchanged = TRUE;

  switch (type) {
  case UNDO_REPLACE_CHAR:
  case UNDO_INTERCALATE_CHAR:
    up->delta.c = (char)arg1;
    break;
  case UNDO_INSERT_BLOCK:
    up->delta.block.size = arg1;
    up->delta.block.text = copy_text_block(pt.n, pt.o, arg1);
    break;
  case UNDO_REPLACE_BLOCK:
    up->delta.block.osize = arg1;
    up->delta.block.size = arg2;
    up->delta.block.text = copy_text_block(pt.n, pt.o, arg1);
    break;
  case UNDO_REMOVE_BLOCK:
    up->delta.block.size = arg1;
    break;
  case UNDO_START_SEQUENCE:
  case UNDO_END_SEQUENCE:
    break;
  }

  up->next = cur_bp->last_undop;
  cur_bp->last_undop = up;

  if (!doing_undo)
    cur_bp->next_undop = up;
}

/*
 * Revert an action.  Return the next undo entry.
 */
static Undo *revert_action(Undo *up)
{
  size_t i;

  doing_undo = TRUE;

  if (up->type == UNDO_END_SEQUENCE) {
    undo_save(UNDO_START_SEQUENCE, up->pt, 0, 0);
    up = up->next;
    while (up->type != UNDO_START_SEQUENCE) {
      up = revert_action(up);
    }
    undo_save(UNDO_END_SEQUENCE, up->pt, 0, 0);
    goto_point(up->pt);
    return up->next;
  }

  goto_point(up->pt);

  switch (up->type) {
  case UNDO_INTERCALATE_CHAR:
    if (up->delta.c == '\n')
      intercalate_newline();
    else
      intercalate_char(up->delta.c);
    break;
  case UNDO_INSERT_BLOCK:
    undo_save(UNDO_REMOVE_BLOCK, up->pt, up->delta.block.size, 0);
    undo_nosave = TRUE;
    for (i = 0; i < up->delta.block.size; ++i)
      if (up->delta.block.text[i] != '\n')
        insert_char(up->delta.block.text[i]);
      else
        insert_newline();
    undo_nosave = FALSE;
    break;
  case UNDO_REMOVE_CHAR:
    delete_char();
    break;
  case UNDO_REMOVE_BLOCK:
    undo_save(UNDO_INSERT_BLOCK, up->pt, up->delta.block.size, 0);
    undo_nosave = TRUE;
    for (i = 0; i < up->delta.block.size; ++i)
      delete_char();
    undo_nosave = FALSE;
    break;
  case UNDO_REPLACE_CHAR:
    undo_save(UNDO_REPLACE_CHAR, up->pt,
              (size_t)(*astr_char(cur_bp->pt.p->item,
                                    (ptrdiff_t)up->pt.o)), 0);
    *astr_char(cur_bp->pt.p->item, (ptrdiff_t)up->pt.o) = up->delta.c;
    cur_bp->flags |= BFLAG_MODIFIED;
    break;
  case UNDO_REPLACE_BLOCK:
    undo_save(UNDO_REPLACE_BLOCK, up->pt,
              up->delta.block.size, up->delta.block.osize);
    undo_nosave = TRUE;
    for (i = 0; i < up->delta.block.size; ++i)
      delete_char();
    for (i = 0; i < up->delta.block.osize; ++i)
      if (up->delta.block.text[i] != '\n')
        insert_char(up->delta.block.text[i]);
      else
        insert_newline();
    undo_nosave = FALSE;
    break;
  }

  doing_undo = FALSE;

  /* If reverting this undo action leaves the buffer unchanged,
     unset the modified flag. */
  if (up->unchanged)
    cur_bp->flags &= ~BFLAG_MODIFIED;

  return up->next;
}

DEFUN("undo", undo)
/*+
Undo some previous changes.
Repeat this command to undo more changes.
+*/
{
  if (cur_bp->flags & BFLAG_NOUNDO) {
    minibuf_error("Undo disabled in this buffer");
    return FALSE;
  }

  if (warn_if_readonly_buffer())
    return FALSE;

  if (cur_bp->next_undop == NULL) {
    minibuf_error("No further undo information");
    cur_bp->next_undop = cur_bp->last_undop;
    return FALSE;
  }

  cur_bp->next_undop = revert_action(cur_bp->next_undop);

  minibuf_write("Undo!");

  return TRUE;
}
END_DEFUN
