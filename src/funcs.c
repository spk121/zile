/* Miscellaneous Emacs functions reimplementation
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
   Copyright (c) 2004 David A. Capello.
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

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "zile.h"
#include "extern.h"
#include "vars.h"


DEFUN("suspend-zile", suspend_zile)
/*+
Stop Zile and return to superior process.
+*/
{
  raise(SIGTSTP);
  return TRUE;
}
END_DEFUN

int cancel(void)
{
  deactivate_mark();
  minibuf_error("Quit");
  return FALSE;
}

DEFUN("keyboard-quit", keyboard_quit)
/*+
Cancel current command.
+*/
{
  return cancel();
}
END_DEFUN

DEFUN("transient-mark-mode", transient_mark_mode)
/*+
Toggle Transient Mark mode.
With arg, turn Transient Mark mode on if arg is positive, off otherwise.
+*/
{
  if (!(lastflag & FLAG_SET_UNIARG)) {
    if (transient_mark_mode())
      set_variable("transient-mark-mode", "nil");
    else
      set_variable("transient-mark-mode", "t");
  } else
    set_variable("transient-mark-mode", uniarg > 0 ? "t" : "nil");

  activate_mark();
  return TRUE;
}
END_DEFUN

static char *make_buffer_flags(Buffer *bp, int iscurrent)
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

static astr make_buffer_modeline(Buffer *bp)
{
  astr as = astr_new();

  if (bp->flags & BFLAG_AUTOFILL)
    astr_cat_cstr(as, " Fill");

  return as;
}

static void print_buf(Buffer *old_bp, Buffer *bp)
{
  astr mode = make_buffer_modeline(bp);

  if (bp->name[0] == ' ')
    return;

  bprintf("%3s %-16s %6u  %-13s",
          make_buffer_flags(bp, old_bp == bp),
          bp->name,
          calculate_buffer_size(bp),
          astr_cstr(mode));
  astr_delete(mode);
  if (bp->filename != NULL) {
    astr shortname = shorten_string(bp->filename, 40);
    insert_string(astr_cstr(shortname));
    astr_delete(shortname);
  }
  insert_newline();
}

void write_temp_buffer(const char *name, void (*func)(va_list ap), ...)
{
  Window *wp, *old_wp = cur_wp;
  Buffer *new_bp;
  va_list ap;

  /* Popup a window with the buffer "name". */
  if ((wp = find_window(name)))
    set_current_window(wp);
  else {
    set_current_window(popup_window());
    switch_to_buffer(find_buffer(name, TRUE));
  }

  /* Remove all the content of that buffer. */
  new_bp = create_buffer(cur_bp->name);
  kill_buffer(cur_bp);
  cur_bp = cur_wp->bp = new_bp;

  /* Make the buffer like a temporary one. */
  cur_bp->flags = BFLAG_NEEDNAME | BFLAG_NOSAVE | BFLAG_NOUNDO;
  set_temporary_buffer(cur_bp);

  /* Use the "callback" routine. */
  va_start(ap, func);
  func(ap);
  va_end(ap);

  gotobob();
  cur_bp->flags |= BFLAG_READONLY;

  /* Restore old current window. */
  set_current_window(old_wp);
}

static void write_buffers_list(va_list ap)
{
  Window *old_wp = va_arg(ap, Window *);
  Buffer *bp;

  bprintf(" MR Buffer           Size    Mode         File\n");
  bprintf(" -- ------           ----    ----         ----\n");

  /* Print buffers. */
  bp = old_wp->bp;
  do {
    /* Print all buffer less this one (the *Buffer List*). */
    if (cur_bp != bp)
      print_buf(old_wp->bp, bp);
    if ((bp = bp->next) == NULL)
      bp = head_bp;
  } while (bp != old_wp->bp);
}

DEFUN("list-buffers", list_buffers)
/*+
Display a list of names of existing buffers.
The list is displayed in a buffer named `*Buffer List*'.
Note that buffers with names starting with spaces are omitted.

The M column contains a * for buffers that are modified.
The R column contains a % for buffers that are read-only.
+*/
{
  write_temp_buffer("*Buffer List*", write_buffers_list, cur_wp);
  return TRUE;
}
END_DEFUN

DEFUN("overwrite-mode", overwrite_mode)
/*+
In overwrite mode, printing characters typed in replace existing
text on a one-for-one basis, rather than pushing it to the right.
At the end of a line, such characters extend the line.
C-q still inserts characters in overwrite mode; this is supposed
to make it easier to insert characters when necessary.
+*/
{
  cur_bp->flags ^= BFLAG_OVERWRITE;
  return TRUE;
}
END_DEFUN

DEFUN("toggle-read-only", toggle_read_only)
/*+
Change whether this buffer is visiting its file read-only.
+*/
{
  cur_bp->flags ^= BFLAG_READONLY;
  return TRUE;
}
END_DEFUN

DEFUN("auto-fill-mode", auto_fill_mode)
/*+
Toggle Auto Fill mode.
In Auto Fill mode, inserting a space at a column beyond `fill-column'
automatically breaks the line at a previous space.
+*/
{
  cur_bp->flags ^= BFLAG_AUTOFILL;
  return TRUE;
}
END_DEFUN

