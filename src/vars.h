#include <stdio.h>
#include "lists.h"

extern le * mainVarList;
extern le * defunList;

le * variableFind( le * varlist, char * key );
#define variableFree( L ) \
                leWipe( (L) );

le * variableSet( le * varlist, char * key, le * value );
le * variableSetString( le * varlist, char * key, char * value );
le * variableGet( le * varlist, char * key );
char * variableGetString( le * varlist, char * key );

void variableDump( le * varlist );
