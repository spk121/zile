#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eval.h"
#include "vars.h"

evalLookupNode evalTable[] = {
  { "+" 	 , eval_cb_add		},
  { "-" 	 , eval_cb_subtract	},
  { "*" 	 , eval_cb_multiply	},
  { "/" 	 , eval_cb_divide	},

  { "1+" 	 , eval_cb_oneplus	},
  { "1-" 	 , eval_cb_oneminus	},

  { "%" 	 , eval_cb_modulus	},

  { "<" 	 , eval_cb_lt		},
  { "<=" 	 , eval_cb_lt_eq	},
  { ">" 	 , eval_cb_gt		},
  { ">=" 	 , eval_cb_gt_eq	},
  { "=" 	 , eval_cb_eqsign	},

  { "and" 	, eval_cb_and		},
  { "or" 	, eval_cb_or		},
  { "not" 	, eval_cb_not		},
  { "null" 	, eval_cb_not		},

  { "atom" 	, eval_cb_atom		},
  { "car" 	, eval_cb_car		},
  { "cdr" 	, eval_cb_cdr		},
  { "cons" 	, eval_cb_cons		},
  { "list" 	, eval_cb_list		},
  { "equal"	, eval_cb_equal		},

  { "if"	, eval_cb_if		},
  { "unless"	, eval_cb_unless	},
  { "when"	, eval_cb_when		},
  { "cond"	, eval_cb_cond		},
  { "select"	, eval_cb_select	},

  { "princ"	, eval_cb_princ		},
  { "terpri"	, eval_cb_terpri	},

  { "eval"	, eval_cb_eval		},
  { "prog1"	, eval_cb_prog1		},
  { "prog2"	, eval_cb_prog2		},
  { "progn"	, eval_cb_progn		},

  { "set"	, eval_cb_set		},
  { "setq"	, eval_cb_setq		},
  { "setf"	, eval_cb_setq		},
  { "enum"	, eval_cb_enum		},

  { "defun"	, eval_cb_defun		},

  { "gc"	, eval_cb_nothing	},
  { "garbage-collect" , eval_cb_nothing	},

  { NULL	, NULL			}
};

    
le * evaluateBranch(le * trybranch)
{
  le * keyword;
  int tryit = 0;
  if (!trybranch) return( NULL );

  if (trybranch->branch)
    {
      keyword = evaluateBranch(trybranch->branch);
    }
  else 
    keyword = leNew( trybranch->data );

  if (!keyword->data)
    {
      leWipe( keyword );
      return( leNew( "NIL" ));
    }

  for ( tryit=0 ; evalTable[tryit].word ; tryit++)
    {
      if (!strcmp(evalTable[tryit].word, keyword->data))
        {
          leWipe( keyword );
          return( evalTable[tryit].callback(
                                            countNodes( trybranch ), 
                                            trybranch) 
                  );
        }
    }

  leWipe( keyword );
  return( evaluateNode( trybranch ));
}
    
le * evaluateNode(le * node)
{
  le * temp;
  le * value;

  if (node->branch)
    {
      if( node->quoted )
        {
          value = leDup( node->branch );
        } else {
          value = evaluateBranch( node->branch );
        }
    } else {
      temp = variableGet( defunList, node->data );

      if (temp)
        {
          value = evaluateDefun( temp, node->list_next );
        } else {
          temp = variableGet( mainVarList, node->data );

          if (temp)
            {
              value = leDup( temp );
            } else {

              value = leNew( node->data );
            }
        }
    }

  return( value );
}
    
