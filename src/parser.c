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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

static astr snagAToken(getcCallback getachar, ungetcCallback ungetachar, enum tokenname *tokenid)
{
  int c;
  int doublequotes = 0;
  astr tok = astr_new();

  *tokenid = T_EOF;

  /* Chew space to next token */
  do {
    c = getachar();

    /* Munch comments */
    if (c == ';')
      do {
        c = getachar();
      } while (c != EOF && c != '\n');
  } while (c != EOF && (c == ' ' || c == '\t'));

  /* Snag token */
  if (c == '(') {
    *tokenid = T_OPENPAREN;
    return tok;
  } else if (c == ')') {
    *tokenid = T_CLOSEPAREN;
    return tok;
  } else if (c == '\'') {
    *tokenid = T_QUOTE;
    return tok;
  } else if (c == '\n') {
    *tokenid = T_NEWLINE;
    return tok;
  } else if (c == EOF) {
    *tokenid = T_EOF;
    return tok;
  }

  /* It looks like a string. Snag to the next whitespace. */
  if (c == '\"') {
    doublequotes = 1;
    c = getachar();
  }

  while (1) {
    astr_cat_char(tok, (char)c);

    if (!doublequotes) {
      if (c == ')' || c == '(' || c == ';' || c == ' ' || c == '\n' || c == '\r' || c == EOF) {
        ungetachar(c);
        astr_truncate(tok, (ptrdiff_t)-1);

        if (!astr_cmp_cstr(tok, "quote")) {
          *tokenid = T_QUOTE;
          return tok;
        }
        *tokenid = T_WORD;
        return tok;
      }
    } else {
      switch (c) {
      case '\n':
      case '\r':
      case EOF:
        ungetachar(c);
        /* Fall through */

      case '\"':
        astr_truncate(tok, (ptrdiff_t)-1);
        *tokenid = T_WORD;
        return tok;

      }
    }

    c = getachar();
  }

  return tok;
}

struct le *parseInFile(getcCallback getachar, ungetcCallback ungetachar, struct le *list, int *line)
{
  astr tok;
  enum tokenname tokenid;
  int isquoted = 0;

  assert(getachar && ungetachar);

  while (1) {
    tok = snagAToken(getachar, ungetachar, &tokenid);

    switch (tokenid) {
    case T_QUOTE:
      isquoted = 1;
      break;

    case T_OPENPAREN:
      list = leAddBranchElement(list,
                                parseInFile(getachar, ungetachar, NULL, line),
                                isquoted);
      isquoted = 0;
      break;

    case T_NEWLINE:
      isquoted = 0;
      *line = *line +1;
      break;

    case T_WORD:
      list = leAddDataElement(list, astr_cstr(tok), isquoted);
      isquoted = 0;
      break;

    case T_CLOSEPAREN:
    case T_EOF:
      isquoted = 0;
      astr_delete(tok);
      return list;
    }

    astr_delete(tok);
  }
}
