/* Search and replace functions

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

#include <limits.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "regex.h"

#include "main.h"
#include "extern.h"

/* Downcasing table for case-folding search */
static char id[UCHAR_MAX + 1];
static char downcase[UCHAR_MAX + 1];

void
init_search (void)
{
  int i;

  for (i = 0; i <= UCHAR_MAX; i++)
    {
      id[i] = i;
      downcase[i] = tolower (i);
    }
}

/* Return true if there are no upper-case letters in the given string.
   If `regex' is true, ignore escaped letters. */
static bool
no_upper (const char *s, size_t len, int regex)
{
  int quote_flag = 0;
  size_t i;

  for (i = 0; i < len; i++)
    {
      if (regex && s[i] == '\\')
        quote_flag = !quote_flag;
      else if (!quote_flag && isupper ((int) s[i]))
        return false;
    }

  return true;
}

static const char *
fold_table (const char *s, int regex)
{
  if (get_variable_bool ("case-fold-search")
      && no_upper (s, strlen (s), regex))
    return downcase;
  else
    return id;
}

static const char *
find_substr (const char *s1, size_t s1size,
             const char *s2, size_t s2size,
             const char translate[UCHAR_MAX + 1])
{
  const char *e1 = s1 + s1size, *e2 = s2 + s2size;

  for (; s1 <= e1 - s2size; s1++)
    {
      const char *sp1 = s1, *sp2 = s2;

      while (translate[(unsigned char) *sp1++] ==
             translate[(unsigned char) *sp2++])
        if (sp2 == e2)
          return sp1;
    }

  return NULL;
}

static const char *
rfind_substr (const char *s1, size_t s1size,
              const char *s2, size_t s2size,
              const char translate[UCHAR_MAX + 1])
{
  const char *e1 = s1 + s1size, *e2 = s2 + s2size;

  for (; e1 >= s1 + s2size; e1--)
    {
      const char *sp1 = e1, *sp2 = e2;

      while (translate[(unsigned char) *--sp1] ==
             translate[(unsigned char) *--sp2])
        if (sp2 == s2)
          return sp1;
    }

  return NULL;
}

static const char *re_find_err = NULL;

static char *
re_find_substr (const char *s1, size_t s1size,
                const char *s2, size_t s2size,
                int bol, int eol, int backward,
                const char translate[UCHAR_MAX + 1])
{
  struct re_pattern_buffer pattern;
  struct re_registers search_regs;
  char *ret = NULL;
  int index;
  reg_syntax_t old_syntax = re_set_syntax (RE_SYNTAX_EMACS);

  search_regs.num_regs = 1;

  /* translate table is never written to, so this cast is safe */
  pattern.translate = (unsigned char *) translate;
  pattern.fastmap = NULL;
  pattern.buffer = NULL;
  pattern.allocated = 0;

  re_find_err = re_compile_pattern (s2, (int) s2size, &pattern);
  if (!re_find_err)
    {
      pattern.not_bol = !bol;
      pattern.not_eol = !eol;

      if (!backward)
        index = re_search (&pattern, s1, (int) s1size, 0, (int) s1size,
                           &search_regs);
      else
        index =
          re_search (&pattern, s1, (int) s1size, (int) s1size, -(int) s1size,
                     &search_regs);

      if (index >= 0)
        {
          if (!backward)
            ret = ((char *) s1) + search_regs.end[0];
          else
            ret = ((char *) s1) + search_regs.start[0];
        }
      else if (index == -1)
        {
          /* no match */
        }
      else
        {
          /* error */
        }
    }

  re_set_syntax (old_syntax);
  pattern.translate = NULL;	/* regfree requires translate to be NULL
                                   or malloced */
  regfree (&pattern);

  return ret;
}

static void
goto_linep (Line * lp)
{
  Point pt;
  set_buffer_pt (cur_bp, point_min ());
  resync_redisplay ();
  for (pt = get_buffer_pt (cur_bp); pt.p != lp; pt = get_buffer_pt (cur_bp))
    next_line ();
}

