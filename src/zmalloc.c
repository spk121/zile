/* Memory allocation functions

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2004, 2005, 2006, 2007, 2008 Reuben Thomas.

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

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

/*
 * Return a zeroed allocated memory area.
 */
void *
zmalloc (size_t size)
{
  void *ptr;

  assert (size > 0);

  ptr = calloc (size, (size_t) 1);
  if (ptr == NULL)
    {
      fprintf (stderr, "%s: cannot allocate memory\n", prog_name);
      zile_exit (FALSE);
    }

  return ptr;
}

/*
 * Resize an allocated memory area.
 */
void *
zrealloc (void *ptr, size_t size)
{
  void *newptr;

  newptr = realloc (ptr, size);
  if (newptr == NULL)
    {
      fprintf (stderr, "%s: cannot reallocate memory\n", prog_name);
      zile_exit (FALSE);
    }

  return newptr;
}

/*
 * Duplicate a string.
 */
char *
zstrdup (const char *s)
{
  return strcpy (zmalloc (strlen (s) + 1), s);
}

/*
 * Wrapper for vasprintf.
 */
#ifndef HAVE_VASPRINTF
int vasprintf (char **ptr, const char *format_string, va_list vargs);
#endif
int
zvasprintf (char **ptr, const char *fmt, va_list vargs)
{
  int retval = vasprintf (ptr, fmt, vargs);

  if (retval == -1)
    {
      fprintf (stderr, "%s: cannot allocate memory for asprintf\n", prog_name);
      zile_exit (FALSE);
    }

  return retval;
}

/*
 * Wrapper for asprintf.
 */
int
zasprintf (char **ptr, const char *fmt, ...)
{
  va_list vargs;
  int retval;

  va_start (vargs, fmt);
  retval = zvasprintf (ptr, fmt, vargs);
  va_end (vargs);

  return retval;
}