le * evaluateDefun( le * fcn, le * params )
{
  le * function;
  le * thisparam;
  le * result;
  int count;

  /* make sure both lists exist */
  if (!fcn)  return( leNew( "NIL" ));

  /* check for the correct number of parameters */
  if (countNodes(fcn->branch) > countNodes(params))
    return( leNew( "NIL" ));

  /* allocate another function definition, since we're gonna hack it */
  function = leDup(fcn);

  /* pass 1:  tag each node properly. 
     for each parameter: (fcn)
     - look for it in the tree, tag those with the value
  */
  count = 0;
  thisparam = fcn->branch;
  while (thisparam)
    {
      leTagData(function, thisparam->data, count);
      thisparam = thisparam->list_next;
      count++;
    }

  /* pass 2:  replace
     for each parameter: (param)
     - evaluate the passed in value
     - replace it in the tree
  */
  count = 0;
  thisparam = params;
  while (thisparam)
    {
      result = evaluateNode( thisparam );
      leTagReplace(function, count, result);
      thisparam = thisparam->list_next;
      leWipe(result);
      count++;
    }

  /* then evaluate the resulting tree */
  result = evaluateBranch( function->list_next );
	
  /* free any space allocated */
  leWipe( function );

  /* return the evaluation */
  return( result );
}

    
int countNodes(le * branch)
{
  int count = 0;

  while (branch)
    {
      count++;
      branch = branch->list_next;
    }
  return( count );
}

    
int evalCastLeToInt( const le * levalue )
{
  if (!levalue) return( 0 );
  if (!levalue->data) return( 0 );
	
  return( atoi(levalue->data) );
}
    
le * evalCastIntToLe( int intvalue )
{
  char buffer[80];
  sprintf (buffer, "%d", intvalue);

  return( leNew(buffer) );
}

    
le * eval_cb_nothing( const int argc, le * branch )
{
  return( leNew( "T" ));
}
    
int
eval_cume_helper(
                 enum cumefcn function,
                 int value,
                 le * branch
                 ) 
{
  int newvalue = 0;
  le * temp = branch;
  le * value_le;
  if (!branch) return( 0 );

  while (temp)
    {
      value_le = evaluateNode(temp); 
      newvalue = evalCastLeToInt(value_le);
      leWipe(value_le);

      switch(function)
        {
        case (C_NONE):
          break;

        case( C_ADD ):
          value += newvalue;
          break;

        case( C_SUBTRACT ):
          value -= newvalue;
          break;

        case( C_MULTIPLY ):
          value *= newvalue;
          break;

        case( C_DIVIDE ):
          value /= newvalue;
          break;
        }

      temp = temp->list_next;
    }

  return( value );
}
    
le * eval_cb_add( const int argc, le * branch )
{
  if (!branch || argc < 2) return( leNew( "NIL" ) );

  return( evalCastIntToLe(
                          eval_cume_helper( 
                                           C_ADD,
                                           0, 
                                           branch->list_next
                                           )
                          )
          );
}
    
le * eval_cb_subtract( const int argc, le * branch )
{
  int firstitem = 0;
  le * lefirst;

  if (!branch || argc < 2) return( leNew( "NIL" ) );

  lefirst = evaluateNode( branch->list_next );
  firstitem = evalCastLeToInt( lefirst );
  leWipe( lefirst );

  if (argc == 2)
    {
      return( evalCastIntToLe( -1 * firstitem) );
    }
	
  return( evalCastIntToLe(
                          eval_cume_helper( 
                                           C_SUBTRACT,
                                           firstitem, 
                                           branch->list_next->list_next
                                           )
                          )
          );
}
    
le * eval_cb_multiply( const int argc, le * branch )
{
  if (!branch || argc < 2) return( leNew( "NIL" ) );

  return( evalCastIntToLe(
                          eval_cume_helper( 
                                           C_MULTIPLY,
                                           1, 
                                           branch->list_next
                                           )
                          )
          );
}
    
le * eval_cb_divide( const int argc, le * branch )
{
  int firstitem = 0;
  le * lefirst;
  if (!branch || argc < 2) return( leNew( "NIL" ) );

  lefirst = evaluateNode( branch->list_next );
  firstitem = evalCastLeToInt( lefirst );
  leWipe( lefirst );

  return( evalCastIntToLe(
                          eval_cume_helper( 
                                           C_DIVIDE,
                                           firstitem, 
                                           branch->list_next->list_next
                                           )
                          )
          );
}
    
le * eval_cb_oneplus( const int argc, le * branch )
{
  le * retle;
  int value;

  if (!branch || argc < 2) return( leNew( "NIL" ) );

  retle = evaluateNode( branch->list_next ); 
  value = evalCastLeToInt( retle );
  leWipe( retle );

  return( evalCastIntToLe(value + 1) );
}
    
le * eval_cb_oneminus( const int argc, le * branch )
{
  le * retle;
  int value;

  if (!branch || argc < 2) return( leNew( "NIL" ) );

  retle = evaluateNode( branch->list_next ); 
  value = evalCastLeToInt( retle );
  leWipe( retle );

  return( evalCastIntToLe(value - 1) );

}
    
