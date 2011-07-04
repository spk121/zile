/* Kill ring facility functions

   Copyright (c) 2001, 2003-2011 Free Software Foundation, Inc.

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
#include <stdarg.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"

static estr kill_ring_text;

static void
maybe_free_kill_ring (void)
{
  if (last_command () != F_kill_region)
    kill_ring_text.as = NULL;
}

static void
kill_ring_push (estr es)
{
  if (kill_ring_text.as == NULL)
    {
      kill_ring_text.as = astr_new ();
      kill_ring_text.eol = coding_eol_lf;
    }
  estr_cat (kill_ring_text, es);
}

static bool
copy_or_kill_region (bool kill, Region r)
{
  kill_ring_push (get_buffer_region (cur_bp, r));

  if (kill)
    {
      if (get_buffer_readonly (cur_bp))
        minibuf_error ("Read only text copied to kill ring");
      else
        assert (delete_region (r));
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
      Region r;
      Point pt = get_buffer_pt (cur_bp);

      set_region_end (&r, pt);
      pt.o = 0;
      set_region_start (&r, pt);

      ok = copy_or_kill_region (true, r);
    }

  return ok;
}

static bool
kill_line (bool whole_line)
{
  bool ok = true;
  bool only_blanks_to_end_of_line = false;
  Point cur_pt = get_buffer_pt (cur_bp);
  castr cur_line = get_line_text (cur_pt.p);

  if (!whole_line)
    {
      size_t i;

      for (i = cur_pt.o; i < astr_len (cur_line); i++)
        {
          char c = astr_get (cur_line, i);
          if (!(c == ' ' || c == '\t'))
            break;
        }

      if (i == astr_len (cur_line))
        only_blanks_to_end_of_line = true;
    }

  if (eobp ())
    {
      minibuf_error ("End of buffer");
      return false;
    }

  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);

  if (!eolp ())
    {
      Region r;
      Point pt = get_buffer_pt (cur_bp);

      set_region_start (&r, pt);
      pt.o = astr_len (cur_line);
      set_region_end (&r, pt);

      ok = copy_or_kill_region (true, r);
    }

  if (ok && (whole_line || only_blanks_to_end_of_line) && !eobp ())
    {
      if (!FUNCALL (delete_char))
        return false;

      estr es = (estr) {.as = astr_new_cstr ("\n"), .eol = get_buffer_text (cur_bp).eol};
      kill_ring_push (es);
      set_this_command (F_kill_region);
    }

  undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);

  return ok;
}

static bool
kill_whole_line (void)
{
  return kill_line (true);
}

static bool
kill_line_backward (void)
{
  return previous_line () && kill_whole_line ();
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
  maybe_free_kill_ring ();

  INT_OR_UNIARG_INIT (arg);

  if (noarg)
    ok = bool_to_lisp (kill_line (bolp () && get_variable_bool ("kill-whole-line")));
  else
    {
      undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
      if (arg <= 0)
        ok = bool_to_lisp (kill_to_bol ());
      if (arg != 0 && ok == leT)
        ok = execute_with_uniarg (true, arg, kill_whole_line, kill_line_backward);
      undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
    }

  deactivate_mark ();
}
END_DEFUN

static bool
copy_or_kill_the_region (bool kill)
{
  bool ok = false;

  if (!warn_if_no_mark ())
    {
      Region r = calculate_the_region ();
      maybe_free_kill_ring ();
      ok = copy_or_kill_region (kill, r);
    }

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
  maybe_free_kill_ring ();

  if (warn_if_readonly_buffer ())
    return leNIL;

  push_mark ();
  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
  mark_func (uniarg, true, NULL);
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
  if (kill_ring_text.as == NULL)
    {
      minibuf_error ("Kill ring is empty");
      return leNIL;
    }

  if (warn_if_readonly_buffer ())
    return leNIL;

  set_mark_interactive ();

  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), 0,
             astr_len (kill_ring_text.as));
  undo_nosave = true;
  insert_estr (kill_ring_text);
  undo_nosave = false;

  deactivate_mark ();
}
END_DEFUN