DEFUN("set-fill-column", set_fill_column)
/*+
Set the fill column.
If an argument value is passed, set the `fill-column' variable with
that value, otherwise with the current column value.
+*/
{
  if (lastflag & FLAG_SET_UNIARG)
    variableSetNumber(&cur_bp->vars, "fill-column", uniarg);
  else
    variableSetNumber(&cur_bp->vars, "fill-column", (int)(cur_bp->pt.o + 1));

  return TRUE;
}
END_DEFUN

int set_mark_command(void)
{
  set_mark();
  minibuf_write("Mark set");
  return TRUE;
}

DEFUN("set-mark-command", set_mark_command)
/*+
Set mark at where point is.
+*/
{
  int ret = set_mark_command();
  activate_mark();
  return ret;
}
END_DEFUN

int exchange_point_and_mark(void)
{
  /* No mark? */
  if (!cur_bp->mark) {
    minibuf_error("No mark set in this buffer");
    return FALSE;
  }

  /* Swap the point with the mark.  */
  swap_point(&cur_bp->pt, &cur_bp->mark->pt);
  return TRUE;
}


DEFUN("exchange-point-and-mark", exchange_point_and_mark)
/*+
Put the mark where point is now, and point where the mark is now.
+*/
{
  if (!exchange_point_and_mark())
    return FALSE;

  /* In transient-mark-mode we must reactivate the mark.  */
  if (transient_mark_mode())
    activate_mark();

  thisflag |= FLAG_NEED_RESYNC;

  return TRUE;
}
END_DEFUN

DEFUN("mark-whole-buffer", mark_whole_buffer)
/*+
Put point at beginning and mark at end of buffer.
+*/
{
  gotoeob();
  FUNCALL(set_mark_command);
  gotobob();
  return TRUE;
}
END_DEFUN

static int quoted_insert_octal(int c1)
{
  int c2, c3;
  minibuf_write("C-q %d-", c1 - '0');
  c2 = getkey();

  if (!isdigit(c2) || c2 - '0' >= 8) {
    insert_char_in_insert_mode(c1 - '0');
    insert_char_in_insert_mode(c2);
    return TRUE;
  }

  minibuf_write("C-q %d %d-", c1 - '0', c2 - '0');
  c3 = getkey();

  if (!isdigit(c3) || c3 - '0' >= 8) {
    insert_char_in_insert_mode((c1 - '0') * 8 + (c2 - '0'));
    insert_char_in_insert_mode(c3);
    return TRUE;
  }

  insert_char_in_insert_mode((c1 - '8') * 64 + (c2 - '0') * 8 + (c3 - '0'));

  return TRUE;
}

DEFUN("quoted-insert", quoted_insert)
/*+
Read next input character and insert it.
This is useful for inserting control characters.
You may also type up to 3 octal digits, to insert a character with that code.
+*/
{
  int c;

  minibuf_write("C-q-");
  c = xgetkey(GETKEY_UNFILTERED, 0);

  if (isdigit(c) && c - '0' < 8)
    quoted_insert_octal(c);
  else
    insert_char_in_insert_mode(c);

  minibuf_clear();

  return TRUE;
}
END_DEFUN

int universal_argument(int keytype, int xarg)
{
  int i = 0, arg = 4, sgn = 1, digit;
  size_t key;
  astr as = astr_new();

  if (keytype == KBD_META) {
    astr_cpy_cstr(as, "ESC");
    ungetkey((size_t)(xarg + '0'));
  } else
    astr_cpy_cstr(as, "C-u");

  for (;;) {
    astr_cat_char(as, '-'); /* Add the '-' character. */
    key = do_completion(as);
    astr_truncate(as, -1); /* Remove the '-' character. */

    /* Cancelled. */
    if (key == KBD_CANCEL)
      return cancel();
    /* Digit pressed. */
    else if (isdigit(key & 0xff)) {
      digit = (key & 0xff) - '0';

      if (key & KBD_META)
        astr_cat_cstr(as, " ESC");

      astr_afmt(as, " %d", digit);

      if (i == 0)
        arg = digit;
      else
        arg = arg * 10 + digit;

      i++;
    } else if (key == (KBD_CTRL | 'u')) {
      astr_cat_cstr(as, " C-u");
      if (i == 0)
        arg *= 4;
      else
        break;
    } else if (key == '-') {
      /* After any number && if sign doesn't change. */
      if (i == 0 && sgn > 0) {
        sgn = -sgn;
        astr_cat_cstr(as, " -");
        /* The default negative arg isn't -4, it's -1. */
        arg = 1;
      } else if (i != 0) {
        /* If i == 0 do nothing (the Emacs behavior is a little
           strange in this case, it waits for one more key that is
           eaten, and then goes back to the normal state). */
        ungetkey(key);
        break;
      }
    } else {
      ungetkey(key);
      break;
    }
  }

  last_uniarg = arg * sgn;
  thisflag |= FLAG_SET_UNIARG;
  minibuf_clear();
  astr_delete(as);

  return TRUE;
}

DEFUN("universal-argument", universal_argument)
/*+
Begin a numeric argument for the following command.
Digits or minus sign following C-u make up the numeric argument.
C-u following the digits or minus sign ends the argument.
C-u without digits or minus sign provides 4 as argument.
Repeating C-u without digits or minus sign multiplies the argument
by 4 each time.
+*/
{
  return universal_argument(KBD_CTRL | 'u', 0);
}
END_DEFUN

