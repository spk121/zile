/* Miscellaneous Emacs functions

   Copyright (c) 1997-2011 Free Software Foundation, Inc.
   Copyright (c) 2011, 2012 Michael L Gran

   This file is part of Michael Gran's unofficial fork of GNU Zile.

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

#include <ctype.h>
#include <libguile.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "pipe-filter.h"

#include "main.h"
#include "extern.h"


SCM_DEFINE (G_suspend_emacs, "suspend-emacs", 0, 0, 0, (void), "\
Stop Zile and return to superior process.")
{
  raise (SIGTSTP);
  return SCM_BOOL_F;
}

SCM_DEFINE (G_keyboard_quit, "keyboard-quit", 0, 0, 0, (void), "\
Cancel current command.")
{
  deactivate_mark ();
  minibuf_error ("Quit");
  return SCM_BOOL_F;
}

void
write_temp_buffer (const char *name, bool show, void (*func) (va_list ap), ...)
{
  /* Popup a window with the buffer "name". */
  Window *old_wp = cur_wp;
  Buffer *old_bp = cur_bp;
  Window *wp = find_window (name);
  if (show && wp)
    set_current_window (wp);
  else
    {
      Buffer *bp = find_buffer (name);
      if (show)
        set_current_window (popup_window ());
      if (bp == NULL)
        {
          bp = buffer_new ();
          set_buffer_name (bp, name);
        }
      switch_to_buffer (bp);
    }

  /* Remove the contents of that buffer. */
  Buffer *new_bp = buffer_new ();
  set_buffer_name (new_bp, get_buffer_name (cur_bp));
  kill_buffer (cur_bp);
  cur_bp = new_bp;
  set_window_bp (cur_wp, cur_bp);

  /* Make the buffer a temporary one. */
  set_buffer_needname (cur_bp, true);
  set_buffer_noundo (cur_bp, true);
  set_buffer_nosave (cur_bp, true);
  set_temporary_buffer (cur_bp);

  /* Use the "callback" routine. */
  va_list ap;
  va_start (ap, func);
  func (ap);
  va_end (ap);

  G_beginning_of_buffer ();
  set_buffer_readonly (cur_bp, true);
  set_buffer_modified (cur_bp, false);

  /* Restore old current window. */
  set_current_window (old_wp);

  /* If we're not showing the new buffer, switch back to the old one. */
  if (!show)
    switch_to_buffer (old_bp);
}

static void
write_buffers_list (va_list ap)
{
  Window *old_wp = va_arg (ap, Window *);
  Buffer *bp;

  /* FIXME: Underline next line properly. */
  bprintf ("CRM Buffer                Size  Mode             File\n");
  bprintf ("--- ------                ----  ----             ----\n");

  /* Print buffers. */
  bp = get_window_bp (old_wp);
  do
    {
      /* Print all buffers whose names don't start with space except
         this one (the *Buffer List*). */
      if (cur_bp != bp && get_buffer_name (bp)[0] == ' ')
        {
          bprintf ("%c%c%c %-19s %6u  %-17s",
                   get_window_bp (old_wp) == bp ? '.' : ' ',
                   get_buffer_readonly (bp) ? '%' : ' ',
                   get_buffer_modified (bp) ? '*' : ' ',
                   get_buffer_name (bp), get_buffer_size (bp), "Fundamental");
          if (get_buffer_filename (bp) != NULL)
            bprintf ("%s", astr_cstr (compact_path (astr_new_cstr (get_buffer_filename (bp)))));
          insert_newline ();
        }
      bp = get_buffer_next (bp);
      if (bp == NULL)
        bp = head_bp;
    }
  while (bp != get_window_bp (old_wp));
}

SCM_DEFINE (G_list_buffers, "list-buffers", 0, 0, 0, (void), "\
Display a list of names of existing buffers.\n\
The list is displayed in a buffer named `*Buffer List*'.\n\
Note that buffers with names starting with spaces are omitted.\n\
\n\
@itemize -\n\
The @samp{M} column contains a @samp{*} for buffers that are modified.\n\
The @samp{R} column contains a @samp{%} for buffers that are read-only.\n\
@end itemize")
{
  write_temp_buffer ("*Buffer List*", true, write_buffers_list, cur_wp);
  return SCM_BOOL_T;
}

SCM_DEFINE (G_toggle_read_only, "toggle-read-only", 0, 0, 0, (void), "\
Change whether this buffer is visiting its file read-only.")
{
  set_buffer_readonly (cur_bp, !get_buffer_readonly (cur_bp));
  return SCM_BOOL_T;
}

