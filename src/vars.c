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
