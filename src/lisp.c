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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"
#include "eval.h"
#include "lists.h"

enum tokenname
{
  T_EOF,
  T_CLOSEPAREN,
  T_OPENPAREN,
  T_NEWLINE,
  T_QUOTE,
  T_WORD
};

typedef int (*getcCallback) (void);
typedef void (*ungetcCallback) (int c);


void
lisp_init (void)
{
  leNIL = leNew ("NIL");
  leT = leNew ("T");
}

void
lisp_finalise (void)
{
  leReallyWipe (leNIL);
  leReallyWipe (leT);
}


static astr
snagAToken (getcCallback getachar, ungetcCallback ungetachar,
	    enum tokenname *tokenid)
{
  int c;
  int doublequotes = 0;
  astr tok = astr_new ();

  *tokenid = T_EOF;

  /* Chew space to next token */
  do
    {
      c = getachar ();

      /* Munch comments */
      if (c == ';')
	do
	  {
	    c = getachar ();
	  }
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
      c = getachar ();
    }

  while (1)
    {
      astr_cat_char (tok, (char) c);

      if (!doublequotes)
	{
	  if (c == ')' || c == '(' || c == ';' || c == ' ' || c == '\n'
	      || c == '\r' || c == EOF)
	    {
	      ungetachar (c);
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
	      ungetachar (c);
	      /* Fall through */

	    case '\"':
	      astr_truncate (tok, (ptrdiff_t) - 1);
	      *tokenid = T_WORD;
	      return tok;
	    }
	}

      c = getachar ();
    }

  return tok;
}

static struct le *
parseInFile (getcCallback getachar, ungetcCallback ungetachar,
	     struct le *list, int *line)
{
  astr tok;
  enum tokenname tokenid;
  int isquoted = 0;

  assert (getachar && ungetachar);

  while (1)
    {
      tok = snagAToken (getachar, ungetachar, &tokenid);

      switch (tokenid)
	{
	case T_QUOTE:
	  isquoted = 1;
	  break;

	case T_OPENPAREN:
	  list = leAddBranchElement (list,
				     parseInFile (getachar, ungetachar, NULL,
						  line), isquoted);
	  isquoted = 0;
	  break;

	case T_NEWLINE:
	  isquoted = 0;
	  *line = *line + 1;
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

static le *
lisp_read (getcCallback getcp, ungetcCallback ungetcp)
{
  int lineno = 0;
  struct le *list = NULL;

  list = parseInFile (getcp, ungetcp, list, &lineno);

  return list;
}


static FILE *fp = NULL;

static int
getc_file (void)
{
  return getc (fp);
}

static void
ungetc_file (int c)
{
  ungetc (c, fp);
}

le *
lisp_read_file (const char *file)
{
  le *list;
  fp = fopen (file, "r");

  if (fp == NULL)
    return NULL;

  list = lisp_read (getc_file, ungetc_file);
  fclose (fp);

  return list;
}
