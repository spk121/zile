/* Lisp eval
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

/*	$Id: eval.c,v 1.15 2005/01/25 20:06:07 rrt Exp $	*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"
#include "eval.h"
#include "vars.h"


static le *eval_cb_command_helper(Function f, int argc, le *branch)
{
  int uniarg = 0, ret = FALSE;
  if (argc == 2) {
    le *value_le = evaluateNode(branch);
    uniarg = evalCastLeToInt(value_le);
    leWipe(value_le);
  }
  if (argc < 3)
    f((argc == 1) ? 1 : uniarg);
  return ret ? leT : leNIL;
}

#define X(zile_name, c_name) \
  static le *eval_cb_ ## c_name(int argc, le *branch) \
  { \
    return eval_cb_command_helper(F_ ## c_name, argc, branch); \
  }
#define X0(zile_name, c_name)                    X(zile_name, c_name)
#define X1(zile_name, c_name, key1)              X(zile_name, c_name)
#define X2(zile_name, c_name, key1, key2)        X(zile_name, c_name)
#define X3(zile_name, c_name, key1, key2, key3)  X(zile_name, c_name)
#include "tbl_funcs.h"
#undef X
#undef X0
#undef X1
#undef X2
#undef X3

evalLookupNode evalTable[] = {
  { "+" 	, eval_cb_add		},
  { "-" 	, eval_cb_subtract	},
  { "*" 	, eval_cb_multiply	},
  { "/" 	, eval_cb_divide	},
  { "%" 	, eval_cb_modulus	},

  { "<" 	, eval_cb_lt		},
  { "<=" 	, eval_cb_lt_eq		},
  { ">" 	, eval_cb_gt		},
  { ">=" 	, eval_cb_gt_eq		},
  { "=" 	, eval_cb_eqsign	},

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

  { "princ"	, eval_cb_princ		},

  { "eval"	, eval_cb_eval		},
  { "prog1"	, eval_cb_prog1		},
  { "prog2"	, eval_cb_prog2		},
  { "progn"	, eval_cb_progn		},

  { "set"	, eval_cb_set		},
  { "setq"	, eval_cb_setq		},
  { "setf"	, eval_cb_setq		},

  { "defun"	, eval_cb_defun		},

#define X0(zile_name, c_name) \
	{ zile_name, eval_cb_ ## c_name },
#define X1(zile_name, c_name, key1) \
	{ zile_name, eval_cb_ ## c_name },
#define X2(zile_name, c_name, key1, key2) \
	{ zile_name, eval_cb_ ## c_name },
#define X3(zile_name, c_name, key1, key2, key3) \
	{ zile_name, eval_cb_ ## c_name },
#include "tbl_funcs.h"
#undef X0
#undef X1
#undef X2
#undef X3
  
  { NULL	, NULL			}
};


le *evaluateBranch(le *trybranch)
{
  le *keyword;
  int tryit = 0;

  if (trybranch == NULL)
    return NULL;

  if (trybranch->branch)
    keyword = evaluateBranch(trybranch->branch);
  else 
    keyword = leNew(trybranch->data);

  if (keyword->data == NULL) {
    leWipe(keyword);
    return leNIL;
  }

  for (tryit = 0; evalTable[tryit].word; tryit++)
    if (strcmp(evalTable[tryit].word, keyword->data) == 0) {
      leWipe(keyword);
      return evalTable[tryit].callback(countNodes(trybranch), trybranch);
    }

  leWipe(keyword);
  return evaluateNode(trybranch);
}
    
le *evaluateNode(le *node)
{
  le *value;

  if (node == NULL)
    return leNIL;
  
  if (node->branch != NULL) {
    if (node->quoted)
      value = leDup(node->branch);
    else
      value = evaluateBranch(node->branch);
        
  } else {
    le *temp = variableGet(defunList, node->data);

    if (temp != NULL)
      value = evaluateDefun(temp, node->list_next);
    else {
      temp = variableGet(mainVarList, node->data);

      if (temp != NULL)
        value = leDup(temp);
      else
        value = leNew(node->data);
    }
  }

  return value;
}
    
le *evaluateDefun(le *fcn, le *params)
{
  le *function;
  le *thisparam;
  le *result;
  int count;

  /* make sure both lists exist */
  if (fcn == NULL || params == NULL)
    return leNIL;

  /* check for the correct number of parameters */
  if (countNodes(fcn->branch) > countNodes(params))
    return leNIL;

  /* allocate another function definition, since we're gonna hack it */
  function = leDup(fcn);

  /* pass 1:  tag each node properly. 
     for each parameter: (fcn)
     - look for it in the tree, tag those with the value */
  count = 0;
  thisparam = fcn->branch;
  while (thisparam) {
    leTagData(function, thisparam->data, count);
    thisparam = thisparam->list_next;
    count++;
  }

  /* pass 2:  replace
     for each parameter: (param)
     - evaluate the passed in value
     - replace it in the tree */
  count = 0;
  thisparam = params;
  while (thisparam) {
    result = evaluateNode(thisparam);
    leTagReplace(function, count, result);
    thisparam = thisparam->list_next;
    leWipe(result);
    count++;
  }

  /* then evaluate the resulting tree */
  result = evaluateBranch(function->list_next);
	
  /* free any space allocated */
  leWipe(function);

  /* return the evaluation */
  return result;
}

    
int countNodes(le *branch)
{
  int count;

  for (count = 0; branch; branch = branch->list_next, count++);
  return count;
}

    
int evalCastLeToInt(const le *levalue)
{
  if (levalue == NULL || levalue->data == NULL)
    return 0;
	
  return atoi(levalue->data);
}
    
