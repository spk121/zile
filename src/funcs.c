/* Miscellaneous Emacs functions

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.

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

#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "main.h"
#include "extern.h"


DEFUN ("suspend-zile", suspend_zile)
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

DEFUN ("transient-mark-mode", transient_mark_mode)
/*+
Toggle Transient Mark mode.
With arg, turn Transient Mark mode on if arg is positive, off otherwise.
+*/
{
  if (!(lastflag & FLAG_SET_UNIARG))
    {
      if (transient_mark_mode ())
        set_variable ("transient-mark-mode", "nil");
      else
        set_variable ("transient-mark-mode", "t");
    }
  else
    set_variable ("transient-mark-mode", uniarg > 0 ? "t" : "nil");

  activate_mark ();
}
END_DEFUN

static char *
make_buffer_flags (Buffer * bp, int iscurrent)
{
  static char buf[4];

  buf[0] = iscurrent ? '.' : ' ';
  buf[1] = (bp->flags & BFLAG_MODIFIED) ? '*' : ' ';
  /* Display the readonly flag if it is set or the buffer is
     the current buffer, i.e. the `*Buffer List*' buffer. */
  buf[2] = (bp->flags & BFLAG_READONLY || bp == cur_bp) ? '%' : ' ';
  buf[3] = '\0';

  return buf;
}

static astr
make_buffer_modeline (Buffer * bp)
{
  astr as = astr_new ();

  if (bp->flags & BFLAG_AUTOFILL)
    astr_cat_cstr (as, " Fill");

  return as;
}

/*
 * Return a string of maximum length `maxlen', prepending `...'
 * if a cut is needed.
 */
static astr
shorten_string (char *s, int maxlen)
{
  astr as = astr_new ();
  int len = strlen (s);

  if (len <= maxlen)
    astr_cpy_cstr (as, s);
  else
    {
      astr_cpy_cstr (as, "...");
      astr_cat_cstr (as, s + len - maxlen + 3);
    }

  return as;
}

static void
print_buf (Buffer * old_bp, Buffer * bp)
{
  astr mode = make_buffer_modeline (bp);

  if (bp->name[0] == ' ')
    return;

  bprintf ("%3s %-16s %6u  %-13s",
           make_buffer_flags (bp, old_bp == bp),
           bp->name, calculate_buffer_size (bp), astr_cstr (mode));
  astr_delete (mode);
  if (bp->filename != NULL)
    {
      astr shortname = shorten_string (bp->filename, 40);
      insert_astr (shortname);
      astr_delete (shortname);
    }
  insert_newline ();
}