SCM_DEFINE (G_auto_fill_mode, "auto-fill-mode", 0, 0, 0, (void), "\
Toggle Auto Fill mode.\n\
In Auto Fill mode, inserting a space at a column beyond `fill-column'\n\
automatically breaks the line at a previous space.")
{
  set_buffer_autofill (cur_bp, !get_buffer_autofill (cur_bp));
  return SCM_BOOL_T;
}

SCM_DEFINE (G_set_fill_column, "set-fill-column", 0, 1, 0, (SCM n), "\
Set `fill-column' to specified argument.\n\
Use C-u followed by a number to specify a column.\n\
Just C-u as argument means to use the current column.")
{
  long c_n = guile_to_long_or_error (s_G_set_fill_column, SCM_ARG1, n);
  size_t o = get_buffer_pt (cur_bp) - get_buffer_line_o (cur_bp);
  size_t fill_col;
  char *buf = NULL;

  if (SCM_UNBNDP (n))
    {
      fill_col = minibuf_read_number ("Set fill-column to (default %d): ", o);
      if (fill_col == LONG_MAX)
        return SCM_BOOL_F;
      else if (fill_col == LONG_MAX - 1)
        fill_col = o;
    }
  else
    fill_col = c_n;

  buf = xasprintf ("%d", fill_col);
  /* Only print message when run interactively. */
  minibuf_write ("Fill column set to %s (was %d)", buf,
		 get_variable_number ("fill-column"));

  set_variable_number ("fill-column", c_n);
  return SCM_BOOL_T;
}


SCM_DEFINE (G_set_mark, "set-mark", 0, 0, 0, (void), "\
Set this buffer's mark to point.")
{
  set_mark ();
  activate_mark ();
  return SCM_BOOL_T;
}

SCM_DEFINE (G_set_mark_command, "set-mark-command", 0, 0, 0, (void), "\
Set the mark where point is.")
{
  SCM ok = SCM_BOOL_T;
  G_set_mark ();
  minibuf_write ("Mark set");
  return ok;
}

SCM_DEFINE (G_exchange_point_and_mark, "exchange-point-and-mark", 0, 0, 0, (void), "\
Put the mark where point is now, and point where the mark is now.")
{
  // FIXME: need interactive logic
  if (get_buffer_mark (cur_bp) == NULL)
    {
      minibuf_error ("No mark set in this buffer");
      return SCM_BOOL_F;
    }

  size_t o = get_buffer_pt (cur_bp);
  goto_offset (get_marker_o (get_buffer_mark (cur_bp)));
  set_marker_o (get_buffer_mark (cur_bp), o);
  activate_mark ();
  thisflag |= FLAG_NEED_RESYNC;
  return SCM_BOOL_T;
}

SCM_DEFINE (G_mark_whole_buffer, "mark-whole-buffer", 0, 0, 0, (void), "\
Put point at beginning and mark at end of buffer.")
{
  G_end_of_buffer ();
  G_set_mark_command ();
  G_beginning_of_buffer ();
  return SCM_BOOL_T;
}

SCM_DEFINE (G_read_char, "read-char", 0, 0, 0, (void), "\
Returns the character of the next key press. Events that are not\n\
characters are ignored.")
{
  size_t c = getkey_unfiltered (GETKEY_DEFAULT);
  return scm_integer_to_char (scm_from_char (c));
}

SCM_DEFINE (G_quoted_insert, "quoted-insert", 0, 0, 0, (void), "\
Read next input character and insert it.\n\
This is useful for inserting control characters.")
{
  // FIXME: need interactive logic
  minibuf_write ("C-q-");
  insert_char (getkey_unfiltered (GETKEY_DEFAULT));
  minibuf_clear ();
  return SCM_BOOL_T;
}

