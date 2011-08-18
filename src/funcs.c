/* Miscellaneous Emacs functions

   Copyright (c) 1997-2011 Free Software Foundation, Inc.

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

#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "pipe-filter.h"

#include "main.h"
#include "extern.h"


DEFUN ("suspend-emacs", suspend_emacs)
/*+
Stop Zile and return to superior process.
+*/
{
  raise (SIGTSTP);
}
END_DEFUN

DEFUN ("keyboard-quit", keyboard_quit)
/*+
Cancel current command.
+*/
{
  deactivate_mark ();
  minibuf_error ("Quit");
  ok = leNIL;
}
END_DEFUN

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

  FUNCALL (beginning_of_buffer);
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

DEFUN ("list-buffers", list_buffers)
/*+
Display a list of names of existing buffers.
The list is displayed in a buffer named `*Buffer List*'.
Note that buffers with names starting with spaces are omitted.

@itemize -
The @samp{M} column contains a @samp{*} for buffers that are modified.
The @samp{R} column contains a @samp{%} for buffers that are read-only.
@end itemize
+*/
{
  write_temp_buffer ("*Buffer List*", true, write_buffers_list, cur_wp);
}
END_DEFUN

DEFUN ("toggle-read-only", toggle_read_only)
/*+
Change whether this buffer is visiting its file read-only.
+*/
{
  set_buffer_readonly (cur_bp, !get_buffer_readonly (cur_bp));
}
END_DEFUN

DEFUN ("auto-fill-mode", auto_fill_mode)
/*+
Toggle Auto Fill mode.
In Auto Fill mode, inserting a space at a column beyond `fill-column'
automatically breaks the line at a previous space.
+*/
{
  set_buffer_autofill (cur_bp, !get_buffer_autofill (cur_bp));
}
END_DEFUN

DEFUN ("set-fill-column", set_fill_column)
/*+
Set `fill-column' to specified argument.
Use C-u followed by a number to specify a column.
Just C-u as argument means to use the current column.
+*/
{
  size_t o = get_buffer_pt (cur_bp) - get_buffer_line_o (cur_bp);
  long fill_col = (lastflag & FLAG_UNIARG_EMPTY) ? o : (unsigned long) uniarg;
  char *buf = NULL;

  if (!(lastflag & FLAG_SET_UNIARG) && arglist == NULL)
    {
      fill_col = minibuf_read_number ("Set fill-column to (default %d): ", o);
      if (fill_col == LONG_MAX)
        return leNIL;
      else if (fill_col == LONG_MAX - 1)
        fill_col = o;
    }

  if (arglist)
    {
      if (arglist->next)
        buf = arglist->next->data;
      else
        {
          minibuf_error ("set-fill-column requires an explicit argument");
          ok = leNIL;
        }
    }
  else
    {
      buf = xasprintf ("%ld", fill_col);
      /* Only print message when run interactively. */
      minibuf_write ("Fill column set to %s (was %d)", buf,
                     get_variable_number ("fill-column"));
    }

  if (ok == leT)
    set_variable ("fill-column", buf);
}
END_DEFUN

DEFUN_NONINTERACTIVE ("set-mark", set_mark)
/*+
Set this buffer's mark to point.
+*/
{
  set_mark ();
  activate_mark ();
}
END_DEFUN

DEFUN ("set-mark-command", set_mark_command)
/*+
Set the mark where point is.
+*/
{
  FUNCALL (set_mark);
  minibuf_write ("Mark set");
}
END_DEFUN

DEFUN ("exchange-point-and-mark", exchange_point_and_mark)
/*+
Put the mark where point is now, and point where the mark is now.
+*/
{
  if (get_buffer_mark (cur_bp) == NULL)
    {
      minibuf_error ("No mark set in this buffer");
      return leNIL;
    }

  size_t o = get_buffer_pt (cur_bp);
  goto_offset (get_marker_o (get_buffer_mark (cur_bp)));
  set_marker_o (get_buffer_mark (cur_bp), o);
  activate_mark ();
  thisflag |= FLAG_NEED_RESYNC;
}
END_DEFUN

DEFUN ("mark-whole-buffer", mark_whole_buffer)
/*+
Put point at beginning and mark at end of buffer.
+*/
{
  FUNCALL (end_of_buffer);
  FUNCALL (set_mark_command);
  FUNCALL (beginning_of_buffer);
}
END_DEFUN

DEFUN ("quoted-insert", quoted_insert)
/*+
Read next input character and insert it.
This is useful for inserting control characters.
+*/
{
  minibuf_write ("C-q-");
  insert_char (getkey_unfiltered (GETKEY_DEFAULT));
  minibuf_clear ();
}
END_DEFUN

