/* Lisp parser
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

/*	$Id: parser.h,v 1.3 2005/01/25 00:28:39 rrt Exp $	*/

#include <stdio.h>
#include "lists.h"

enum tokenname { 
  T_NONE, 
  T_CLOSEPAREN, 
  T_OPENPAREN, 
  T_NEWLINE, 
  T_QUOTE,
  T_WORD, 
  T_EOF
};

typedef int (*getcCallback)(void);
typedef void (*ungetcCallback)(int c);

struct le *parseInFile(getcCallback getachar, ungetcCallback ungetachar,
            struct le * list, int *line);