static int
search_forward (Line * startp, size_t starto, const char *s, int regexp)
{
  Line *lp;
  const char *sp, *sp2, *translate = fold_table (s, regexp);
  size_t s1size, s2size = strlen (s);

  if (s2size < 1)
    return false;

  for (lp = startp; lp != get_buffer_lines (cur_bp); lp = get_line_next (lp))
    {
      if (lp == startp)
        {
          sp = astr_cstr (get_line_text (lp)) + starto;
          s1size = astr_len (get_line_text (lp)) - starto;
        }
      else
        {
          sp = astr_cstr (get_line_text (lp));
          s1size = astr_len (get_line_text (lp));
        }
      if (s1size < 1)
        continue;

      if (regexp)
        sp2 = re_find_substr (sp, s1size, s, s2size,
                              sp == astr_cstr (get_line_text (lp)), true, false,
                              translate);
      else
        sp2 = find_substr (sp, s1size, s, s2size, translate);

      if (sp2 != NULL)
        {
          Point pt;
          goto_linep (lp);
          pt = get_buffer_pt (cur_bp);
          pt.o = sp2 - astr_cstr (get_line_text (lp));
          set_buffer_pt (cur_bp, pt);
          return true;
        }
    }

  return false;
}

static int
search_backward (Line * startp, size_t starto, const char *s, int regexp)
{
  Line *lp;
  const char *sp, *sp2, *translate = fold_table (s, regexp);
  size_t s1size, ssize = strlen (s);

  if (ssize < 1)
    return false;

  for (lp = startp; lp != get_buffer_lines (cur_bp); lp = get_line_prev (lp))
    {
      sp = astr_cstr (get_line_text (lp));
      if (lp == startp)
        s1size = starto;
      else
        s1size = astr_len (get_line_text (lp));
      if (s1size < 1)
        continue;

      if (regexp)
        sp2 = re_find_substr (sp, s1size, s, ssize,
                              true, s1size == astr_len (get_line_text (lp)), true,
                              translate);
      else
        sp2 = rfind_substr (sp, s1size, s, ssize, translate);

      if (sp2 != NULL)
        {
          Point pt;
          goto_linep (lp);
          pt = get_buffer_pt (cur_bp);
          pt.o = sp2 - astr_cstr (get_line_text (lp));
          set_buffer_pt (cur_bp, pt);
          return true;
        }
    }

  return false;
}

static char *last_search = NULL;

typedef int (*Searcher) (Line * startp, size_t starto, const char *s, int regexp);

static le *
search (Searcher searcher, bool regexp, const char *pattern, const char *search_msg)
{
  le * ok = leT;
  const char *ms = NULL;

  if (pattern == NULL)
    pattern = ms = minibuf_read (search_msg, last_search);

  if (pattern == NULL)
    return FUNCALL (keyboard_quit);
  if (pattern[0] == '\0')
    ok = leNIL;
  else
    {
      free (last_search);
      last_search = xstrdup (pattern);

      if (!searcher (get_buffer_pt (cur_bp).p, get_buffer_pt (cur_bp).o, pattern, regexp))
        {
          minibuf_error ("Search failed: \"%s\"", pattern);
          ok = leNIL;
        }
    }

  free ((char *) ms);
  return ok;
}

DEFUN_ARGS ("search-forward", search_forward,
            STR_ARG (pattern))
/*+
Search forward from point for the user specified text.
+*/
{
  STR_INIT (pattern);
  ok = search (search_forward, false, pattern, "Search: ");
  STR_FREE (pattern);
}
END_DEFUN

DEFUN_ARGS ("search-backward", search_backward,
            STR_ARG (pattern))
/*+
Search backward from point for the user specified text.
+*/
{
  STR_INIT (pattern);
  ok = search (search_backward, false, pattern, "Search backward: ");
  STR_FREE (pattern);
}
END_DEFUN

DEFUN_ARGS ("search-forward-regexp", search_forward_regexp,
            STR_ARG (pattern))
/*+
Search forward from point for regular expression REGEXP.
+*/
{
  STR_INIT (pattern);
  ok = search (search_forward, true, pattern, "RE search: ");
  STR_FREE (pattern);
}
END_DEFUN

DEFUN_ARGS ("search-backward-regexp", search_backward_regexp,
            STR_ARG (pattern))
/*+
Search backward from point for match for regular expression REGEXP.
+*/
{
  STR_INIT (pattern);
  ok = search (search_backward, true, pattern, "RE search backward: ");
  STR_FREE (pattern);
}
END_DEFUN

#define ISEARCH_FORWARD		1
#define ISEARCH_BACKWARD	2
/*
 * Incremental search engine.
 */
