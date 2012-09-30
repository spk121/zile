/* Minibuffer facility functions

   Copyright (c) 1997-2004, 2008-2011 Free Software Foundation, Inc.

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
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"

static History *files_history = NULL;
static char *minibuf_contents = NULL;

/*--------------------------------------------------------------------------
 * Minibuffer wrapper functions.
 *--------------------------------------------------------------------------*/

void
init_minibuf (void)
{
  files_history = history_new ();
}

int
minibuf_no_error (void)
{
  return minibuf_contents == NULL;
}

void
minibuf_refresh (void)
{
  if (cur_wp)
    {
      if (minibuf_contents)
        term_minibuf_write (minibuf_contents);

      /* Redisplay (and leave the cursor in the correct position). */
      term_redraw_cursor ();
      term_refresh ();
    }
}

static void
minibuf_vwrite (const char *fmt, va_list ap)
{
  char *s = xvasprintf (fmt, ap);
  if (minibuf_contents == NULL || strcmp (s, minibuf_contents))
    {
      minibuf_contents = s;
      minibuf_refresh ();
    }
}

/*
 * Write the specified string in the minibuffer.
 */
void
minibuf_write (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  minibuf_vwrite (fmt, ap);
  va_end (ap);
}

/*
 * Write the specified error string in the minibuffer and signal an error.
 */
void
minibuf_error (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  minibuf_vwrite (fmt, ap);
  va_end (ap);

  ding ();
}

/*
 * Read a string from the minibuffer.
 */
castr
minibuf_read (const char *fmt, const char *value, ...)
{
  va_list ap;
  char *buf;

  va_start (ap, value);
  buf = xvasprintf (fmt, ap);
  va_end (ap);

  return term_minibuf_read (buf, value ? value : "", SIZE_MAX, NULL, NULL);
}

/*
 * Read a non-negative number from the minibuffer.
 */
long
minibuf_read_number (const char *fmt, ...)
{
  va_list ap;
  char *buf;
  unsigned long n;

  va_start (ap, fmt);
  buf = xvasprintf (fmt, ap);
  va_end (ap);

  do
    {
      castr as = minibuf_read (buf, "");
      if (as == NULL || astr_cstr (as) == NULL)
        {
          n = LONG_MAX;
          FUNCALL (keyboard_quit);
          break;
        }
      if (strlen (astr_cstr (as)) == 0)
        n = LONG_MAX - 1;
      else
        n = strtoul (astr_cstr (as), NULL, 10);
      if (n == LONG_MAX)
        {
          minibuf_write ("Please enter a number.");
          ding ();
        }
    }
  while (n == LONG_MAX);

  return n;
}

/*
 * Read a filename from the minibuffer.
 */
castr
minibuf_read_filename (const char *fmt, const char *value,
                       const char *file, ...)
{
  castr p = NULL;

  astr as = astr_new_cstr (value);
  if (file == NULL && astr_len (as) > 0 && astr_get (as, astr_len (as) - 1) != '/')
    astr_cat_char (as, '/');

  if (expand_path (as))
    {
      va_list ap;
      va_start (ap, file);
      char *buf = xvasprintf (fmt, ap);
      va_end (ap);

      as = compact_path (as);

      Completion *cp = completion_new (true);
      size_t pos = astr_len (as);
      if (file)
        pos -= strlen (file);
      p = term_minibuf_read (buf, astr_cstr (as), pos, cp, files_history);

      if (p != NULL)
        {
          astr q = astr_cpy (astr_new (), p);
          if (expand_path (q))
            add_history_element (files_history, astr_cstr (q));
          else
            q = NULL;
          p = q;
        }
    }

  return p;
}

bool
minibuf_test_in_completions (const char *ms, gl_list_t completions)
{
  return gl_sortedlist_search (completions, completion_strcmp, ms) != NULL;
}

int
minibuf_read_yn (const char *fmt, ...)
{
  va_list ap;
  char *buf;
  const char *errmsg = "";
  int ret;

  va_start (ap, fmt);
  buf = xvasprintf (fmt, ap);
  va_end (ap);

  for (ret = -2; ret == -2;) {
    size_t key;

    minibuf_write ("%s%s", errmsg, buf);
    key = getkeystroke (GETKEY_DEFAULT);
    switch (key)
      {
      case 'y':
        ret = true;
        break;
      case 'n':
        ret = false;
        break;
      case KBD_CTRL | 'g':
        ret = -1;
        break;
      default:
        errmsg = "Please answer y or n.  ";
      }
  }

  return ret;
}

int
minibuf_read_yesno (const char *fmt, ...)
{
  va_list ap;
  const char *errmsg = "Please answer yes or no.";
  Completion *cp = completion_new (false);
  int ret = -1;

  gl_sortedlist_add (get_completion_completions (cp), completion_strcmp, xstrdup ("yes"));
  gl_sortedlist_add (get_completion_completions (cp), completion_strcmp, xstrdup ("no"));

  va_start (ap, fmt);
  castr ms = minibuf_vread_completion (fmt, "", cp, NULL, errmsg,
                                       minibuf_test_in_completions, errmsg, ap);
  va_end (ap);

  if (ms != NULL)
    {
      gl_list_node_t n = gl_sortedlist_search (get_completion_completions (cp),
                                               completion_strcmp, astr_cstr (ms));
      assert (n);
      ret = STREQ ((const char *) gl_list_node_value (get_completion_completions (cp), n),
                   "yes");
    }

  return ret;
}

castr
minibuf_read_completion (const char *fmt, const char *value, Completion * cp,
                         History * hp, ...)
{
  va_list ap;
  char *buf;

  va_start (ap, hp);
  buf = xvasprintf (fmt, ap);
  va_end (ap);

  return term_minibuf_read (buf, value, SIZE_MAX, cp, hp);
}

/*
 * Read a string from the minibuffer using a completion.
 */
castr
minibuf_vread_completion (const char *fmt, const char *value, Completion * cp,
                          History * hp, const char *empty_err,
                          bool (*test) (const char *s, gl_list_t completions),
                          const char *invalid_err, va_list ap)
{
  castr ms;
  char *buf = xvasprintf (fmt, ap);

  for (;;)
    {
      ms = term_minibuf_read (buf, value, SIZE_MAX, cp, hp);

      if (ms == NULL) /* Cancelled. */
        {
          FUNCALL (keyboard_quit);
          break;
        }
      else if (astr_len (ms) == 0)
        {
          minibuf_error (empty_err);
          ms = NULL;
          break;
        }
      else
        {
          astr as = astr_cpy (astr_new (), ms);
          /* Complete partial words if possible. */
          if (completion_try (cp, as, false) == COMPLETION_MATCHED)
            ms = astr_new_cstr (get_completion_match (cp));

          if (test (astr_cstr (ms), get_completion_completions (cp)))
            {
              if (hp)
                add_history_element (hp, astr_cstr (ms));
              minibuf_clear ();
              break;
            }
          else
            {
              minibuf_error (invalid_err, astr_cstr (ms));
              waitkey ();
            }
        }
    }

  return ms;
}

/*
 * Clear the minibuffer.
 */
void
minibuf_clear (void)
{
  term_minibuf_write ("");
}
