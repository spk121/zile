/* Guile interaction helper functions

   Copyright (c) 2011, 2012, 2013, 2018 Michael L. Gran

   This file is part of Michael's fork of GNU Zile.

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

#define _GNU_SOURCE
#include <config.h>
#include <assert.h>
#include <libguile.h>
#include <stdbool.h>
#include "main.h"
#include "extern.h"
#define NAME_LEN_MAX (200)

scm_t_port_type *minibuf_port_type;
SCM minibuf_port;
const size_t minibuf_buffer_size = 80;
char minibuf_buffer[80];

SCM stderr_port;

static SCM _guile_false_error_handler (void *data, SCM key, SCM exception);

/****************************************************************
 * PROCEDURE LOOKUP FUNCTIONS
 *
 ****************************************************************/
static SCM
lookup_procedure (const char *name)
{
  SCM sym = scm_from_locale_symbol (name);
  if (!scm_is_symbol (sym))
    return SCM_BOOL_F;

  SCM var = guile_lookup_safe (sym);
  if (!scm_is_true (scm_variable_p (var)))
    return SCM_BOOL_F;

  SCM ref = scm_variable_ref (var);
  if (!scm_is_true (scm_procedure_p (ref)))
    return SCM_BOOL_F;

  return ref;
}

#define F_LOOKUP(x)				\
  do {						\
    static SCM proc = SCM_BOOL_F;		\
    if (!scm_is_true (proc))			\
      proc = lookup_procedure (x);		\
    return proc;				\
  }						\
  while (0)

SCM F_mark_paragraph ()
{
  F_LOOKUP("mark-paragraph");
}

SCM F_universal_argument ()
{
  F_LOOKUP("universal-argument");
}

SCM F_undo ()
{
  F_LOOKUP ("undo");
}

SCM F_self_insert_command ()
{
  F_LOOKUP ("self-insert-command");
}

SCM F_next_line ()
{
  F_LOOKUP ("next-line");
}

SCM F_previous_line ()
{
  F_LOOKUP ("previous-line");
}

SCM F_kill_region ()
{
  F_LOOKUP ("kill-region");
}

SCM F_procedure_completions ()
{
  F_LOOKUP ("%procedure-completions");
}

SCM F_process_use_modules ()
{
  F_LOOKUP ("process-use-modules");
}

/****************************************************************
 * SAFE FUNCTIONS -- versions of Guile functions that
 *    will never have non-local exits.  A lot of this is
 *    probably overkill, but, it saves me the effort of
 *    checking for new non-local-exits with each version of
 *    Guile.
 *
 *    Only SCM_DEFINED's functions are allowed to use
 *    Guile commands that aren't on this list, because their
 *    exceptions will be caught.
 *
 ****************************************************************/

struct name_value
{
  SCM name;
  SCM value;
};

static SCM
_guile_c_define_safe_body (void *data)
{
  struct name_value *nv = (struct name_value *) data;
  return scm_define (nv->name, nv->value);
}

SCM
guile_c_define_safe (const char *name, SCM value)
{
  struct name_value nv;
  nv.name = scm_from_locale_symbol (name);
  nv.value = value;
  SCM ret = scm_c_catch (SCM_BOOL_T,
			 _guile_c_define_safe_body, &nv,
			 _guile_false_error_handler, (void *) name,
			 NULL, NULL);
  return ret;
}

static SCM
_guile_lookup_safe_body (void *data)
{
  return scm_lookup (SCM_PACK (data));
}

SCM
guile_lookup_safe (SCM sym)
{
  SCM ret = scm_c_catch (SCM_BOOL_T,
			 _guile_lookup_safe_body,  (void *) SCM_UNPACK (sym),
			 _guile_false_error_handler, NULL,
			 NULL, NULL);
  return ret;
}

static SCM
_guile_c_resolve_module_safe_body (void *data)
{
  return scm_c_resolve_module ((const char *) data);
}

