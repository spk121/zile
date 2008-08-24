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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "size_max.h"

#include "zile.h"
#include "extern.h"

static History files_history;
char *minibuf_contents = NULL;

/*--------------------------------------------------------------------------
 * Minibuffer wrapper functions.
 *--------------------------------------------------------------------------*/

void
free_minibuf (void)
{
  free_history_elements (&files_history);
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
 * Write the specified error string in the minibuffer and beep.
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
      p = term_minibuf_read (buf, astr_cstr (as), pos, cp, &files_history);
      free_completion (cp);
      astr_delete (as);
      free (buf);

      if (p != NULL)
        {
          as = expand_path (astr_new_cstr (p));
          if (as)
            {
              add_history_element (&files_history, p);
              p = xstrdup (astr_cstr (as));
              astr_delete (as);
            }
          else
            p = NULL;
        }
    }

  return p;
}

static int
minibuf_read_forced (const char *fmt, const char *errmsg,
		     Completion * cp, ...)
{
  va_list ap;
  char *buf;
  const char *s;

  va_start (ap, cp);
  xvasprintf (&buf, fmt, ap);
  va_end (ap);

  for (;;)
    {
      s = term_minibuf_read (buf, "", SIZE_MAX, cp, NULL);
      if (s == NULL) /* Cancelled. */
	{
	  free (buf);
	  return -1;
	}
      else
	{
	  size_t i;
	  astr as = astr_new ();

	  /* Complete partial words if possible. */
	  astr_cpy_cstr (as, s);
	  if (completion_try (cp, as, false) == COMPLETION_MATCHED)
	    s = cp->match;
	  astr_delete (as);

	  for (i = 0; i < gl_list_size (cp->completions); i++)
	    if (!strcmp (s, (char *) gl_list_get_at (cp->completions, i)))
	      {
		free (buf);
		return i;
	      }

	  minibuf_error (errmsg);
	  waitkey (WAITKEY_DEFAULT);
	}
    }

  abort ();
}

int
minibuf_read_yesno (const char *fmt, ...)
{
  va_list ap;
  char *buf;
  Completion *cp;
  int retvalue;

  va_start (ap, fmt);
  xvasprintf (&buf, fmt, ap);
  va_end (ap);

  cp = completion_new (false);
  gl_sortedlist_add (cp->completions, completion_strcmp, xstrdup ("yes"));
  gl_sortedlist_add (cp->completions, completion_strcmp, xstrdup ("no"));

  retvalue = minibuf_read_forced (buf, "Please answer yes or no.", cp);
  if (retvalue != -1)
    {
      /* The completions may be sorted by the minibuf completion
	 routines. */
      if (!strcmp (gl_list_get_at (cp->completions, (size_t) retvalue), "yes"))
	retvalue = true;
      else
	retvalue = false;
    }
  free_completion (cp);
  free (buf);

  return retvalue;
}

/*
 * Read a string from the minibuffer using a completion.
 */
char *
minibuf_read_completion (const char *fmt, char *value, Completion * cp,
			 History * hp, ...)
{
  va_list ap;
  char *buf, *p;

  va_start (ap, hp);
  xvasprintf (&buf, fmt, ap);
  va_end (ap);

  p = term_minibuf_read (buf, value, SIZE_MAX, cp, hp);
  free (buf);

  return p;
}

/*
 * Clear the minibuffer.
 */
void
minibuf_clear (void)
{
  term_minibuf_write ("");
}
