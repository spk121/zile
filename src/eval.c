/* Lisp eval
   Copyright (c) 2001 Scott "Jerry" Lawrence.
   Copyright (c) 2005-2007 Reuben Thomas.
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"
#include "eval.h"
#include "vars.h"


static le *eval_cb_command_helper(Function f, int argc, le *branch)
{
  int ret = f(argc, branch);
  return ret ? leT : leNIL;
}

#define X(zile_name, c_name) \
  static le *eval_cb_ ## c_name(int argc, le *branch) \
  { \
    return eval_cb_command_helper(F_ ## c_name, argc, branch); \
  }
#define X0(zile_name, c_name)                    X(zile_name, c_name)
#define X1(zile_name, c_name, key1)              X(zile_name, c_name)
#define X2(zile_name, c_name, key1, key2)        X(zile_name, c_name)
#define X3(zile_name, c_name, key1, key2, key3)  X(zile_name, c_name)
#include "tbl_funcs.h"
#undef X
#undef X0
#undef X1
#undef X2
#undef X3


static evalLookupNode evalTable[] = {
  { "setq"	, eval_cb_setq },
#define X0(zile_name, c_name) \
  { zile_name   , eval_cb_ ## c_name },
#define X1(zile_name, c_name, key1) \
  { zile_name   , eval_cb_ ## c_name },
#define X2(zile_name, c_name, key1, key2) \
  { zile_name   , eval_cb_ ## c_name },
#define X3(zile_name, c_name, key1, key2, key3) \
  { zile_name   , eval_cb_ ## c_name },
#include "tbl_funcs.h"
#undef X0
#undef X1
#undef X2
#undef X3
  { NULL	, NULL },
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
    return prim(0, trybranch);

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
    value = variableGet(mainVarList, node->data);

    if (value != NULL)
      value = leDup(value);
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

le *eval_cb_setq(int argc, le *branch)
{
  le *newvalue = leNIL, *current;
  argc = countNodes(branch);

  if (branch != NULL && argc >= 3) {
    for (current = branch->list_next; current; current = current->list_next->list_next) {
      if (newvalue != leNIL)
        leWipe(newvalue);

      newvalue = evaluateNode(current->list_next);

      variableSet(&mainVarList, current->data, newvalue);

      if (current->list_next == NULL)
        break;
    }
  }

  return newvalue;
}