SCM
guile_c_resolve_module_safe (const char *name)
{
  SCM ret = scm_c_catch (SCM_BOOL_T,
			 _guile_c_resolve_module_safe_body, (void *) name,
			 _guile_false_error_handler, (void *) name,
			 NULL, NULL);
  return ret;
}

static SCM
_guile_from_locale_string_safe_body (void *str)
{
  return scm_from_locale_string ((const char *) str);
}

SCM
guile_from_locale_string_safe (const char *str)
{
  SCM ret = scm_c_catch (SCM_BOOL_T,
			 _guile_from_locale_string_safe_body, (void *) str,
			 _guile_false_error_handler, (void *) str,
			 NULL, NULL);
  return ret;
}

struct scm_str
{
  SCM scm;
  char *str;
};

static SCM
_guile_to_locale_string_safe_body (void *data)
{
  struct scm_str *ss = (struct scm_str *) data;
  ss->str = scm_to_locale_string (ss->scm);
  return ss->scm;
}

static SCM
_guile_to_locale_string_safe_handler (void *data, SCM key, SCM args)
{
  struct scm_str *ss = (struct scm_str *) data;
  ss->str = NULL;
  return SCM_BOOL_F;
}

char *
guile_to_locale_string_safe (SCM x)
{
  struct scm_str ss;
  ss.scm = x;
  ss.str = NULL;
  scm_c_catch (SCM_BOOL_T,
	       _guile_to_locale_string_safe_body,
	       &ss,
	       _guile_to_locale_string_safe_handler,
	       &ss,
	       NULL, NULL);
  return ss.str;
}

static SCM
_guile_variable_ref_safe_body (void *data)
{
  return scm_variable_ref (SCM_PACK ((scm_t_bits) data));
}

SCM
guile_variable_ref_safe (SCM var)
{
  SCM ret = scm_c_catch (SCM_BOOL_T,
			 _guile_variable_ref_safe_body, (void *) SCM_UNPACK (var),
			 _guile_false_error_handler, "guile_variable_ref_safe",
			 NULL, NULL);
  return ret;
}

static SCM
_guile_procedure_documentation_body (void *data)
{
  return scm_procedure_documentation (SCM_PACK ((scm_t_bits) data));
}

SCM
guile_procedure_documentation_safe (SCM var)
{
  SCM ret = scm_c_catch (SCM_BOOL_T,
			 _guile_procedure_documentation_body, (void *) SCM_UNPACK (var),
			 _guile_false_error_handler, "guile_procedure_documentation",
			 NULL, NULL);
  return ret;
}

static SCM
_guile_procedure_name_body (void *data)
{
  return scm_procedure_name (SCM_PACK ((scm_t_bits) data));
}

SCM
guile_procedure_name_safe (SCM proc)
{
  SCM ret = scm_c_catch (SCM_BOOL_T,
			 _guile_procedure_name_body, (void *) SCM_UNPACK (proc),
			 _guile_false_error_handler, "guile_procedure_name",
			 NULL, NULL);
  return ret;
}


SCM
guile_use_zile_module (void *unused)
{
  return scm_call_1 (scm_variable_ref (scm_c_lookup ("process-use-modules")),
		     scm_list_1
		     (scm_list_1
		      (scm_list_1
		       (scm_string_to_symbol
			(scm_from_locale_string ("zile"))))));
}

#define MAX_DOCUMENTATION_LENGTH (80*24)
const char *
guile_get_procedure_documentation_by_name (const char *name)
{
  SCM sym, proc, doc;
  int len;
  static char buf[MAX_DOCUMENTATION_LENGTH];

  sym = scm_from_locale_symbol (name);
  if (!guile_symbol_is_name_of_defined_function (sym))
    return NULL;

  proc = guile_variable_ref_safe (guile_lookup_safe (sym));
  doc = guile_procedure_documentation_safe (proc);
  if (!scm_is_true (doc))
    return NULL;

  len = scm_to_locale_stringbuf (doc, buf, MAX_DOCUMENTATION_LENGTH - 1);
  if (len >= MAX_DOCUMENTATION_LENGTH -1)
    return NULL;

  buf[len] = '\0';
  return buf;
}

