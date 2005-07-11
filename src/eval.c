/* Lisp eval
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

/*	$Id: eval.c,v 1.24 2005/07/11 06:10:25 rrt Exp $	*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"
#include "eval.h"
#include "vars.h"


static evalLookupNode evalTable[] = {
  { "set"	, eval_cb_set		},
  { "setq"	, eval_cb_setq		},
  { NULL	, NULL			}
};


eval_cb lookupFunction(char *name)
{
  int i;
  for (i = 0; evalTable[i].word; i++)
    if (!strcmp(evalTable[i].word, name))
      return evalTable[i].callback;

  return NULL;
}


le *evaluateBranch(le *trybranch)
{
  le *keyword;
  eval_cb prim;

  if (trybranch == NULL)
    return NULL;

  if (trybranch->branch)
    keyword = evaluateBranch(trybranch->branch);
  else
    keyword = leNew(trybranch->data);

  if (keyword->data == NULL) {
    leWipe(keyword);
    return leNIL;
  }

  prim = lookupFunction(keyword->data);
  leWipe(keyword);
  if (prim)
    return prim(countNodes(trybranch), trybranch);

  return NULL;
}

le *evaluateNode(le *node)
{
  le *value;

  if (node == NULL)
    return leNIL;

  if (node->branch != NULL) {
    if (node->quoted)
      value = leDup(node->branch);
    else
      value = evaluateBranch(node->branch);

  } else {
    le *temp = variableGet(mainVarList, node->data);

    if (temp != NULL)
      value = leDup(temp);
    else
      value = leNew(node->data);
  }

  return value;
}


int countNodes(le *branch)
{
  int count;

  for (count = 0; branch; branch = branch->list_next, count++);
  return count;
}


le *eval_cb_set_helper(enum setfcn function, int argc, le *branch)
{
  le *newkey = NULL /* XXX unnecessary */, *newvalue = leNIL, *current;

  if (branch != NULL && argc >= 3) {
    for (current = branch->list_next; current; current = current->list_next->list_next) {
      if (newvalue != leNIL)
        leWipe(newvalue);

      newvalue = evaluateNode(current->list_next);

      variableSet(&mainVarList,
                  function == S_SET ? (newkey = evaluateNode(current))->data : current->data,
                  newvalue);

      if (function == S_SET)
        leWipe(newkey);

      if (current->list_next == NULL)
        break;
    }
  }

  return newvalue;
}

le* eval_cb_set(int argc, le *branch)
{
  return eval_cb_set_helper(S_SET, argc, branch);
}

le *eval_cb_setq(int argc, le *branch)
{
  return eval_cb_set_helper(S_SETQ, argc, branch);
}
