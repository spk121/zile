/* Lisp eval

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 2001 Scott "Jerry" Lawrence.
   Copyright (c) 2005, 2006, 2007 Reuben Thomas.

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

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
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
#define X(zile_name, c_name, doc)               \
  {zile_name, F_ ## c_name, true, doc},
#include "tbl_funcs.h"
#undef X
};

#define fentry_table_size (sizeof (fentry_table) / sizeof (fentry_table[0]))

static fentry *
get_fentry (const char *name)
{
  size_t i;
  assert (name);
  for (i = 0; i < fentry_table_size; ++i)
    if (!strcmp (name, fentry_table[i].name))
      return &fentry_table[i];
  return NULL;
}

Function
get_function (const char *name)
{
  fentry * f = get_fentry (name);
  return f ? f->func : NULL;
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
  size_t i;
  for (i = 0; i < fentry_table_size; ++i)
    if (fentry_table[i].func == p)
      return fentry_table[i].name;
  return NULL;
}


le *leNIL, *leT;

size_t
countNodes (le * branch)
{
  int count;

  for (count = 0; branch; branch = branch->next, count++);
  return count;
}

DEFUN ("setq", setq)
/*+
(setq [sym val]...)

Set each sym to the value of its val.
The symbols sym are variables; they are literal (not evaluated).
The values val are expressions; they are evaluated.
+*/
{
  le *newvalue = leNIL, *current;
  size_t argc = countNodes (arglist);

  if (arglist != NULL && argc >= 2)
    {
      for (current = arglist->next; current;
	   current = current->next->next)
	{
	  if (newvalue != leNIL)
	    leWipe (newvalue);
	  newvalue = evaluateNode (current->next);
	  set_variable (current->data, newvalue->data);
	  if (current->next == NULL)
	    break; /* Cope with odd-length argument lists. */
	}
    }

  return newvalue;
}
END_DEFUN

le *
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

le *
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
    {
      leWipe (keyword);
      return leNIL;
    }

  func = get_fentry (keyword->data);
  leWipe (keyword);
  if (func)
    return func->func (0, trybranch);

  return NULL;
}

void
leEval (le * list)
{
  for (; list; list = list->next)
    leWipe (evaluateBranch (list->branch));
}

le *
execute_with_uniarg (bool undo, int uniarg, int (*forward) (void), int (*backward) (void))
{
  int uni, ret = true;
  int (*func) (void) = forward;

  if (backward && uniarg < 0)
    {
      func = backward;
      uniarg = -uniarg;
    }
  if (undo)
    undo_save (UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  for (uni = 0; ret && uni < uniarg; ++uni)
    ret = func ();
  if (undo)
    undo_save (UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return bool_to_lisp (ret);
}

le *
execute_function (const char *name, int uniarg)
{
  Function func = get_function (name);
  Macro *mp;

  if (func)
    return func (uniarg, NULL);
  else
    {
      mp = get_macro (name);
      if (mp)
	{
	  call_macro (mp);
	  return leT;
	}
      return leNIL;
    }
}

DEFUN ("execute-extended-command", execute_extended_command)
/*+
Read function name, then read its arguments and call it.
+*/
{
  le * ret;
  const char *name;
  astr msg = astr_new ();

  if (lastflag & FLAG_SET_UNIARG)
    astr_afmt (msg, "%d M-x ", uniarg);
  else
    astr_cat_cstr (msg, "M-x ");

  name = minibuf_read_function_name (astr_cstr (msg));
  astr_delete (msg);
  if (name == NULL)
    return false;

  ret = execute_function (name, uniarg);
  free ((char *) name);

  return ret;
}
END_DEFUN

/*
 * Read a function name from the minibuffer.
 */
static History *functions_history = NULL;
const char *
minibuf_read_function_name (const char *fmt, ...)
{
  va_list ap;
  size_t i;
  char *buf;
  const char *ms;
  Completion *cp;

  va_start (ap, fmt);
  xvasprintf (&buf, fmt, ap);
  va_end (ap);

  cp = completion_new (false);
  for (i = 0; i < fentry_table_size; ++i)
    if (fentry_table[i].interactive)
      gl_sortedlist_add (cp->completions, completion_strcmp,
			 xstrdup (fentry_table[i].name));

  for (;;)
    {
      ms = minibuf_read_completion (buf, "", cp, functions_history);

      if (ms == NULL) /* Cancelled. */
	{
	  free (buf);
	  FUNCALL (keyboard_quit);
	  break;
	}
      else if (ms[0] == '\0')
	{
	  minibuf_error ("No function name given");
	  free ((char *) ms);
	  ms = NULL;
	  break;
	}
      else
	{
	  astr as = astr_new ();
	  astr_cpy_cstr (as, ms);
	  /* Complete partial words if possible. */
	  if (completion_try (cp, as, false) == COMPLETION_MATCHED)
	    {
	      free ((char *) ms);
	      ms = xstrdup (cp->match);
	    }
	  astr_delete (as);

	  for (i = 0; i < gl_list_size (cp->completions); i++)
	    {
	      char *s = (char *) gl_list_get_at (cp->completions, i);
	      if (!strcmp (ms, s))
	      {
                free ((char *) ms);
		ms = xstrdup (s);
		break;
	      }
	    }

	  if (get_function (ms) || get_macro (ms))
	    {
	      add_history_element (functions_history, ms);
	      minibuf_clear ();
	      break;
	    }
	  else
	    {
	      minibuf_error ("Undefined function name `%s'", ms);
	      waitkey (WAITKEY_DEFAULT);
	    }
	}
    }

  free (buf);
  free_completion (cp);

  return ms;
}

void
init_eval (void)
{
  functions_history = history_new ();
  get_fentry ("setq")->interactive = false;
}

void
free_eval (void)
{
  free_history (functions_history);
}