bool
guile_get_procedure_interactive_by_name (const char *name)
{
  SCM sym, proc;
  int required, optional;

  sym = scm_from_locale_symbol (name);
  if (!guile_symbol_is_name_of_defined_function (sym))
    return NULL;

  proc = guile_variable_ref_safe (guile_lookup_safe (sym));

  guile_get_procedure_arity (proc, &required, &optional);

  return required == 0;
}


#define MAX_NAME_LENGTH (80)

/* This is a pretty silly operation. Could probably replace this
   with "return name;". */
const char *
guile_get_procedure_name_by_name (const char *name)
{
  SCM sym, proc, procname;
  int len;
  static char buf[MAX_NAME_LENGTH];

  sym = scm_from_locale_symbol (name);
  if (!guile_symbol_is_name_of_defined_function (sym))
    return NULL;

  proc = guile_variable_ref_safe (guile_lookup_safe (sym));
  if (!scm_is_true (proc))
    return NULL;

  procname = guile_procedure_name_safe (proc);
  if (!scm_is_true (procname))
    return NULL;

  len = scm_to_locale_stringbuf (scm_symbol_to_string (procname), buf,
				 MAX_NAME_LENGTH - 1);

  if (len >= MAX_NAME_LENGTH - 1)
    return NULL;

  buf[len] = '\0';
  return buf;
}


/****************************************************************
 * ELISP Types
 *
 * If guile_elisp_compat is true, it will use ELisp's nil
 * instead of false.
 ****************************************************************/

int guile_elisp_compat = 0;

SCM
guile_nil_or_unspecified ()
{
#if HAVE_SCM_ELISP_NIL == 1
  if (guile_elisp_compat)
    return SCM_ELISP_NIL;
#endif
  return SCM_UNSPECIFIED;
}

SCM
guile_false_or_nil ()
{
#if HAVE_SCM_ELISP_NIL == 1
  if (guile_elisp_compat)
    return SCM_ELISP_NIL;
#endif
  return SCM_BOOL_F;
}

int
guile_is_nil_or_undefined (SCM x)
{
#if HAVE_SCM_NILP == 1
  if (SCM_NILP (x))
    return 1;
#endif
  return SCM_UNBNDP (x);
}

int
guile_is_nil_or_unspecified (SCM x)
{
#if HAVE_SCM_NILP == 1
  if (SCM_NILP (x))
    return 1;
#endif
  return x == SCM_UNSPECIFIED;
}

/****************************************************************
 * Emacs Error Types
 *
 * These are the error types as identified in EMACS.
 *
 * All SCM_DEFINE'd functions should only throw these errors.
 ****************************************************************/

/* ERRORS */
SCM_SYMBOL (guile_error_key, "zile-error");

SCM_SYMBOL (guile_beginning_of_buffer_error_key,
	    "zile-beginning-of-buffer-error");

SCM_SYMBOL (guile_end_of_buffer_error_key, "zile-end-of-buffer-error");

/* Number too big to be converted into a long.  */
SCM_SYMBOL (guile_overflow_error_key, "zile-overflow-error");

SCM_SYMBOL (guile_quit_error_key, "zile-quit-error");

SCM_SYMBOL (guile_read_only_error_key, "zile-read-only-error");

/* Function does not exist.  */
SCM_SYMBOL (guile_void_function_error_key, "zile-void-function-error");

SCM_SYMBOL (guile_wrong_number_of_arguments_error_key,
	    "zile-wrong-number-of-arguments");

SCM_SYMBOL (guile_wrong_type_argument_error_key,
	    "zile-wrong-type-argument-error");

static SCM
guile_append_newline_if_necessary (SCM s_msg)
{
  if (!scm_is_true (scm_string_any (SCM_MAKE_CHAR ('\n'), s_msg,
				    scm_from_int (0),
				    scm_string_length (s_msg))))
    s_msg = scm_string_append (scm_list_2 (s_msg,
					   scm_string
					   (scm_list_1
					    (SCM_MAKE_CHAR ('\n')))));
  return s_msg;
}

