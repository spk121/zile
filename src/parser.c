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

/*	$Id: parser.c,v 1.2 2005/01/13 07:45:52 rrt Exp $	*/

#include <stdlib.h>
#include <string.h>

#include "parser.h"

    
struct le *
parseInFile(
	    getcCallback getachar,
	    ungetcCallback ungetachar,
	    struct le * list,
	    int * line
            )
{
  char * temp = NULL;
  enum tokenname tokenid = T_NONE;
  int isquoted = 0;

  if (!getachar || !ungetachar)  return( NULL );

  while (1){
	    
    temp = snagAToken(getachar, ungetachar, &tokenid);

    switch (tokenid)
      {
      case (T_NONE):
        break;

      case (T_QUOTE):
        isquoted = 1;
        break;

      case (T_OPENPAREN):
        list = leAddBranchElement(
                                  list, 
                                  parseInFile(getachar, 
                                              ungetachar, 
                                              NULL,
                                              line),
                                  isquoted
                                  );
        isquoted = 0;
        break;

      case (T_NEWLINE):
        isquoted = 0;
        *line = *line +1;
        break;

      case (T_WORD):
        list = leAddDataElement(
				list,
				temp,
				isquoted
				);
        free(temp);
        isquoted = 0;
        break;
	    
      case (T_CLOSEPAREN):
      case (T_EOF):
        isquoted = 0;
        return (list);
      }
  }
}
    
char *
snagAToken(
           getcCallback getachar,
           ungetcCallback ungetachar,
           enum tokenname * tokenid
           )
{
  unsigned int pos = 0;
  int c;
  int doublequotes = 0;
  char temp[128];

  *tokenid = T_EOF;

  if (!getachar || !ungetachar)
    {
      *tokenid = T_EOF;
      return( NULL );
    }

  /* chew space to next token */
  while (1)
    {
      c = getachar();

      /* munch comments */
      if (    (c == '#') 
              || (c == ';')
              )
        {
          do {
            c = getachar();
          } while (c != '\n');
        }
		
      if ((    (c == '(')
               || (c == ')')
               || (c == '\n')
               || (c == '\"')
               || (c == '\'')
               || (c == EOF)
               || (c > '-')
               || (c <= 'z')
	       ) && ( c != ' ') && ( c != '\t') )
        {
          break;
        }
    }

  /* snag token */
  if (c == '(')
    {
      *tokenid = T_OPENPAREN;
      return( NULL );
    } else 

      if (c == ')')
	{
          *tokenid = T_CLOSEPAREN;
          return( NULL );
	} else 

          if (c == '\'')
            {
              *tokenid = T_QUOTE;
              return( NULL );
            } else 

              if (c == '\n')
                {
                  *tokenid = T_NEWLINE;
                  return( NULL );
                } else 

                  if (c == EOF)
                    {
                      *tokenid = T_EOF;
                      return( NULL );
                    }

  /* oh well. it looks like a string.  snag to the next whitespace. */

  if (c == '\"')
    {
      doublequotes = 1;
      c = getachar();
    }


  while (1)
    {
      temp[pos++] = (char) c;

      if (!doublequotes)
        { 
          if (    (c == ')')
                  || (c == '(')
                  || (c == ';')
                  || (c == '#')
                  || (c == ' ')
                  || (c == '\n')
                  || (c == '\r')
                  || (c == EOF)
                  )
            {
              ungetachar(c);
              temp[pos-1] = '\0';

              if ( !strcmp(temp, "quote") )
                {
                  *tokenid = T_QUOTE;
                  return( NULL );
                }
              *tokenid = T_WORD;
              return( strdup(temp) );
            }
        } else {
          switch (c)
            {
            case ( '\n' ):
            case ( '\r' ):
            case ( EOF ):
              ungetachar(c);

            case ( '\"' ):
              temp[pos-1] = '\0';
              *tokenid = T_WORD;
              return( strdup(temp) );
    
            }
        }

      c = getachar();
    }
  return( NULL );
}