le * eval_cb_modulus( const int argc, le * branch )
{
  le * letemp;
  int value1, value2;

  if (!branch || argc != 3) return( leNew( "NIL" ) );

  letemp = evaluateNode( branch->list_next );
  value1 = evalCastLeToInt( letemp );
  leWipe( letemp );

  letemp = evaluateNode( branch->list_next->list_next );
  value2 = evalCastLeToInt( letemp );
  leWipe( letemp );

  return( evalCastIntToLe ( value1 % value2 ) );
}

    
le * eval_cb_lt( const int argc, le * branch )
{
  le * letemp;
  int value1, value2;

  if (!branch || argc != 3 ) return( leNew( "NIL" ) );

  letemp = evaluateNode( branch->list_next );
  value1 = evalCastLeToInt( letemp );
  leWipe( letemp );

  letemp = evaluateNode( branch->list_next->list_next );
  value2 = evalCastLeToInt( letemp );
  leWipe( letemp );

  return( leNew ( (value1 < value2 )?"T":"NIL" ) );
}
    
le * eval_cb_lt_eq( const int argc, le * branch )
{
  le * letemp;
  int value1, value2;

  if (!branch || argc != 3 ) return( leNew( "NIL" ) );

  letemp = evaluateNode( branch->list_next );
  value1 = evalCastLeToInt( letemp );
  leWipe( letemp );

  letemp = evaluateNode( branch->list_next->list_next );
  value2 = evalCastLeToInt( letemp );
  leWipe( letemp );

  return( leNew ( (value1 <= value2 )?"T":"NIL" ) );
}
    
le * eval_cb_gt( const int argc, le * branch )
{
  le * letemp;
  int value1, value2;

  if (!branch || argc != 3 ) return( leNew( "NIL" ) );

  letemp = evaluateNode( branch->list_next );
  value1 = evalCastLeToInt( letemp );
  leWipe( letemp );

  letemp = evaluateNode( branch->list_next->list_next );
  value2 = evalCastLeToInt( letemp );
  leWipe( letemp );

  return( leNew ( (value1 > value2 )?"T":"NIL" ) );
}
    
le * eval_cb_gt_eq( const int argc, le * branch )
{
  le * letemp;
  int value1, value2;

  if (!branch || argc != 3 ) return( leNew( "NIL" ) );

  letemp = evaluateNode( branch->list_next );
  value1 = evalCastLeToInt( letemp );
  leWipe( letemp );

  letemp = evaluateNode( branch->list_next->list_next );
  value2 = evalCastLeToInt( letemp );
  leWipe( letemp );

  return( leNew ( (value1 >= value2 )?"T":"NIL" ) );
}
    
le * eval_cb_eqsign( const int argc, le * branch )
{
  le * letemp;
  int value1, value2;

  if (!branch || argc != 3 ) return( leNew( "NIL" ) );

  letemp = evaluateNode( branch->list_next );
  value1 = evalCastLeToInt( letemp );
  leWipe( letemp );

  letemp = evaluateNode( branch->list_next->list_next );
  value2 = evalCastLeToInt( letemp );
  leWipe( letemp );

  return( leNew ( (value1 == value2 )?"T":"NIL" ) );
}

    
le * eval_cb_and( const int argc, le * branch )
{
  le * temp;
  le * result = NULL;
  if (!branch || argc < 2 ) return( leNew( "NIL" ));

  temp = branch->list_next;
  while( temp )
    {
      if( result )  leWipe( result );

      result = evaluateNode(temp);
      if (result->data)
        {
          if (!strcmp ( result->data, "NIL" ))
            {
              return( result );
            }
        }
      temp = temp->list_next;
    }
  return( result );
}
    
le * eval_cb_or( const int argc, le * branch )
{
  le * temp;
  le * result = NULL;
  if (!branch || argc < 2 ) return( leNew( "NIL" ));

  temp = branch->list_next;
  while( temp )
    {
      if( result )  leWipe( result );

      result = evaluateNode(temp);
      if (result->data)
        {
          if (strcmp ( result->data, "NIL" ))
            {
              return( result );
            }
        }
      temp = temp->list_next;
    }
  return( result );
}
    