void
guile_error (const char *function_name, const char *msg)
{
  SCM s_function_name = scm_from_locale_string (function_name);
  SCM s_msg = scm_from_locale_string (msg);

  s_msg = guile_append_newline_if_necessary (s_msg);

  scm_throw (guile_error_key,
	     scm_list_4 (s_function_name,
			 s_msg,
			 SCM_EOL,
			 SCM_BOOL_F));
}


void
guile_beginning_of_buffer_error (const char *function_name)
{
  SCM s_function_name = scm_from_locale_string (function_name);
  SCM s_message = scm_from_locale_string ("beginning of buffer");

  s_message = guile_append_newline_if_necessary (s_message);

  scm_throw (guile_beginning_of_buffer_error_key,
	     scm_list_4 (s_function_name,
			 s_message,
			 SCM_EOL,
			 SCM_BOOL_F));
}

void
guile_end_of_buffer_error (const char *function_name)
{
  SCM s_function_name = scm_from_locale_string (function_name);
  SCM s_message = scm_from_locale_string ("end of buffer");

  s_message = guile_append_newline_if_necessary (s_message);

  scm_throw (guile_end_of_buffer_error_key,
	     scm_list_4 (s_function_name,
			 s_message,
			 SCM_EOL,
			 SCM_BOOL_F));
}

void
guile_overflow_error (const char *function_name, int position, SCM variable)
{
  SCM s_function_name = scm_from_locale_string (function_name);
  SCM s_message = scm_from_locale_string ("overflow error in position ~A: ~S");
  SCM s_position = scm_from_int (position);

  s_message = guile_append_newline_if_necessary (s_message);

  scm_throw (guile_overflow_error_key,
	     scm_list_4 (s_function_name,
			 s_message,
			 scm_list_2 (s_position, variable),
			 SCM_BOOL_F));
}

void
guile_quit_error (const char *function_name)
{
  SCM s_function_name = scm_from_locale_string (function_name);

  scm_throw (guile_quit_error_key,
	     scm_list_4 (s_function_name,
			 scm_from_locale_string ("Quit\n"),
			 SCM_EOL,
			 SCM_BOOL_F));
}


void
guile_read_only_error (const char *function_name, const char *buffer_name)
{
  SCM s_function_name = scm_from_locale_string (function_name);
  SCM s_buffer_name = scm_from_locale_string (buffer_name);
  SCM s_message = scm_from_locale_string ("Buffer is read-only: ~A");

  s_message = guile_append_newline_if_necessary (s_message);

  scm_throw (guile_read_only_error_key,
	     scm_list_4 (s_function_name,
			 s_message,
			 scm_list_1 (s_buffer_name),
			 SCM_BOOL_F));
}

void
guile_void_function_error (const char *function_name)
{
  SCM s_function_name = scm_from_locale_string (function_name);
  SCM s_message = scm_from_locale_string ("void function: ~A");

  s_message = guile_append_newline_if_necessary (s_message);

  scm_throw (guile_read_only_error_key,
	     scm_list_4 (s_function_name,
			 s_message,
			 scm_list_1 (s_function_name),
			 SCM_BOOL_F));
}

void
guile_wrong_number_of_arguments_error (const char *function_name)
{
  SCM s_function_name = scm_from_locale_string (function_name);
  SCM s_message = scm_from_locale_string ("wrong number of arguments");

  s_message = guile_append_newline_if_necessary (s_message);

  scm_throw (guile_wrong_number_of_arguments_error_key,
	     scm_list_4 (s_function_name,
			 s_message,
			 SCM_EOL,
			 SCM_BOOL_F));
}