le *evalCastIntToLe(int intvalue)
{
  char *buf;
  le *list;

  asprintf(&buf, "%d", intvalue);
  list = leNew(buf);
  free(buf);

  return list;
}

    
int eval_cume_helper(enum cumefcn function, int value, le *branch) 
{
  int newvalue = 0;
  le *value_le;

  if (branch == NULL)
    return 0;

  for (; branch; branch = branch->list_next) {
    value_le = evaluateNode(branch); 
    newvalue = evalCastLeToInt(value_le);
    leWipe(value_le);

    switch (function) {
    case C_ADD:
      value += newvalue;
      break;
    case C_SUBTRACT:
      value -= newvalue;
      break;
    case C_MULTIPLY:
      value *= newvalue;
      break;
    case C_DIVIDE:
      value /= newvalue;
      break;
    default:
      assert(0);
    }
  }

  return value;
}
    
le *eval_cb_add(int argc, le *branch)
{
  if (branch == NULL || argc < 2)
    return leNIL;

  return evalCastIntToLe(eval_cume_helper(C_ADD, 0, branch->list_next));
}
    
le *eval_cb_subtract(int argc, le *branch)
{
  int firstitem = 0;
  le *lefirst;

  if (branch == NULL || argc < 2)
    return leNIL;

  lefirst = evaluateNode(branch->list_next);
  firstitem = evalCastLeToInt(lefirst);
  leWipe(lefirst);

  if (argc == 2)
    return evalCastIntToLe(-firstitem);
	
  return evalCastIntToLe(eval_cume_helper(C_SUBTRACT, firstitem, branch->list_next->list_next));
}
    
le *eval_cb_multiply(int argc, le *branch)
{
  if (branch == NULL || argc < 2)
    return leNIL;

  return evalCastIntToLe(eval_cume_helper(C_MULTIPLY, 1, branch->list_next));
}
    
le *eval_cb_divide(int argc, le *branch)
{
  int firstitem = 0;
  le *lefirst;
  if (branch == NULL || argc < 2)
    return leNIL;

  lefirst = evaluateNode(branch->list_next);
  firstitem = evalCastLeToInt(lefirst);
  leWipe(lefirst);

  return evalCastIntToLe(eval_cume_helper(C_DIVIDE, firstitem, branch->list_next->list_next));
}

le *eval_cb_modulus(int argc, le *branch)
{
  le *letemp;
  int value1, value2;

  if (branch == NULL || argc != 3)
    return leNIL;

  letemp = evaluateNode(branch->list_next);
  value1 = evalCastLeToInt(letemp);
  leWipe(letemp);

  letemp = evaluateNode(branch->list_next->list_next);
  value2 = evalCastLeToInt(letemp);
  leWipe(letemp);

  return evalCastIntToLe(value1 % value2);
}

typedef int (*eval_boolean)(int a, int b);

static le *eval_cb_boolean(int argc, le *branch, eval_boolean f)
{
  le *letemp;
  int value1, value2;

  if (branch == NULL || argc != 3)
    return leNIL;

  letemp = evaluateNode(branch->list_next);
  value1 = evalCastLeToInt(letemp);
  leWipe(letemp);

  letemp = evaluateNode(branch->list_next->list_next);
  value2 = evalCastLeToInt(letemp);
  leWipe(letemp);

  return f(value1, value2) ? leT : leNIL;
}

static int lt_helper(int a, int b)
{
  return a < b;
}

le *eval_cb_lt(int argc, le *branch)
{
  return eval_cb_boolean(argc, branch, lt_helper);
}
    
