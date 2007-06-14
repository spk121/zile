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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lists.h"

enum tokenname {
  T_EOF,
  T_CLOSEPAREN,
  T_OPENPAREN,
  T_NEWLINE,
  T_QUOTE,
  T_WORD
};

typedef int (*getcCallback)(void);
typedef void (*ungetcCallback)(int c);

struct le *parseInFile(getcCallback getachar, ungetcCallback ungetachar,
            struct le * list, int *line);

#endif
