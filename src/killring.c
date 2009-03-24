/* Kill ring facility functions

   Copyright (c) 2008, 2009 Free Software Foundation, Inc.

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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "extern.h"

static astr kill_ring_text;

void
free_kill_ring (void)
{
  if (kill_ring_text != NULL)
    astr_delete (kill_ring_text);
  kill_ring_text = NULL;
}

static void
kill_ring_push (int c)
{
  if (kill_ring_text == NULL)
    kill_ring_text = astr_new ();
  astr_cat_char (kill_ring_text, c);
}

static void
kill_ring_push_nstring (char *s, size_t size)
{
  if (kill_ring_text == NULL)
    kill_ring_text = astr_new ();
  astr_ncat_cstr (kill_ring_text, s, size);
}

static bool
copy_or_kill_region (bool kill, Region * r)
{
  char *p = copy_text_block (r->start.n, r->start.o, r->size);

  if (!(lastflag & FLAG_DONE_KILL))
    free_kill_ring ();
  kill_ring_push_nstring (p, r->size);
  free (p);

  if (kill)
    {
      if (get_buffer_readonly (cur_bp))
        minibuf_error ("Read only text copied to kill ring");
      else
        assert (delete_region (r));
    }

  thisflag |= FLAG_DONE_KILL;
  deactivate_mark ();

  return true;
}

static int
kill_line (int literally)
{
  if (eobp ())
    {
      minibuf_error ("End of buffer");
      return false;
    }

  if (!eolp ())
    {
      Region r;

      r.start = get_buffer_pt (cur_bp);
      r.size = astr_len (get_buffer_pt (cur_bp).p->text) - get_buffer_pt (cur_bp).o;

      if (!copy_or_kill_region (true, &r))
        return false;

      if (!literally)
        return true;
    }

  if (!eobp ())
    {
      if (!FUNCALL (delete_char))
        return false;

      kill_ring_push ('\n');
      thisflag |= FLAG_DONE_KILL;
    }

  return true;
}

static int
kill_line_literally (void)
{
  return kill_line (true);
}

DEFUN ("kill-line", kill_line)
/*+
Kill the rest of the current line; if no nonblanks there, kill thru newline.
With prefix argument, kill that many lines from point.
+*/
{
  le * ret = leT;

  if (!(lastflag & FLAG_DONE_KILL))
    free_kill_ring ();

  if (!(lastflag & FLAG_SET_UNIARG))
    kill_line (get_variable_bool ("kill-whole-line"));
  else
    ret = execute_with_uniarg (true, uniarg, kill_line_literally, NULL);

  deactivate_mark ();
}
END_DEFUN

static bool
copy_or_kill_the_region (bool kill)
{
  Region r;

  if (!calculate_the_region (&r))
    return false;

  return copy_or_kill_region (kill, &r);
}

DEFUN ("kill-region", kill_region)
/*+
Kill between point and mark.
The text is deleted but saved in the kill ring.
The command @kbd{C-y} (yank) can retrieve it from there.
If the buffer is read-only, Zile will beep and refrain from deleting
the text, but put the text in the kill ring anyway.  This means that
you can use the killing commands to copy text from a read-only buffer.
If the previous command was also a kill command,
the text killed this time appends to the text killed last time
to make one entry in the kill ring.
+*/
{
  ok = bool_to_lisp (copy_or_kill_the_region (true));
}
END_DEFUN

DEFUN ("copy-region-as-kill", copy_region_as_kill)
/*+
Save the region as if killed, but don't kill it.
+*/
{
  ok = bool_to_lisp (copy_or_kill_the_region (false));
}
END_DEFUN

static le *
kill (int uniarg, Function mark_func)
{
  if (!(lastflag & FLAG_DONE_KILL))
    free_kill_ring ();

  if (warn_if_readonly_buffer ())
    return leNIL;

  push_mark ();
  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
  mark_func (uniarg, NULL);
  FUNCALL (kill_region);
  undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
  pop_mark ();

  thisflag |= FLAG_DONE_KILL;
  minibuf_write ("");		/* Erase "Set mark" message.  */
  return leT;
}

DEFUN ("kill-word", kill_word)
/*+
Kill characters forward until encountering the end of a word.
With argument, do this that many times.
+*/
{
  ok = kill (uniarg, F_mark_word);
}
END_DEFUN

DEFUN ("backward-kill-word", backward_kill_word)
/*+
Kill characters backward until encountering the end of a word.
With argument, do this that many times.
+*/
{
  ok = kill (-uniarg, F_mark_word);
}
END_DEFUN

DEFUN ("kill-sexp", kill_sexp)
/*+
Kill the sexp (balanced expression) following the cursor.
With ARG, kill that many sexps after the cursor.
Negative arg -N means kill N sexps before the cursor.
+*/
{
  ok = kill (uniarg, F_mark_sexp);
}
END_DEFUN

DEFUN ("yank", yank)
/*+
Reinsert the last stretch of killed text.
More precisely, reinsert the stretch of killed text most recently
killed @i{or} yanked.  Put point at end, and set mark at beginning.
+*/
{
  if (kill_ring_text == NULL)
    {
      minibuf_error ("Kill ring is empty");
      return leNIL;
    }

  if (warn_if_readonly_buffer ())
    return leNIL;

  set_mark_interactive ();

  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), 0,
             astr_len (kill_ring_text));
  undo_nosave = true;
  insert_astr (kill_ring_text);
  undo_nosave = false;

  deactivate_mark ();
}
END_DEFUN