static le *
isearch (int dir, int regexp)
{
  int c;
  int last = true;
  astr buf = astr_new ();
  astr pattern = astr_new ();
  Point start, cur;
  Marker *old_mark = copy_marker (get_buffer_mark (get_window_bp (cur_wp)));

  start = cur = get_buffer_pt (cur_bp);

  /* I-search mode. */
  set_buffer_isearch (get_window_bp (cur_wp), true);

  for (;;)
    {
      /* Make the minibuf message. */
      astr_truncate (buf, 0);
      astr_afmt (buf, "%sI-search%s: %s",
                 (last ?
                  (regexp ? "Regexp " : "") :
                  (regexp ? "Failing regexp " : "Failing ")),
                 (dir == ISEARCH_FORWARD) ? "" : " backward",
                 astr_cstr (pattern));

      /* Regex error. */
      if (re_find_err)
        {
          if ((strncmp (re_find_err, "Premature ", 10) == 0) ||
              (strncmp (re_find_err, "Unmatched ", 10) == 0) ||
              (strncmp (re_find_err, "Invalid ", 8) == 0))
            {
              re_find_err = "incomplete input";
            }
          astr_afmt (buf, " [%s]", re_find_err);
          re_find_err = NULL;
        }

      minibuf_write ("%s", astr_cstr (buf));

      c = getkey ();

      if (c == KBD_CANCEL)
        {
          set_buffer_pt (cur_bp, start);
          thisflag |= FLAG_NEED_RESYNC;

          /* Quit. */
          FUNCALL (keyboard_quit);

          /* Restore old mark position. */
          if (get_buffer_mark (cur_bp))
            free_marker (get_buffer_mark (cur_bp));

          set_buffer_mark (cur_bp, copy_marker (old_mark));
          break;
        }
      else if (c == KBD_BS)
        {
          if (astr_len (pattern) > 0)
            {
              astr_truncate (pattern, astr_len (pattern) - 1);
              cur = start;
              set_buffer_pt (cur_bp, start);
              thisflag |= FLAG_NEED_RESYNC;
            }
          else
            ding ();
        }
      else if (c & KBD_CTRL && (c & 0xff) == 'q')
        {
          minibuf_write ("%s^Q-", astr_cstr (buf));
          astr_cat_char (pattern, xgetkey (GETKEY_UNFILTERED, 0));
          minibuf_write ("%s", astr_cstr (buf));
        }
      else if (c & KBD_CTRL && ((c & 0xff) == 'r' || (c & 0xff) == 's'))
        {
          /* Invert direction. */
          if ((c & 0xff) == 'r' && dir == ISEARCH_FORWARD)
            dir = ISEARCH_BACKWARD;
          else if ((c & 0xff) == 's' && dir == ISEARCH_BACKWARD)
            dir = ISEARCH_FORWARD;
          if (astr_len (pattern) > 0)
            {
              /* Find next match. */
              cur = get_buffer_pt (cur_bp);
              /* Save search string. */
              free (last_search);
              last_search = xstrdup (astr_cstr (pattern));
            }
          else if (last_search != NULL)
            astr_cpy_cstr (pattern, last_search);
        }
      else if (c & KBD_META || c & KBD_CTRL || c > KBD_TAB)
        {
          if (c == KBD_RET && astr_len (pattern) == 0)
            {
              if (dir == ISEARCH_FORWARD)
                {
                  if (regexp)
                    FUNCALL (search_forward_regexp);
                  else
                    FUNCALL (search_forward);
                }
              else
                {
                  if (regexp)
                    FUNCALL (search_backward_regexp);
                  else
                    FUNCALL (search_backward);
                }
            }
          else
            {
              if (astr_len (pattern) > 0)
                {
                  /* Save mark. */
                  set_mark ();
                  set_marker_pt (get_buffer_mark (cur_bp), start);

                  /* Save search string. */
                  free (last_search);
                  last_search = xstrdup (astr_cstr (pattern));

                  minibuf_write ("Mark saved when search started");
                }
              else
                minibuf_clear ();
              if (c != KBD_RET)
                ungetkey (c);
            }
          break;
        }
      else
        astr_cat_char (pattern, c);

      if (astr_len (pattern) > 0)
        {
          if (dir == ISEARCH_FORWARD)
            last = search_forward (cur.p, cur.o, astr_cstr (pattern), regexp);
          else
            last =
              search_backward (cur.p, cur.o, astr_cstr (pattern), regexp);
        }
      else
        last = true;

      if (thisflag & FLAG_NEED_RESYNC)
        resync_redisplay ();
    }

  /* done */
  set_buffer_isearch (get_window_bp (cur_wp), false);

  astr_delete (buf);
  astr_delete (pattern);

  if (old_mark)
    free_marker (old_mark);

  return leT;
}

