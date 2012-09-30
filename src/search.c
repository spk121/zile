/* Search and replace functions

   Copyright (c) 1997-2012 Free Software Foundation, Inc.

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

#include <stdlib.h>
#include <ctype.h>
#include <regex.h>

#include "main.h"
#include "extern.h"

/* Return true if there are no upper-case letters in the given string.
   If `regex' is true, ignore escaped characters. */
static bool
no_upper (const char *s, size_t len, int regex)
{
  int quote_flag = 0;

  for (size_t i = 0; i < len; i++)
    {
      if (regex && s[i] == '\\')
        quote_flag = !quote_flag;
      else if (!quote_flag && isupper ((int) s[i]))
        return false;
    }

  return true;
}

static const char *re_find_err = NULL;

static int
find_substr (castr as1, castr as2, const char *n, size_t nsize, size_t from, size_t to,
             bool forward, bool notbol, bool noteol, bool regex, bool icase)
{
  int ret = -1;
  struct re_pattern_buffer pattern;
  struct re_registers search_regs;
  reg_syntax_t syntax = RE_SYNTAX_EMACS;

  memset (&pattern, 0, sizeof (pattern));

  if (!regex)
    syntax |= RE_PLAIN;
  if (icase)
    syntax |= RE_ICASE;
  re_set_syntax (syntax);
  search_regs.num_regs = 1;

  re_find_err = re_compile_pattern (n, (int) nsize, &pattern);
  pattern.not_bol = notbol;
  pattern.not_eol = noteol;
  if (!re_find_err)
    /* FIXME: The current implementation memcpys the two strings into
       a freshly malloced block, i.e. is horribly inefficient for
       large buffers. */
    ret = re_search_2 (&pattern,
                       astr_cstr (as1), (int) astr_len (as1),
                       astr_cstr (as2), (int) astr_len (as2),
                       forward ? from : to - 1,
                       forward ? (to - from) : -(to - 1 - from),
                       &search_regs,
                       (int) astr_len (as1) + astr_len (as2));

  if (ret >= 0)
    ret = forward ? search_regs.end[0] : ret;

  return ret;
}

static bool
search (size_t o, const char *s, int forward, int regexp)
{
  size_t ssize = strlen (s);
  if (ssize < 1)
    return false;

  /* Attempt match. */
  bool notbol = forward ? o > 0 : false;
  bool noteol = forward ? false : o < get_buffer_size (cur_bp);
  size_t from = forward ? o : 0;
  size_t to = forward ? get_buffer_size (cur_bp) : o;
  int pos = find_substr (get_buffer_pre_point (cur_bp), get_buffer_post_point (cur_bp),
                         s, ssize, from, to, forward, notbol, noteol, regexp,
                         get_variable_bool ("case-fold-search") && no_upper (s, ssize, regexp));
  if (pos < 0)
    return false;

  goto_offset (pos);
  thisflag |= FLAG_NEED_RESYNC;
  return true;
}

static castr last_search = NULL;

static le *
do_search (bool forward, bool regexp, castr pattern)
{
  le * ok = leNIL;

  if (pattern == NULL)
    pattern = minibuf_read ("%s%s: ", last_search ? astr_cstr (last_search) : NULL,
                            regexp ? "RE search" : "Search", forward ? "" : " backward");

  if (pattern == NULL)
    return FUNCALL (keyboard_quit);
  if (astr_len (pattern) != 0)
    {
      last_search = pattern;

      if (!search (get_buffer_pt (cur_bp), astr_cstr (pattern), forward, regexp))
        minibuf_error ("Search failed: \"%s\"", pattern);
      else
        ok = leT;
    }

  return ok;
}

DEFUN_ARGS ("search-forward", search_forward,
            STR_ARG (pattern))
/*+
Search forward from point for the user specified text.
+*/
{
  STR_INIT (pattern);
  ok = do_search (true, false, pattern);
}
END_DEFUN

DEFUN_ARGS ("search-backward", search_backward,
            STR_ARG (pattern))
/*+
Search backward from point for the user specified text.
+*/
{
  STR_INIT (pattern);
  ok = do_search (false, false, pattern);
}
END_DEFUN

DEFUN_ARGS ("search-forward-regexp", search_forward_regexp,
            STR_ARG (pattern))
/*+
Search forward from point for regular expression REGEXP.
+*/
{
  STR_INIT (pattern);
  ok = do_search (true, true, pattern);
}
END_DEFUN

DEFUN_ARGS ("search-backward-regexp", search_backward_regexp,
            STR_ARG (pattern))
/*+
Search backward from point for match for regular expression REGEXP.
+*/
{
  STR_INIT (pattern);
  ok = do_search (false, true, pattern);
}
END_DEFUN

/*
 * Incremental search engine.
 */
