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

/*	$Id: lists.h,v 1.2 2005/01/13 07:45:52 rrt Exp $	*/

#ifndef __LISTS_H__
#define __LISTS_H__

#include <stdio.h>

typedef struct le{
  /* either data or a branch */
  struct le * branch;
  char * data;
  int quoted;
  int tag;

  /* for the next in the list in the current parenlevel */
  struct le * list_prev;
  struct le * list_next;
} le;

le * leNew(char * text);
void leDelete(le * element);
void leWipe(le * list);

le * leAddHead(le * list, le * element);
le * leAddTail(le * list, le * element);

le * leAddBranchElement( le * list, le * branch, int quoted );
le * leAddDataElement( le * list, char * data, int quoted );
le * leDup( le * list );

void leClearTag( le * list );
void leTagData(le * list, char * data, int tagval);
void leTagReplace(le * list, int tagval, le * newinfo);

void leDump( le * list, int indent );
void leDumpEval( le * list, int indent );
void leDumpEvalTree( le * list, int indent );
void leDumpReformat(FILE * of, le * tree);
#endif
