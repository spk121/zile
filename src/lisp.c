/* Lisp main routine
   Copyright (c) 2001 Scott "Jerry" Lawrence.
   Copyright (c) 2005 Reuben Thomas.
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

#include <stdio.h>
#include <assert.h>
#include "zile.h"
#include "extern.h"
#include "vars.h"
#include "eval.h"


void lisp_init(void)
{
  leNIL = leNew("NIL");
  leT = leNew("T");
}


void lisp_finalise(void)
{
  leReallyWipe(leNIL);
  leReallyWipe(leT);
}


le *lisp_read(getcCallback getcp, ungetcCallback ungetcp)
{
  int lineno = 0;
  struct le *list = NULL;

  list = parseInFile(getcp, ungetcp, list, &lineno);

  return list;
}


static FILE *fp = NULL;

static int getc_file(void)
{
  return getc(fp);
}

static void ungetc_file(int c)
{
  ungetc(c, fp);
}

le *lisp_read_file(const char *file)
{
  le *list;
  fp = fopen(file, "r");

  if (fp == NULL)
    return NULL;

  list = lisp_read(getc_file, ungetc_file);
  fclose(fp);

  return list;
}


astr lisp_dump(le *list)
{
  astr as = astr_new();

  astr_cat_cstr(as, "Eval results:\n");
  astr_cat_delete(as, leDumpEval(list, 0));
  astr_cat_cstr(as, "\n\nVariables:\n");
  astr_cat_delete(as, variableDump(mainVarList));

  return as;
}
