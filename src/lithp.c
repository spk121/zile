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

/*	$Id: lithp.c,v 1.2 2005/01/13 07:45:52 rrt Exp $	*/

#include <stdio.h>
#include "zile.h"
#include "extern.h"
#include "parser.h"
#include "vars.h"

FILE * fp = NULL;
    
static void myungetc(int c)
{
  ungetc(c, fp);
}
    
static int mygetc(void)
{
  return getc(fp);
}

void lithp(char* file)
{
  int lineno;
  struct le *list = NULL;

  /* parse in the file */
  printf("==== File %s\n", file);
  fp = fopen(file, "r");

  if (!fp)
    fprintf(stderr, "ERROR: Couldn't open \"%s\".\n", file);
  else {
    lineno = 0;
    list = parseInFile(mygetc, myungetc, list, &lineno);
    fclose(fp);
    fp = NULL;
  }

  /* evaluate the read-in lists and free */
  leDumpEval(list, 0); 
  leWipe(list);

  /* display the variables and free */
  printf("Variables:\n");
  variableDump(mainVarList);
  variableFree(mainVarList);

  /* display the user-defined functions and free */
  printf("defun's:\n");
  variableDump(defunList);
  variableFree(defunList);
}
