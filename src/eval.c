/* Lisp eval

   Copyright (c) 2001, 2005-2011 Free Software Foundation, Inc.

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

#include <config.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"


/*
 * Zile Lisp functions.
 */

struct fentry
{
  const char *name;		/* The function name. */
  Function func;		/* The function pointer. */
  bool interactive;             /* Whether function can be used interactively. */
  const char *doc;		/* Documentation string. */
};
typedef struct fentry fentry;

static fentry fentry_table[] = {
#define X(zile_name, c_name, interactive, doc)   \
  {zile_name, F_ ## c_name, interactive, doc},
#include "tbl_funcs.h"
#undef X
};

#define fentry_table_size (sizeof (fentry_table) / sizeof (fentry_table[0]))

static _GL_ATTRIBUTE_PURE fentry *
get_fentry (const char *name)
{
  assert (name);
  for (size_t i = 0; i < fentry_table_size; ++i)
    if (STREQ (name, fentry_table[i].name))
      return &fentry_table[i];
  return NULL;
}

Function
get_function (const char *name)
{
  fentry * f = get_fentry (name);
  return f ? f->func : NULL;
}

/* Return function's interactive flag, or -1 if not found. */
int
get_function_interactive (const char *name)
{
  fentry * f = get_fentry (name);
  return f ? f->interactive : -1;
}

const char *
get_function_doc (const char *name)
{
  fentry * f = get_fentry (name);
  return f ? f->doc : NULL;
}

const char *
get_function_name (Function p)
{
  for (size_t i = 0; i < fentry_table_size; ++i)
    if (fentry_table[i].func == p)
      return fentry_table[i].name;
  return NULL;
}


le *leNIL, *leT;

size_t
countNodes (le * branch)
{
  int count;
  for (count = 0; branch; branch = branch->next, count++)
    ;
  return count;
}

static le *
evaluateBranch (le * trybranch)
{
  le *keyword;
  fentry * func;

  if (trybranch == NULL)
    return NULL;

  if (trybranch->branch)
    keyword = evaluateBranch (trybranch->branch);
  else
    keyword = leNew (trybranch->data);

  if (keyword->data == NULL)
    return leNIL;

  func = get_fentry (keyword->data);
  if (func)
    return call_command (func->func, 1, false, trybranch);

  return NULL;
}

static le *
evaluateNode (le * node)
{
  le *value;

  if (node == NULL)
    return leNIL;

  if (node->branch != NULL)
    {
      if (node->quoted)
        value = leDup (node->branch);
      else
        value = evaluateBranch (node->branch);
    }
  else
    {
      const char *s = get_variable (node->data);
      value = leNew (s ? s : node->data);
    }

  return value;
}

DEFUN_NONINTERACTIVE ("setq", setq)
/*+
(setq [sym val]...)

Set each sym to the value of its val.
The symbols sym are variables; they are literal (not evaluated).
The values val are expressions; they are evaluated.
+*/
{
  le *newvalue = leNIL;

  if (arglist != NULL && countNodes (arglist) >= 2)
    {
      for (le *current = arglist->next; current;
           current = current->next->next)
        {
          newvalue = evaluateNode (current->next);
          set_variable (current->data, newvalue->data);
          if (current->next == NULL)
            break; /* Cope with odd-length argument lists. */
        }
    }

  ok = newvalue;
}
END_DEFUN

void
leEval (le * list)
{
  for (; list; list = list->next)
    evaluateBranch (list->branch);
}

le *
execute_with_uniarg (bool undo, int uniarg, bool (*forward) (void), bool (*backward) (void))
{
  if (backward && uniarg < 0)
    {
      forward = backward;
      uniarg = -uniarg;
    }
  if (undo)
    undo_start_sequence ();
  bool ret = true;
  for (int uni = 0; ret && uni < uniarg; ++uni)
    ret = forward ();
  if (undo)
    undo_end_sequence ();

  return bool_to_lisp (ret);
}

le *
move_with_uniarg (int uniarg, bool (*move) (int dir))
{
  bool ret = true;
  for (unsigned long uni = 0; ret && uni < (unsigned) abs (uniarg); ++uni)
    ret = move (uniarg < 0 ? - 1 : 1);
  return bool_to_lisp (ret);
}

le *
execute_function (const char *name, int uniarg, bool is_uniarg)
{
  Function func = get_function (name);
  return func ? call_command (func, uniarg, is_uniarg, NULL) : NULL;
}

DEFUN ("execute-extended-command", execute_extended_command)
/*+
Read function name, then read its arguments and call it.
+*/
{
  astr msg = astr_new ();

  if (lastflag & FLAG_SET_UNIARG)
    {
      if (lastflag & FLAG_UNIARG_EMPTY)
        msg = astr_fmt ("C-u ");
      else
        msg = astr_fmt ("%d ", uniarg);
    }
  astr_cat_cstr (msg, "M-x ");

  castr name = minibuf_read_function_name (astr_cstr (msg));
  if (name == NULL)
    return false;

  ok = bool_to_lisp (execute_function (astr_cstr (name), uniarg, lastflag & FLAG_SET_UNIARG) == leT);
}
END_DEFUN

/*
 * Read a function name from the minibuffer.
 */
static History *functions_history = NULL;
castr
minibuf_read_function_name (const char *fmt, ...)
{
  va_list ap;
  Completion *cp = completion_new (false);

  for (size_t i = 0; i < fentry_table_size; ++i)
    if (fentry_table[i].interactive)
      gl_sortedlist_add (get_completion_completions (cp), completion_strcmp,
                         xstrdup (fentry_table[i].name));

  va_start (ap, fmt);
  castr ms = minibuf_vread_completion (fmt, "", cp, functions_history,
                                       "No function name given",
                                       minibuf_test_in_completions,
                                       "Undefined function name `%s'", ap);
  va_end (ap);

  return ms;
}

void
init_eval (void)
{
  functions_history = history_new ();
}