static int lt_eq_helper(int a, int b)
{
  return a <= b;
}

le *eval_cb_lt_eq(int argc, le *branch)
{
  return eval_cb_boolean(argc, branch, lt_eq_helper);
}
    
static int gt_helper(int a, int b)
{
  return a > b;
}

le *eval_cb_gt(int argc, le *branch)
{
  return eval_cb_boolean(argc, branch, gt_helper);
}

static int gt_eq_helper(int a, int b)
{
  return a >= b;
}

le *eval_cb_gt_eq(int argc, le *branch)
{
  return eval_cb_boolean(argc, branch, gt_eq_helper);
}

static int eqsign_helper(int a, int b)
{
  return a == b;
}

le *eval_cb_eqsign(int argc, le *branch)
{
  return eval_cb_boolean(argc, branch, eqsign_helper);
}

le *eval_cb_and(int argc, le *branch)
{
  le *result = NULL;

  if (branch == NULL || argc < 2)
    return leNIL;

  for (branch = branch->list_next; branch; branch = branch->list_next) {
    if (result)
      leWipe(result);

    result = evaluateNode(branch);
    if (result->data && strcmp(result->data, "NIL") == 0)
      break;
  }

  return result;
}
    
le *eval_cb_or(int argc, le *branch)
{
  le *result = NULL;

  if (branch == NULL || argc < 2)
    return leNIL;

  for (branch = branch->list_next; branch; branch = branch->list_next) {
    if (result)
      leWipe(result);

    result = evaluateNode(branch);
    if (result->data && strcmp(result->data, "NIL") != 0)
      break;
  }

  return result;
}
    
le *eval_cb_not(int argc, le *branch)
{
  le *result = NULL;

  if (branch == NULL || argc != 2)
    return leNIL;

  result = evaluateNode(branch->list_next);

  if (result->data) {
    if (strcmp (result->data, "NIL") == 0) {
      leWipe(result);
      return leT;
    } else {
      leWipe(result);
      return leNIL;
    }
  } else if (result->branch) {
    leWipe(result);
    return leNIL;
  }
	
  leWipe(result);
  return leT;
}

    
le *eval_cb_atom(int argc, le *branch)
{
  le *result = NULL;

  if (branch == NULL || argc != 2)
    return leNIL;

  result = evaluateNode(branch->list_next);

  if (countNodes(result) == 1) {
    leWipe(result);
    return leT;
  }

  leWipe(result);
  return leNIL;
}
    
le *eval_cb_car(int argc, le *branch)
{
  le *result = NULL;
  le *temp = NULL;

  if (branch == NULL || argc != 2)
    return leNIL;

  result = evaluateNode(branch->list_next);
  if (result == NULL)
    return leNIL;

  if (countNodes(result) <= 1) {
    if (result->branch) {
      temp = result;
      result = result->branch;
      temp->branch = NULL;
      leWipe(temp);
    }
    return result;
  }

  result->list_next->list_prev = NULL;
  leWipe(result->list_next);
  result->list_next = NULL;

  if (result->branch) {
    temp = result;
    result = result->branch;
    temp->branch = NULL;
    leWipe(temp);
  }

  return result;
}
    
le *eval_cb_cdr(int argc, le *branch)
{
  le *result = NULL;
  le *temp = NULL;

  if (branch == NULL || argc != 2)
    return leNIL;

  result = evaluateNode(branch->list_next);
  if (result == NULL || countNodes(result) == 1)
    return leNIL;

  temp = result;
  temp->list_next->list_prev = NULL;
  result = result->list_next;

  temp->list_next = NULL;
  leWipe(temp);

  return result;
}
    
le *eval_cb_cons(int argc, le *branch)
{
  le *result1 = NULL;
  le *result2 = NULL;

  if (branch == NULL || argc != 3)
    return leNIL;

  result1 = evaluateNode(branch->list_next);
  if (result1 == NULL)
    return leNIL;

  result2 = evaluateNode(branch->list_next->list_next);
  if (result2 == NULL) {
    leWipe(result1);
    return leNIL;
  }

  if (countNodes(result1) > 1) {
    le *temp = leNew(NULL);
    temp->branch = result1;
    result1 = temp;
  } 
  result1->list_next = result2;
  result2->list_prev = result1;

  return result1;
}
    