#define TAB_TABIFY	1
#define TAB_UNTABIFY	2

static void edit_tab_line(Line **lp, size_t lineno, size_t offset, size_t size, int action)
{
  char *src, *dest;
  size_t col, i, t = tab_width(cur_bp);

  if (size == 0)
    return;

  src = (char *)zmalloc(size + 1);
  dest = (char *)zmalloc(size * t + 1);
  strncpy(src, astr_cstr((*lp)->item) + offset, size);
  src[size] = '\0';

  /* Get offset's column.  */
  col = 0;
  for (i = 0; i < offset; i++) {
    if (*astr_char((*lp)->item, (ptrdiff_t)i) == '\t')
      col |= t - 1;
    ++col;
  }

  /* Call un/tabify function.  */
  if (action == TAB_UNTABIFY)
    untabify_string(dest, src, col, t);
  else
    tabify_string(dest, src, col, t);

  if (strcmp(src, dest) != 0) {
    undo_save(UNDO_REPLACE_BLOCK, make_point(lineno, offset),
              size, strlen(dest));
    line_replace_text(lp, offset, size, dest, strlen(dest), FALSE);
  }

  free(src);
  free(dest);
}

static int edit_tab_region(int action)
{
  Region r;
  Line *lp;
  size_t lineno;
  Marker *marker;

  if (warn_if_readonly_buffer() || warn_if_no_mark())
    return FALSE;

  calculate_the_region(&r);
  if (r.size == 0)
    return TRUE;

  marker = point_marker();

  undo_save(UNDO_START_SEQUENCE, marker->pt, 0, 0);
  for (lp = r.start.p, lineno = r.start.n; ; lp = list_next(lp), ++lineno) {
    /* First line.  */
    if (lineno == r.start.n) {
      /* Region on a sole line. */
      if (lineno == r.end.n)
        edit_tab_line(&lp, lineno, r.start.o,
                      r.size, action);
      /* Region is multi-line. */
      else
        edit_tab_line(&lp, lineno, r.start.o,
                      astr_len(lp->item) - r.start.o, action);
    }
    /* Last line of multi-line region. */
    else if (lineno == r.end.n)
      edit_tab_line(&lp, lineno, 0, r.end.o, action);
    /* Middle line of multi-line region. */
    else
      edit_tab_line(&lp, lineno, 0, astr_len(lp->item), action);
    /* Done?  */
    if (lineno == r.end.n)
      break;
  }
  cur_bp->pt = marker->pt;
  undo_save(UNDO_END_SEQUENCE, marker->pt, 0, 0);
  free_marker(marker);
  deactivate_mark();

  return TRUE;
}

DEFUN("tabify", tabify)
/*+
Convert multiple spaces in region to tabs when possible.
A group of spaces is partially replaced by tabs
when this can be done without changing the column they end at.
The variable `tab-width' controls the spacing of tab stops.
+*/
{
  return edit_tab_region(TAB_TABIFY);
}
END_DEFUN

DEFUN("untabify", untabify)
/*+
Convert all tabs in region to multiple spaces, preserving columns.
The variable `tab-width' controls the spacing of tab stops.
+*/
{
  return edit_tab_region(TAB_UNTABIFY);
}
END_DEFUN

DEFUN("back-to-indentation", back_to_indentation)
/*+
Move point to the first non-whitespace character on this line.
+*/
{
  cur_bp->pt = line_beginning_position(1);
  while (!eolp()) {
    if (!isspace(following_char()))
      break;
    forward_char();
  }
  return TRUE;
}
END_DEFUN


/***********************************************************************
			  Transpose functions
***********************************************************************/

static void astr_append_region(astr s)
{
  Region r;

  activate_mark();
  calculate_the_region(&r);

  if (r.size > 0) {
    char *t = copy_text_block(r.start.n, r.start.o, r.size);
    if (t) {
      size_t i;
      for (i = 0; i < r.size; i++)
        astr_cat_char(s, t[i]);
      free(t);
    }
  }
}

