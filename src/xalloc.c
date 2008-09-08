/* Checked memory-allocating functions

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

#include <stdio.h>
#include <stdlib.h>

#include "zile.h"
#include "extern.h"

/*
 * Routine called by gnulib's xalloc routines on OOM
 */
void
xalloc_die (void)
{
  fprintf (stderr, "%s: cannot allocate memory\n", prog_name);
  zile_exit (false);
  abort ();
}

/*
 * Checked vasprintf.
 */
int
xvasprintf (char **ptr, const char *fmt, va_list vargs)
{
  int retval = vasprintf (ptr, fmt, vargs);

  if (retval == -1)
    {
      fprintf (stderr, "%s: cannot allocate memory for asprintf\n", prog_name);
      zile_exit (false);
    }

  return retval;
}

/*
 * Checked asprintf.
 */
int
xasprintf (char **ptr, const char *fmt, ...)
{
  va_list vargs;
  int retval;

  va_start (vargs, fmt);
  retval = xvasprintf (ptr, fmt, vargs);
  va_end (vargs);

  return retval;
}

/*
 * Wrapper for free used as gl_listelement_dispose_fn.
 */
void
list_free (const void *p)
{
  free ((void *)p);
}
