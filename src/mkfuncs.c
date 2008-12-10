/* Produce tbl_funcs.h

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Reuben Thomas.

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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
do_func (FILE *fp, const char *name)
{
  char buf[BUFSIZ], cname[BUFSIZ], *p = cname;
  unsigned s = 0;

  strcpy (cname, name);
  while ((p = strchr (p, '-')) != NULL)
    *p = '_';

  printf ("X(\"%s\", %s, \"\\\n", name, cname);

  while (fgets (buf, BUFSIZ, fp) != NULL)
    {
      size_t len = strlen (buf);
      if (buf[len - 1] == '\n')
        buf[len - 1] = '\0';
      if (s == 1)
        {
          if (!strncmp (buf, "+*/", (size_t) 3))
            break;
          printf ("%s\\n\\\n", buf);
        }
      if (!strncmp (buf, "/*+", (size_t) 3))
        s = 1;
      else if (*buf == '{')
        break;
    }

  printf ("\")\n");
}

int
main (int argc, char **argv)
{
  int i;

  printf ("\
/* Zile command to C function bindings and docstrings.\n\
\n\
   THIS FILE IS AUTO-GENERATED.\n\
*/\n\
\n");

  for (i = 0; i < argc; i++) {
    char buf[BUFSIZ], *filename = argv[i];
    FILE *fp = fopen (filename, "r");

    if (fp == NULL)
      {
        fprintf (stderr, "mkfuncs:%s: %s\n", filename, strerror (errno));
        exit (1);
      }

    while (fgets (buf, BUFSIZ, fp) != NULL)
      {
        if (!strncmp (buf, "DEFUN (", (size_t) 6))
          {
            char *p, *q, *r;
          p = buf;
          q = strchr (p, '\"');
          r = strrchr (p, '\"');
          if (q == NULL || r == NULL || q == r)
            {
              fprintf (stderr, "mkdoc: invalid DEFUN () syntax\n");
              exit (1);
            }
          *r = '\0';
          do_func (fp, q + 1);
          }
      }

    fclose (fp);
  }

  return 0;
}