static int transpose_subr(Function f)
{
  Marker *p0, *p1, *p2;
  astr s1 = NULL;
  astr s2 = NULL;
  int seq_started = FALSE;

  p0 = point_marker();

  /* For transpose-chars. */
  if (f == F_forward_char) {
    if (eolp())
      f(-1, NULL);
  }
  /* For transpose-lines. */
  else if (f == F_forward_line) {
    /* If we are in first line, go to next line. */
    if (list_prev(cur_bp->pt.p) == cur_bp->lines) {
      f(1, NULL);
    }
  }

  /* Backward. */
  if (!f(-1, NULL)) {
    minibuf_error("Beginning of buffer");
    free_marker(p0);
    return FALSE;
  }

  /* Save mark. */
  push_mark();

  /* Mark the beginning of first string. */
  set_mark();
  p1 = point_marker();

  /* Check end of buffer (only to check if we could make the
     operation). */
  if (!f(2, NULL)) {
    /* For transpose-lines. */
    if (f == F_forward_line) {
      if (!seq_started) {
        seq_started = TRUE;
        undo_save(UNDO_START_SEQUENCE, p0->pt, 0, 0);
      }

      /* When last line has characters. */
      if (!is_empty_line())
        /* We must insert the '\n' in the end
           of line (not in the beginning). */
        FUNCALL(end_of_line);

      /* Insert a newline. */
      FUNCALL(newline);
    }
    else {
      pop_mark();
      goto_point(p1->pt);
      minibuf_error("End of buffer");

      free_marker(p0);
      free_marker(p1);
      return FALSE;
    }
  }

  goto_point(p1->pt);

  /* Forward. */
  f(1, NULL);

  /* Save and delete 1st marked region. */
  s1 = astr_new();
  astr_append_region(s1);

  if (!seq_started) {
    seq_started = TRUE;
    undo_save(UNDO_START_SEQUENCE, p0->pt, 0, 0);
  }

  FUNCALL(delete_region);

  /* Forward. */
  f(1, NULL);

  /* For transpose-lines. */
  if (f == F_forward_line) {
    p2 = point_marker();
  }
  else {
    /* Mark the end of second string. */
    set_mark();

    /* Backward. */
    f(-1, NULL);
    p2 = point_marker();

    /* Save and delete the marked region. */
    s2 = astr_new();
    astr_append_region(s2);
    FUNCALL(delete_region);
  }

  /* Insert the second string in the first position. */
  if (s2) {
    goto_point(p1->pt);
    if (astr_len(s2) > 0)
      insert_string(astr_cstr(s2));
  }

  /* Insert the first string in the second position. */
  if (s1) {
    goto_point(p2->pt);
    if (astr_len(s1) > 0)
      insert_string(astr_cstr(s1));
  }

  if (seq_started)
    undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  /* Delete temporary strings. */
  if (s1) astr_delete(s1);
  if (s2) astr_delete(s2);

  /* Restore mark. */
  pop_mark();

  deactivate_mark();

  /* Free markers. */
  free_marker(p0);
  free_marker(p1);
  free_marker(p2);
  return TRUE;
}

DEFUN("transpose-chars", transpose_chars)
/*+
Interchange characters around point, moving forward one character.
With prefix arg ARG, effect is to take character before point
and drag it forward past ARG other characters.  If no argument and at
end of line, the previous two chars are exchanged.
+*/
{
  if (warn_if_readonly_buffer())
    return FALSE;

  if (!(lastflag & FLAG_SET_UNIARG))
    return transpose_subr(F_forward_char);

  minibuf_error("transpose-chars doesn't support uniarg yet");
  return FALSE;
}
END_DEFUN

DEFUN("transpose-words", transpose_words)
/*+
Interchange words around point, leaving point at end of them.
With prefix arg ARG, effect is to take word before or around point
and drag it forward past ARG other words. If ARG is zero, the words
around or after point and around or after mark are interchanged.
+*/
{
  if (warn_if_readonly_buffer())
    return FALSE;

  if (!(lastflag & FLAG_SET_UNIARG))
    return transpose_subr(F_forward_word);

  minibuf_error("transpose-words doesn't support uniarg yet");
  return FALSE;
}
END_DEFUN

DEFUN("transpose-sexps", transpose_sexps)
/*+
Like M-t but applies to sexps.
+*/
{
  if (warn_if_readonly_buffer())
    return FALSE;

  if (!(lastflag & FLAG_SET_UNIARG))
    return transpose_subr(F_forward_sexp);

  minibuf_error("transpose-sexps doesn't support uniarg yet");
  return FALSE;
}
END_DEFUN

DEFUN("transpose-lines", transpose_lines)
/*+
Exchange current line and previous line, leaving point after both.
With argument ARG, takes previous line and moves it past ARG lines.
With argument 0, interchanges line point is in with line mark is in.
+*/
{
  if (warn_if_readonly_buffer())
    return FALSE;

  if (!(lastflag & FLAG_SET_UNIARG))
    return transpose_subr(F_forward_line);

  minibuf_error("transpose-lines doesn't support uniarg yet");
  return FALSE;
}
END_DEFUN


/***********************************************************************
			  Move through words
***********************************************************************/

#define ISWORDCHAR(c)	(isalnum(c) || c == '$')

static int forward_word(void)
{
  int gotword = FALSE;
  for (;;) {
    while (!eolp()) {
      int c = following_char();
      if (!ISWORDCHAR(c)) {
        if (gotword)
          return TRUE;
      } else
        gotword = TRUE;
      cur_bp->pt.o++;
    }
    if (gotword)
      return TRUE;
    cur_bp->pt.o = astr_len(cur_bp->pt.p->item);
    if (!next_line())
      break;
    cur_bp->pt.o = 0;
  }
  return FALSE;
}

DEFUN("forward-word", forward_word)
/*+
Move point forward one word (backward if the argument is negative).
With argument, do this that many times.
+*/
{
  int uni;

  if (uniarg < 0)
    return FUNCALL_ARG(backward_word, -uniarg);

  for (uni = 0; uni < uniarg; ++uni)
    if (!forward_word())
      return FALSE;

  return TRUE;
}
END_DEFUN

static int backward_word(void)
{
  int gotword = FALSE;
  for (;;) {
    if (bolp()) {
      if (!previous_line())
        break;
      cur_bp->pt.o = astr_len(cur_bp->pt.p->item);
    }
    while (!bolp()) {
      int c = preceding_char();
      if (!ISWORDCHAR(c)) {
        if (gotword)
          return TRUE;
      } else
        gotword = TRUE;
      cur_bp->pt.o--;
    }
    if (gotword)
      return TRUE;
  }
  return FALSE;
}