void
guile_wrong_type_argument_error (const char *function_name,
				 int position, SCM variable, const char *type)
{
  SCM s_function_name = scm_from_locale_string (function_name);
  SCM s_message = scm_from_locale_string
    ("wrong type argument in position ~A (expecting ~A): ~S");
  SCM s_position = scm_from_int (position);
  SCM s_type = scm_from_locale_string (type);

  s_message = guile_append_newline_if_necessary (s_message);

  scm_throw (guile_wrong_type_argument_error_key,
	     scm_list_4 (s_function_name,
			 s_message,
			 scm_list_3 (s_position,
				     s_type,
				     variable),
			 SCM_BOOL_F));
}



/* Catching errors:
 * 1) print to stderr and abort. This is if we are very early in the startup.
 * 2) print the error to the minibuffer and beep.
 */


/* A simple error handler that returns false when an error occurs.  */
static SCM
_guile_false_error_handler (void *data, SCM key, SCM exception)
{
  return SCM_BOOL_F;
}

/* A basic error handler that prints an error message to the current
   error port.  DATA holds the filename as a C string.  */
static SCM
_guile_default_error_handler (void *data, SCM key, SCM exception)
{
  const char *filename = data;
  char *c_key;
  SCM subr, message, args, rest;
  SCM message_args, formatted_message;
  char *c_message;

  /* Key is the exception type, a symbol. */
  /* exception is a list of 4 elements:
     - subr: a subroutine name (symbol?) or #f
     - message: a format string
     - args: a list of arguments that are tokens for the message
     - rest: the errno, if any */
  if (scm_is_true (key))
    c_key = scm_to_locale_string (scm_symbol_to_string (key));
  else
    c_key = NULL;

  subr = scm_list_ref (exception, scm_from_int (0));
  message = scm_list_ref (exception, scm_from_int (1));
  args = scm_list_ref (exception, scm_from_int (2));
  rest = scm_list_ref (exception, scm_from_int (3));

  message_args = scm_simple_format (SCM_BOOL_F, message, args);

  if (key == guile_quit_error_key)
    formatted_message = scm_from_locale_string ("Quit\n");
  else
    {
      if (scm_is_true (subr))
	formatted_message = scm_simple_format (SCM_BOOL_F,
					       scm_from_locale_string ("Error ~S: ~A~%"),
					       scm_list_2 (subr, message_args));
      else
	formatted_message = scm_simple_format (SCM_BOOL_F,
					       scm_from_locale_string ("Error: ~A~%"),
					       scm_list_1 (message_args));
    }

  /* BEGIN DEBUG BLOCK */
  c_message = scm_to_locale_string (formatted_message);
  free (c_message);
  /* END DEBUG BLOCK */

  scm_simple_format (scm_current_error_port (), formatted_message, SCM_EOL);
  return SCM_UNSPECIFIED;
}

SCM
guile_error_handler (void *data, SCM key, SCM exception)
{
  const char *filename = data;
  char *c_key;
  SCM subr, message, args, rest;
  SCM message_args, formatted_message;
  char *c_message;

  /* Key is the exception type, a symbol. */
  /* exception is a list of 4 elements:
     - subr: a subroutine name (symbol?) or #f
     - message: a format string
     - args: a list of arguments that are tokens for the message
     - rest: the errno, if any */
  if (scm_is_true (key))
    c_key = scm_to_locale_string (scm_symbol_to_string (key));
  else
    c_key = NULL;

  subr = scm_list_ref (exception, scm_from_int (0));
  message = scm_list_ref (exception, scm_from_int (1));
  args = scm_list_ref (exception, scm_from_int (2));
  rest = scm_list_ref (exception, scm_from_int (3));

  message_args = scm_simple_format (SCM_BOOL_F, message, args);

  if (scm_is_true (subr))
    formatted_message = scm_simple_format (SCM_BOOL_F,
					   scm_from_locale_string ("Guile error in ~S, ~A"),
					   scm_list_2 (subr, message_args));
  else
    formatted_message = scm_simple_format (SCM_BOOL_F,
					   scm_from_locale_string ("Guile error: ~A"),
					   scm_list_1 (message_args));
  c_message = scm_to_locale_string (formatted_message);
  minibuf_error (c_message);
  free (c_message);
  return SCM_UNSPECIFIED;
}


