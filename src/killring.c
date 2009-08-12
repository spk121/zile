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
copy_or_kill_region (bool kill, Region * rp)
{
  char *p = copy_text_block (get_region_start (rp).n, get_region_start (rp).o, get_region_size (rp));

  if (last_command () != F_kill_region)
    free_kill_ring ();
  kill_ring_push_nstring (p, get_region_size (rp));
  free (p);

  if (kill)
    {
      if (get_buffer_readonly (cur_bp))
        minibuf_error ("Read only text copied to kill ring");
      else
        assert (delete_region (rp));
    }

  set_this_command (F_kill_region);
  deactivate_mark ();

  return true;
}

static bool
kill_to_bol (void)
{
  bool ok = true;

  if (!bolp ())
    {
      Region * rp = region_new ();
      Point pt = get_buffer_pt (cur_bp);

      set_region_size (rp, pt.o);
      pt.o = 0;
      set_region_start (rp, pt);

      ok = copy_or_kill_region (true, rp);
      free (rp);
    }

  return ok;
}

static bool
kill_line (bool literally)
{
  bool ok = true;
  bool start_of_line = bolp ();

  if (eobp ())
    {
      minibuf_error ("End of buffer");
      return false;
    }

  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);

  if (!eolp ())
    {
      Region * rp = region_new ();

      set_region_start (rp, get_buffer_pt (cur_bp));
      set_region_size (rp, astr_len (get_line_text (get_buffer_pt (cur_bp).p)) - get_buffer_pt (cur_bp).o);

      ok = copy_or_kill_region (true, rp);
      free (rp);
    }

  if (ok && (literally && start_of_line) && !eobp ())
    {
      if (!FUNCALL (delete_char))
        return false;

      kill_ring_push ('\n');
      set_this_command (F_kill_region);
    }

  undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);

  return ok;
}

static bool
kill_line_literally (void)
{
  return kill_line (true);
}

static bool
kill_line_backward (void)
{
  return previous_line () && kill_line_literally ();
}

DEFUN_ARGS ("kill-line", kill_line,
            INT_OR_UNIARG (arg))
/*+
Kill the rest of the current line; if no nonblanks there, kill thru newline.
With prefix argument @i{arg}, kill that many lines from point.
Negative arguments kill lines backward.
With zero argument, kills the text before point on the current line.

If `kill-whole-line' is non-nil, then this command kills the whole line
including its terminating newline, when used at the beginning of a line
with no argument.
+*/
{
  if (last_command () != F_kill_region)
    free_kill_ring ();

  INT_OR_UNIARG_INIT (arg);

  if (noarg)
    ok = bool_to_lisp (kill_line (get_variable_bool ("kill-whole-line")));
  else
    {
      undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
      if (arg <= 0)
        ok = bool_to_lisp (kill_to_bol ());
      if (arg != 0 && ok == leT)
        ok = execute_with_uniarg (true, arg, kill_line_literally, kill_line_backward);
      undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
    }

  deactivate_mark ();
}
END_DEFUN

static bool
copy_or_kill_the_region (bool kill)
{
  Region * rp = region_new ();
  bool ok = false;

  if (calculate_the_region (rp))
    ok = copy_or_kill_region (kill, rp);

  free (rp);
  return ok;
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
kill_text (int uniarg, Function mark_func)
{
  if (last_command () != F_kill_region)
    free_kill_ring ();

  if (warn_if_readonly_buffer ())
    return leNIL;

  push_mark ();
  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
  mark_func (uniarg, NULL);
  FUNCALL (kill_region);
  undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
  pop_mark ();

  set_this_command (F_kill_region);
  minibuf_write ("");		/* Erase "Set mark" message.  */
  return leT;
}

DEFUN_ARGS ("kill-word", kill_word,
            INT_OR_UNIARG (arg))
/*+
Kill characters forward until encountering the end of a word.
With argument @i{arg}, do this that many times.
+*/
{
  INT_OR_UNIARG_INIT (arg);
  ok = kill_text (arg, F_mark_word);
}
END_DEFUN

DEFUN_ARGS ("backward-kill-word", backward_kill_word,
            INT_OR_UNIARG (arg))
/*+
Kill characters backward until encountering the end of a word.
With argument @i{arg}, do this that many times.
+*/
{
  INT_OR_UNIARG_INIT (arg);
  ok = kill_text (-arg, F_mark_word);
}
END_DEFUN

DEFUN ("kill-sexp", kill_sexp)
/*+
Kill the sexp (balanced expression) following the cursor.
With @i{arg}, kill that many sexps after the cursor.
Negative arg -N means kill N sexps before the cursor.
+*/
{
  ok = kill_text (uniarg, F_mark_sexp);
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