SCM_DEFINE (G_universal_argument, "universal-argument", 0, 0, 0, (void), "\
Begin a numeric argument for the following command.\n\
Digits or minus sign following @kbd{C-u} make up the numeric argument.\n\
@kbd{C-u} following the digits or minus sign ends the argument.\n\
@kbd{C-u} without digits or minus sign provides 4 as argument.\n\
Repeating @kbd{C-u} without digits or minus sign multiplies the argument\n\
by 4 each time.")
{
  SCM ok = SCM_BOOL_T;
  int i = 0, arg = 1, sgn = 1;
  astr as = astr_new ();

  /* Need to process key used to invoke universal-argument. */
  pushkey (lastkey ());

  thisflag |= FLAG_UNIARG_EMPTY;

  // FIXME: need interactive logic
  for (;;)
    {
      size_t key = do_binding_completion (as);

      /* Cancelled. */
      if (key == KBD_CANCEL)
        {
          ok = G_keyboard_quit ();
          break;
        }
      /* Digit pressed. */
      else if (isdigit (key & 0xff))
        {
          int digit = (key & 0xff) - '0';
          thisflag &= ~FLAG_UNIARG_EMPTY;

          if (key & KBD_META)
            {
              if (astr_len (as) > 0)
                astr_cat_char (as, ' ');
              astr_cat_cstr (as, "ESC");
            }

          astr_cat (as, astr_fmt (" %d", digit));

          if (i == 0)
            arg = digit;
          else
            arg = arg * 10 + digit;

          i++;
        }
      else if (key == (KBD_CTRL | 'u'))
        {
          astr_cat_cstr (as, "C-u");
          if (i == 0)
            arg *= 4;
          else
            break;
        }
      else if ((key & ~KBD_META) == '-' && i == 0)
        {
          if (sgn > 0)
            {
              sgn = -sgn;
              astr_cat_cstr (as, " -");
              /* The default negative arg is -1, not -4. */
              arg = 1;
              thisflag &= ~FLAG_UNIARG_EMPTY;
            }
        }
      else
        {
          ungetkey (key);
          break;
        }
    }

  if (ok == SCM_BOOL_T)
    {
      last_uniarg = arg * sgn;
      thisflag |= FLAG_SET_UNIARG;
      minibuf_clear ();
    }
  return ok;
}

SCM_DEFINE (G_back_to_indentation, "back-to-indentation", 0, 0, 0, (void), "\
Move point to the first non-whitespace character on this line.")
{
  goto_offset (get_buffer_line_o (cur_bp));
  while (!eolp () && isspace (following_char ()))
    move_char (1);
  return SCM_BOOL_T;
}

/***********************************************************************
                          Move through words
***********************************************************************/
static inline bool
iswordchar (int c)
{
  return isalnum (c) || c == '$';
}

static bool
move_word (int dir)
{
  bool gotword = false;
  do
    {
      for (; !(dir > 0 ? eolp () : bolp ()); move_char (dir))
        {
          if (iswordchar (get_buffer_char (cur_bp, get_buffer_pt (cur_bp) - (dir < 0))))
            gotword = true;
          else if (gotword)
            break;
        }
    }
  while (!gotword && move_char (dir));
  return gotword;
}

SCM_DEFINE (G_forward_word, "forward-word", 0, 1, 0, (SCM n), "\
Move point forward one word (backward if the argument is negative).\n\
With argument, do this that many times.")
{
  long uniarg = guile_to_long_or_error ("forward-word", SCM_ARG1, n);
  return move_with_uniarg (uniarg, move_word);
}

SCM_DEFINE (G_backward_word, "backward-word", 0, 1, 0, (SCM n), "\
Move backward until encountering the end of a word (forward if the\n\
argument is negative).\n\
With argument, do this that many times.")
{
  long uniarg = guile_to_long_or_error (s_G_backward_word, SCM_ARG1, n);
  return move_with_uniarg (-uniarg, move_word);
}

/***********************************************************************
               Move through balanced expressions (sexps)
***********************************************************************/
static inline bool
isopenbracketchar (int c, bool single_quote, bool double_quote)
{
  return ((c == '(') || (c == '[') || ( c== '{') ||
          ((c == '\"') && !double_quote) ||
          ((c == '\'') && !single_quote));
}

static inline bool
isclosebracketchar (int c, bool single_quote, bool double_quote)
{
  return ((c == ')') || (c == ']') || (c == '}') ||
          ((c == '\"') && double_quote) ||
          ((c == '\'') && single_quote));
}