void
set_guile_error_port_to_minibuffer (void)
{
  scm_set_current_error_port (minibuf_port);
}

void
set_guile_error_port_to_stderr (void)
{
  scm_set_current_error_port (stderr_port);
}


/****************************************************************
 * Calling Functions
 *
 * This is the framework for calling a Guile function by name,
 * properly handling uniargs and errors.
 ****************************************************************/

/* This alist contains the uniarg behavior of a function.  The
 * function symbol is the alist key.  The alist value is either
 * 'integer or 'bool. A function in this list will receive a uniarg
 * when called, if one is available. */
SCM_SYMBOL (guile_procedure_uniarg_alist, "%procedure-uniarg-alist");

/* Identifies FUNC as a Zile procedure that can take an optional
   integer argument. */
void
guile_procedure_set_uniarg_integer (const char *func)
{
  SCM sym;
  assert (func != NULL && strlen (func) > 0 && strlen (func) < NAME_LEN_MAX);
  sym = scm_string_to_symbol (scm_from_locale_string (func));

  guile_procedure_uniarg_alist = scm_assq_set_x (guile_procedure_uniarg_alist,
						 sym,
						 scm_from_locale_symbol ("integer"));
}

/* Identifies FUNC as a Zile procedure that can take an optional
   boolean argument. */
void
guile_procedure_set_uniarg_boolean (const char *func)
{
  SCM sym;
  assert (func != NULL && strlen (func) > 0 && strlen (func) < NAME_LEN_MAX);
  sym = scm_string_to_symbol (scm_from_locale_string (func));

  guile_procedure_uniarg_alist = scm_assq_set_x (guile_procedure_uniarg_alist,
						 sym,
						 scm_from_locale_symbol ("bool"));
}

/* Identifies FUNC as a Zile procedure that takes no arguments. */
void
guile_procedure_set_uniarg_none (const char *func)
{
  SCM sym;
  assert (func != NULL && strlen (func) > 0 && strlen (func) < NAME_LEN_MAX);
  sym = scm_string_to_symbol (scm_from_locale_string (func));

  guile_procedure_uniarg_alist = scm_assq_remove_x (guile_procedure_uniarg_alist,
						    sym);
}

static SCM
uniarg_alist_ref (SCM proc)
{
  SCM sym = guile_procedure_name_safe (proc);
  if (!scm_is_true (sym))
    return SCM_BOOL_F;

  return scm_assq_ref (guile_procedure_uniarg_alist, sym);
}

bool
guile_procedure_takes_uniarg_integer (SCM proc)
{
  SCM val = uniarg_alist_ref (proc);
  return scm_is_true (scm_eq_p (val, scm_from_locale_symbol ("integer")));
}

bool
guile_procedure_takes_uniarg_boolean (SCM proc)
{
  SCM val = uniarg_alist_ref (proc);
  return scm_is_true (scm_eq_p (val, scm_from_locale_symbol ("boolean")));
}

bool
guile_procedure_takes_uniarg_none (SCM proc)
{
  SCM val = uniarg_alist_ref (proc);
  return scm_is_true (scm_eq_p (val, SCM_BOOL_F));
}


/* Calls FUNC with zero arguments, catching errors. */

static SCM
_guile_call_procedure_body (void *data)
{
  SCM var = SCM_PACK ((scm_t_bits) data);
  return scm_call_0 (var);
}

SCM
guile_call_procedure (SCM proc)
{
  return scm_c_catch (SCM_BOOL_T,
		      _guile_call_procedure_body, (void *) SCM_UNPACK (proc),
		      _guile_default_error_handler, NULL,
		      NULL, NULL);
}

static SCM
_guile_call_procedure_with_long_body (void *data)
{
  struct name_value *nv = (struct name_value *) data;
  return scm_call_1 (nv->name, nv->value);
}