le * eval_cb_not( const int argc, le * branch )
{
  le * result = NULL;
  if (!branch || argc != 2 ) return( leNew( "NIL" ));

  result = evaluateNode(branch->list_next);

  if (result->data)
    {
      if (!strcmp (result->data, "NIL" ))
        {
          leWipe( result );
          return( leNew( "T" ) );
        } else {
          leWipe( result );
          return( leNew( "NIL" ) );
        }
    } else if (result->branch) {
      leWipe( result );
      return( leNew( "NIL" ) );
    }
	
  leWipe( result );
  return( leNew( "T" ));
}

    
le * eval_cb_atom( const int argc, le * branch )
{
  le * result = NULL;
  if (!branch || argc != 2 ) return( leNew( "NIL" ));

  result = evaluateNode(branch->list_next);

  if (countNodes(result) == 1)
    {
      leWipe( result );
      return( leNew( "T" ) );
    }
  return( leNew( "NIL" ) );
}
    
le * eval_cb_car( const int argc, le * branch )
{
  le * result = NULL;
  le * temp = NULL;
  if (!branch || argc != 2 ) return( leNew( "NIL" ));

  result = evaluateNode(branch->list_next);

  if( result == NULL )  return( leNew( "NIL" ) );

  if (countNodes(result) <= 1)
    {
      if (result->branch)
        {
          temp = result;
          result = result->branch;
          temp->branch = NULL;
          leWipe( temp );
        }
      return( result );
    }

  result->list_next->list_prev = NULL;
  leWipe( result->list_next );
  result->list_next = NULL;

  if (result->branch)
    {
      temp = result;
      result = result->branch;
      temp->branch = NULL;
      leWipe( temp );
    }

  return( result );
}
    
le * eval_cb_cdr( const int argc, le * branch )
{
  le * result = NULL;
  le * temp = NULL;
  if (!branch || argc != 2 ) return( leNew( "NIL" ));

  result = evaluateNode(branch->list_next);

  if( result == NULL )  return( leNew( "NIL" ) );

  if (result == NULL  || countNodes(result) == 1)
    {
      return( leNew( "NIL" ) );
    }

  temp = result;
  temp->list_next->list_prev = NULL;
  result = result->list_next;

  temp->list_next = NULL;
  leWipe( temp );


  return( result );
}
    
le * eval_cb_cons( const int argc, le * branch )
{
  le * result1 = NULL;
  le * result2 = NULL;

  if (!branch || argc != 3 ) return( leNew( "NIL" ));

  result1 = evaluateNode(branch->list_next);
  if ( result1 == NULL ) return( leNew( "NIL" ));

  result2 = evaluateNode(branch->list_next->list_next);
  if ( result2 == NULL )
    {
      leWipe( result1 );
      return( leNew( "NIL" ));
    }

  if ( countNodes(result1) > 1 )
    {
      le * temp = leNew( NULL );
      temp->branch = result1;
      result1 = temp;
    } 
  result1->list_next = result2;
  result2->list_prev = result1;

  return( result1 );
}
    
le * eval_cb_list( const int argc, le * branch )
{
  le * currelement = NULL;
  le * finaltree = NULL;
  le * lastadded = NULL;
  le * result = NULL;

  if (!branch) return( leNew( "NIL" ));

  currelement = branch->list_next;
  while (currelement)
    {
      result = evaluateNode(currelement);
      if ( result == NULL )
        {
          leWipe( finaltree );
          return( leNew( "NIL" ));
        }

      if( countNodes(result) > 1)
        {
          le * temp = leNew( NULL );
          temp->branch = result;
          result = temp;
        }
    
      if (!finaltree)
        {
          finaltree = result;
          lastadded = result;
        } else {
          lastadded->list_next = result;
          result->list_prev    = lastadded;
          lastadded = result;
		
        }
	    
      currelement = currelement->list_next;
    }

  if (!finaltree)
    {
      return( leNew( "NIL" ));
    }
  return( finaltree );
}
    