DEFUN ("universal-argument", universal_argument)
/*+
Begin a numeric argument for the following command.
Digits or minus sign following @kbd{C-u} make up the numeric argument.
@kbd{C-u} following the digits or minus sign ends the argument.
@kbd{C-u} without digits or minus sign provides 4 as argument.
Repeating @kbd{C-u} without digits or minus sign multiplies the argument
by 4 each time.
+*/
{
  int i = 0, arg = 1, sgn = 1;
  astr as = astr_new ();

  /* Need to process key used to invoke universal-argument. */
  pushkey (lastkey ());

  thisflag |= FLAG_UNIARG_EMPTY;

  for (;;)
    {
      size_t key = do_binding_completion (as);

      /* Cancelled. */
      if (key == KBD_CANCEL)
        {
          ok = FUNCALL (keyboard_quit);
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

  if (ok == leT)
    {
      last_uniarg = arg * sgn;
      thisflag |= FLAG_SET_UNIARG;
      minibuf_clear ();
    }
}
END_DEFUN

DEFUN ("back-to-indentation", back_to_indentation)
/*+
Move point to the first non-whitespace character on this line.
+*/
{
  goto_offset (get_buffer_line_o (cur_bp));
  while (!eolp () && isspace (following_char ()))
    move_char (1);
}
END_DEFUN

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

DEFUN ("forward-word", forward_word)
/*+
Move point forward one word (backward if the argument is negative).
With argument, do this that many times.
+*/
{
  ok = move_with_uniarg (uniarg, move_word);
}
END_DEFUN

DEFUN ("backward-word", backward_word)
/*+
Move backward until encountering the end of a word (forward if the
argument is negative).
With argument, do this that many times.
+*/
{
  ok = move_with_uniarg (-uniarg, move_word);
}
END_DEFUN

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
        FUNCALL (beginning_of_line);
      else
        FUNCALL (end_of_line);
    }
  return false;
}

DEFUN ("forward-sexp", forward_sexp)
/*+
Move forward across one balanced expression (sexp).
With argument, do it that many times.  Negative arg -N means
move backward across N balanced expressions.
+*/
{
  ok = move_with_uniarg (uniarg, move_sexp);
}
END_DEFUN

DEFUN ("backward-sexp", backward_sexp)
/*+
Move backward across one balanced expression (sexp).
With argument, do it that many times.  Negative arg -N means
move forward across N balanced expressions.
+*/
{
  ok = move_with_uniarg (-uniarg, move_sexp);
}
END_DEFUN

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
          FUNCALL (end_of_line);
          FUNCALL (newline);
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

  FUNCALL (delete_region);

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
      FUNCALL (delete_region);
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

static le *
transpose (int uniarg, bool (*move) (int dir))
{
  if (warn_if_readonly_buffer ())
    return leNIL;

  bool ret = true;
  undo_start_sequence ();
  for (unsigned long uni = 0; ret && uni < (unsigned) abs (uniarg); ++uni)
    ret = transpose_subr (move);
  undo_end_sequence ();

  return bool_to_lisp (ret);
}

DEFUN ("transpose-chars", transpose_chars)
/*+
Interchange characters around point, moving forward one character.
With prefix arg ARG, effect is to take character before point
and drag it forward past ARG other characters (backward if ARG negative).
If no argument and at end of line, the previous two chars are exchanged.
+*/
{
  ok = transpose (uniarg, move_char);
}
END_DEFUN

DEFUN ("transpose-words", transpose_words)
/*+
Interchange words around point, leaving point at end of them.
With prefix arg ARG, effect is to take word before or around point
and drag it forward past ARG other words (backward if ARG negative).
If ARG is zero, the words around or after point and around or after mark
are interchanged.
+*/
{
  ok = transpose (uniarg, move_word);
}
END_DEFUN

DEFUN ("transpose-sexps", transpose_sexps)
/*+
Like @kbd{M-x transpose-words} but applies to sexps.
+*/
{
  ok = transpose (uniarg, move_sexp);
}
END_DEFUN

DEFUN ("transpose-lines", transpose_lines)
/*+
Exchange current line and previous line, leaving point after both.
With argument ARG, takes previous line and moves it past ARG lines.
With argument 0, interchanges line point is in with line mark is in.
+*/
{
  ok = transpose (uniarg, move_line);
}
END_DEFUN


static le *
mark (int uniarg, Function func)
{
  le * ret;
  FUNCALL (set_mark_command);
  ret = func (uniarg, true, NULL);
  if (ret)
    FUNCALL (exchange_point_and_mark);
  return ret;
}

