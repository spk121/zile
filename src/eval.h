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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id: eval.h,v 1.10 2005/02/07 01:36:43 rrt Exp $	*/

#ifndef EVAL_H
#define EVAL_H

#include "lists.h"

typedef le *(*eval_cb)(int argc, le *branch);
typedef struct evalLookupNode {
  char *word;
  eval_cb callback;
} evalLookupNode;

eval_cb lookupFunction(char *name);
le *evaluateBranch(le *trybranch);
le *evaluateNode(le *node);
le *evaluateDefun(le *fcn, le *params);

int countNodes(le *branch);

int evalCastLeToInt(const le *levalue);
le *evalCastIntToLe(int intvalue);

enum cumefcn {
  C_ADD,
  C_SUBTRACT,
  C_MULTIPLY,
  C_DIVIDE
};

int eval_cume_helper(enum cumefcn function, int value, le *branch) ;
le *eval_cb_add(int argc, le *branch);
le *eval_cb_subtract(int argc, le *branch);
le *eval_cb_multiply(int argc, le *branch);
le *eval_cb_divide(int argc, le *branch);
le *eval_cb_modulus(int argc, le *branch);

le *eval_cb_lt(int argc, le *branch);
le *eval_cb_lt_eq(int argc, le *branch);
le *eval_cb_gt(int argc, le *branch);
le *eval_cb_gt_eq(int argc, le *branch);
le *eval_cb_eqsign(int argc, le *branch);

le *eval_cb_and(int argc, le *branch);
le *eval_cb_or(int argc, le *branch);
le *eval_cb_not(int argc, le *branch);

le *eval_cb_atom(int argc, le *branch);
le *eval_cb_car(int argc, le *branch);
le *eval_cb_cdr(int argc, le *branch);
le *eval_cb_cons(int argc, le *branch);
le *eval_cb_list(int argc, le *branch);
int eval_cb_lists_same( le *list1, le *list2);
le *eval_cb_equal(int argc, le *branch);

le *eval_cb_if(int argc, le *branch);

enum whenunless {
  WU_WHEN,
  WU_UNLESS
};

le *eval_cb_whenunless_helper(enum whenunless which, int argc, le *branch);
le *eval_cb_unless(int argc, le *branch);
le *eval_cb_when(int argc, le *branch);
le *eval_cb_cond(int argc, le *branch);

le *eval_cb_princ(int argc, le *branch);

le *eval_cb_eval(int argc, le *branch);
le *eval_cb_prog(int argc, le *branch, int returnit);
le *eval_cb_prog1(int argc, le *branch);
le *eval_cb_prog2(int argc, le *branch);
le *eval_cb_progn(int argc, le *branch);

enum setfcn {
  S_SET,
  S_SETQ
};

le *eval_cb_set_helper(enum setfcn function, int argc, le *branch);
le *eval_cb_set(int argc, le *branch);
le *eval_cb_setq(int argc, le *branch);

le *eval_cb_defun(int argc, le *branch);

le *leNIL, *leT;
void eval_init(void);
void eval_finalise(void);

#endif