int eval_cb_lists_same( le * list1, le * list2 )
{
  if (!list1 && !list2)    return( 1 );

  while( list1 )
    {
      /* if list2 ended prematurely, fail */
      if (list2 == NULL)
        {
          return( 0 );
        }

      /* if one has data and the other doesn't, fail */
      if (   (list1->data && ! list2->data)
             || (list2->data && ! list1->data))
        {
          return( 0 );
        }

      /* if the data is different, fail */
      if (list1->data && list2->data)
        {
          if (strcmp( list1->data, list2->data ))
            {
              return( 0 );
            }
        }

      /* if one is quoted and the other isn't, fail */
      if (list1->quoted != list2->quoted)
        {
          return( 0 );
        }

      /* if their branches aren't the same, fail */
      if (!eval_cb_lists_same( list1->branch, list2->branch ))
        {
          return( 0 );
        }

      /* try the next in the list */
      list1 = list1->list_next;
      list2 = list2->list_next;
    }

  /* if list2 goes on, fail */
  if (list2)
    {
      return( 0 );
    }

  return( 1 );
}
    
le * eval_cb_equal( const int argc, le * branch )
{
  le * list1 = NULL;
  le * list2 = NULL;
  int retval = 0;

  if (!branch || argc != 3 ) return( leNew( "NIL" ) );

  list1 = evaluateNode( branch->list_next );
  list2 = evaluateNode( branch->list_next->list_next );

  retval = eval_cb_lists_same( list1, list2 );

  leWipe( list1 );
  leWipe( list2 );

  return( leNew ( (retval == 1) ? "T" : "NIL" ) );
}

    
le * eval_cb_if( const int argc, le * branch )
{
  le * retcond = NULL;

  if (!branch || argc < 3 || argc > 4) return( leNew( "NIL" ));

  /* if */
  retcond = evaluateNode(branch->list_next);

  if (!strcmp ( retcond->data, "NIL" ))
    {
      /* else */
      if (argc == 3) /* no else */
        return( retcond );

      leWipe( retcond );
      return( evaluateNode( branch->list_next->list_next->list_next ) );
    }

  /* then */
  leWipe( retcond );
  return( evaluateNode(branch->list_next->list_next) );
}
    
le *
eval_cb_whenunless_helper(
                          enum whenunless which,
                          const int argc, 
                          le * branch
                          )
{
  le * retval = NULL;
  le * trythis = NULL;

  if (!branch || argc < 3 ) return( leNew( "NIL" ));

  /* conditional */
  retval = evaluateNode(branch->list_next);

  if ( which == WU_UNLESS )
    {
      /* unless - it wasn't true... bail */
      if ( strcmp( retval->data, "NIL" ))
        {
          leWipe( retval );
          return( leNew( "NIL" ) );
        }
    } else {
      /* when:   it wasn't false... bail */
      if ( !strcmp( retval->data, "NIL" ))
        {
          return( retval );
        }
    }

  trythis = branch->list_next->list_next;
  while( trythis )
    {
      if (retval)  leWipe( retval );

      retval = evaluateNode(trythis);
      trythis = trythis->list_next;
    }
  return( retval );
}
    
le * eval_cb_unless( const int argc, le * branch )
{
  return( eval_cb_whenunless_helper(
                                    WU_UNLESS,
                                    argc,
                                    branch ) );
}
    
le * eval_cb_when( const int argc, le * branch )
{
  return( eval_cb_whenunless_helper(
                                    WU_WHEN,
                                    argc,
                                    branch ) );
}
    
le * eval_cb_cond( const int argc, le * branch )
{
  le * retval = NULL;
  le * retblock = NULL;
  le * trythis = NULL;
  le * tryblock = NULL;
  int newargc;

  if (!branch || argc < 2 ) return( leNew( "NIL" ));

  trythis = branch->list_next;
  while (trythis)
    {
      newargc = countNodes( trythis->branch );
      if (newargc == 0)  continue;

      /* conditional */
      if (retval)  leWipe(retval);
      retval = evaluateNode(trythis->branch);

      if ( strcmp(retval->data, "NIL" )) 
        {
          if (newargc == 1)
            {
              return( retval );
            }

          tryblock = trythis->branch->list_next;
          while (tryblock)
            {
              if (retblock)  leWipe(retblock);
              retblock = NULL;

              retblock = evaluateNode(tryblock);
              tryblock = tryblock->list_next;
            }
          return( retblock );
        }
		
      trythis = trythis->list_next;
    }
  return( retval );
}
    
