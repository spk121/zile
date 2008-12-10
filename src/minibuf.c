/* Minibuffer facility functions

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.

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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

static History *files_history = NULL;
char *minibuf_contents = NULL;

/*--------------------------------------------------------------------------
 * Minibuffer wrapper functions.
 *--------------------------------------------------------------------------*/

void
init_minibuf (void)
{
  files_history = history_new ();
}

void
free_minibuf (void)
{
  free_history (files_history);
  free (minibuf_contents);
}

static void
minibuf_vwrite (const char *fmt, va_list ap)
{
  free (minibuf_contents);

  xvasprintf (&minibuf_contents, fmt, ap);

  if (cur_wp)
    {
      term_minibuf_write (minibuf_contents);

      /* Redisplay (and leave the cursor in the correct position). */
      term_redisplay ();
      term_refresh ();
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
char *
minibuf_read (const char *fmt, const char *value, ...)
{
  va_list ap;
  char *buf, *p;

  va_start (ap, value);
  xvasprintf (&buf, fmt, ap);
  va_end (ap);

  p = term_minibuf_read (buf, value ? value : "", SIZE_MAX, NULL, NULL);
  free (buf);

  return p;
}

/*
 * Read a non-negative number from the minibuffer.
 */
size_t
minibuf_read_number (const char *fmt, ...)
{
  va_list ap;
  char *buf;
  size_t n;

  va_start (ap, fmt);
  xvasprintf (&buf, fmt, ap);
  va_end (ap);

  do
    {
      char *ms = minibuf_read (buf, "");
      if (ms == NULL)
        {
          n = SIZE_MAX;
          FUNCALL (keyboard_quit);
          break;
        }
      if (strlen (ms) == 0)
        n = SIZE_MAX - 1;
      else
        {
          unsigned long un = strtoul (ms, NULL, 10);
          n = un == ULONG_MAX ? SIZE_MAX : (size_t) un;
        }
      free (ms);
      if (n == SIZE_MAX)
        minibuf_write ("Please enter a number.");
    }
  while (n == SIZE_MAX);

  return n;
}

/*
 * Read a filename from the minibuffer.
 */
char *
minibuf_read_filename (const char *fmt, const char *value,
                       const char *file, ...)
{
  va_list ap;
  char *buf, *p = NULL;
  Completion *cp;
  astr as;
  ptrdiff_t pos;

  va_start (ap, file);
  xvasprintf (&buf, fmt, ap);
  va_end (ap);

  as = expand_path (astr_new_cstr (value));
  if (as)
    {
      as = compact_path (as);

      cp = completion_new (true);
      pos = astr_len (as);
      if (file)
        pos -= strlen (file);
      p = term_minibuf_read (buf, astr_cstr (as), pos, cp, files_history);
      free_completion (cp);
      astr_delete (as);
      free (buf);

      if (p != NULL)
        {
          as = expand_path (astr_new_cstr (p));
          if (as)
            {
              add_history_element (files_history, p);
              p = xstrdup (astr_cstr (as));
              astr_delete (as);
            }
          else
            p = NULL;
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
minibuf_read_yesno (const char *fmt, ...)
{
  va_list ap;
  char *ms;
  const char *errmsg = "Please answer yes or no.";
  Completion *cp = completion_new (false);
  int retvalue = false;

  gl_sortedlist_add (cp->completions, completion_strcmp, xstrdup ("yes"));
  gl_sortedlist_add (cp->completions, completion_strcmp, xstrdup ("no"));

  va_start (ap, fmt);
  ms = vminibuf_read_completion (fmt, "", cp, NULL, errmsg,
                                 minibuf_test_in_completions, errmsg, ap);
  va_end (ap);

  if (ms != NULL)
    {
      gl_list_node_t n = gl_sortedlist_search (cp->completions, completion_strcmp, ms);
      assert (n);
      retvalue = !strcmp ((char *) gl_list_node_value (cp->completions, n), "yes");
    }
  free_completion (cp);

  return retvalue;
}

char *
minibuf_read_completion (const char *fmt, char *value, Completion * cp,
                         History * hp, ...)
{
  va_list ap;
  char *buf, *ms;

  va_start (ap, hp);
  xvasprintf (&buf, fmt, ap);
  va_end (ap);

  ms = term_minibuf_read (buf, value, SIZE_MAX, cp, hp);

 free (buf);
 return ms;
}

/*
 * Read a string from the minibuffer using a completion.
 */
char *
vminibuf_read_completion (const char *fmt, char *value, Completion * cp,
                          History * hp, const char *empty_err,
                          bool (*test) (const char *s, gl_list_t completions),
                          const char *invalid_err, va_list ap)
{
  char *buf, *ms;

  xvasprintf (&buf, fmt, ap);

  for (;;)
    {
      ms = term_minibuf_read (buf, value, SIZE_MAX, cp, hp);

      if (ms == NULL) /* Cancelled. */
        {
          FUNCALL (keyboard_quit);
          break;
        }
      else if (ms[0] == '\0')
        {
          minibuf_error (empty_err);
          free ((char *) ms);
          ms = NULL;
          break;
        }
      else
        {
          astr as = astr_new ();
          astr_cpy_cstr (as, ms);
          /* Complete partial words if possible. */
          if (completion_try (cp, as, false) == COMPLETION_MATCHED)
            {
              free ((char *) ms);
              ms = xstrdup (cp->match);
            }
          astr_delete (as);

          if (test (ms, cp->completions))
            {
              if (hp)
                add_history_element (hp, ms);
              minibuf_clear ();
              break;
            }
          else
            {
              minibuf_error (invalid_err, ms);
              waitkey (WAITKEY_DEFAULT);
            }
        }
    }

  free (buf);

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