static bool
move_sexp (int dir)
{
  int gotsexp = false, level = 0;
  int single_quote = dir < 0, double_quote = single_quote;

  for (;;)
    {
      while (dir > 0 ? !eolp () : !bolp ())
        {
          size_t o = get_buffer_pt (cur_bp) - (dir < 0 ? 1 : 0);
          char c = get_buffer_char (cur_bp, o);

          /* Skip escaped quotes. */
          if ((c == '\"' || c == '\'') && o > get_buffer_line_o (cur_bp) &&
              get_buffer_char (cur_bp, o - 1) == '\\')
            {
              move_char (dir);
              /* Treat escaped ' and " like word chars. */
              c = 'a';
            }

          if ((dir > 0 ? isopenbracketchar : isclosebracketchar) (c, single_quote, double_quote))
            {
              if (level == 0 && gotsexp)
                return true;

              level++;
              gotsexp = true;
              if (c == '\"')
                double_quote = !double_quote;
              if (c == '\'')
                single_quote = !double_quote;
            }
          else if ((dir > 0 ? isclosebracketchar : isopenbracketchar) (c, single_quote, double_quote))
            {
              if (level == 0 && gotsexp)
                return true;

              level--;
              gotsexp = true;
              if (c == '\"')
                double_quote = !double_quote;
              if (c == '\'')
                single_quote = !single_quote;

              if (level < 0)
                {
                  minibuf_error ("Scan error: \"Containing "
                                 "expression ends prematurely\"");
                  return false;
                }
            }

          move_char (dir);

          if (!(isalnum (c) || c == '$' || c == '_'))
            {
              if (gotsexp && level == 0)
                {
                  if (!(isopenbracketchar (c, single_quote, double_quote) ||
                        isclosebracketchar (c, single_quote, double_quote)))
                    move_char (-dir);
                  return true;
                }
            }
          else
            gotsexp = true;
        }
      if (gotsexp && level == 0)
        return true;
      if (dir > 0 ? !next_line () : !previous_line ())
        {
          if (level != 0)
            minibuf_error ("Scan error: \"Unbalanced parentheses\"");
          break;
        }
      if (dir > 0)
        G_beginning_of_line ();
      else
        G_end_of_line ();
    }
  return false;
}

SCM_DEFINE (G_forward_sexp, "forward-sexp", 0, 1, 0, (SCM n), "\
Move forward across one balanced expression (sexp).\n\
With argument, do it that many times.  Negative arg -N means\n\
move backward across N balanced expressions.")
{
  long uniarg = guile_to_long_or_error (s_G_forward_sexp, SCM_ARG1, n);
  return move_with_uniarg (uniarg, move_sexp);
}

SCM_DEFINE (G_backward_sexp, "backward-sexp", 0, 1, 0, (SCM n), "\
Move backward across one balanced expression (sexp).\n\
With argument, do it that many times.  Negative arg -N means\n\
move forward across N balanced expressions.")
{
  long uniarg = guile_to_long_or_error (s_G_backward_sexp, SCM_ARG1, n);
  return move_with_uniarg (-uniarg, move_sexp);
}

/***********************************************************************
                          Transpose functions
***********************************************************************/
static void
astr_append_region (astr s)
{
  activate_mark ();
  astr_cat (s, get_buffer_region (cur_bp, calculate_the_region ()).as);
}

static bool
transpose_subr (bool (*move_func) (int dir))
{
  /* For transpose-chars. */
  if (move_func == move_char && eolp ())
    move_func (-1);

  /* For transpose-lines. */
  if (move_func == move_line && get_buffer_line_o (cur_bp) == 0)
    move_func (1);

  /* Backward. */
  if (!move_func (-1))
    {
      minibuf_error ("Beginning of buffer");
      return false;
    }

  /* Mark the beginning of first string. */
  push_mark ();
  Marker *m1 = point_marker ();

  /* Check to make sure we can go forwards twice. */
  if (!move_func (1) || !move_func (1))
    {
      if (move_func == move_line)
        { /* Add an empty line. */
          G_end_of_line ();
          G_znewline (scm_from_int (1));
        }
      else
        {
          pop_mark ();
          goto_offset (get_marker_o (m1));
          minibuf_error ("End of buffer");

          unchain_marker (m1);
          return false;
        }
    }

  goto_offset (get_marker_o (m1));

  /* Forward. */
  move_func (1);

  /* Save and delete 1st marked region. */
  astr as1 = astr_new ();
  astr_append_region (as1);

  G_delete_region ();

  /* Forward. */
  move_func (1);

  /* For transpose-lines. */
  astr as2 = NULL;
  Marker *m2;
  if (move_func == move_line)
    m2 = point_marker ();
  else
    {
      /* Mark the end of second string. */
      set_mark ();

      /* Backward. */
      move_func (-1);
      m2 = point_marker ();

      /* Save and delete 2nd marked region. */
      as2 = astr_new ();
      astr_append_region (as2);
      G_delete_region ();
    }

  /* Insert the first string. */
  goto_offset (get_marker_o (m2));
  unchain_marker (m2);
  bprintf ("%s", astr_cstr (as1));

  /* Insert the second string. */
  if (as2)
    {
      goto_offset (get_marker_o (m1));
      bprintf ("%s", astr_cstr (as2));
    }
  unchain_marker (m1);

  /* Restore mark. */
  pop_mark ();
  deactivate_mark ();

  /* Move forward if necessary. */
  if (move_func != move_line)
    move_func (1);

  return true;
}

