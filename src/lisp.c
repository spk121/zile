/* Lisp parser

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 2001 Scott "Jerry" Lawrence.
   Copyright (c) 2005 Reuben Thomas.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"
#include "eval.h"
#include "lists.h"


void
lisp_init (void)
{
  leNIL = leNew ("nil");
  leT = leNew ("t");
}

void
lisp_finalise (void)
{
  leReallyWipe (leNIL);
  leReallyWipe (leT);
}


enum tokenname
{
  T_EOF,
  T_CLOSEPAREN,
  T_OPENPAREN,
  T_NEWLINE,
  T_QUOTE,
  T_WORD
};

static int
read_char (astr as, ptrdiff_t * pos)
{
  if ((size_t) *pos < astr_len (as))
    return *astr_char (as, (*pos)++);
  return EOF;
}

static astr
read_token (enum tokenname *tokenid, astr as, ptrdiff_t * pos)
{
  int c;
  int doublequotes = 0;
  astr tok = astr_new ();

  *tokenid = T_EOF;

  /* Chew space to next token */
  do
    {
      c = read_char (as, pos);

      /* Munch comments */
      if (c == ';')
	do
          c = read_char (as, pos);
	while (c != EOF && c != '\n');
    }
  while (c != EOF && (c == ' ' || c == '\t'));

  /* Snag token */
  if (c == '(')
    {
      *tokenid = T_OPENPAREN;
      return tok;
    }
  else if (c == ')')
    {
      *tokenid = T_CLOSEPAREN;
      return tok;
    }
  else if (c == '\'')
    {
      *tokenid = T_QUOTE;
      return tok;
    }
  else if (c == '\n')
    {
      *tokenid = T_NEWLINE;
      return tok;
    }
  else if (c == EOF)
    {
      *tokenid = T_EOF;
      return tok;
    }

  /* It looks like a string. Snag to the next whitespace. */
  if (c == '\"')
    {
      doublequotes = 1;
      c = read_char (as, pos);
    }

  while (1)
    {
      astr_cat_char (tok, (char) c);

      if (!doublequotes)
	{
	  if (c == ')' || c == '(' || c == ';' || c == ' ' || c == '\n'
	      || c == '\r' || c == EOF)
	    {
	      (*pos)--;
	      astr_truncate (tok, (ptrdiff_t) - 1);
	      *tokenid = T_WORD;
	      return tok;
	    }
	}
      else
	{
	  switch (c)
	    {
	    case '\n':
	    case '\r':
	    case EOF:
	      (*pos)--;
	      /* Fall through */

	    case '\"':
	      astr_truncate (tok, (ptrdiff_t) - 1);
	      *tokenid = T_WORD;
	      return tok;
	    }
	}

      c = read_char (as, pos);
    }

  return tok;
}

le *
lisp_read (le * list, astr as, ptrdiff_t * pos)
{
  astr tok;
  enum tokenname tokenid;
  int isquoted = 0;

  while (1)
    {
      tok = read_token (&tokenid, as, pos);

      switch (tokenid)
	{
	case T_QUOTE:
	  isquoted = 1;
	  break;

	case T_OPENPAREN:
	  list = leAddBranchElement (list, lisp_read (NULL, as, pos), isquoted);
	  isquoted = 0;
	  break;

	case T_NEWLINE:
	  isquoted = 0;
	  break;

	case T_WORD:
	  list = leAddDataElement (list, astr_cstr (tok), isquoted);
	  isquoted = 0;
	  break;

	case T_CLOSEPAREN:
	case T_EOF:
	  isquoted = 0;
	  astr_delete (tok);
	  return list;
	}

      astr_delete (tok);
    }
}