DEFUN("backward-word", backward_word)
/*+
Move backward until encountering the end of a word (forward if the
argument is negative).
With argument, do this that many times.
+*/
{
  int uni;

  if (uniarg < 0)
    return FUNCALL_ARG(forward_word, -uniarg);

  for (uni = 0; uni < uniarg; ++uni)
    if (!backward_word())
      return FALSE;

  return TRUE;
}
END_DEFUN


/***********************************************************************
	       Move through balanced expressions (sexp)
***********************************************************************/

#define ISSEXPCHAR(c)	      (isalnum(c) || c == '$' || c == '_')

#define ISOPENBRACKETCHAR(c)  ((c=='(') || (c=='[') || (c=='{')	||	\
			       ((c=='\"') && !double_quote) ||		\
			       ((c=='\'') && !single_quote))

#define ISCLOSEBRACKETCHAR(c) ((c==')') || (c==']') || (c=='}')	||	\
			       ((c=='\"') && double_quote) ||		\
			       ((c=='\'') && single_quote))

#define ISSEXPSEPARATOR(c)    (ISOPENBRACKETCHAR(c) ||	\
			       ISCLOSEBRACKETCHAR(c))

#define CONTROL_SEXP_LEVEL(open, close)					\
	if (open(c)) {							\
		if (level == 0 && gotsexp)				\
			return TRUE;					\
									\
		level++;						\
		gotsexp = TRUE;						\
		if (c == '\"') double_quote ^= 1;			\
		if (c == '\'') single_quote ^= 1;			\
	}								\
	else if (close(c)) {						\
		if (level == 0 && gotsexp)				\
			return TRUE;					\
									\
		level--;						\
		gotsexp = TRUE;						\
		if (c == '\"') double_quote ^= 1;			\
		if (c == '\'') single_quote ^= 1;			\
									\
		if (level < 0) {					\
			minibuf_error("Scan error: \"Containing "	\
				      "expression ends prematurely\"");	\
			return FALSE;					\
		}							\
	}

int forward_sexp(void)
{
  int gotsexp = FALSE;
  int level = 0;
  int double_quote = 0;
  int single_quote = 0;

  for (;;) {
    while (!eolp()) {
      int c = following_char();

      /* Jump quotes that aren't sexp separators. */
      if (c == '\\'
          && cur_bp->pt.o+1 < astr_len(cur_bp->pt.p->item)
          && ((*astr_char(cur_bp->pt.p->item, (ptrdiff_t)(cur_bp->pt.o + 1))
               == '\"') ||
              (*astr_char(cur_bp->pt.p->item, (ptrdiff_t)(cur_bp->pt.o + 1))
                  == '\''))) {
        cur_bp->pt.o++;
        c = 'a'; /* Convert \' and \" into a word char. */
      }

      CONTROL_SEXP_LEVEL(ISOPENBRACKETCHAR,
                         ISCLOSEBRACKETCHAR);

      cur_bp->pt.o++;

      if (!ISSEXPCHAR(c)) {
        if (gotsexp && level == 0) {
          if (!ISSEXPSEPARATOR(c))
            cur_bp->pt.o--;
          return TRUE;
        }
      }
      else
        gotsexp = TRUE;
    }
    if (gotsexp && level == 0)
      return TRUE;
    cur_bp->pt.o = astr_len(cur_bp->pt.p->item);
    if (!next_line()) {
      if (level != 0)
        minibuf_error("Scan error: \"Unbalanced parentheses\"");
      break;
    }
    cur_bp->pt.o = 0;
  }
  return FALSE;
}

DEFUN("forward-sexp", forward_sexp)
/*+
Move forward across one balanced expression (sexp).
With argument, do it that many times.  Negative arg -N means
move backward across N balanced expressions.
+*/
{
  int uni;

  if (uniarg < 0)
    return FUNCALL_ARG(backward_sexp, -uniarg);

  for (uni = 0; uni < uniarg; ++uni)
    if (!forward_sexp())
      return FALSE;

  return TRUE;
}
END_DEFUN

int backward_sexp(void)
{
  int gotsexp = FALSE;
  int level = 0;
  int double_quote = 1;
  int single_quote = 1;

  for (;;) {
    if (bolp()) {
      if (!previous_line()) {
        if (level != 0)
          minibuf_error("Scan error: \"Unbalanced parentheses\"");
        break;
      }
      cur_bp->pt.o = astr_len(cur_bp->pt.p->item);
    }
    while (!bolp()) {
      int c = preceding_char();

      /* Jump quotes that doesn't are sexp separators.  */
      if (((c == '\'') || (c == '\"'))
          && cur_bp->pt.o-1 > 0
          && (*astr_char(cur_bp->pt.p->item, (ptrdiff_t)(cur_bp->pt.o - 2))
              == '\\')) {
        cur_bp->pt.o--;
        c = 'a'; /* Convert \' and \" like a
                    word char */
      }

      CONTROL_SEXP_LEVEL(ISCLOSEBRACKETCHAR,
                         ISOPENBRACKETCHAR);

      cur_bp->pt.o--;

      if (!ISSEXPCHAR(c)) {
        if (gotsexp && level == 0) {
          if (!ISSEXPSEPARATOR(c))
            cur_bp->pt.o++;
          return TRUE;
        }
      } else
        gotsexp = TRUE;
    }
    if (gotsexp && level == 0)
      return TRUE;
  }
  return FALSE;
}

