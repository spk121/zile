/* Lisp parser

   Copyright (c) 2001, 2005, 2008-2011 Free Software Foundation, Inc.
   Copyright (c) 2011 Michael L Gran

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
#include <stdlib.h>

#include "main.h"
#include "extern.h"

static SCM
guile_use_module_body (void *data)
{
  const char *filename = data;
  scm_c_use_module (filename);
  return SCM_UNSPECIFIED;
}

SCM
guile_use_module_handler (void *data, SCM key, SCM exception)
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

  if (scm_to_int (scm_length (exception)) > 0)
    subr = scm_list_ref (exception, scm_from_int (0));
  else
    subr = SCM_BOOL_F;
  if (scm_to_int (scm_length (exception)) > 1)
    {
      message = scm_list_ref (exception, scm_from_int (1));
      if (scm_to_int (scm_length (exception)) > 2)
	args = scm_list_ref (exception, scm_from_int (2));
      else
	args = SCM_EOL;
    }
  else
    {
      message = scm_from_locale_string ("unknown error");
      args = SCM_EOL;
    }
  // rest = scm_list_ref (exception, scm_from_int (3));

  message_args = scm_simple_format (SCM_BOOL_F, message, args);

  if (scm_is_true (subr))
    formatted_message 
      = scm_simple_format (SCM_BOOL_F,
			   scm_from_locale_string ("Guile error in ~S, ~A~%"),
			   scm_list_2 (subr, message_args));
  else
    formatted_message 
      = scm_simple_format (SCM_BOOL_F,
			   scm_from_locale_string ("Guile error: ~A~%"),
			   scm_list_1 (message_args));
  /* One aggravating bug I made was when I put the load path
     to the srcdir and then scm_c_use_module tried to load
     the zile executable as a module.  When you do that, you 
     end up with "Unbound variable: \\x7ELF <binary junk>.
     The binary junk caused the following scm_to_locale_string to 
     fail because the string contains nulls. */
  //c_message = scm_to_locale_string (formatted_message);

  /* Send the message to the best handler. */
  scm_simple_format (scm_current_error_port (), formatted_message, SCM_EOL);
  //fprintf (stderr, "%s\n", c_message);
  //fflush (stderr);
  //free (c_message);
  return SCM_UNSPECIFIED;
}

void
init_lisp (void)
{
  /* Load Zile's Guile procedures into the top level environment.  */
  /* Init Guile first. */
  guile_c_resolve_module_safe ("zile");
  init_guile_guile_procedures ();


  init_guile_basic_procedures ();
  init_guile_bind_procedures ();
  init_guile_buffer_procedures ();
  init_guile_eval_procedures ();
  init_guile_file_procedures ();
  init_guile_funcs_procedures ();
  init_guile_help_procedures ();
  init_guile_keycode_procedures ();
  init_guile_killring_procedures ();
  init_guile_line_procedures ();
  init_guile_lisp_procedures ();
  init_guile_macro_procedures ();
  init_guile_redisplay_procedures ();
  init_guile_registers_procedures ();
  init_guile_search_procedures ();
  init_guile_undo_procedures ();
  init_guile_variables_procedures ();
  init_guile_window_procedures ();

  /* Now switch back to the REPL's module */
  guile_c_resolve_module_safe ("guile-user");
  /* And import the zile module */
  scm_c_catch (SCM_BOOL_T,
	       guile_use_zile_module, "zile module",
	       guile_error_handler, NULL,
	       NULL, NULL);
}



SCM_DEFINE (G_console, "console", 0, 0, 0, (void), "\
Suspend the editor and bring up the REPL for this buffer's module.")
{
  SCM var, func;

  /* Suspend the CRT screen handling.  */
  interactive = false;
  set_guile_error_port_to_stderr ();
  term_close ();
  /* Switch to the current buffer's module.  */
  printf ("\n\n");
  printf ("+-------------------------------------------------------+\n");
  printf ("| This is the Zile debug console for this buffer.       |\n");
  printf ("| \"(quit)\" to quit back to the editor                   |\n");
  printf ("| \"(zile-intro)\" for an introduction                    |\n");
  printf ("| \"(zile-procedures)\" to see Zile's functions           |\n");
  printf ("| \"(key-map)\" to see Zile's keymap                      |\n");
  printf ("+-------------------------------------------------------+\n");
  printf ("\n\n");
  
  fflush (stdout);
#if 0
  if (cur_bp)
    scm_set_current_module (scm_c_resolve_module (get_buffer_module (cur_bp)));
  else
    scm_set_current_module (scm_c_resolve_module ("zile"));
  // FIXME was system repl repl
  scm_call_1 (scm_c_module_lookup (scm_current_module (), "start-repl"),
              scm_from_locale_symbol ("scheme"));
#endif

  var = scm_c_lookup ("scm-style-repl");
  func = scm_variable_ref (var);
  scm_call_0 (func);

  printf ("Press <ENTER> key to re-enter zile...\n");
  set_guile_error_port_to_minibuffer ();
  term_getkey (-1);
  term_refresh ();
  interactive = true;
  return SCM_BOOL_T;
}

void
init_guile_lisp_procedures (void)
{
#include "lisp.x"
  scm_c_export ("console",
		0);
}