/* FUNC is a symbol that is the name of a procedure.  */
SCM
guile_call_procedure_with_long (SCM proc, long uniarg)
{
  struct name_value nv;
  nv.name = proc;
  nv.value = scm_from_long (uniarg);

  return scm_c_catch (SCM_BOOL_T,
		      _guile_call_procedure_with_long_body, &nv,
		      _guile_default_error_handler, NULL,
		      NULL, NULL);
}

static SCM
_guile_call_with_boolean_body (void *data)
{
  struct name_value *nv = (struct name_value *) data;
  return scm_call_1 (nv->name, nv->value);
}

/* FUNC is a symbol that is the name of a procedure.  */
SCM
guile_call_procedure_with_boolean (SCM proc, bool uniarg)
{
  struct name_value nv;
  nv.name = proc;
  nv.value = scm_from_bool (uniarg);

  return scm_c_catch (SCM_BOOL_T,
		      _guile_call_with_boolean_body, &nv,
		      _guile_default_error_handler, NULL,
		      NULL, NULL);
}

/****************************************************************
 * Loading Functions
 *
 * This is the framework for reading a Guile script into the
 * current environment, handling errors.
 ****************************************************************/

/* GUILE LOAD
 *
 * On startup, the .zile file is loaded, along with any file
 * passed with --load on the command line.  To do what EMACS
 * does, Guile errors should show up in the minibuffer,
 * and, on errors, the program should start anyway.
 */

static SCM
_guile_load_body (void *data)
{
  const char *filename = data;
  return scm_c_primitive_load (data);
}


void
guile_load (const char *filename)
{
  scm_c_catch (SCM_BOOL_T,
	       _guile_load_body, (void *) filename,
	       _guile_default_error_handler, (void *) filename,
	       NULL, NULL);
}


/****************************************************************
 * Minibuffer Port
 *
 * This is a scheme port that writes into the Zile
 * minibuffer at the bottom of the screen.
 ****************************************************************/

/* GUILE ERROR PORT
 *
 * Some Guile warnings can't be caught and are printed to
 * the Guile's current-error-port.  To handle this, we
 * make a custom error port that would print errors to
 * the minibuffer. */

/* The port receives data using this function. */
static size_t
guile_error_port_write (SCM port, SCM src, size_t start, size_t size)
{
  char *buf = scm_to_locale_string (src);
  char *cr;

  buf[size] = '\0';
  if (strlen (minibuf_buffer) + size > minibuf_buffer_size - 1)
    {
      while ((cr = strchr (minibuf_buffer, '\n')) != NULL)
	cr[0] = ' ';
      minibuf_error (minibuf_buffer);
      memset (minibuf_buffer, 0, 80);
    }

  strncat (minibuf_buffer, buf, minibuf_buffer_size - 1);

  //if (strchr (minibuf_buffer, '\n'))
  //  {
  while ((cr = strchr (minibuf_buffer, '\n')) != NULL)
    cr[0] = ' ';
  minibuf_error (minibuf_buffer);
  memset (minibuf_buffer, 0, 80);
  //  }

  free (buf);
  return size;
}

/* This function would be used to fill the read-out buffer of the port
   before getting a character from the port, but we don't need this
   since this port will be write-only. */
static size_t
guile_error_port_fill_input (SCM port, SCM dst, size_t start, size_t count)
{
  return 0;
}

/****************************************************************
 * Function definition helper functions.
 *
 * Used when writing Guile functions.
 ****************************************************************/

bool
guile_symbol_is_name_of_defined_function (SCM sym)
{
  if (!scm_is_symbol (sym))
    return 0;

  SCM var = guile_lookup_safe (sym);
  if (!scm_is_true (var) || !scm_is_true (scm_variable_p (var)))
    return 0;

  SCM ref = guile_variable_ref_safe (var);
  if (!scm_is_true (ref) || !scm_is_true (scm_procedure_p (ref)))
    return 0;

  return 1;
}