DEFUN("backward-sexp", backward_sexp)
/*+
Move backward across one balanced expression (sexp).
With argument, do it that many times.  Negative arg -N means
move forward across N balanced expressions.
+*/
{
  int uni;

  if (uniarg < 0)
    return FUNCALL_ARG(forward_sexp, -uniarg);

  for (uni = 0; uni < uniarg; ++uni)
    if (!backward_sexp())
      return FALSE;

  return TRUE;
}
END_DEFUN

DEFUN("mark-word", mark_word)
/*+
Set mark argument words away from point.
+*/
{
  int ret;
  FUNCALL(set_mark_command);
  ret = FUNCALL_ARG(forward_word, uniarg);
  if (ret)
    FUNCALL(exchange_point_and_mark);
  return ret;
}
END_DEFUN

DEFUN("mark-sexp", mark_sexp)
/*+
Set mark argument sexps from point.
The place mark goes is the same place C-M-f would
move to with the same argument.
+*/
{
  int ret;
  FUNCALL(set_mark_command);
  ret = FUNCALL_ARG(forward_sexp, uniarg);
  if (ret)
    FUNCALL(exchange_point_and_mark);
  return ret;
}
END_DEFUN

DEFUN("forward-line", forward_line)
/*+
Move N lines forward (backward if N is negative).
Precisely, if point is on line I, move to the start of line I + N.
+*/
{
  FUNCALL(beginning_of_line);

  if (uniarg < 0) {
    while (uniarg++)
      if (!previous_line())
        return FALSE;
  } else {
    if (uniarg == 0)
      uniarg = 1;

    while (uniarg--)
      if (!next_line())
        return FALSE;
  }

  return TRUE;
}
END_DEFUN

DEFUN("backward-paragraph", backward_paragraph)
/*+
Move backward to start of paragraph.  With argument N, do it N times.
+*/
{
  if (uniarg < 0)
    return FUNCALL_ARG(forward_paragraph, -uniarg);

  do {
    while (is_empty_line() && previous_line())
      ;
    while (!is_empty_line() && previous_line())
      ;
  } while (--uniarg > 0);

  FUNCALL(beginning_of_line);

  return TRUE;
}
END_DEFUN

DEFUN("forward-paragraph", forward_paragraph)
/*+
Move forward to end of paragraph.  With argument N, do it N times.
+*/
{
  if (uniarg < 0)
    return FUNCALL_ARG(backward_paragraph, -uniarg);

  do {
    while (is_empty_line() && next_line())
      ;
    while (!is_empty_line() && next_line())
      ;
  } while (--uniarg > 0);

  if (is_empty_line())
    FUNCALL(beginning_of_line);
  else
    FUNCALL(end_of_line);

  return TRUE;
}
END_DEFUN

DEFUN("mark-paragraph", mark_paragraph)
/*+
Put point at beginning of this paragraph, mark at end.
The paragraph marked is the one that contains point or follows point.
+*/
{
  if (last_command() == F_mark_paragraph) {
    FUNCALL(exchange_point_and_mark);
    FUNCALL_ARG(forward_paragraph, uniarg);
    FUNCALL(exchange_point_and_mark);
  } else {
    FUNCALL_ARG(forward_paragraph, uniarg);
    FUNCALL(set_mark_command);
    FUNCALL_ARG(backward_paragraph, uniarg);
  }

  return TRUE;
}
END_DEFUN

DEFUN("fill-paragraph", fill_paragraph)
/*+
Fill paragraph at or after point.
+*/
{
  int i, start, end;
  Marker *m = point_marker();

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);

  FUNCALL(forward_paragraph);
  end = cur_bp->pt.n;
  if (is_empty_line())
    end--;

  FUNCALL(backward_paragraph);
  start = cur_bp->pt.n;
  if (is_empty_line()) {  /* Move to next line if between two paragraphs. */
    next_line();
    start++;
  }

  for (i = start; i < end; i++) {
    FUNCALL(end_of_line);
    delete_char();
    FUNCALL(delete_horizontal_space);
    insert_char_in_insert_mode(' ');
  }

  FUNCALL(end_of_line);
  while (get_goalc() > (size_t)get_variable_number("fill-column") + 1)
    fill_break_line();

  thisflag &= ~FLAG_DONE_CPCN;

  cur_bp->pt = m->pt;
  free_marker(m);

  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return TRUE;
}
END_DEFUN

#define UPPERCASE		1
#define LOWERCASE		2
#define CAPITALIZE		3

static int setcase_word(int rcase)
{
  int gotword;
  size_t i, size;

  if (!ISWORDCHAR(following_char())) {
    if (!forward_word())
      return FALSE;
    if (!backward_word())
      return FALSE;
  }

  i = cur_bp->pt.o;
  while (i < astr_len(cur_bp->pt.p->item)) {
    if (!ISWORDCHAR(*astr_char(cur_bp->pt.p->item, (ptrdiff_t)i)))
      break;
    ++i;
  }
  size = i-cur_bp->pt.o;
  if (size > 0)
    undo_save(UNDO_REPLACE_BLOCK, cur_bp->pt, size, size);

  gotword = FALSE;
  while (cur_bp->pt.o < astr_len(cur_bp->pt.p->item)) {
    char *p = astr_char(cur_bp->pt.p->item, (ptrdiff_t)cur_bp->pt.o);
    if (!ISWORDCHAR(*p))
      break;
    if (isalpha(*p)) {
      int oldc = *p, newc;
      if (rcase == UPPERCASE)
        newc = toupper(oldc);
      else if (rcase == LOWERCASE)
        newc = tolower(oldc);
      else { /* rcase == CAPITALIZE */
        if (!gotword)
          newc = toupper(oldc);
        else
          newc = tolower(oldc);
      }
      if (oldc != newc) {
        *p = newc;
      }
    }
    gotword = TRUE;
    cur_bp->pt.o++;
  }

  cur_bp->flags |= BFLAG_MODIFIED;

  return TRUE;
}