static le *
isearch (int forward, int regexp)
{
  Marker *old_mark = copy_marker (get_buffer_mark (get_window_bp (cur_wp)));

  set_buffer_isearch (get_window_bp (cur_wp), true);

  int last = true;
  astr pattern = astr_new ();
  size_t start = get_buffer_pt (cur_bp), cur = start;
  for (;;)
    {
      /* Make the minibuf message. */
      astr buf = astr_fmt ("%sI-search%s: %s",
                           (last ?
                            (regexp ? "Regexp " : "") :
                            (regexp ? "Failing regexp " : "Failing ")),
                           forward ? "" : " backward",
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
          astr_cat (buf, astr_fmt (" [%s]", re_find_err));
          re_find_err = NULL;
        }

      minibuf_write ("%s", astr_cstr (buf));

      int c = getkey (GETKEY_DEFAULT);

      if (c == KBD_CANCEL)
        {
          goto_offset (start);
          thisflag |= FLAG_NEED_RESYNC;

          /* Quit. */
          FUNCALL (keyboard_quit);

          /* Restore old mark position. */
          if (get_buffer_mark (cur_bp))
            unchain_marker (get_buffer_mark (cur_bp));

          set_buffer_mark (cur_bp, copy_marker (old_mark));
          break;
        }
      else if (c == KBD_BS)
        {
          if (astr_len (pattern) > 0)
            {
              astr_truncate (pattern, astr_len (pattern) - 1);
              cur = start;
              goto_offset (start);
              thisflag |= FLAG_NEED_RESYNC;
            }
          else
            ding ();
        }
      else if (c & KBD_CTRL && (c & 0xff) == 'q')
        {
          minibuf_write ("%s^Q-", astr_cstr (buf));
          astr_cat_char (pattern, getkey_unfiltered (GETKEY_DEFAULT));
        }
      else if (c & KBD_CTRL && ((c & 0xff) == 'r' || (c & 0xff) == 's'))
        {
          /* Invert direction. */
          if ((c & 0xff) == 'r')
            forward = false;
          else if ((c & 0xff) == 's')
            forward = true;
          if (astr_len (pattern) > 0)
            {
              /* Find next match. */
              cur = get_buffer_pt (cur_bp);
              /* Save search string. */
              last_search = astr_cpy (astr_new (), pattern);
            }
          else if (last_search != NULL)
            astr_cpy (pattern, last_search);
        }
      else if (c & KBD_META || c & KBD_CTRL || c > KBD_TAB)
        {
          if (c == KBD_RET && astr_len (pattern) == 0)
            do_search (forward, regexp, NULL);
          else
            {
              if (astr_len (pattern) > 0)
                {
                  /* Save mark. */
                  set_mark ();
                  set_marker_o (get_buffer_mark (cur_bp), start);

                  /* Save search string. */
                  last_search = astr_cpy (astr_new (), pattern);

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
        last = search (cur, astr_cstr (pattern), forward, regexp);
      else
        last = true;

      if (thisflag & FLAG_NEED_RESYNC)
        {
          window_resync (cur_wp);
          term_redisplay ();
        }
    }

  /* done */
  set_buffer_isearch (get_window_bp (cur_wp), false);

  if (old_mark)
    unchain_marker (old_mark);

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
  ok = isearch (true, lastflag & FLAG_SET_UNIARG);
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
  ok = isearch (false, lastflag & FLAG_SET_UNIARG);
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
  ok = isearch (true, !(lastflag & FLAG_SET_UNIARG));
}
END_DEFUN

DEFUN ("isearch-backward-regexp", isearch_backward_regexp)
/*+
Do incremental search backward for regular expression.
With a prefix argument, do a regular string search instead.
Like ordinary incremental search except that your input
is treated as a regexp.  See @kbd{M-x isearch-backward} for more info.
+*/
{
  ok = isearch (false, !(lastflag & FLAG_SET_UNIARG));
}
END_DEFUN

/*
 * Check the case of a string.
 * Returns 2 if it is all upper case, 1 if just the first letter is,
 * and 0 otherwise.
 */
static _GL_ATTRIBUTE_PURE int
check_case (astr as)
{
  size_t i;
  for (i = 0; i < astr_len (as) && isupper ((int) astr_get (as, i)); i++)
    ;
  if (i == astr_len (as))
    return 2;
  else if (i == 1)
    for (; i < astr_len (as) && !isupper ((int) astr_get (as, i)); i++)
      ;
  return i == astr_len (as);
}

DEFUN ("query-replace", query_replace)
/*+
Replace occurrences of a string with other text.
As each match is found, the user must type a character saying
what to do with it.
+*/
{
  castr find = minibuf_read ("Query replace string: ", "");
  if (find == NULL)
    return FUNCALL (keyboard_quit);
  if (astr_len (find) == 0)
    return leNIL;
  bool find_no_upper = no_upper (astr_cstr (find), astr_len (find), false);

  castr repl = minibuf_read ("Query replace `%s' with: ", "", astr_cstr (find));
  if (repl == NULL)
    return FUNCALL (keyboard_quit);

  bool noask = false;
  size_t count = 0;
  while (search (get_buffer_pt (cur_bp), astr_cstr (find), true, false))
    {
      int c = ' ';

      if (!noask)
        {
          if (thisflag & FLAG_NEED_RESYNC)
            window_resync (cur_wp);
          for (;;)
            {
              minibuf_write
                ("Query replacing `%s' with `%s' (y, n, !, ., q)? ", astr_cstr (find),
                 astr_cstr (repl));
              c = getkey (GETKEY_DEFAULT);
              if (c == KBD_CANCEL || c == KBD_RET || c == ' ' || c == 'y'
                  || c == 'n' || c == 'q' || c == '.' || c == '!')
                break;
              minibuf_error ("Please answer y, n, !, . or q.");
              waitkey ();
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
      ++count;
      castr case_repl = repl;
      Region r = region_new (get_buffer_pt (cur_bp) - astr_len (find), get_buffer_pt (cur_bp));
      if (find_no_upper && get_variable_bool ("case-replace"))
        {
          int case_type = check_case (get_buffer_region (cur_bp, r).as);

          if (case_type != 0)
            case_repl = astr_recase (astr_cpy (astr_new (), repl),
                                     case_type == 1 ? case_capitalized : case_upper);
        }

      Marker *m = point_marker ();
      goto_offset (r.start);
      replace_estr (astr_len (find), estr_new_astr (case_repl));
      goto_offset (get_marker_o (m));
      unchain_marker (m);

      if (c == '.')		/* Replace and quit. */
        break;
    }

  if (thisflag & FLAG_NEED_RESYNC)
    window_resync (cur_wp);

  if (ok)
    minibuf_write ("Replaced %d occurrences", count);
}
END_DEFUN
