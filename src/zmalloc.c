/* Memory allocation functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004 Reuben Thomas.
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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id: zmalloc.c,v 1.3 2005/02/05 13:49:06 rrt Exp $	*/


#include "config.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

/*
 * Return a zeroed allocated memory area.
 */
void *zmalloc(size_t size)
{
  void *ptr;

  assert(size > 0);

  if ((ptr = calloc(size, 1)) == NULL) {
    fprintf(stderr, "zile: cannot allocate memory\n");
    zile_exit(1);
  }

  return ptr;
}

/*
 * Resize an allocated memory area.
 */
void *zrealloc(void *ptr, size_t size)
{
  void *newptr;

  assert(size > 0);

  if ((newptr = realloc(ptr, size)) == NULL) {
    fprintf(stderr, "zile: cannot reallocate memory\n");
    zile_exit(1);
  }

  return newptr;
}

/*
 * Duplicate a string.
 */
char *zstrdup(const char *s)
{
  return strcpy(zmalloc(strlen(s) + 1), s);
}

/*
 * Wrapper for vasprintf.
 */
int zvasprintf(char **ptr, const char *fmt, va_list vargs)
{
  int retval = vasprintf(ptr, fmt, vargs);

  if (retval == -1) {
    fprintf(stderr, "zile: cannot allocate memory for asprintf\n");
    zile_exit(1);
  }

  return retval;
}

/*
 * Wrapper for asprintf.
 */
int zasprintf(char **ptr, const char *fmt, ...)
{
  va_list vargs;
  int retval;

  va_start(vargs, fmt);
  retval = zvasprintf(ptr, fmt, vargs);
  va_end(vargs);

  return retval;
}