le * eval_cb_select( const int argc, le * branch )
{
  le * result;

  if (argc < 2)  return( leNew( "NIL" ));

  branch = branch->list_next;
  result = evaluateNode(branch);

  branch = branch->list_next;
  while( branch )
    {
      if( branch->branch )
        {
          le * check = branch->branch;
          if (check && check->data
              && (!strcmp( check->data, result->data )))
            {
              /* we're in the right place, evaluate and return */
              le * computelist = check->list_next;
              while( computelist )
                {
                  leWipe( result );
                  result = evaluateNode( computelist );
                  computelist = computelist->list_next;
                }
              return( result );
            }
        }
	    
      branch = branch->list_next;
    }

  return( result );
}

    
le * eval_cb_princ( const int argc, le * branch )
{
  le * thisnode;
  le * retblock = NULL;
  if (!branch || argc < 1 ) return( leNew( "NIL" ));

  thisnode = branch->list_next;
  while (thisnode)
    {
      if (retblock)  leWipe( retblock );
      retblock = evaluateNode(thisnode);
      leDumpReformat(stdout, retblock);

      thisnode = thisnode->list_next;
    }
  return( retblock );
}
    
le * eval_cb_terpri( const int argc, le * branch )
{
  if (!branch || argc != 1 ) return( leNew( "NIL" ));

  printf( "\n" );
  return( leNew( "NIL" ) );
}

    
le * eval_cb_eval( const int argc, le * branch )
{
  le * temp;
  le * retval;
  if (!branch || argc != 2 ) return( leNew( "NIL" ));

  temp = evaluateNode(branch->list_next);
  retval = evaluateBranch(temp);
  leWipe( temp );
  return( retval );
}
    
le * eval_cb_prog( const int argc, le * branch, int returnit )
{
  le * curr;
  le * retval = NULL;
  le * tempval = NULL;
  int current = 0;
  if (!branch || argc < (returnit +1) ) return( leNew( "NIL" ));

  curr = branch->list_next;
  while (curr)
    {
      ++current;

      if ( tempval ) leWipe (tempval);
      tempval = evaluateNode( curr );

      if (current == returnit)
        retval = leDup( tempval );
	    
      curr = curr->list_next;
    }

  if (!retval)  retval = tempval;

  return( retval );
}
    
le * eval_cb_prog1( const int argc, le * branch )
{
  return( eval_cb_prog( argc, branch, 1 ));
}
    
le * eval_cb_prog2( const int argc, le * branch )
{
  return( eval_cb_prog( argc, branch, 2 ));
}
    
le * eval_cb_progn( const int argc, le * branch )
{
  return( eval_cb_prog( argc, branch, -1 ));
}

    
le *eval_cb_set_helper(enum setfcn function,
                       const int argc, 
                       le * branch)
{
  le *newkey, *newvalue = leNew("NIL"), *current;

  if (branch && argc >= 3) {
    current = branch->list_next;
    while (current) {
      if (!current->list_next)
        newvalue = leNew( "NIL" );
      else
        newvalue = evaluateNode(current->list_next);

      mainVarList = variableSet(mainVarList,
                                (function == S_SET) ? (newkey = evaluateNode(current))->data : current->data,
                                newvalue);

      if (function == S_SET)
        leWipe(newkey);

      if (!current->list_next)
        current = NULL;
      else
        current = current->list_next->list_next;
    }
  }

  return leDup(newvalue);
}
    
le * eval_cb_set( const int argc, le * branch )
{
  return( eval_cb_set_helper( S_SET, argc, branch ) );
}
    
le * eval_cb_setq( const int argc, le * branch )
{
  return( eval_cb_set_helper( S_SETQ, argc, branch ) );
}
    
le * eval_cb_enum( const int argc, le * branch )
{
  le * current;
  int count = -1;
  char value[16];

  if (!branch || argc < 2)  return( leNew( "NIL" ) );

  current = branch->list_next;
  while ( current )
    {
      if (current->data)
        {
          sprintf( value, "%d", ++count);

          mainVarList = variableSetString( 
                                          mainVarList,
                                          current->data,
                                          value
                                          );
        }
      current = current->list_next;
    }

  if (count == -1)
    return( leNew( "NIL" ) );
  else
    return( evalCastIntToLe(count) );
}

    
le *eval_cb_defun(const int argc, le * branch)
{
  if (!branch || argc < 4)
    return leNew("NIL");

  if (!branch->list_next->data)
    return leNew("NIL");

  defunList = variableSet(defunList,
                          branch->list_next->data,
                          branch->list_next->list_next);

  return leNew(branch->list_next->data);
}
