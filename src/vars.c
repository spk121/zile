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

/*	$Id: vars.c,v 1.2 2005/01/13 07:45:52 rrt Exp $	*/

#include "vars.h"
#include "eval.h"
#include <string.h>

le * mainVarList = NULL;
le * defunList = NULL;

    
le * variableFind( le * varlist, char * key )
{
  le * temp = varlist;

  if (!varlist || !key) return( NULL );

  while (temp)
    {
      if (!strcmp(key, temp->data))
        {
          return( temp );
        }
      temp = temp->list_next;
    }

  return( NULL );
}

    
le * variableSet( le * varlist, char * key, le * value )
{
  le * temp;

  if (!key || !value)  return( varlist );

  temp = variableFind( varlist, key );
  if ( temp )
    {
      leWipe( temp->branch );
      temp->branch = leDup( value );
    } else {
      temp = leNew( key );
      temp->branch = leDup( value );
      varlist = leAddHead( varlist, temp );
    }
  return( varlist );
}
    
le * variableSetString( le * varlist, char * key, char * value )
{
  le * temp;

  if (!key || !value)  return( varlist );

  temp = leNew(value);

  varlist = variableSet( varlist, key, temp );
	
  leWipe( temp );

  return( varlist );
}
    
le * variableGet( le * varlist, char * key )
{
  le * temp = variableFind(varlist, key);
  if (temp && temp->branch)
    return( temp->branch );
  return( NULL );
}
    
char * variableGetString( le * varlist, char * key )
{
  le * temp = variableFind(varlist, key);
  if (   temp
         && temp->branch
         && temp->branch->data
         && countNodes(temp->branch) == 1
         )
    return( strdup(temp->branch->data) );
  return( strdup("-1") );
}

    
void variableDump( le * varlist )
{
  le * temp = varlist;
  while (temp)
    {
      if (temp->branch && temp->data)
        {
          printf("%s \t", temp->data);
          leDumpReformat( stdout, temp->branch );
          printf("\n");
        }
      temp = temp->list_next;
    }
}
