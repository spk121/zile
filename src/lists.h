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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#ifndef LISTS_H
#define LISTS_H

#include "astr.h"

typedef struct le {
  /* either data or a branch */
  struct le *branch;
  char *data;
  int quoted;
  int tag;

  /* for the next in the list in the current parenlevel */
  struct le *list_prev;
  struct le *list_next;
} le;

le *leNew(const char *text);
void leReallyWipe(le *list);
void leWipe(le *list);

le *leAddHead(le *list, le *element);
le *leAddTail(le *list, le *element);

le *leAddBranchElement(le *list, le *branch, int quoted);
le *leAddDataElement(le *list, const char *data, int quoted);
le *leDup(le *list);

void leClearTag(le *list);
void leTagData(le *list, char *data, int tagval);
void leTagReplace(le *list, int tagval, le *newinfo);

astr leDump(le *list, int indent);
astr leDumpEval(le *list, int indent);
astr leDumpEvalTree(le *list, int indent);
astr leDumpReformat(le *tree);

#endif