DEFUN ("mark-word", mark_word)
/*+
Set mark argument words away from point.
+*/
{
  ok = mark (uniarg, F_forward_word);
}
END_DEFUN

DEFUN ("mark-sexp", mark_sexp)
/*+
Set mark @i{arg} sexps from point.
The place mark goes is the same place @kbd{C-M-f} would
move to with the same argument.
+*/
{
  ok = mark (uniarg, F_forward_sexp);
}
END_DEFUN

DEFUN_ARGS ("forward-line", forward_line,
            INT_OR_UNIARG (n))
/*+
Move N lines forward (backward if N is negative).
Precisely, if point is on line I, move to the start of line I + N.
+*/
{
  INT_OR_UNIARG_INIT (n);
  if (ok == leT)
    {
      FUNCALL (beginning_of_line);
      ok = bool_to_lisp (move_line (n));
    }
}
END_DEFUN

static le *
move_paragraph (int uniarg, bool (*forward) (void), bool (*backward) (void),
                     Function line_extremum)
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
    FUNCALL (beginning_of_line);
  else
    line_extremum (1, false, NULL);

  return leT;
}

DEFUN ("backward-paragraph", backward_paragraph)
/*+
Move backward to start of paragraph.  With argument N, do it N times.
+*/
{
  ok = move_paragraph (uniarg, previous_line, next_line, F_beginning_of_line);
}
END_DEFUN

DEFUN ("forward-paragraph", forward_paragraph)
/*+
Move forward to end of paragraph.  With argument N, do it N times.
+*/
{
  ok = move_paragraph (uniarg, next_line, previous_line, F_end_of_line);
}
END_DEFUN

DEFUN ("mark-paragraph", mark_paragraph)
/*+
Put point at beginning of this paragraph, mark at end.
The paragraph marked is the one that contains point or follows point.
+*/
{
  if (last_command () == F_mark_paragraph)
    {
      FUNCALL (exchange_point_and_mark);
      FUNCALL_ARG (forward_paragraph, uniarg);
      FUNCALL (exchange_point_and_mark);
    }
  else
    {
      FUNCALL_ARG (forward_paragraph, uniarg);
      FUNCALL (set_mark_command);
      FUNCALL_ARG (backward_paragraph, uniarg);
    }
}
END_DEFUN

DEFUN ("fill-paragraph", fill_paragraph)
/*+
Fill paragraph at or after point.
+*/
{
  Marker *m = point_marker ();

  undo_start_sequence ();

  FUNCALL (forward_paragraph);
  if (is_empty_line ())
    previous_line ();
  Marker *m_end = point_marker ();

  FUNCALL (backward_paragraph);
  if (is_empty_line ())
    /* Move to next line if between two paragraphs. */
    next_line ();

  while (buffer_end_of_line (cur_bp, get_buffer_pt (cur_bp)) < get_marker_o (m_end))
    {
      FUNCALL (end_of_line);
      delete_char ();
      FUNCALL (just_one_space);
    }
  unchain_marker (m_end);

  FUNCALL (end_of_line);
  while (get_goalc () > (size_t) get_variable_number ("fill-column") + 1
         && fill_break_line ())
    ;

  goto_offset (get_marker_o (m));
  unchain_marker (m);

  undo_end_sequence ();
}
END_DEFUN

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

DEFUN_ARGS ("downcase-word", downcase_word,
            INT_OR_UNIARG (arg))
/*+
Convert following word (or @i{arg} words) to lower case, moving over.
+*/
{
  INT_OR_UNIARG_INIT (arg);
  ok = execute_with_uniarg (true, arg, setcase_word_lowercase, NULL);
}
END_DEFUN

static bool
setcase_word_uppercase (void)
{
  return setcase_word (case_upper);
}

DEFUN_ARGS ("upcase-word", upcase_word,
            INT_OR_UNIARG (arg))
/*+
Convert following word (or @i{arg} words) to upper case, moving over.
+*/
{
  INT_OR_UNIARG_INIT (arg);
  ok = execute_with_uniarg (true, arg, setcase_word_uppercase, NULL);
}
END_DEFUN

static bool
setcase_word_capitalize (void)
{
  return setcase_word (case_capitalized);
}

DEFUN_ARGS ("capitalize-word", capitalize_word,
            INT_OR_UNIARG (arg))
/*+
Capitalize the following word (or @i{arg} words), moving over.
This gives the word(s) a first character in upper case
and the rest lower case.
+*/
{
  INT_OR_UNIARG_INIT (arg);
  ok = execute_with_uniarg (true, arg, setcase_word_capitalize, NULL);
}
END_DEFUN

