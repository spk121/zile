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

int countNodes(le *branch);

enum setfcn {
  S_SET,
  S_SETQ
};

le *eval_cb_set(int argc, le *branch);
le *eval_cb_setq(int argc, le *branch);

le *leNIL, *leT;
void eval_init(void);
void eval_finalise(void);

#endif