static SCM
transpose (int uniarg, bool (*move) (int dir))
{
  if (warn_if_readonly_buffer ())
    return SCM_BOOL_F;

  bool ret = true;
  undo_start_sequence ();
  for (unsigned long uni = 0; ret && uni < (unsigned) abs (uniarg); ++uni)
    ret = transpose_subr (move);
  undo_end_sequence ();

  return scm_from_bool (ret);
}

SCM_DEFINE (G_transpose_chars, "transpose-chars", 0, 1, 0, (SCM n), "\
Interchange characters around point, moving forward one character.\n\
With prefix arg ARG, effect is to take character before point\n\
and drag it forward past ARG other characters (backward if ARG negative).\n\
If no argument and at end of line, the previous two chars are exchanged.")
{
  long c_n = guile_to_long_or_error (s_G_transpose_chars, SCM_ARG1, n);
  return transpose (c_n, move_char);
}

SCM_DEFINE (G_transpose_words, "transpose-words", 0, 1, 0, (SCM n), "\
Interchange words around point, leaving point at end of them.\n\
With prefix arg ARG, effect is to take word before or around point\n\
and drag it forward past ARG other words (backward if ARG negative).\n\
If ARG is zero, the words around or after point and around or after mark\n\
are interchanged.")
{
  long c_n = guile_to_long_or_error (s_G_transpose_words, SCM_ARG1, n);
  return transpose (c_n, move_word);
}

SCM_DEFINE (G_transpose_sexps, "transpose-sexps", 0, 1, 0, (SCM n), "\
Like @kbd{M-x transpose-words} but applies to sexps.")
{
  long c_n = guile_to_long_or_error (s_G_transpose_sexps, SCM_ARG1, n);
  return transpose (c_n, move_sexp);
}

SCM_DEFINE (G_transpose_lines, "transpose-lines", 0, 1, 0, (SCM n), "\
Exchange current line and previous line, leaving point after both.\n\
With argument ARG, takes previous line and moves it past ARG lines.\n\
With argument 0, interchanges line point is in with line mark is in.")
{
  long c_n = guile_to_long_or_error (s_G_transpose_sexps, SCM_ARG1, n);
  return transpose (c_n, move_line);
}

SCM_DEFINE (G_mark_word, "mark-word", 0, 1, 0, (SCM n), "\
Set mark argument words away from point.")
{
  SCM ok;
  G_set_mark_command ();
  ok = G_forward_word (n);
  if (scm_is_true (ok))
    G_exchange_point_and_mark ();
  return ok;
}

SCM_DEFINE (G_mark_sexp, "mark-sexp", 0, 1, 0, (SCM n), "\
Set mark @i{arg} sexps from point.\n\
The place mark goes is the same place @kbd{C-M-f} would\n\
move to with the same argument.")
{
  SCM ok;
  G_set_mark_command ();
  ok = G_forward_sexp (n);

  if (scm_is_true (ok))
    G_exchange_point_and_mark ();
  return ok;
}

SCM_DEFINE (G_forward_line, "forward-line", 0, 1, 0, (SCM gn), "\
Move N lines forward (backward if N is negative).\n\
Precisely, if point is on line I, move to the start of line I + N.")
{
  long n = guile_to_long_or_error ("forward-line", SCM_ARG1, gn);
  G_beginning_of_line ();
  return scm_from_bool (move_line (n));
}

static SCM
move_paragraph (int uniarg, bool (*forward) (void), bool (*backward) (void),
                     SCM (*line_extremum)())
{
  if (uniarg < 0)
    {
      uniarg = -uniarg;
      forward = backward;
    }

  while (uniarg-- > 0)
    {
      while (is_empty_line () && forward ())
        ;
      while (!is_empty_line () && forward ())
        ;
    }

  if (is_empty_line ())
    G_beginning_of_line ();
  else
    line_extremum ();

  return SCM_BOOL_T;
}

SCM_DEFINE (G_backward_paragraph, "backward-paragraph", 0, 1, 0, (SCM n), "\
Move backward to start of paragraph.  With argument N, do it N times.")
{
  long uniarg = guile_to_long_or_error ("backward-paragraph", SCM_ARG1, n);
  return move_paragraph (uniarg, previous_line, next_line, G_beginning_of_line);
}

SCM_DEFINE (G_forward_paragraph, "forward-paragraph", 0, 1, 0, (SCM n), "\
Move forward to end of paragraph.  With argument N, do it N times.")
{
  long uniarg = guile_to_long_or_error ("forward-paragraph", SCM_ARG1, n);
  return move_paragraph (uniarg, next_line, previous_line, G_end_of_line);
}