/*
 * Set the region case.
 */
static le *
setcase_region (int (*func) (int))
{
  if (warn_if_readonly_buffer () || warn_if_no_mark ())
    return leNIL;

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

  return leT;
}

DEFUN ("upcase-region", upcase_region)
/*+
Convert the region to upper case.
+*/
{
  ok = setcase_region (toupper);
}
END_DEFUN

DEFUN ("downcase-region", downcase_region)
/*+
Convert the region to lower case.
+*/
{
  ok = setcase_region (tolower);
}
END_DEFUN

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

static le *
pipe_command (castr cmd, astr input, bool do_insert, bool do_replace)
{
  const char *prog_argv[] = { "/bin/sh", "-c", astr_cstr (cmd), NULL };
  pipe_data inout = { .in = input, .out = astr_new (), .done = 0 };
  if (pipe_filter_ii_execute (PACKAGE_NAME, "/bin/sh", prog_argv, true, false,
                              prepare_write, done_write, prepare_read, done_read,
                              &inout) != 0)
    return leNIL;

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

  return leT;
}

static castr
minibuf_read_shell_command (void)
{
  castr ms = minibuf_read ("Shell command: ", "");

  if (ms == NULL)
    {
      FUNCALL (keyboard_quit);
      return NULL;
    }
  if (astr_len (ms) == 0)
    return NULL;

  return ms;
}

DEFUN_ARGS ("shell-command", shell_command,
            STR_ARG (cmd)
            BOOL_ARG (insert))
/*+
Execute string @i{command} in inferior shell; display output, if any.
With prefix argument, insert the command's output at point.

Command is executed synchronously.  The output appears in the buffer
`*Shell Command Output*'.  If the output is short enough to display
in the echo area, it is shown there, but it is nonetheless available
in buffer `*Shell Command Output*' even though that buffer is not
automatically displayed.

The optional second argument @i{output-buffer}, if non-nil,
says to insert the output in the current buffer.
+*/
{
  STR_INIT (cmd)
  else
    cmd = minibuf_read_shell_command ();
  BOOL_INIT (insert)
  else
    insert = lastflag & FLAG_SET_UNIARG;

  if (cmd != NULL)
    ok = pipe_command (cmd, astr_new (), insert, false);
}
END_DEFUN

/* The `start' and `end' arguments are fake, hence their string type,
   so they can be ignored. */
DEFUN_ARGS ("shell-command-on-region", shell_command_on_region,
            STR_ARG (start)
            STR_ARG (end)
            STR_ARG (cmd)
            BOOL_ARG (insert))
/*+
Execute string command in inferior shell with region as input.
Normally display output (if any) in temp buffer `*Shell Command Output*';
Prefix arg means replace the region with it.  Return the exit code of
command.

If the command generates output, the output may be displayed
in the echo area or in a buffer.
If the output is short enough to display in the echo area, it is shown
there.  Otherwise it is displayed in the buffer `*Shell Command Output*'.
The output is available in that buffer in both cases.
+*/
{
  STR_INIT (start);
  STR_INIT (end);
  STR_INIT (cmd)
  else
    cmd = minibuf_read_shell_command ();
  BOOL_INIT (insert)
  else
    insert = lastflag & FLAG_SET_UNIARG;

  if (cmd != NULL)
    {
      if (warn_if_no_mark ())
        ok = leNIL;
      else
        ok = pipe_command (cmd, get_buffer_region (cur_bp, calculate_the_region ()).as, insert, true);
    }
}
END_DEFUN

DEFUN ("delete-region", delete_region)
/*+
Delete the text between point and mark.
+*/
{
  if (warn_if_no_mark () || !delete_region (calculate_the_region ()))
    ok = leNIL;
  else
    deactivate_mark ();
}
END_DEFUN

DEFUN ("delete-blank-lines", delete_blank_lines)
/*+
On blank line, delete all surrounding blank lines, leaving just one.
On isolated blank line, delete that one.
On nonblank line, delete any immediately following blank lines.
+*/
{
  Marker *m = point_marker ();
  Region r;
  r.start = r.end = get_buffer_line_o (cur_bp);

  undo_start_sequence ();

  /* Find following blank lines.  */
  if (FUNCALL (forward_line) == leT && is_blank_line ())
    {
      r.start = get_buffer_pt (cur_bp);
      do
        r.end = buffer_next_line (cur_bp, get_buffer_pt (cur_bp));
      while (FUNCALL (forward_line) == leT && is_blank_line ());
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
      while (FUNCALL_ARG (forward_line, -1) == leT && is_blank_line ());
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
}
END_DEFUN