le *eval_cb_list(int argc, le *branch)
{
  le *currelement = NULL;
  le *finaltree = NULL;
  le *lastadded = NULL;
  le *result = NULL;

  if (branch == NULL)
    return leNIL;

  currelement = branch->list_next;
  while (currelement) {
    result = evaluateNode(currelement);
    if (result == NULL) {
      leWipe(finaltree);
      return leNIL;
    }

    if (countNodes(result) > 1) {
      le *temp = leNew(NULL);
      temp->branch = result;
      result = temp;
    }
    
    if (finaltree == NULL) {
      finaltree = result;
      lastadded = result;
    } else {
      lastadded->list_next = result;
      result->list_prev = lastadded;
      lastadded = result;
    }
	    
    currelement = currelement->list_next;
  }

  return finaltree ? finaltree : leNIL;
}
    
int eval_cb_lists_same(le *list1, le *list2)
{
  while (list1) {
    /* if list2 ended prematurely, fail */
    if (list2 == NULL)
      return 0;

    /* if one has data and the other doesn't, fail */
    if ((list1->data && list2->data == NULL)
        || (list2->data && list1->data == NULL))
      return 0;

    /* if the data is different, fail */
    if (list1->data && list2->data)
      if (strcmp(list1->data, list2->data))
        return 0;

    /* if one is quoted and the other isn't, fail */
    if (list1->quoted != list2->quoted)
      return 0;

    /* if their branches aren't the same, fail */
    if (!eval_cb_lists_same(list1->branch, list2->branch))
      return 0;

    /* try the next in the list */
    list1 = list1->list_next;
    list2 = list2->list_next;
  }

  /* if list2 goes on, fail */
  if (list2 != NULL)
    return 0;

  return 1;
}
    
le *eval_cb_equal(int argc, le *branch)
{
  le *list1 = NULL;
  le *list2 = NULL;
  int retval = 0;

  if (branch == NULL || argc != 3)
    return leNIL;

  list1 = evaluateNode(branch->list_next);
  list2 = evaluateNode(branch->list_next->list_next);

  retval = eval_cb_lists_same(list1, list2);

  leWipe(list1);
  leWipe(list2);

  return (retval == 1) ? leT : leNIL;
}

    
le *eval_cb_if(int argc, le *branch)
{
  le *retcond = NULL;

  if (branch == NULL || argc < 3 || argc > 4)
    return leNIL;

  /* if */
  retcond = evaluateNode(branch->list_next);

  if (strcmp(retcond->data, "NIL") == 0) {
    /* else */
    if (argc == 3) /* no else */
      return retcond;

    leWipe(retcond);
    return evaluateNode(branch->list_next->list_next->list_next);
  }

  /* then */
  leWipe(retcond);
  return evaluateNode(branch->list_next->list_next);
}
    
le *eval_cb_whenunless_helper(enum whenunless which, int argc, le *branch)
{
  le *retval = NULL;
  le *trythis = NULL;

  if (branch == NULL || argc < 3)
    return leNIL;

  /* conditional */
  retval = evaluateNode(branch->list_next);

  if (which == WU_UNLESS) {
    /* unless - it wasn't true... bail */
    if (strcmp(retval->data, "NIL")) {
      leWipe(retval);
      return leNIL ;
    }
  } else if (strcmp(retval->data, "NIL") == 0)
    /* when: it wasn't false... bail */
    return retval;

  trythis = branch->list_next->list_next;
  while (trythis) {
    if (retval)
      leWipe(retval);

    retval = evaluateNode(trythis);
    trythis = trythis->list_next;
  }

  return retval;
}
    
le *eval_cb_unless(int argc, le *branch)
{
  return eval_cb_whenunless_helper(WU_UNLESS, argc, branch);
}
    
le *eval_cb_when(int argc, le *branch)
{
  return eval_cb_whenunless_helper(WU_WHEN, argc, branch);
}
    
le *eval_cb_cond(int argc, le *branch)
{
  le *retval = NULL;
  le *retblock = NULL;
  le *trythis = NULL;
  le *tryblock = NULL;
  int newargc;

  if (branch == NULL || argc < 2)
    return leNIL;

  for (trythis = branch->list_next; trythis; trythis = trythis->list_next) {
    newargc = countNodes(trythis->branch);
    if (newargc == 0)
      continue;

    /* conditional */
    if (retval)
      leWipe(retval);
    retval = evaluateNode(trythis->branch);

    if (strcmp(retval->data, "NIL")) {
      if (newargc == 1)
        return retval;

      tryblock = trythis->branch->list_next;
      while (tryblock) {
        if (retblock)
          leWipe(retblock);
        retblock = NULL;

        retblock = evaluateNode(tryblock);
        tryblock = tryblock->list_next;
      }
      return retblock;
    }
  }

  return retval;
}
    
    
le *eval_cb_princ(int argc, le *branch)
{
  le *retblock = NULL;

  if (branch != NULL && argc >= 1) {
    le *thisnode;
    for (thisnode = branch->list_next; thisnode; thisnode = thisnode->list_next) {
      if (retblock)
        leWipe(retblock);
      retblock = evaluateNode(thisnode);
      leDumpReformat(retblock); /* XXX */
    }
  }
  
  return retblock;
}
    
    
le *eval_cb_eval(int argc, le *branch)
{
  le *temp;
  le *retval;
  if (branch == NULL || argc != 2)
    return leNIL;

  temp = evaluateNode(branch->list_next);
  retval = evaluateBranch(temp);
  leWipe(temp);
  return retval;
}
    
