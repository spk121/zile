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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id: lisp.c,v 1.1 2005/01/19 00:40:52 rrt Exp $	*/

#include <stdio.h>
#include <assert.h>
#include "zile.h"
#include "extern.h"
#include "vars.h"
#include "eval.h"


char *s;

astr lisp_read(getcCallback getcp, ungetcCallback ungetcp)
{
  astr as = astr_new();
  int lineno = 0;
  struct le *list = NULL;

  eval_init();
    
  list = parseInFile(getcp, ungetcp, list, &lineno);

  /* evaluate the read-in lists, display the result, and the variables
     and defuns */
  astr_cat_delete(as, leDumpEval(list, 0));
  astr_cat_cstr(as, "Variables:\n");
  astr_cat_delete(as, variableDump(mainVarList));
  astr_cat_cstr(as, "defuns:\n");
  astr_cat_delete(as, variableDump(defunList));

  /* Free the state */
  leWipe(list);
  variableFree(mainVarList);
  variableFree(defunList);

  eval_finalise();

  return as;
}


static int getc_string(void)
{
  return *s++;
}

static void ungetc_string(int c)
{
  s--;
}

astr lisp_read_string(char *string)
{
  assert(string);
  s = string;
  return lisp_read(getc_string, ungetc_string);
}


FILE *fp = NULL;

static int getc_file(void)
{
  return getc(fp);
}

static void ungetc_file(int c)
{
  ungetc(c, fp);
}
    
astr lisp_read_file(char *file)
{
  astr as = astr_new();
  fp = fopen(file, "r");
  astr_afmt(as, "==== File %s\n", file);

  if (!fp)
    astr_afmt(as, "ERROR: Couldn't open \"%s\".\n", file);
  else {
    astr_cat_delete(as, lisp_read(getc_file, ungetc_file));
    fclose(fp);
  }

  return as;
}