void
write_temp_buffer (const char *name, bool show, void (*func) (va_list ap), ...)
{
  Window *wp, *old_wp = cur_wp;
  Buffer *new_bp, *old_bp = cur_bp;
  va_list ap;

  /* Popup a window with the buffer "name". */
  wp = find_window (name);
  if (show && wp)
    set_current_window (wp);
  else
    {
      if (show)
        set_current_window (popup_window ());
      switch_to_buffer (find_buffer (name, true));
    }

  /* Remove all the content of that buffer. */
  new_bp = create_buffer (cur_bp->name);
  kill_buffer (cur_bp);
  cur_bp = cur_wp->bp = new_bp;

  /* Make the buffer like a temporary one. */
  cur_bp->flags = BFLAG_NEEDNAME | BFLAG_NOSAVE | BFLAG_NOUNDO;
  set_temporary_buffer (cur_bp);

  /* Use the "callback" routine. */
  va_start (ap, func);
  func (ap);
  va_end (ap);

  gotobob ();
  cur_bp->flags |= BFLAG_READONLY;

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

  bprintf (" MR Buffer           Size    Mode         File\n");
  bprintf (" -- ------           ----    ----         ----\n");

  /* Print buffers. */
  bp = old_wp->bp;
  do
    {
      /* Print all buffers less this one (the *Buffer List*). */
      if (cur_bp != bp)
        print_buf (old_wp->bp, bp);
      bp = bp->next;
      if (bp == NULL)
        bp = head_bp;
    }
  while (bp != old_wp->bp);
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

DEFUN ("overwrite-mode", overwrite_mode)
/*+
In overwrite mode, printing characters typed in replace existing
text on a one-for-one basis, rather than pushing it to the right.
At the end of a line, such characters extend the line.
@kbd{C-q} still inserts characters in overwrite mode; this
is supposed to make it easier to insert characters when necessary.
+*/
{
  cur_bp->flags ^= BFLAG_OVERWRITE;
}
END_DEFUN

DEFUN ("toggle-read-only", toggle_read_only)
/*+
Change whether this buffer is visiting its file read-only.
+*/
{
  cur_bp->flags ^= BFLAG_READONLY;
}
END_DEFUN

DEFUN ("auto-fill-mode", auto_fill_mode)
/*+
Toggle Auto Fill mode.
In Auto Fill mode, inserting a space at a column beyond `fill-column'
automatically breaks the line at a previous space.
+*/
{
  cur_bp->flags ^= BFLAG_AUTOFILL;
}
END_DEFUN

DEFUN ("set-fill-column", set_fill_column)
/*+
Set `fill-column' to specified argument.
Use C-u followed by a number to specify a column.
Just C-u as argument means to use the current column.
+*/
{
  long fill_col = (lastflag & FLAG_UNIARG_EMPTY) ?
    cur_bp->pt.o : (unsigned long) uniarg;
  char *buf;

  if (!(lastflag & FLAG_SET_UNIARG) && arglist == NULL)
    {
      fill_col = minibuf_read_number ("Set fill-column to (default %d): ", cur_bp->pt.o);
      if (fill_col == LONG_MAX)
        return leNIL;
      else if (fill_col == LONG_MAX - 1)
        fill_col = cur_bp->pt.o;
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
      xasprintf (&buf, "%ld", fill_col);
      /* Only print message when run interactively. */
      minibuf_write ("Fill column set to %s (was %d)", buf,
                     get_variable_number ("fill-column"));
    }

  if (ok == leT)
    {
      le *branch = leAddDataElement (leAddDataElement (leAddDataElement (NULL, "", 0), "fill-column", 0), buf, 0);
      F_set_variable (0, branch);
      leWipe (branch);
    }

  if (arglist == NULL)
    free (buf);
}
END_DEFUN

void
set_mark_interactive (void)
{
  set_mark ();
  minibuf_write ("Mark set");
}

DEFUN ("set-mark-command", set_mark_command)
/*+
Set mark at where point is.
+*/
{
  set_mark_interactive ();
  activate_mark ();
}
END_DEFUN

DEFUN ("exchange-point-and-mark", exchange_point_and_mark)
/*+
Put the mark where point is now, and point where the mark is now.
+*/
{
  if (!cur_bp->mark)
    {
      minibuf_error ("No mark set in this buffer");
      return leNIL;
    }

  swap_point (&cur_bp->pt, &cur_bp->mark->pt);

  /* In transient-mark-mode we must reactivate the mark.  */
  if (transient_mark_mode ())
    activate_mark ();

  thisflag |= FLAG_NEED_RESYNC;
}
END_DEFUN

DEFUN ("mark-whole-buffer", mark_whole_buffer)
/*+
Put point at beginning and mark at end of buffer.
+*/
{
  gotoeob ();
  FUNCALL (set_mark_command);
  gotobob ();
}
END_DEFUN

static int
quoted_insert_octal (int c1)
{
  int c2, c3;
  minibuf_write ("C-q %d-", c1 - '0');
  c2 = getkey ();

  if (!isdigit (c2) || c2 - '0' >= 8)
    {
      insert_char_in_insert_mode (c1 - '0');
      insert_char_in_insert_mode (c2);
      return true;
    }

  minibuf_write ("C-q %d %d-", c1 - '0', c2 - '0');
  c3 = getkey ();

  if (!isdigit (c3) || c3 - '0' >= 8)
    {
      insert_char_in_insert_mode ((c1 - '0') * 8 + (c2 - '0'));
      insert_char_in_insert_mode (c3);
      return true;
    }

  insert_char_in_insert_mode ((c1 - '8') * 64 + (c2 - '0') * 8 + (c3 - '0'));

  return true;
}

DEFUN ("quoted-insert", quoted_insert)
/*+
Read next input character and insert it.
This is useful for inserting control characters.
You may also type up to 3 octal digits, to insert a character with that code.
+*/
{
  int c;

  minibuf_write ("C-q-");
  c = xgetkey (GETKEY_UNFILTERED, 0);

  if (isdigit (c) && c - '0' < 8)
    quoted_insert_octal (c);
  else
    insert_char_in_insert_mode (c);

  minibuf_clear ();
}
END_DEFUN

le *
universal_argument (int keytype, int xarg)
{
  int i = 0, arg = 4, sgn = 1, digit;
  size_t key;
  astr as = astr_new ();

  thisflag &= ~FLAG_UNIARG_EMPTY;

  if (keytype == KBD_META)
    {
      astr_cpy_cstr (as, "ESC");
      pushkey ((size_t) (xarg + '0'));
    }
  else
    {
      astr_cpy_cstr (as, "C-u");
      thisflag |= FLAG_UNIARG_EMPTY;
    }

  for (;;)
    {
      astr_cat_char (as, '-'); /* Add the '-' character. */
      key = do_binding_completion (as);
      astr_truncate (as, astr_len (as) - 1); /* Remove the '-' character. */

      /* Cancelled. */
      if (key == KBD_CANCEL)
        return FUNCALL (keyboard_quit);
      /* Digit pressed. */
      else if (isdigit (key & 0xff))
        {
          digit = (key & 0xff) - '0';
          thisflag &= ~FLAG_UNIARG_EMPTY;

          if (key & KBD_META)
            astr_cat_cstr (as, " ESC");

          astr_afmt (as, " %d", digit);

          if (i == 0)
            arg = digit;
          else
            arg = arg * 10 + digit;

          i++;
        }
      else if (key == (KBD_CTRL | 'u'))
        {
          astr_cat_cstr (as, " C-u");
          if (i == 0)
            arg *= 4;
          else
            break;
        }
      else if (key == '-' && i == 0)
        {
          if (sgn > 0)
            {
              sgn = -sgn;
              astr_cat_cstr (as, " -");
              /* The default negative arg isn't -4, it's -1. */
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

  last_uniarg = arg * sgn;
  thisflag |= FLAG_SET_UNIARG;
  minibuf_clear ();
  astr_delete (as);

  return leT;
}

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
  ok = universal_argument (KBD_CTRL | 'u', 0);
}
END_DEFUN

/*
 * Compact the spaces into tabulations according to the `tw' tab width.
 */
static void
tabify_string (char *dest, char *src, size_t scol, size_t tw)
{
  char *sp, *dp;
  size_t dcol = scol, ocol = scol;

  for (sp = src, dp = dest;; ++sp)
    switch (*sp)
      {
      case ' ':
        ++dcol;
        break;
      case '\t':
        dcol += tw;
        dcol -= dcol % tw;
        break;
      default:
        while (((ocol + tw) - (ocol + tw) % tw) <= dcol)
          {
            if (ocol + 1 == dcol)
              break;
            *dp++ = '\t';
            ocol += tw;
            ocol -= ocol % tw;
          }
        while (ocol < dcol)
          {
            *dp++ = ' ';
            ocol++;
          }
        *dp++ = *sp;
        if (*sp == '\0')
          return;
        ++ocol;
        ++dcol;
      }
}

/*
 * Expand the tabulations into spaces according to the `tw' tab width.
 * The output buffer should be big enough to contain the expanded string,
 * i.e. strlen (src) * tw + 1.
 */
static void
untabify_string (char *dest, char *src, size_t scol, size_t tw)
{
  char *sp, *dp;
  int col = scol;

  for (sp = src, dp = dest; *sp != '\0'; ++sp)
    if (*sp == '\t')
      {
        do
          *dp++ = ' ', ++col;
        while ((col % tw) > 0);
      }
    else
      *dp++ = *sp, ++col;
  *dp = '\0';
}

#define TAB_TABIFY	1
#define TAB_UNTABIFY	2
static void
edit_tab_line (Line ** lp, size_t lineno, size_t offset, size_t size,
               int action)
{
  char *src, *dest;
  size_t col, i, t = tab_width (cur_bp);

  if (size == 0)
    return;

  src = (char *) xzalloc (size + 1);
  dest = (char *) xzalloc (size * t + 1);
  strncpy (src, astr_cstr ((*lp)->text) + offset, size);
  src[size] = '\0';

  /* Get offset's column.  */
  col = 0;
  for (i = 0; i < offset; i++)
    {
      if (astr_get ((*lp)->text, i) == '\t')
        col |= t - 1;
      ++col;
    }

  /* Call un/tabify function.  */
  if (action == TAB_UNTABIFY)
    untabify_string (dest, src, col, t);
  else
    tabify_string (dest, src, col, t);

  if (strcmp (src, dest) != 0)
    {
      undo_save (UNDO_REPLACE_BLOCK, make_point (lineno, offset),
                 size, strlen (dest));
      line_replace_text (lp, offset, size, dest, false);
    }

  free (src);
  free (dest);
}

static le *
edit_tab_region (int action)
{
  Region r;
  Line *lp;
  size_t lineno;
  Marker *marker;

  if (warn_if_readonly_buffer () || !calculate_the_region (&r))
    return leNIL;

  if (r.size == 0)
    return leT;

  marker = point_marker ();

  undo_save (UNDO_START_SEQUENCE, marker->pt, 0, 0);
  for (lp = r.start.p, lineno = r.start.n;; lp = lp->next, ++lineno)
    {
      /* First line.  */
      if (lineno == r.start.n)
        {
          /* Region on a sole line. */
          if (lineno == r.end.n)
            edit_tab_line (&lp, lineno, r.start.o, r.size, action);
          /* Region is multi-line. */
          else
            edit_tab_line (&lp, lineno, r.start.o,
                           astr_len (lp->text) - r.start.o, action);
        }
      /* Last line of multi-line region. */
      else if (lineno == r.end.n)
        edit_tab_line (&lp, lineno, 0, r.end.o, action);
      /* Middle line of multi-line region. */
      else
        edit_tab_line (&lp, lineno, 0, astr_len (lp->text), action);
      /* Done?  */
      if (lineno == r.end.n)
        break;
    }
  cur_bp->pt = marker->pt;
  undo_save (UNDO_END_SEQUENCE, marker->pt, 0, 0);
  free_marker (marker);
  deactivate_mark ();

  return leT;
}

DEFUN ("tabify", tabify)
/*+
Convert multiple spaces in region to tabs when possible.
A group of spaces is partially replaced by tabs
when this can be done without changing the column they end at.
The variable `tab-width' controls the spacing of tab stops.
+*/
{
  ok = edit_tab_region (TAB_TABIFY);
}
END_DEFUN

DEFUN ("untabify", untabify)
/*+
Convert all tabs in region to multiple spaces, preserving columns.
The variable @samp{tab-width} controls the spacing of tab stops.
+*/
{
  ok = edit_tab_region (TAB_UNTABIFY);
}
END_DEFUN

DEFUN ("back-to-indentation", back_to_indentation)
/*+
Move point to the first non-whitespace character on this line.
+*/
{
  cur_bp->pt = line_beginning_position (1);
  while (!eolp ())
    {
      if (!isspace (following_char ()))
        break;
      forward_char ();
    }
}
END_DEFUN

/***********************************************************************
                          Move through words
***********************************************************************/
#define ISWORDCHAR(c)	(isalnum (c) || c == '$')
static int
move_word (int dir, int (*next_char) (void), int (*move_char) (void), int (*at_extreme) (void))
{
  int gotword = false;
  for (;;)
    {
      while (!at_extreme ())
        {
          int c = next_char ();
          if (!ISWORDCHAR (c))
            {
              if (gotword)
                return true;
            }
          else
            gotword = true;
          cur_bp->pt.o += dir;
        }
      if (gotword)
        return true;
      if (!move_char ())
        break;
    }
  return false;
}

static int
forward_word (void)
{
  return move_word (1, following_char, forward_char, eolp);
}

static int
backward_word (void)
{
  return move_word (-1, preceding_char, backward_char, bolp);
}

DEFUN ("forward-word", forward_word)
/*+
Move point forward one word (backward if the argument is negative).
With argument, do this that many times.
+*/
{
  ok = execute_with_uniarg (false, uniarg, forward_word, backward_word);
}
END_DEFUN

DEFUN ("backward-word", backward_word)
/*+
Move backward until encountering the end of a word (forward if the
argument is negative).
With argument, do this that many times.
+*/
{
  ok = execute_with_uniarg (false, uniarg, backward_word, forward_word);
}
END_DEFUN

/***********************************************************************
               Move through balanced expressions (sexp)
***********************************************************************/
#define ISSEXPCHAR(c)         (isalnum (c) || c == '$' || c == '_')
#define ISOPENBRACKETCHAR(c)  ((c == '(') || (c == '[') || ( c== '{') ||\
                               ((c == '\"') && !double_quote) ||	\
                               ((c == '\'') && !single_quote))
#define ISCLOSEBRACKETCHAR(c) ((c == ')') || (c == ']') || (c == '}') ||\
                               ((c == '\"') && double_quote) ||		\
                               ((c == '\'') && single_quote))
#define ISSEXPSEPARATOR(c)    (ISOPENBRACKETCHAR (c) ||	\
                               ISCLOSEBRACKETCHAR (c))