le *eval_cb_prog(int argc, le *branch, int returnit)
{
  le *curr;
  le *retval = NULL;
  le *tempval = NULL;
  int current = 0;

  if (branch == NULL || argc < returnit +1)
    return leNIL;

  for (curr = branch->list_next; curr; curr = curr->list_next) {
    if (tempval)
      leWipe(tempval);
    tempval = evaluateNode(curr);

    if (++current == returnit)
      retval = leDup(tempval);
  }

  return retval ? retval : tempval;
}
    
le *eval_cb_prog1(int argc, le *branch)
{
  return eval_cb_prog(argc, branch, 1);
}
    
le *eval_cb_prog2(int argc, le *branch)
{
  return eval_cb_prog(argc, branch, 2);
}
    
le *eval_cb_progn(int argc, le *branch)
{
  return eval_cb_prog(argc, branch, 0);
}

    
le *eval_cb_set_helper(enum setfcn function, int argc, le *branch)
{
  le *newkey, *newvalue = leNIL, *current;

  if (branch != NULL && argc >= 3) {
    for (current = branch->list_next; current; current = current->list_next->list_next) {
      if (newvalue != leNIL)
        leWipe(newvalue);
      
      newvalue = evaluateNode(current->list_next);

      mainVarList = variableSet(mainVarList,
                                function == S_SET ? (newkey = evaluateNode(current))->data : current->data,
                                newvalue);

      if (function == S_SET)
        leWipe(newkey);

      if (current->list_next == NULL)
        break;
    }
  }

  return newvalue;
}
    
le* eval_cb_set(int argc, le *branch)
{
  return eval_cb_set_helper(S_SET, argc, branch);
}
    
le *eval_cb_setq(int argc, le *branch)
{
  return eval_cb_set_helper(S_SETQ, argc, branch);
}
    
    
le *eval_cb_defun(int argc, le *branch)
{
  if (branch == NULL || argc < 4 || branch->list_next->data == NULL)
    return leNIL;

  defunList = variableSet(defunList,
                          branch->list_next->data,
                          branch->list_next->list_next);

  return leNew(branch->list_next->data);
}


void eval_init(void)
{
  leNIL = leNew("NIL");
  leT = leNew("T");
}


void eval_finalise(void)
{
  leReallyWipe(leNIL);
  leReallyWipe(leT);
}


static le *eval_expression(char *expr)
{
  le *list = lisp_read_string(expr);
  astr as = leDumpEval(list, 0);
  
  if (lastflag & FLAG_SET_UNIARG)
    insert_string(astr_cstr(as));
  else
    term_minibuf_write(astr_cstr(as));
  astr_delete(as);

  return list;
}

DEFUN("eval-expression", eval_expression)
  /*+
    Evaluate EVAL-EXPRESSION-ARG and print value in the minibuffer.
    Value is also consed on to front of the variable `values'.
    Optional argument EVAL-EXPRESSION-INSERT-VALUE, if non-nil, means
    insert the result into the current buffer instead of printing it in
    the minibuffer.
    +*/
{
  char *expr;
  le *list;

  if ((expr = minibuf_read("Eval: ", "")) == NULL)
    return FALSE;

  list = eval_expression(expr);
  /* XXX cons value on to front of values */
  leWipe(list);

  return list == NULL;
}

DEFUN("eval-last-sexp", eval_last_sexp)
  /*+
    Evaluate sexp before point; print value in minibuffer.
    Interactively, with prefix argument, print output into current buffer.
    +*/
{
  char *expr;
  le *list;
  Marker *m = point_marker();
  Region r;

  backward_sexp();
  calculate_region(&r, cur_bp->pt, m->pt);
  expr = copy_text_block(r.start.n, r.start.o, r.size);
  cur_bp->pt = m->pt;

  list = eval_expression(expr);
  free(expr);
  leWipe(list);

  return list == NULL;
}