SCM_DEFINE (G_mark_paragraph, "mark-paragraph", 0, 1, 0, (SCM n), "\
Put point at beginning of this paragraph, mark at end.\n\
The paragraph marked is the one that contains point or follows point.")
{
  if (scm_is_eq (last_command (), F_mark_paragraph ()))
    {
      G_exchange_point_and_mark ();
      G_forward_paragraph (n);
      G_exchange_point_and_mark ();
    }
  else
    {
      G_forward_paragraph (n);
      G_set_mark_command ();
      G_backward_paragraph(n);
    }
  return SCM_BOOL_T;
}

SCM_DEFINE (G_fill_paragraph, "fill-paragraph", 0, 0, 0, (void), "\
Fill paragraph at or after point.")
{
  Marker *m = point_marker ();
  long fill_column;

  undo_start_sequence ();

  G_forward_paragraph (scm_from_int (1));
  if (is_empty_line ())
    previous_line ();
  Marker *m_end = point_marker ();

  G_backward_paragraph (scm_from_int (1));
  if (is_empty_line ())
    /* Move to next line if between two paragraphs. */
    next_line ();

  while (buffer_end_of_line (cur_bp, get_buffer_pt (cur_bp)) < get_marker_o (m_end))
    {
      G_end_of_line ();
      delete_char ();
      G_just_one_space ();
    }
  unchain_marker (m_end);

  G_end_of_line ();
  while (get_goalc () > (size_t) get_variable_number ("fill-column") + 1
         && fill_break_line ())
    ;

  goto_offset (get_marker_o (m));
  unchain_marker (m);

  undo_end_sequence ();
  return SCM_BOOL_T;
}
static bool
setcase_word (int rcase)
{
  if (!iswordchar (following_char ()))
    if (!move_word (1) || !move_word (-1))
      return false;

  astr as = astr_new ();
  char c;
  for (size_t i = get_buffer_pt (cur_bp) - get_buffer_line_o (cur_bp);
       i < buffer_line_len (cur_bp, get_buffer_pt (cur_bp)) &&
         iswordchar ((int) (c = get_buffer_char (cur_bp, get_buffer_line_o (cur_bp) + i)));
       i++)
    astr_cat_char (as, c);

  if (astr_len (as) > 0)
    {
      astr_recase (as, rcase);
      replace_estr (astr_len (as), estr_new_astr (as));
    }

  set_buffer_modified (cur_bp, true);

  return true;
}

static bool
setcase_word_lowercase (void)
{
  return setcase_word (case_lower);
}

SCM_DEFINE (G_downcase_word, "downcase-word", 0, 1, 0, (SCM n), "\
Convert following word (or @i{arg} words) to lower case, moving over.")
{
  long arg = guile_to_long_or_error (s_G_downcase_word, SCM_ARG1, n);
  return execute_with_uniarg (true, arg, setcase_word_lowercase, NULL);
}

static bool
setcase_word_uppercase (void)
{
  return setcase_word (case_upper);
}

SCM_DEFINE (G_upcase_word, "upcase-word", 0, 1, 0, (SCM n), "\
Convert following word (or @i{arg} words) to upper case, moving over.")
{
  long arg = guile_to_long_or_error (s_G_upcase_word, SCM_ARG1, n);
  return execute_with_uniarg (true, arg, setcase_word_uppercase, NULL);
}

static bool
setcase_word_capitalize (void)
{
  return setcase_word (case_capitalized);
}

SCM_DEFINE (G_capitalize_word, "capitalize-word", 0, 1, 0, (SCM n), "\
Capitalize the following word (or @i{arg} words), moving over.\n\
This gives the word(s) a first character in upper case\n\
and the rest lower case.")
{
  long arg = guile_to_long_or_error (s_G_capitalize_word, SCM_ARG1, n);
  return execute_with_uniarg (true, arg, setcase_word_capitalize, NULL);
}

/*
 * Set the region case.
 */
static SCM
setcase_region (int (*func) (int))
{
  if (warn_if_readonly_buffer () || warn_if_no_mark ())
    return SCM_BOOL_F;

  Region r = calculate_the_region ();
  undo_start_sequence ();

  Marker *m = point_marker ();
  goto_offset (r.start);
  for (size_t size = get_region_size (r); size > 0; size--)
    {
      char c = func (following_char ());
      delete_char ();
      insert_char (c);
    }
  goto_offset (get_marker_o (m));
  unchain_marker (m);

  undo_end_sequence ();

  return SCM_BOOL_T;
}

SCM_DEFINE (G_upcase_region, "upcase-region", 0, 0, 0, (void), "\
Convert the region to upper case.")
{
  return setcase_region (toupper);
}