#define PRECEDINGQUOTEDQUOTE(c) (c == '\\' \
    && cur_bp->pt.o + 1 < astr_len (cur_bp->pt.p->text) \
    && ((astr_get (cur_bp->pt.p->text, cur_bp->pt.o + 1) == '\"') || \
        (astr_get (cur_bp->pt.p->text, cur_bp->pt.o + 1) == '\'')))
#define FOLLOWINGQUOTEDQUOTE(c) (c == '\\' \
    && cur_bp->pt.o + 1 < astr_len (cur_bp->pt.p->text) \
    && ((astr_get (cur_bp->pt.p->text, cur_bp->pt.o + 1) == '\"') || \
        (astr_get (cur_bp->pt.p->text, cur_bp->pt.o + 1) == '\'')))

static int
move_sexp (int dir)
{
  int gotsexp = false;
  int level = 0;
  int double_quote = dir < 0;
  int single_quote = dir < 0;

  for (;;)
    {
      while (dir > 0 ? !eolp () : !bolp ())
        {
          int c = dir > 0 ? following_char () : preceding_char ();

          /* Jump quotes that aren't sexp separators. */
          if (dir > 0 ? PRECEDINGQUOTEDQUOTE (c) : FOLLOWINGQUOTEDQUOTE (c))
            {
              cur_bp->pt.o += dir;
              c = 'a';		/* Treat ' and " like word chars. */
            }

          if (dir > 0 ? ISOPENBRACKETCHAR (c) : ISCLOSEBRACKETCHAR (c))
            {
              if (level == 0 && gotsexp)
                return true;

              level++;
              gotsexp = true;
              if (c == '\"')
                double_quote ^= 1;
              if (c == '\'')
                single_quote ^= 1;
            }
          else if (dir > 0 ? ISCLOSEBRACKETCHAR (c) : ISOPENBRACKETCHAR (c))
            {
              if (level == 0 && gotsexp)
                return true;

              level--;
              gotsexp = true;
              if (c == '\"')
                double_quote ^= 1;
              if (c == '\'')
                single_quote ^= 1;

              if (level < 0)
                {
                  minibuf_error ("Scan error: \"Containing "
                                 "expression ends prematurely\"");
                  return false;
                }
            }

          cur_bp->pt.o += dir;

          if (!ISSEXPCHAR (c))
            {
              if (gotsexp && level == 0)
                {
                  if (!ISSEXPSEPARATOR (c))
                    cur_bp->pt.o -= dir;
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
      cur_bp->pt.o = dir > 0 ? 0 : astr_len (cur_bp->pt.p->text);
    }
  return false;
}

static int
forward_sexp (void)
{
  return move_sexp (1);
}

static int
backward_sexp (void)
{
  return move_sexp (-1);
}

DEFUN ("forward-sexp", forward_sexp)
/*+
Move forward across one balanced expression (sexp).
With argument, do it that many times.  Negative arg -N means
move backward across N balanced expressions.
+*/
{
  ok = execute_with_uniarg (false, uniarg, forward_sexp, backward_sexp);
}
END_DEFUN

DEFUN ("backward-sexp", backward_sexp)
/*+
Move backward across one balanced expression (sexp).
With argument, do it that many times.  Negative arg -N means
move forward across N balanced expressions.
+*/
{
  ok = execute_with_uniarg (false, uniarg, backward_sexp, forward_sexp);
}
END_DEFUN

/***********************************************************************
                          Transpose functions
***********************************************************************/
static void
astr_append_region (astr s)
{
  Region r;
  char *t;

  activate_mark ();
  calculate_the_region (&r);

  t = copy_text_block (r.start.n, r.start.o, r.size);
  astr_ncat_cstr (s, t, r.size);
  free (t);
}

static bool
transpose_subr (int (*forward_func) (void), int (*backward_func) (void))
{
  Marker *p0 = point_marker (), *p1, *p2;
  astr as1, as2 = NULL;

  /* For transpose-chars. */
  if (forward_func == forward_char && eolp ())
    backward_func ();
  /* For transpose-lines. */
  if (forward_func == next_line && cur_bp->pt.n == 0)
    forward_func ();

  /* Backward. */
  if (!backward_func ())
    {
      minibuf_error ("Beginning of buffer");
      free_marker (p0);
      return false;
    }

  /* Save mark. */
  push_mark ();

  /* Mark the beginning of first string. */
  set_mark ();
  p1 = point_marker ();

  /* Check to make sure we can go forwards twice. */
  if (!forward_func () || !forward_func ())
    {
      if (forward_func == next_line)
        { /* Add an empty line. */
          FUNCALL (end_of_line);
          FUNCALL (newline);
        }
      else
        {
          pop_mark ();
          goto_point (p1->pt);
          minibuf_error ("End of buffer");

          free_marker (p0);
          free_marker (p1);
          return false;
        }
    }

  goto_point (p1->pt);

  /* Forward. */
  forward_func ();

  /* Save and delete 1st marked region. */
  as1 = astr_new ();
  astr_append_region (as1);

  free_marker (p0);

  FUNCALL (delete_region);

  /* Forward. */
  forward_func ();

  /* For transpose-lines. */
  if (forward_func == next_line)
    p2 = point_marker ();
  else
    {
      /* Mark the end of second string. */
      set_mark ();

      /* Backward. */
      backward_func ();
      p2 = point_marker ();

      /* Save and delete 2nd marked region. */
      as2 = astr_new ();
      astr_append_region (as2);
      FUNCALL (delete_region);
    }

  /* Insert the first string. */
  goto_point (p2->pt);
  free_marker (p2);
  insert_astr (as1);
  astr_delete (as1);

  /* Insert the second string. */
  if (as2)
    {
      goto_point (p1->pt);
      insert_astr (as2);
      astr_delete (as2);
    }
  free_marker (p1);

  /* Restore mark. */
  pop_mark ();
  deactivate_mark ();

  /* Move forward if necessary. */
  if (forward_func != next_line)
    forward_func ();

  return true;
}

static le *
transpose (int uniarg, int (*forward_func) (void), int (*backward_func) (void))
{
  int uni, ret = true;

  if (warn_if_readonly_buffer ())
    return leNIL;

  if (uniarg < 0)
    {
      int (*tmp_func) (void) = forward_func;
      forward_func = backward_func;
      backward_func = tmp_func;
      uniarg = -uniarg;
    }
  undo_save (UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  for (uni = 0; ret && uni < uniarg; ++uni)
    ret = transpose_subr (forward_func, backward_func);
  undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

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
  ok = transpose (uniarg, forward_char, backward_char);
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
  ok = transpose (uniarg, forward_word, backward_word);
}
END_DEFUN

DEFUN ("transpose-sexps", transpose_sexps)
/*+
Like @kbd{M-x transpose-words} but applies to sexps.
+*/
{
  ok = transpose (uniarg, forward_sexp, backward_sexp);
}
END_DEFUN

DEFUN ("transpose-lines", transpose_lines)
/*+
Exchange current line and previous line, leaving point after both.
With argument ARG, takes previous line and moves it past ARG lines.
With argument 0, interchanges line point is in with line mark is in.
+*/
{
  ok = transpose (uniarg, next_line, previous_line);
}
END_DEFUN


static le *
mark (int uniarg, Function func)
{
  le * ret;
  FUNCALL (set_mark_command);
  ret = func (uniarg, NULL);
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
            INT_ARG (n))
/*+
Move N lines forward (backward if N is negative).
Precisely, if point is on line I, move to the start of line I + N.
+*/
{
  INT_INIT (n)
  else
    n = uniarg;
  if (ok == leT)
    {
      FUNCALL (beginning_of_line);
      ok = execute_with_uniarg (false, n, next_line, previous_line);
    }
  INT_FREE (n);
}
END_DEFUN

static le *
move_paragraph (int uniarg, int (*forward) (void), int (*backward) (void),
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
    line_extremum (1, NULL);

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
  int i, start, end;
  Marker *m = point_marker ();

  undo_save (UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);

  FUNCALL (forward_paragraph);
  end = cur_bp->pt.n;
  if (is_empty_line ())
    end--;

  FUNCALL (backward_paragraph);
  start = cur_bp->pt.n;
  if (is_empty_line ())
    { /* Move to next line if between two paragraphs. */
      next_line ();
      start++;
    }

  for (i = start; i < end; i++)
    {
      FUNCALL (end_of_line);
      delete_char ();
      FUNCALL (just_one_space);
    }

  FUNCALL (end_of_line);
  while (get_goalc () > (size_t) get_variable_number ("fill-column") + 1
         && fill_break_line ())
    ;

  thisflag &= ~FLAG_DONE_CPCN;

  cur_bp->pt = m->pt;
  free_marker (m);

  undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
}
END_DEFUN

/* FIXME: Use astr_recase. */
static int
setcase_word (int rcase)
{
  int gotword;
  size_t i, size;
  char c;

  if (!ISWORDCHAR (following_char ()))
    if (!forward_word () || !backward_word ())
      return false;

  for (i = cur_bp->pt.o;
       i < astr_len (cur_bp->pt.p->text) &&
         ISWORDCHAR ((int) astr_get (cur_bp->pt.p->text, i));
       i++)
    ;
  size = i - cur_bp->pt.o;
  if (size > 0)
    undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, size, size);

  gotword = false;
  for (gotword = false;
       cur_bp->pt.o < astr_len (cur_bp->pt.p->text) &&
         ISWORDCHAR ((c = astr_get (cur_bp->pt.p->text, cur_bp->pt.o)));
       cur_bp->pt.o++, gotword = true)
    {
      if (isalpha ((int) c))
        {
          switch (rcase)
            {
            case UPPERCASE:
              c = toupper (c);
              break;
            case LOWERCASE:
              c = tolower (c);
              break;
            case CAPITALIZED:
              c = (gotword ? tolower : toupper) (c);
              break;
            default:
              break;
            }
          astr_nreplace_cstr (cur_bp->pt.p->text, cur_bp->pt.o, 1, &c, 1);
        }
    }

  cur_bp->flags |= BFLAG_MODIFIED;

  return true;
}

static int
setcase_word_lowercase (void)
{
  return setcase_word (LOWERCASE);
}

DEFUN ("downcase-word", downcase_word)
/*+
Convert following word (or argument N words) to lower case, moving over.
+*/
{
  ok = execute_with_uniarg (true, uniarg, setcase_word_lowercase, NULL);
}
END_DEFUN

static int
setcase_word_uppercase (void)
{
  return setcase_word (UPPERCASE);
}

DEFUN ("upcase-word", upcase_word)
/*+
Convert following word (or argument N words) to upper case, moving over.
+*/
{
  ok = execute_with_uniarg (true, uniarg, setcase_word_uppercase, NULL);
}
END_DEFUN

static int
setcase_word_capitalize (void)
{
  return setcase_word (CAPITALIZED);
}

DEFUN ("capitalize-word", capitalize_word)
/*+
Capitalize the following word (or argument N words), moving over.
This gives the word(s) a first character in upper case and the rest
lower case.
+*/
{
  ok = execute_with_uniarg (true, uniarg, setcase_word_capitalize, NULL);
}
END_DEFUN

/*
 * Set the region case.
 */
static le *
setcase_region (int rcase)
{
  Region r;
  Line *lp;
  size_t i;
  int (*func) (int) = rcase == UPPERCASE ? toupper : tolower;

  if (warn_if_readonly_buffer () || !calculate_the_region (&r))
    return leNIL;

  undo_save (UNDO_REPLACE_BLOCK, r.start, r.size, r.size);

  lp = r.start.p;
  i = r.start.o;
  while (r.size--)
    {
      if (i < astr_len (lp->text))
        {
          char c = func (astr_get (lp->text, i));
          astr_nreplace_cstr (lp->text, i, 1, &c, 1);
          ++i;
        }
      else
        {
          lp = lp->next;
          i = 0;
        }
    }

  cur_bp->flags |= BFLAG_MODIFIED;

  return leT;
}

DEFUN ("upcase-region", upcase_region)
/*+
Convert the region to upper case.
+*/
{
  ok = setcase_region (UPPERCASE);
}
END_DEFUN

DEFUN ("downcase-region", downcase_region)
/*+
Convert the region to lower case.
+*/
{
  ok = setcase_region (LOWERCASE);
}
END_DEFUN

static void
write_shell_output (va_list ap)
{
  astr out = va_arg (ap, astr);

  insert_astr (out);
}

static bool
pipe_command (const char *cmd, const char *tempfile, bool insert, bool replace)
{
  astr out;
  bool more_than_one_line = false;
  char *cmdline, *eol;
  FILE * pipe;

  xasprintf (&cmdline, "%s 2>&1 <%s", cmd, tempfile);
  pipe = popen (cmdline, "r");
  if (pipe == NULL)
    {
      minibuf_error ("Cannot open pipe to process");
      return false;
    }
  free (cmdline);

  out = astr_fread (pipe);
  pclose (pipe);
  eol = strchr (astr_cstr (out), '\n');
  if (eol && eol != astr_cstr (out) + astr_len (out) - 1)
    more_than_one_line = true;

  if (astr_len (out) == 0)
    minibuf_write ("(Shell command succeeded with no output)");
  else
    {
      if (insert)
        {
          if (replace)
            {
              undo_save (UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
              FUNCALL (delete_region);
            }
            insert_astr (out);
            if (replace)
              undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
        }
      else
        {
          write_temp_buffer ("*Shell Command Output*", more_than_one_line,
                             write_shell_output, out);
          if (!more_than_one_line)
            minibuf_write ("%s", astr_cstr (out));
        }
    }
  astr_delete (out);

  return true;
}

static char *
minibuf_read_shell_command (void)
{
  char *ms = minibuf_read ("Shell command: ", "");

  if (ms == NULL)
    {
      FUNCALL (keyboard_quit);
      return NULL;
    }
  if (ms[0] == '\0')
    {
      free (ms);
      return NULL;
    }

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
    ok = bool_to_lisp (pipe_command (cmd, "/dev/null", insert, false));

  STR_FREE (cmd);
  BOOL_FREE (insert);
}
END_DEFUN

DEFUN_ARGS ("shell-command-on-region", shell_command_on_region,
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
  STR_INIT (cmd)
  else
    cmd = minibuf_read_shell_command ();
  BOOL_INIT (insert)
  else
    insert = lastflag & FLAG_SET_UNIARG;

  if (cmd != NULL)
    {
      Region r;

      if (!calculate_the_region (&r))
        ok = leNIL;
      else
        {
          char tempfile[] = P_tmpdir "/zileXXXXXX";
          int fd = mkstemp (tempfile);

          if (fd == -1)
            {
              minibuf_error ("Cannot open temporary file");
              ok = leNIL;
            }
          else
            {
              char *p = copy_text_block (r.start.n, r.start.o, r.size);
              ssize_t written = write (fd, p, r.size);

              free (p);
              close (fd);

              if (written != (ssize_t) r.size)
                {
                  if (written == -1)
                    minibuf_error ("Error writing to temporary file: %s",
                                   strerror (errno));
                  else
                    minibuf_error ("Error writing to temporary file");
                  ok = leNIL;
                }
            }

          if (ok == leT)
            ok = bool_to_lisp (pipe_command (cmd, tempfile, insert, true));

          remove (tempfile);
        }
    }

  STR_FREE (cmd);
}
END_DEFUN

bool
delete_region (const Region * r)
{
  size_t size = r->size;

  if (warn_if_readonly_buffer ())
    return false;

  if (cur_bp->pt.p != r->start.p || r->start.o != cur_bp->pt.o)
    FUNCALL (exchange_point_and_mark);

  undo_save (UNDO_REPLACE_BLOCK, cur_bp->pt, size, 0);
  undo_nosave = true;
  while (size--)
    FUNCALL (delete_char);
  undo_nosave = false;

  return true;
}

DEFUN ("delete-region", delete_region)
/*+
Delete the text between point and mark.
+*/
{
  Region r;

  if (!calculate_the_region (&r))
    ok = leNIL;
  else if (!delete_region (&r))
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
  Marker *old_marker = point_marker ();
  int seq_started = false;

  /* Delete any immediately following blank lines.  */
  if (next_line ())
    {
      if (is_blank_line ())
        {
          push_mark ();
          FUNCALL (beginning_of_line);
          set_mark ();
          activate_mark ();
          while (FUNCALL (forward_line) == leT && is_blank_line ())
            ;
          if (!seq_started)
            {
              seq_started = true;
              undo_save (UNDO_START_SEQUENCE, old_marker->pt, 0, 0);
            }
          FUNCALL (delete_region);
          pop_mark ();
        }
      previous_line ();
    }

  /* Delete any immediately preceding blank lines.  */
  if (is_blank_line ())
    {
      int forward = true;
      push_mark ();
      FUNCALL (beginning_of_line);
      set_mark ();
      activate_mark ();
      do
        {
          if (!FUNCALL_ARG (forward_line, -1))
            {
              forward = false;
              break;
            }
        }
      while (is_blank_line ());
      if (forward)
        FUNCALL (forward_line);
      if (cur_bp->pt.p != old_marker->pt.p)
        {
          if (!seq_started)
            {
              seq_started = true;
              undo_save (UNDO_START_SEQUENCE, old_marker->pt, 0, 0);
            }
          FUNCALL (delete_region);
        }
      pop_mark ();
    }

  /* Isolated blank line, delete that one.  */
  if (!seq_started && is_blank_line ())
    {
      push_mark ();
      FUNCALL (beginning_of_line);
      set_mark ();
      activate_mark ();
      FUNCALL (forward_line);
      FUNCALL (delete_region);	/* Just one action, without a
                                   sequence. */
      pop_mark ();
    }

  cur_bp->pt = old_marker->pt;

  if (seq_started)
    undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  free_marker (old_marker);
  deactivate_mark ();
}
END_DEFUN