long
guile_to_long_or_error (const char *function_name, int position, SCM n)
{
  if (guile_is_nil_or_undefined (n))
    return 1;
  else if (!scm_is_integer (n) || !scm_is_true (scm_exact_p (n)))
    guile_wrong_type_argument_error (function_name, SCM_ARG1, n,
				     "exact integer");
  else if (!scm_is_signed_integer (n, LONG_MIN, LONG_MAX))
    guile_overflow_error (function_name, SCM_ARG1, n);

  return scm_to_long (n);
}

/****************************************************************/

void
init_guile_guile_procedures (void)
{
  stderr_port = scm_current_error_port ();
  minibuf_port_type = scm_make_port_type ("minibuf-port",
					  guile_error_port_fill_input,
					  guile_error_port_write);
  minibuf_port = scm_c_make_port (minibuf_port_type, SCM_OPN | SCM_WRTNG,
				  0);
  stderr_port = scm_current_error_port ();
  scm_set_current_error_port (minibuf_port);
  #include "guile.x"
}




/* Return the arity of a procedure: the number of its required and
   optional arguments.  */
bool
guile_get_procedure_arity (SCM proc, int *required, int *optional)
{
  /* FIXME: error checking */
  SCM arity = scm_procedure_property (proc, scm_from_locale_symbol ("arity"));
  *required = scm_to_int (scm_car (arity));
  *optional = scm_to_int (scm_cadr (arity));
  return 1;
}

#if 0
const char *
guile_get_procedure_documentation (SCM proc)
{
  SCM doc = scm_procedure_documentation (proc);
  if (scm_is_true (doc))
    return scm_i_string_chars (doc);
  else
    return NULL;
}

/* Set a Guile variable *uniarg* to either an integer or #f.

   If OK is false, *uniarg* will be set to #f.
   If OK is true, *uniarg* will be set to the integer N.
*/
void
guile_set_uniarg (long n, bool ok)
{
  scm_dynwind_begin (0);
  scm_dynwind_unwind_handler (guile_error_handler, NULL, 0);

  if (ok)
    scm_c_define ("*uniarg*", scm_from_long (n));
  else
    scm_c_define ("*uniarg*", SCM_BOOL_F);

  scm_dynwind_end ();
}

/* Lookup the Guile *uniarg* variable and get its value.

   Return true if the variable exists and has a valid value.
   Return false otherwise.
   Valid values for uniarg are exact long integers or #f.
   If uniarg is an integer, 'n' will be set and 'ok' will be true.
   If uniarg is #f, 'n' will be 1 and 'ok' will be false.
   If uniarg has an invalid value, 'n' will be 1 and 'ok' will be false.
 */
bool
guile_get_uniarg (long *n, bool *ok)
{
  bool found;
  SCM val;

  scm_dynwind_begin (0);
  scm_dynwind_unwind_handler (guile_error_handler, NULL, 0);

  found = guile_c_lookup_ref ("*uniarg*", &val);
  if (found && scm_is_integer (val))
    {
      *n = scm_to_long (val);
      *ok = true;
    }
  else
    {
      *n = 1;
      *ok = false;
    }

  scm_dynwind_end ();

  return found;
}

SCM
gfuncall (SCM (*f)())
{
  SCM ret;
  scm_dynwind_begin (0);
  scm_dynwind_unwind_handler (guile_error_handler, NULL, 0);
  guile_set_uniarg (1, false);
  ret = f();
  scm_dynwind_end ();
  return ret;
}

SCM
guile_call_by_name_0 (const char *f)
{
  SCM func, ok;
  scm_dynwind_begin (0);
  scm_dynwind_unwind_handler (guile_error_handler, NULL, 0);
  if (guile_c_lookup_ref (f, &func))
    ok = scm_call_0 (func);
  else
    ok = SCM_BOOL_F;
  scm_dynwind_end ();
  return ok;
}

SCM
gfuncall_arg (SCM (*f)(SCM x), long uniarg)
{
  guile_set_uniarg (uniarg, true);
  return f(SCM_UNDEFINED);
}

#endif