SCM_DEFINE (G_downcase_region, "downcase-region", 0, 0, 0, (void), "\
Convert the region to lower case.")
{
  return setcase_region (tolower);
}

static void
write_shell_output (va_list ap)
{
  insert_estr ((estr) {.as = va_arg (ap, astr), .eol = coding_eol_lf});
}

typedef struct {
  astr in;
  astr out;
  size_t done;
  char buf[BUFSIZ];
} pipe_data;

static const void *
prepare_write (size_t *n, void *priv)
{
  *n = astr_len (((pipe_data *) priv)->in) - ((pipe_data *) priv)->done;
  return *n ? astr_cstr (((pipe_data *) priv)->in) + ((pipe_data *) priv)->done : NULL;
}

static void
done_write (void *data _GL_UNUSED_PARAMETER, size_t n, void *priv)
{
  ((pipe_data *) priv)->done += n;
}

static void *
prepare_read (size_t *n, void *priv)
{
  *n = BUFSIZ;
  return ((pipe_data *) priv)->buf;
}

static void
done_read (void *data, size_t n, void *priv)
{
  astr_cat_nstr (((pipe_data *) priv)->out, (const char *)data, n);
}

static SCM
pipe_command (castr cmd, astr input, bool do_insert, bool do_replace)
{
  const char *prog_argv[] = { "/bin/sh", "-c", astr_cstr (cmd), NULL };
  pipe_data inout = { .in = input, .out = astr_new (), .done = 0 };
  if (pipe_filter_ii_execute (PACKAGE_NAME, "/bin/sh", prog_argv, true, false,
                              prepare_write, done_write, prepare_read, done_read,
                              &inout) != 0)
    return SCM_BOOL_F;

  char *eol = strchr (astr_cstr (inout.out), '\n');

  if (astr_len (inout.out) == 0)
    minibuf_write ("(Shell command succeeded with no output)");
  else
    {
      if (do_insert)
        {
          size_t del = 0;
          if (do_replace && !warn_if_no_mark ())
            {
              Region r = calculate_the_region ();
              goto_offset (r.start);
              del = get_region_size (r);
            }
          replace_estr (del, estr_new_astr (inout.out));
        }
      else
        {
          bool more_than_one_line = eol != NULL &&
            eol != astr_cstr (inout.out) + astr_len (inout.out) - 1;
          write_temp_buffer ("*Shell Command Output*", more_than_one_line,
                             write_shell_output, inout.out);
          if (!more_than_one_line)
            minibuf_write ("%s", astr_cstr (inout.out));
        }
    }

  return SCM_BOOL_T;
}

static castr
minibuf_read_shell_command (void)
{
  castr ms = minibuf_read ("Shell command: ", "");

  if (ms == NULL)
    {
      G_keyboard_quit ();
      return NULL;
    }
  if (astr_len (ms) == 0)
    return NULL;

  return ms;
}

SCM_DEFINE (G_shell_command, "shell-command", 0, 2, 0,
	    (SCM gcmd, SCM ginsert), "\
Execute string @i{command} in inferior shell; display output, if any.\n\
With prefix argument, insert the command's output at point.\n\
\n\
Command is executed synchronously.  The output appears in the buffer\n\
`*Shell Command Output*'.  If the output is short enough to display\n\
in the echo area, it is shown there, but it is nonetheless available\n\
in buffer `*Shell Command Output*' even though that buffer is not\n\
automatically displayed.\n\
\n\
The optional second argument @i{output-buffer}, if non-nil,\n\
says to insert the output in the current buffer.")
{
  char *str;
  astr cmd = NULL;
  bool insert;

  // FIXME: need interactive logic
  str = guile_to_locale_string_safe (gcmd);
  if (str != NULL)
    cmd = astr_new_cstr (str);
  if (cmd == NULL)
    cmd = minibuf_read_shell_command ();
  if (SCM_UNBNDP (ginsert))
    insert = lastflag & FLAG_SET_UNIARG;
  else
    insert = scm_to_bool (ginsert);

  if (cmd != NULL)
    return pipe_command (cmd, astr_new (), insert, false);
  return SCM_BOOL_T;
}


