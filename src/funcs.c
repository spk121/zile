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
+*/
{
  size_t o = get_buffer_pt (cur_bp) - get_buffer_line_o (cur_bp);
  long fill_col = o;
  char *buf = NULL;

  if (arglist == NULL)
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

static void
quoted_insert_octal (int c1)
{
  int c2, c3;
  minibuf_write ("C-q %c-", c1);
  c2 = getkey (GETKEY_DEFAULT);

  if (!isdigit (c2) || c2 - '0' >= 8)
    {
      insert_char (c1 - '0');
      insert_char (c2);
    }
  else
    {
      minibuf_write ("C-q %c %c-", c1, c2);
      c3 = getkey (GETKEY_DEFAULT);

      if (!isdigit (c3) || c3 - '0' >= 8)
        {
          insert_char ((c1 - '0') * 8 + (c2 - '0'));
          insert_char (c3);
        }
      else
        insert_char ((c1 - '0') * 64 + (c2 - '0') * 8 + (c3 - '0'));
    }
}

DEFUN ("quoted-insert", quoted_insert)
/*+
Read next input character and insert it.
This is useful for inserting control characters.
You may also type up to 3 octal digits, to insert a character with that code.
+*/
{
  minibuf_write ("C-q-");
  int *codes;
  size_t n = getkey_unfiltered (GETKEY_DEFAULT, &codes);

  if (n == 1 && isdigit (*codes) && *codes - '0' < 8)
    quoted_insert_octal (*codes);
  else
    for (size_t i = 0; i < n; i++)
      insert_char (codes[i]);

  minibuf_clear ();
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

bool
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
  ok = bool_to_lisp (move_word (1));
}
END_DEFUN

DEFUN ("backward-word", backward_word)
/*+
Move backward until encountering the end of a word (forward if the
argument is negative).
With argument, do this that many times.
+*/
{
  ok = bool_to_lisp (move_word (-1));
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

bool
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
  ok = bool_to_lisp (move_sexp (1));
}
END_DEFUN

DEFUN ("backward-sexp", backward_sexp)
/*+
Move backward across one balanced expression (sexp).
With argument, do it that many times.  Negative arg -N means
move forward across N balanced expressions.
+*/
{
  ok = bool_to_lisp (move_sexp (-1));
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

static le *
transpose (bool (*move_func) (int dir))
{
  if (warn_if_readonly_buffer ())
    return leNIL;

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
      return leNIL;
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
          return leNIL;
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

  return leT;
}

DEFUN ("transpose-chars", transpose_chars)
/*+
Interchange characters around point, moving forward one character.
+*/
{
  ok = transpose (move_char);
}
END_DEFUN

DEFUN ("transpose-words", transpose_words)
/*+
Interchange words around point, leaving point at end of them.
+*/
{
  ok = transpose (move_word);
}
END_DEFUN

DEFUN ("transpose-sexps", transpose_sexps)
/*+
Like @kbd{M-x transpose-words} but applies to sexps.
+*/
{
  ok = transpose (move_sexp);
}
END_DEFUN

DEFUN ("transpose-lines", transpose_lines)
/*+
Exchange current line and previous line, leaving point after both.
+*/
{
  ok = transpose (move_line);
}
END_DEFUN


bool
move_and_mark (int dir, bool (*func) (int dir))
{
  FUNCALL (set_mark_command);
  bool ret = func (dir);
  if (ret)
    FUNCALL (exchange_point_and_mark);
  return ret;
}

DEFUN ("mark-word", mark_word)
/*+
Set mark argument words away from point.
+*/
{
  ok = bool_to_lisp (move_and_mark (1, move_word));
}
END_DEFUN

DEFUN ("mark-sexp", mark_sexp)
/*+
Set mark @i{arg} sexps from point.
The place mark goes is the same place @kbd{C-M-f} would
move to with the same argument.
+*/
{
  ok = bool_to_lisp (move_and_mark (1, move_sexp));
}
END_DEFUN

DEFUN_ARGS ("forward-line", forward_line,
            INT_ARG (n))
/*+
Move N lines forward (backward if N is negative).
Precisely, if point is on line I, move to the start of line I + N.
+*/
{
  INT_INIT (n);
  if (ok == leT)
    {
      FUNCALL (beginning_of_line);
      ok = bool_to_lisp (move_line (n));
    }
}
END_DEFUN

static le *
move_paragraph (bool (*forward) (void), Function line_extremum)
{
  while (is_empty_line () && forward ())
    ;
  while (!is_empty_line () && forward ())
    ;

  if (is_empty_line ())
    FUNCALL (beginning_of_line);
  else
    line_extremum (NULL);

  return leT;
}

DEFUN ("backward-paragraph", backward_paragraph)
/*+
Move backward to start of paragraph.  With argument N, do it N times.
+*/
{
  ok = move_paragraph (previous_line, F_beginning_of_line);
}
END_DEFUN

DEFUN ("forward-paragraph", forward_paragraph)
/*+
Move forward to end of paragraph.  With argument N, do it N times.
+*/
{
  ok = move_paragraph (next_line, F_end_of_line);
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
      FUNCALL (forward_paragraph);
      FUNCALL (exchange_point_and_mark);
    }
  else
    {
      FUNCALL (forward_paragraph);
      FUNCALL (set_mark_command);
      FUNCALL (backward_paragraph);
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

DEFUN ("downcase-word", downcase_word)
/*+
Convert following word to lower case, moving over.
+*/
{
  ok = bool_to_lisp (setcase_word_lowercase ());
}
END_DEFUN

static bool
setcase_word_uppercase (void)
{
  return setcase_word (case_upper);
}

DEFUN ("upcase-word", upcase_word)
/*+
Convert following word to upper case, moving over.
+*/
{
  ok = bool_to_lisp (setcase_word_uppercase ());
}
END_DEFUN

static bool
setcase_word_capitalize (void)
{
  return setcase_word (case_capitalized);
}

DEFUN ("capitalize-word", capitalize_word)
/*+
Capitalize the following word moving over.
This gives the word(s) a first character in upper case
and the rest lower case.
+*/
{
  ok = bool_to_lisp (setcase_word_capitalize ());
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
      while (FUNCALL_ARG (forward_line, -1L) == leT && is_blank_line ());
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
