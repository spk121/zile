/* Kill ring facility functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
   All rights reserved.

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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "zile.h"
#include "extern.h"

static char *kill_ring_text;
static size_t kill_ring_size;
static size_t kill_ring_maxsize;

static void flush_kill_ring(void)
{
  kill_ring_size = 0;
  if (kill_ring_maxsize > 1024) {
    /*
     * Deallocate memory area, since is big enough.
     */
    kill_ring_maxsize = 0;
    free(kill_ring_text);
    kill_ring_text = NULL;
  }
}

static void kill_ring_push(int c)
{
  if (kill_ring_text == NULL) {
    kill_ring_maxsize = 16;
    kill_ring_text = (char *)zmalloc(kill_ring_maxsize);
  } else if (kill_ring_size + 1 >= kill_ring_maxsize) {
    kill_ring_maxsize += 16;
    kill_ring_text = (char *)zrealloc(kill_ring_text, kill_ring_maxsize);
  }
  kill_ring_text[kill_ring_size++] = (char)c;
}

static void kill_ring_push_nstring(char *s, size_t size)
{
  if (kill_ring_text == NULL) {
    kill_ring_maxsize = size;
    kill_ring_text = (char *)zmalloc(size);
  } else if (kill_ring_size + (int)size >= kill_ring_maxsize) {
    kill_ring_maxsize += size;
    kill_ring_text = (char *)zrealloc(kill_ring_text, kill_ring_maxsize);
  }
  memcpy(kill_ring_text + kill_ring_size, s, size);
  kill_ring_size += size;
}

static int kill_line(int literally)
{
  if (!eolp()) {
    if (warn_if_readonly_buffer())
      return FALSE;

    undo_save(UNDO_INSERT_BLOCK, cur_bp->pt,
              astr_len(cur_bp->pt.p->item) - cur_bp->pt.o, 0);
    undo_nosave = TRUE;
    while (!eolp()) {
      kill_ring_push(following_char());
      FUNCALL(delete_char);
    }
    undo_nosave = FALSE;

    thisflag |= FLAG_DONE_KILL;

    if (!literally)
      return TRUE;
  }

  if (list_next(cur_bp->pt.p) != cur_bp->lines) {
    if (!FUNCALL(delete_char))
      return FALSE;

    kill_ring_push('\n');

    thisflag |= FLAG_DONE_KILL;

    return TRUE;
  }

  minibuf_error("End of buffer");

  return FALSE;
}

DEFUN("kill-line", kill_line)
/*+
Kill the rest of the current line; if no nonblanks there, kill thru newline.
With prefix argument, kill that many lines from point.
+*/
{
  int uni, ret = TRUE;

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_ring();

  if (!(lastflag & FLAG_SET_UNIARG))
    kill_line(lookup_bool_variable("kill-whole-line"));
  else {
    undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
    for (uni = 0; uni < uniarg; ++uni)
      if (!kill_line(TRUE)) {
        ret = FALSE;
        break;
      }
    undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
  }

  deactivate_mark();
  return ret;
}
END_DEFUN

DEFUN("kill-region", kill_region)
/*+
Kill between point and mark.
The text is deleted but saved in the kill ring.
The command C-y (yank) can retrieve it from there.
If the buffer is read-only, Zile will beep and refrain from deleting
the text, but put the text in the kill ring anyway.  This means that
you can use the killing commands to copy text from a read-only buffer.
If the previous command was also a kill command,
the text killed this time appends to the text killed last time
to make one entry in the kill ring.
+*/
{
  Region r;

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_ring();

  if (warn_if_no_mark())
    return FALSE;

  calculate_the_region(&r);

  if (cur_bp->flags & BFLAG_READONLY) {
    /*
     * The buffer is readonly; save only in the kill buffer
     * and complain.
     */
    char *p;

    p = copy_text_block(r.start.n, r.start.o, r.size);
    kill_ring_push_nstring(p, r.size);
    free(p);

    warn_if_readonly_buffer();
  } else {
    size_t size = r.size;

    if (cur_bp->pt.p != r.start.p || r.start.o != cur_bp->pt.o)
      FUNCALL(exchange_point_and_mark);
    undo_save(UNDO_INSERT_BLOCK, cur_bp->pt, size, 0);
    undo_nosave = TRUE;
    while (size--) {
      if (!eolp())
        kill_ring_push(following_char());
      else
        kill_ring_push('\n');
      FUNCALL(delete_char);
    }
    undo_nosave = FALSE;
  }

  thisflag |= FLAG_DONE_KILL;
  deactivate_mark();

  return TRUE;
}
END_DEFUN

DEFUN("copy-region-as-kill", copy_region_as_kill)
/*+
Save the region as if killed, but don't kill it.
+*/
{
  Region r;
  char *p;

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_ring();

  if (warn_if_no_mark())
    return FALSE;

  calculate_the_region(&r);

  p = copy_text_block(r.start.n, r.start.o, r.size);
  kill_ring_push_nstring(p, r.size);
  free(p);

  thisflag |= FLAG_DONE_KILL;
  deactivate_mark();

  return TRUE;
}
END_DEFUN

DEFUN("kill-word", kill_word)
/*+
Kill characters forward until encountering the end of a word.
With argument, do this that many times.
+*/
{
  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_ring();

  if (warn_if_readonly_buffer())
    return FALSE;

  push_mark();
  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  FUNCALL_ARG(mark_word, uniarg);
  FUNCALL(kill_region);
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
  pop_mark();

  thisflag |= FLAG_DONE_KILL;

  minibuf_write("");	/* Don't write "Set mark" message.  */

  return TRUE;
}
END_DEFUN

DEFUN("backward-kill-word", backward_kill_word)
/*+
Kill characters backward until encountering the end of a word.
With argument, do this that many times.
+*/
{
  return FUNCALL_ARG(kill_word, !uniarg ? -1 : -uniarg);
}
END_DEFUN

DEFUN("kill-sexp", kill_sexp)
/*+
Kill the sexp (balanced expression) following the cursor.
With ARG, kill that many sexps after the cursor.
Negative arg -N means kill N sexps before the cursor.
+*/
{
  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_ring();

  if (warn_if_readonly_buffer())
    return FALSE;

  push_mark();
  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  FUNCALL_ARG(mark_sexp, uniarg);
  FUNCALL(kill_region);
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
  pop_mark();

  thisflag |= FLAG_DONE_KILL;

  minibuf_write("");	/* Don't write "Set mark" message.  */

  return TRUE;
}
END_DEFUN

DEFUN("yank", yank)
/*+
Reinsert the last stretch of killed text.
More precisely, reinsert the stretch of killed text most recently
killed OR yanked.  Put point at end, and set mark at beginning.
+*/
{
  if (kill_ring_size == 0) {
    minibuf_error("Kill ring is empty");
    return FALSE;
  }

  if (warn_if_readonly_buffer())
    return FALSE;

  set_mark_command();

  undo_save(UNDO_REMOVE_BLOCK, cur_bp->pt, kill_ring_size, 0);
  undo_nosave = TRUE;
  insert_nstring(kill_ring_text, kill_ring_size);
  undo_nosave = FALSE;

  deactivate_mark();

  return TRUE;
}
END_DEFUN

void free_kill_ring(void)
{
  free(kill_ring_text);
}
