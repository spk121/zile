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

/*	$Id: vars.h,v 1.5 2005/01/26 23:04:48 rrt Exp $	*/

#include <stdio.h>
#include "lists.h"
#include "astr.h"

extern le *mainVarList;
extern le *defunList;

le *variableFind(le *varlist, char *key);
#define variableFree(L) \
  leWipe(L)

le *variableSet(le *varlist, char *key, le *value);
le *variableSetString(le *varlist, char *key, char *value);
le *variableSetNumber(le *varlist, char *key, int value);
le *variableGet(le *varlist, char *key);
char *variableGetString(le *varlist, char *key);

astr variableDump(le *varlist);