DEFUN("downcase-word", downcase_word)
/*+
Convert following word (or argument N words) to lower case, moving over.
+*/
{
  int uni, ret = TRUE;

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  for (uni = 0; uni < uniarg; ++uni)
    if (!setcase_word(LOWERCASE)) {
      ret = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return ret;
}
END_DEFUN

DEFUN("upcase-word", upcase_word)
/*+
Convert following word (or argument N words) to upper case, moving over.
+*/
{
  int uni, ret = TRUE;

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  for (uni = 0; uni < uniarg; ++uni)
    if (!setcase_word(UPPERCASE)) {
      ret = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return ret;
}
END_DEFUN

DEFUN("capitalize-word", capitalize_word)
/*+
Capitalize the following word (or argument N words), moving over.
This gives the word(s) a first character in upper case and the rest
lower case.
+*/
{
  int uni, ret = TRUE;

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  for (uni = 0; uni < uniarg; ++uni)
    if (!setcase_word(CAPITALIZE)) {
      ret = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return ret;
}
END_DEFUN

/*
 * Set the region case.
 */
static int setcase_region(int rcase)
{
  Region r;
  Line *lp;
  size_t size, i;

  if (warn_if_readonly_buffer() || warn_if_no_mark())
    return FALSE;

  calculate_the_region(&r);
  size = r.size;

  undo_save(UNDO_REPLACE_BLOCK, r.start, size, size);

  lp = r.start.p;
  i = r.start.o;
  while (size--) {
    if (i < astr_len(lp->item)) {
      if (rcase == UPPERCASE)
        *astr_char(lp->item, (ptrdiff_t)i) =
          toupper(*astr_char(lp->item, (ptrdiff_t)i));
      else
        *astr_char(lp->item, (ptrdiff_t)i) =
          tolower(*astr_char(lp->item, (ptrdiff_t)i));
      ++i;
    } else {
      lp = list_next(lp);
      i = 0;
    }
  }

  cur_bp->flags |= BFLAG_MODIFIED;

  return TRUE;
}

DEFUN("upcase-region", upcase_region)
/*+
Convert the region to upper case.
+*/
{
  return setcase_region(UPPERCASE);
}
END_DEFUN

DEFUN("downcase-region", downcase_region)
/*+
Convert the region to lower case.
+*/
{
  return setcase_region(LOWERCASE);
}
END_DEFUN

static void write_shell_output(va_list ap)
{
  astr out = va_arg(ap, astr);

  insert_string((char *)astr_cstr(out));
}

DEFUN("shell-command", shell_command)
/*+
Reads a line of text using the minibuffer and creates an inferior shell
to execute the line as a command.
Standard input from the command comes from the null device.  If the
shell command produces any output, the output goes to a Zile buffer
named `*Shell Command Output*', which is displayed in another window
but not selected.
If the output is one line, it is displayed in the echo area.
A numeric argument, as in `M-1 M-!' or `C-u M-!', directs this
command to insert any output into the current buffer.
+*/
{
  char *ms;
  FILE *pipe;
  astr out, s;
  int lines = 0;
  astr cmd;

  if ((ms = minibuf_read("Shell command: ", "")) == NULL)
    return cancel();
  if (ms[0] == '\0')
    return FALSE;

  cmd = astr_new();
  astr_afmt(cmd, "%s 2>&1 </dev/null", ms);
  if ((pipe = popen(astr_cstr(cmd), "r")) == NULL) {
    minibuf_error("Cannot open pipe to process");
    return FALSE;
  }
  astr_delete(cmd);

  out = astr_new();
  while ((s = astr_fgets(pipe)) != NULL) {
    ++lines;
    astr_cat(out, s);
    astr_cat_char(out, '\n');
    astr_delete(s);
  }
  pclose(pipe);

  if (lines == 0)
    minibuf_write("(Shell command succeeded with no output)");
  else { /* lines >= 1 */
    if (lastflag & FLAG_SET_UNIARG)
      insert_string((char *)astr_cstr(out));
    else {
      if (lines > 1)
        write_temp_buffer("*Shell Command Output*",
                          write_shell_output, out);
      else /* lines == 1 */
        minibuf_write("%s", astr_cstr(out));
    }
  }
  astr_delete(out);

  return TRUE;
}
END_DEFUN

DEFUN("shell-command-on-region", shell_command_on_region)
/*+
Reads a line of text using the minibuffer and creates an inferior shell
to execute the line as a command; passes the contents of the region as
input to the shell command.
If the shell command produces any output, the output goes to a Zile buffer
named `*Shell Command Output*', which is displayed in another window
but not selected.
If the output is one line, it is displayed in the echo area.
A numeric argument, as in `M-1 M-|' or `C-u M-|', directs output to the
current buffer, then the old region is deleted first and the output replaces
it as the contents of the region.
+*/
{
  char *ms;
  FILE *pipe;
  astr out, s;
  int lines = 0;
  astr cmd;
  char tempfile[] = P_tmpdir "/zileXXXXXX";

  if ((ms = minibuf_read("Shell command: ", "")) == NULL)
    return cancel();
  if (ms[0] == '\0')
    return FALSE;

  if (warn_if_no_mark())
    return FALSE;

  cmd = astr_new();

  {
    Region r;
    char *p;
    int fd;

    fd = mkstemp(tempfile);
    if (fd == -1) {
      minibuf_error("Cannot open temporary file");
      return FALSE;
    }

    calculate_the_region(&r);
    p = copy_text_block(r.start.n, r.start.o, r.size);
    write(fd, p, r.size);
    free(p);

    close(fd);

    astr_afmt(cmd, "%s 2>&1 <%s", ms, tempfile);
  }

  if ((pipe = popen(astr_cstr(cmd), "r")) == NULL) {
    minibuf_error("Cannot open pipe to process");
    return FALSE;
  }
  astr_delete(cmd);

  out = astr_new();
  while (astr_len(s = astr_fgets(pipe)) > 0) {
    ++lines;
    astr_cat(out, s);
    astr_cat_char(out, '\n');
    astr_delete(s);
  }
  astr_delete(s);
  pclose(pipe);
  remove(tempfile);

  if (lines == 0)
    minibuf_write("(Shell command succeeded with no output)");
  else { /* lines >= 1 */
    if (lastflag & FLAG_SET_UNIARG) {
      undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
      {
        Region r;
        calculate_the_region(&r);
        if (cur_bp->pt.p != r.start.p
            || r.start.o != cur_bp->pt.o)
          FUNCALL(exchange_point_and_mark);
        undo_save(UNDO_INSERT_BLOCK, cur_bp->pt, r.size, 0);
        undo_nosave = TRUE;
        while (r.size--)
          FUNCALL(delete_char);
        undo_nosave = FALSE;
      }
      insert_string((char *)astr_cstr(out));
      undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
    } else {
      if (lines > 1)
        write_temp_buffer("*Shell Command Output*",
                          write_shell_output, out);
      else /* lines == 1 */
        minibuf_write("%s", astr_cstr(out));
    }
  }
  astr_delete(out);

  return TRUE;
}
END_DEFUN

DEFUN("delete-region", delete_region)
/*+
Delete the text between point and mark.
+*/
{
  Region r;

  if (warn_if_no_mark())
    return FALSE;

  calculate_the_region(&r);

  if (cur_bp->flags & BFLAG_READONLY) {
    warn_if_readonly_buffer();
  } else {
    size_t size = r.size;

    if (cur_bp->pt.p != r.start.p || r.start.o != cur_bp->pt.o)
      FUNCALL(exchange_point_and_mark);

    undo_save(UNDO_INSERT_BLOCK, cur_bp->pt, size, 0);
    undo_nosave = TRUE;
    while (size--)
      FUNCALL(delete_char);
    undo_nosave = FALSE;
  }

  deactivate_mark();
  return TRUE;
}
END_DEFUN

DEFUN("delete-blank-lines", delete_blank_lines)
/*+
On blank line, delete all surrounding blank lines, leaving just one.
On isolated blank line, delete that one.
On nonblank line, delete any immediately following blank lines.
+*/
{
  Marker *old_marker = point_marker();
  int seq_started = FALSE;

  /* Delete any immediately following blank lines.  */
  if (next_line()) {
    if (is_blank_line()) {
      push_mark();
      FUNCALL(beginning_of_line);
      set_mark();
      activate_mark();
      do {
        if (!FUNCALL(forward_line))
          break;
      } while (is_blank_line());
      if (!seq_started) {
        seq_started = TRUE;
        undo_save(UNDO_START_SEQUENCE, old_marker->pt, 0, 0);
      }
      FUNCALL(delete_region);
      pop_mark();
    }
    previous_line();
  }

  /* Delete any immediately preceding blank lines.  */
  if (is_blank_line()) {
    int forward = TRUE;
    push_mark();
    FUNCALL(beginning_of_line);
    set_mark();
    activate_mark();
    do {
      if (!FUNCALL_ARG(forward_line, -1)) {
        forward = FALSE;
        break;
      }
    } while (is_blank_line());
    if (forward)
      FUNCALL(forward_line);
    if (cur_bp->pt.p != old_marker->pt.p) {
      if (!seq_started) {
        seq_started = TRUE;
        undo_save(UNDO_START_SEQUENCE, old_marker->pt, 0, 0);
      }
      FUNCALL(delete_region);
    }
    pop_mark();
  }

  /* Isolated blank line, delete that one.  */
  if (!seq_started && is_blank_line()) {
    push_mark();
    FUNCALL(beginning_of_line);
    set_mark();
    activate_mark();
    FUNCALL(forward_line);
    FUNCALL(delete_region); /* Just one action, without a
                               sequence. */
    pop_mark();
  }

  cur_bp->pt = old_marker->pt;

  if (seq_started)
    undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  free_marker(old_marker);
  deactivate_mark();

  return TRUE;
}
END_DEFUN