SCM_DEFINE (G_shell_command_on_region, "shell-command-on-region",
	    0, 2, 0, (SCM gcmd, SCM ginsert), "\
Execute string command in inferior shell with region as input.\n\
Normally display output (if any) in temp buffer `*Shell Command Output*';\n\
Prefix arg means replace the region with it.  Return the exit code of\n\
command.\n\
\n\
If the command generates output, the output may be displayed\n\
in the echo area or in a buffer.\n\
If the output is short enough to display in the echo area, it is shown\n\
there.  Otherwise it is displayed in the buffer `*Shell Command Output*'.\n\
The output is available in that buffer in both cases.")
{
  char *str;
  astr cmd = NULL;
  SCM ok = SCM_BOOL_T;
  bool insert;

  // FIXME: need interactive logic here
  if (SCM_UNBNDP (gcmd))
    cmd = minibuf_read_shell_command ();
  else
    {
      str = guile_to_locale_string_safe (gcmd);
      cmd = astr_new_cstr (str);
    }

  if SCM_UNBNDP (ginsert)
    insert = lastflag & FLAG_SET_UNIARG;
  else
    insert = SCM_BOOL_F;

  if (cmd != NULL)
    {
      if (warn_if_no_mark ())
        ok = SCM_BOOL_F;
      else
        ok = pipe_command (cmd, get_buffer_region (cur_bp, calculate_the_region ()).as, insert, true);
    }
  return ok;
}

SCM_DEFINE (G_delete_region, "delete-region", 0, 0, 0, (void), "\
Delete the text between point and mark.")
{
  SCM ok = SCM_BOOL_T;
  if (warn_if_no_mark () || !delete_region (calculate_the_region ()))
    ok = SCM_BOOL_F;
  else
    deactivate_mark ();
  return ok;
}

SCM_DEFINE (G_delete_blank_lines, "delete-blank-lines", 0, 0, 0, (void), "\
On blank line, delete all surrounding blank lines, leaving just one.\n\
On isolated blank line, delete that one.\n\
On nonblank line, delete any immediately following blank lines.")
{
  Marker *m = point_marker ();
  Region r;
  r.start = r.end = get_buffer_line_o (cur_bp);

  undo_start_sequence ();

  /* Find following blank lines.  */
  if (G_forward_line (scm_from_int (1)) == SCM_BOOL_T && is_blank_line ())
    {
      r.start = get_buffer_pt (cur_bp);
      do
        r.end = buffer_next_line (cur_bp, get_buffer_pt (cur_bp));
      while (G_forward_line (scm_from_int (1)) == SCM_BOOL_T && is_blank_line ());
      r.end = MIN (r.end, get_buffer_size (cur_bp));
    }
  goto_offset (get_marker_o (m));

  /* If this line is blank, find any preceding blank lines.  */
  bool singleblank = true;
  if (is_blank_line ())
    {
      r.end = MAX (r.end, buffer_next_line (cur_bp, get_buffer_pt (cur_bp)));
      do
        r.start = get_buffer_line_o (cur_bp);
      while (G_forward_line (scm_from_int (-1)) == SCM_BOOL_T && is_blank_line ());
      goto_offset (get_marker_o (m));
      if (r.start != get_buffer_line_o (cur_bp) ||
          r.end > buffer_next_line (cur_bp, get_buffer_pt (cur_bp)))
        singleblank = false;
      r.end = MIN (r.end, get_buffer_size (cur_bp));
    }

  /* If we are deleting to EOB, need to fudge extra line. */
  bool at_eob = r.end == get_buffer_size (cur_bp) && r.start > 0;
  if (at_eob)
    r.start -= strlen (get_buffer_eol (cur_bp));

  /* Delete any blank lines found. */
  if (r.start < r.end)
    delete_region (r);

  /* If we found more than one blank line, leave one. */
  if (!singleblank)
    {
      if (!at_eob)
        intercalate_newline ();
      else
        insert_newline ();
    }

  undo_end_sequence ();

  unchain_marker (m);
  deactivate_mark ();
  return SCM_BOOL_T;
}


void
init_guile_funcs_procedures (void)
{
#include "funcs.x"
  scm_c_export ("suspend-emacs",
		"keyboard-quit",
		"list-buffers",
		"toggle-read-only",
		"set-fill-column",
		"set-mark",
		"set-mark-command",
		"exchange-point-and-mark",
		"mark-whole-buffer",
		"read-char",
		"quoted-insert",
		"universal-argument",
		"back-to-indentation",
		"forward-word",
		"backward-word",
		"forward-sexp",
		"backward-sexp",
		"transpose-chars",
		"transpose-words",
		"transpose-sexps",
		"transpose-lines",
		"mark-word",
		"mark-sexp",
		"forward-line",
		"backward-paragraph",
		"forward-paragraph",
		"mark-paragraph",
		"fill-paragraph",
		"downcase-word",
		"upcase-word",
		"capitalize-word",
		"upcase-region",
		"downcase-region",
		"shell-command",
		"shell-command-on-region",
		"delete-region",
		"delete-blank-lines",
		NULL);
}
