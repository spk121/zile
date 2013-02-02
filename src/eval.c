/* Lisp eval

   Copyright (c) 2001, 2005-2011 Free Software Foundation, Inc.
   Copyright (c) 2012 Michael L. Gran

   This file is part of Michael Gran's unofficial fork of GNU Zile.

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

#include <libguile.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"


static History *functions_history = NULL;

/* Return function's interactive flag, or -1 if not found. */
/* In the Guile port, all functions are interactive, for now. */
#if 0
SCM
get_function (SCM sym)
{
  if (!guile_symbol_is_name_of_defined_function (sym))
    return SCM_BOOL_F;

  return sym;
}

/* For us, all functions are interactive if they have no required
   arguments. */
int
get_function_interactive (SCM sym)
{
  if (!guile_symbol_is_name_of_defined_function (sym))
    return 0;

  SCM proc = guile_variable_ref_safe (guile_lookup_safe (sym));
  int required, optional;
  guile_get_procedure_arity (proc, &required, &optional);

  return required == 0;
}

#define MAX_DOCUMENTATION_LENGTH (80*24)
const char *
get_function_doc (SCM sym)
{
  int len;
  static char buf[MAX_DOCUMENTATION_LENGTH];
  SCM func = guile_lookup_safe (sym);

  if (!scm_is_true (func))
    return NULL;

  SCM doc = guile_procedure_documentation_safe (func);
  if (!scm_is_true (doc))
    return NULL;

  len = scm_to_locale_stringbuf (doc, buf, MAX_DOCUMENTATION_LENGTH - 1);
  if (len >= MAX_DOCUMENTATION_LENGTH -1)
    return NULL;

  buf[len] = '\0';
  return buf;
}

const char *
get_function_name (SCM p)
{
  const int maxlen = 256;
  int len;
  static buf[maxlen];
  len = scm_to_locale_stringbuf (scm_symbol_to_string (p), buf, maxlen-1);
  if (len < maxlen - 1)
  {
    buf[len] = '\0';
    return buf;
  }

  return NULL;
}
#endif

SCM
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

  return scm_from_bool (ret);
}

SCM
move_with_uniarg (int uniarg, bool (*move) (int dir))
{
  bool ret = true;
  for (unsigned long uni = 0; ret && uni < (unsigned) abs (uniarg); ++uni)
    ret = move (uniarg < 0 ? - 1 : 1);
  return scm_from_bool (ret);
}

SCM_DEFINE (G_execute_extended_command, "execute-extended-command",
	    0, 0, 0, (void), "\
Read function name, then read its arguments and call it.")
{
  SCM sym;
  astr msg = astr_new ();

  if (!interactive)
    guile_error ("execute-extended-command",
		 "this command cannot be used from the REPL");

  /* FIXME: uniarg repeating could be implemented */
  astr_cat_cstr (msg, "M-x ");

  castr name = minibuf_read_function_name (astr_cstr (msg));
  if (name == NULL)
    return SCM_BOOL_F;

  sym = scm_from_locale_symbol (astr_cstr (name));
  if (!guile_symbol_is_name_of_defined_function (sym))
    return SCM_BOOL_F;

  SCM ret = call_command (guile_variable_ref_safe (guile_lookup_safe (sym)),
			  1, false);
  SCM sret = scm_simple_format (SCM_BOOL_F, scm_from_locale_string ("~s"),
				scm_list_1 (ret));
  minibuf_write (scm_to_locale_string (sret));
  return ret;
}

/*
 * Read a function name from the minibuffer.
 */
castr
minibuf_read_function_name (const char *fmt, ...)
{
  va_list ap;
  Completion *cp = completion_new (false);
  SCM completion_func;

  completion_func = F_procedure_completions();
  if (scm_is_true (completion_func))
    {
      SCM guile_completion_list = scm_call_0 (completion_func);
      size_t len = scm_to_size_t (scm_length (guile_completion_list));
      for (size_t i = 0; i < len; ++i)
	{
	  SCM entry = scm_list_ref (guile_completion_list, scm_from_size_t (i));
	  char *name = scm_to_locale_string (scm_symbol_to_string (entry));
	  gl_sortedlist_add (get_completion_completions (cp),
			     completion_strcmp,
			     name);
	}
    }
  else
    gl_sortedlist_add (get_completion_completions (cp), completion_strcmp,
		      xstrdup(""));

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

void
init_guile_eval_procedures (void)
{
#include "eval.x"
  scm_c_export ("execute-extended-command", NULL);
}
