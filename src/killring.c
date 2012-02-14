/* Kill ring facility functions

   Copyright (c) 2001, 2003-2011 Free Software Foundation, Inc.
   Copyright (c) 2012 Michael L. Gran

   This file is part of Michael Gran's unofficial fork ok GNU Zile.

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
#include <libguile.h>
#include <stdarg.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"

static estr kill_ring_text;

static void
maybe_free_kill_ring (void)
{
  if (!scm_is_eq (last_command (), F_kill_region ()))
    kill_ring_text.as = NULL;
}

static void
kill_ring_push (estr es)
{
  if (kill_ring_text.as == NULL)
    kill_ring_text = (estr) {.as = astr_new (), .eol = coding_eol_lf};
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

  set_this_command (F_kill_region ());
  deactivate_mark ();

  return true;
}

static bool
kill_line (bool whole_line)
{
  bool ok = true;
  bool only_blanks_to_end_of_line = false;
  size_t cur_line_len = buffer_line_len (cur_bp, get_buffer_pt (cur_bp));

  if (!whole_line)
    {
      size_t i;
      for (i = get_buffer_pt (cur_bp) - get_buffer_line_o (cur_bp); i < cur_line_len; i++)
        {
          char c = get_buffer_char (cur_bp, get_buffer_line_o (cur_bp) + i);
          if (!(c == ' ' || c == '\t'))
            break;
        }

      only_blanks_to_end_of_line = i == cur_line_len;
    }

  if (eobp ())
    {
      minibuf_error ("End of buffer");
      return false;
    }

  undo_start_sequence ();

  if (!eolp ())
    ok = copy_or_kill_region (true, region_new (get_buffer_pt (cur_bp), get_buffer_line_o (cur_bp) + cur_line_len));

  if (ok && (whole_line || only_blanks_to_end_of_line) && !eobp ())
    {
      if (!scm_is_true (G_delete_char (scm_from_int (1))))
        return false;

      kill_ring_push ((estr) {.as = astr_new_cstr ("\n"), .eol = coding_eol_lf});
      set_this_command (F_kill_region ());
    }

  undo_end_sequence ();

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

SCM_DEFINE (G_kill_line, "kill-line", 0, 1, 0, (SCM garg), "\
Kill the rest of the current line; if no nonblanks there, kill thru newline.\n\
With prefix argument @i{arg}, kill that many lines from point.\n\
Negative arguments kill lines backward.\n\
With zero argument, kills the text before point on the current line.\n\
\n\
If `kill-whole-line' is non-nil, then this command kills the whole line\n\
including its terminating newline, when used at the beginning of a line\n\
with no argument.")
{
  SCM ok = SCM_BOOL_T;
  bool kill_whole_line_var;
  long arg = guile_to_long_or_error (s_G_kill_line, SCM_ARG1, garg);
  maybe_free_kill_ring ();

  if (SCM_UNBNDP (garg))
    {
      kill_whole_line_var = get_variable_bool ("kill-whole-line");
      ok = scm_from_bool (kill_line (bolp () && kill_whole_line_var));
    }
  else
    {
      undo_start_sequence ();
      if (arg <= 0)
        ok = scm_from_bool (bolp () ||
                           copy_or_kill_region (true, region_new (get_buffer_line_o (cur_bp), get_buffer_pt (cur_bp))));
      if (arg != 0 && ok == SCM_BOOL_T)
        ok = execute_with_uniarg (false, arg, kill_whole_line, kill_line_backward);
      undo_end_sequence ();
    }

  deactivate_mark ();
  return ok;
}

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

SCM_DEFINE (G_kill_region, "kill-region", 0, 0, 0, (void), "\
Kill between point and mark.\n\
The text is deleted but saved in the kill ring.\n\
The command @kbd{C-y} (yank) can retrieve it from there.\n\
If the buffer is read-only, Zile will beep and refrain from deleting\n\
the text, but put the text in the kill ring anyway.  This means that\n\
you can use the killing commands to copy text from a read-only buffer.\n\
If the previous command was also a kill command,\n\
the text killed this time appends to the text killed last time\n\
to make one entry in the kill ring.")
{
 return scm_from_bool (copy_or_kill_the_region (true));
}

SCM_DEFINE (G_copy_region_as_kill, "copy-region-as-kill", 0, 0, 0, (void), "\
Save the region as if killed, but don't kill it.")
{
  return  scm_from_bool (copy_or_kill_the_region (false));
}


static SCM
kill_text (int uniarg, SCM (*mark_func)(SCM n))
{
  maybe_free_kill_ring ();

  if (warn_if_readonly_buffer ())
    return SCM_BOOL_F;

  push_mark ();
  undo_start_sequence ();
  mark_func (scm_from_int (uniarg));
  G_kill_region ();
  undo_end_sequence ();
  pop_mark ();

  set_this_command (F_kill_region ());
  minibuf_write ("");		/* Erase "Set mark" message.  */
  return SCM_BOOL_T;
}

SCM_DEFINE (G_kill_word, "kill-word", 0, 1, 0, (SCM garg), "\
Kill characters forward until encountering the end of a word.\n\
With argument @i{arg}, do this that many times.")
{
  long arg = guile_to_long_or_error (s_G_kill_word, SCM_ARG1, garg);
  return kill_text (arg, G_mark_word);
}

SCM_DEFINE (G_backward_kill_word, "backward-kill-word", 0, 1, 0, (SCM garg), "\
Kill characters backward until encountering the end of a word.\n\
With argument @i{arg}, do this that many times.")
{
  long arg = guile_to_long_or_error (s_G_kill_word, SCM_ARG1, garg);
  return kill_text (-arg, G_mark_word);
}

SCM_DEFINE (G_kill_sexp, "kill-sexp", 0, 1, 0, (SCM guniarg), "\
Kill the sexp (balanced expression) following the cursor.\n\
With @i{arg}, kill that many sexps after the cursor.\n\
Negative arg -N means kill N sexps before the cursor.")
{
  long uniarg = guile_to_long_or_error (s_G_kill_sexp, SCM_ARG1, guniarg);
  return kill_text (uniarg, G_mark_sexp);
}

SCM_DEFINE (G_yank, "yank", 0, 0, 0, (void), "\
Reinsert the last stretch of killed text.\n\
More precisely, reinsert the stretch of killed text most recently\n\
killed @i{or} yanked.  Put point at end, and set mark at beginning.")
{
  // FIXME: interactive logic
  if (kill_ring_text.as == NULL)
    {
      minibuf_error ("Kill ring is empty");
      return SCM_BOOL_F;
    }

  if (warn_if_readonly_buffer ())
    return SCM_BOOL_T;

  G_set_mark_command ();
  insert_estr (kill_ring_text);
  deactivate_mark ();
  return SCM_BOOL_T;
}

void
init_guile_killring_procedures (void)
{
#include "killring.x"
  scm_c_export ("kill-line",
		"kill-region",
		"copy-region-as-kill",
		"kill-word",
		"backward-kill-word",
		"kill-sexp",
		"yank",
		NULL);
}
