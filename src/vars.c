/* Lisp variables
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

/*	$Id: vars.c,v 1.6 2005/01/22 01:06:00 rrt Exp $	*/

#include <string.h>

#include "zile.h"
#include "extern.h"
#include "vars.h"
#include "eval.h"


le *mainVarList = NULL;
le *defunList = NULL;


le *variableFind(le *varlist, char *key)
{
  if (varlist != NULL && key != NULL)
    for (; varlist; varlist = varlist->list_next)
      if (!strcmp(key, varlist->data))
        return varlist;

  return NULL;
}

le *variableSet(le *varlist, char *key, le *value)
{
  if (key && value) {
    le *temp = variableFind(varlist, key);

    if (temp)
      leWipe(temp->branch);
    else {
      temp = leNew(key);
      varlist = leAddHead(varlist, temp);
    }

    temp->branch = leDup(value);
  }

  return varlist;
}
    
le *variableSetString(le *varlist, char *key, char *value)
{
  if (key && value) {
    le *temp = leNew(value);
    varlist = variableSet(varlist, key, temp);	
    leWipe(temp);
  }
  return varlist;
}
    
le *variableGet(le *varlist, char *key)
{
  le *temp = variableFind(varlist, key);
  return temp ? temp->branch : NULL;
}
    
char *variableGetString(le *varlist, char *key)
{
  le *temp = variableFind(varlist, key);
  if (temp && temp->branch && temp->branch->data
      && countNodes(temp->branch) == 1)
    return temp->branch->data;
  return "NIL";
}
    
astr variableDump(le *varlist)
{
  astr as = astr_new();
  
  for (; varlist; varlist = varlist->list_next) {
    if (varlist->branch && varlist->data) {
      astr_afmt(as, "%s \t", varlist->data);
      astr_cat_delete(as, leDumpReformat(varlist->branch));
      astr_cat_char(as, '\n');
    }
  }

  return as;
}