DEFUN ("isearch-forward", isearch_forward)
/*+
Do incremental search forward.
With a prefix argument, do an incremental regular expression search instead.
As you type characters, they add to the search string and are found.
Type return to exit, leaving point at location found.
Type @kbd{C-s} to search again forward, @kbd{C-r} to search again backward.
@kbd{C-g} when search is successful aborts and moves point to starting point.
+*/
{
  ok = isearch (ISEARCH_FORWARD, lastflag & FLAG_SET_UNIARG);
}
END_DEFUN

DEFUN ("isearch-backward", isearch_backward)
/*+
Do incremental search backward.
With a prefix argument, do a regular expression search instead.
As you type characters, they add to the search string and are found.
Type return to exit, leaving point at location found.
Type @kbd{C-r} to search again backward, @kbd{C-s} to search again forward.
@kbd{C-g} when search is successful aborts and moves point to starting point.
+*/
{
  ok = isearch (ISEARCH_BACKWARD, lastflag & FLAG_SET_UNIARG);
}
END_DEFUN

DEFUN ("isearch-forward-regexp", isearch_forward_regexp)
/*+
Do incremental search forward for regular expression.
With a prefix argument, do a regular string search instead.
Like ordinary incremental search except that your input
is treated as a regexp.  See @kbd{M-x isearch-forward} for more info.
+*/
{
  ok = isearch (ISEARCH_FORWARD, !(lastflag & FLAG_SET_UNIARG));
}
END_DEFUN

DEFUN ("isearch-backward-regexp", isearch_backward_regexp)
/*+
Do incremental search forward for regular expression.
With a prefix argument, do a regular string search instead.
Like ordinary incremental search except that your input
is treated as a regexp.  See @kbd{M-x isearch-forward} for more info.
+*/
{
  ok = isearch (ISEARCH_BACKWARD, !(lastflag & FLAG_SET_UNIARG));
}
END_DEFUN

DEFUN ("query-replace", query_replace)
/*+
Replace occurrences of a string with other text.
As each match is found, the user must type a character saying
what to do with it.
+*/
{
  bool noask = false, find_no_upper;
  size_t find_len, count = 0;
  char *find = minibuf_read ("Query replace string: ", "");
  char *repl;

  if (find == NULL)
    return FUNCALL (keyboard_quit);
  if (*find == '\0')
    {
      free (find);
      return leNIL;
    }
  find_len = strlen (find);
  find_no_upper = no_upper (find, find_len, false);

  repl = minibuf_read ("Query replace `%s' with: ", "", find);
  if (repl == NULL)
    {
      free (find);
      return FUNCALL (keyboard_quit);
    }

  while (search_forward (get_buffer_pt (cur_bp).p, get_buffer_pt (cur_bp).o, find, false))
    {
      Point pt;
      int c = ' ';

      if (!noask)
        {
          if (thisflag & FLAG_NEED_RESYNC)
            resync_redisplay ();
          for (;;)
            {
              minibuf_write
                ("Query replacing `%s' with `%s' (y, n, !, ., q)? ", find,
                 repl);
              c = getkey ();
              if (c == KBD_CANCEL || c == KBD_RET || c == ' ' || c == 'y'
                  || c == 'n' || c == 'q' || c == '.' || c == '!')
                break;
              minibuf_error ("Please answer y, n, !, . or q.");
              waitkey (WAITKEY_DEFAULT);
            }
          minibuf_clear ();

          if (c == 'q')			/* Quit immediately. */
            break;
          else if (c == KBD_CANCEL)	/* C-g */
            {
             ok = FUNCALL (keyboard_quit);
             break;
            }
          else if (c == '!')		/* Replace all without asking. */
            noask = true;
          else if (c == 'n' || c == KBD_RET || c == KBD_DEL) /* Do not replace. */
            continue;
        }

      /* Perform replacement. */
      pt = get_buffer_pt (cur_bp);
      ++count;
      undo_save (UNDO_REPLACE_BLOCK,
                 make_point (pt.n, pt.o - find_len), find_len, strlen (repl));
      line_replace_text (pt.p, pt.o - find_len, find_len, repl, find_no_upper);

      if (c == '.')		/* Replace and quit. */
        break;
    }

  free (find);
  free (repl);

  if (thisflag & FLAG_NEED_RESYNC)
    resync_redisplay ();

  if (ok)
    minibuf_write ("Replaced %d occurrences", count);
}
END_DEFUN

void free_search (void)
{
  free (last_search);
}
