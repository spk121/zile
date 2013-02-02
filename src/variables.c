/* Zile variables handling functions

   Copyright (c) 1997-2011 Free Software Foundation, Inc.
   Copyright (c) 2011, 2012 Michael L Gran

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

#include <assert.h>
#include <libguile.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "hash.h"

#include "main.h"
#include "extern.h"

SCM_GLOBAL_VARIABLE_INIT (Gvar_inhibit_splash_screen, "inhibit-splash-screen",
			  SCM_BOOL_F);
SCM_GLOBAL_VARIABLE_INIT (Gvar_standard_indent, "standard-indent",
			  scm_from_long (4));
SCM_GLOBAL_VARIABLE_INIT (Gvar_tab_width, "tab-width",
			  scm_from_long (8));
SCM_GLOBAL_VARIABLE_INIT (Gvar_tab_always_indent, "tab-always-indent",
			  SCM_BOOL_T);
SCM_GLOBAL_VARIABLE_INIT (Gvar_indent_tabs_mode, "indent-tabs-mode",
			  SCM_BOOL_T);
SCM_GLOBAL_VARIABLE_INIT (Gvar_fill_column, "fill-column",
			  scm_from_long (70));
SCM_GLOBAL_VARIABLE_INIT (Gvar_auto_fill_mode, "%auto-fill-mode",
			  SCM_BOOL_F);
SCM_GLOBAL_VARIABLE_INIT (Gvar_kill_whole_line, "kill-whole-line",
			  SCM_BOOL_F);
SCM_GLOBAL_VARIABLE_INIT (Gvar_case_fold_search, "case-fold-search",
			  SCM_BOOL_T);
SCM_GLOBAL_VARIABLE_INIT (Gvar_case_replace, "case-replace",
			  SCM_BOOL_T);
SCM_GLOBAL_VARIABLE_INIT (Gvar_transient_mark_mode, "transient-mark-mode",
			  SCM_BOOL_T);
SCM_GLOBAL_VARIABLE_INIT (Gvar_highlight_nonselected_windows,
			  "highlight-nonselected-windows",
			  SCM_BOOL_F);
SCM_GLOBAL_VARIABLE_INIT (Gvar_make_backup_files, "make-backup-files",
			  SCM_BOOL_T);
SCM_GLOBAL_VARIABLE_INIT (Gvar_backup_directory, "backup-directory",
			  SCM_BOOL_F);
SCM_GLOBAL_VARIABLE_INIT (Gvar_t, "t", SCM_BOOL_T);
SCM_GLOBAL_VARIABLE (Gvar_setq, "setq");

void
init_variables (void)
{
}


/* Set variable to bool, number, or string based on */
void
set_variable (const char *var, const char *val)
{
  /* Decide if VAL is boolean, integer, or string. */
  if (strcmp (val, "#t") == 0)
    set_variable_bool (var, 1);
  else if (strcmp (val, "#f") == 0)
    set_variable_bool (var, 0);
  else
    {
      char *tail;
      long n;
      errno = 0;
      n = strtol (val, &tail, 0);
      if (errno == 0 && ((int) tail - (int) val == strlen (val) - 1))
        set_variable_number (var, n);
      else
        set_variable_string (var, val);
    }
}

void
set_variable_bool (const char *var, int val)
{
  guile_c_define_safe (var, scm_from_bool (val));
}

#if HAVE_SCM_ELISP_NIL == 1
void
set_variable_nil (const char *var)
{
  guile_c_define_safe (var, SCM_ELISP_NIL);
}
#endif

void
set_variable_string (const char *var, const char *str)
{
  guile_c_define_safe (var, guile_from_locale_string_safe (str));
}

void
set_variable_number (const char *var, long val)
{
  guile_c_define_safe (var, scm_from_long (val));
}

SCM
get_variable_entry (const char *var)
{
  SCM gvar;

  //if (bp)
    //gvar = scm_c_private_variable (get_buffer_module (bp), var);
  //else if (cur_bp)
    //gvar = scm_c_private_variable (get_buffer_module (cur_bp), var);
  //else
    //gvar = scm_c_private_variable ("zile", var);

  gvar = guile_lookup_safe (scm_from_locale_symbol (var));

  return gvar;
}

/* Since the return value is #f when var is false, and is
   also #f, when var is not found, OK can be used
   to clarify.  */
bool
get_variable_bool (const char *var)
{
  SCM gvar, gval;
  gvar = get_variable_entry (var);
  if (!scm_is_true (gvar))
    {
      return SCM_BOOL_F;
    }
  gval = guile_variable_ref_safe (gvar);
  return scm_is_true (gval);
}

long
get_variable_number (const char *var)
{
  SCM gvar, gval;
  gvar = get_variable_entry (var);
  if (!scm_is_true (gvar))
    {
      return 1;
    }
  gval = guile_variable_ref_safe (gvar);
  if (!scm_is_integer (gval) || !scm_is_true (scm_exact_p (gval)))
    {
      return 1;
    }
  return scm_to_long (gval);
}

char *
get_variable_string (const char *var)
{
  SCM gvar, gval;
  gvar = get_variable_entry (var);
  if (!scm_is_true (gvar))
    {
      return NULL;
    }
  gval = guile_variable_ref_safe (gvar);
  if (!scm_is_string (gval))
    {
      return NULL;
    }
  return guile_to_locale_string_safe (gval);
}

#if 0
char *
get_variable_bp (Buffer * bp, const char *var)
{
  SCM gvar = get_variable_entry_bp (bp, var);
  SCM val;
  if (scm_is_true (gvar))
    {
      val = scm_variable_ref (gvar);
      if (!scm_is_string (val))
        scm_misc_error ("zile","'~s' is not an string", scm_list_1 (gvar));
      return scm_to_locale_string (val);
    }
  scm_misc_error ("zile", "'~a' not found", scm_list_1 (gvar));

  /* unreachable */
  return NULL;
}

char *
get_variable (const char *var)
{
  return get_variable_bp (cur_bp, var);
}

long
get_variable_number_bp (Buffer *bp, const char *var)
{
  SCM gvar = get_variable_entry_bp (bp, var);
  SCM val;
  if (scm_is_true (gvar))
    {
      val = scm_variable_ref (gvar);
      if (!scm_is_integer (val) || !scm_is_true (scm_exact_p (val)))
        scm_misc_error ("zile","'~s' is not an integer", scm_list_1 (gvar));
      return scm_to_long (val);
    }
  scm_misc_error ("zile", "'~a' not found", scm_list_1 (gvar));

  /* unreachable */
  return LONG_MAX;
}

long
get_variable_number (const char *var)
{
  return get_variable_number_bp (cur_bp, var);
}

bool
get_variable_bool_bp (Buffer *bp, const char *var)
{
  SCM gvar = get_variable_entry (bp, var);
  SCM val;
  if (scm_is_true (gvar))
    {
      val = scm_variable_ref (gvar);
      return scm_to_bool (val);
    }
  scm_misc_error ("zile", "'~a' not found", scm_list_1 (gvar));

  /* unreachable */
  return 0;
}

bool
get_variable_bool (const char *var)
{
  return get_variable_bool_bp (cur_bp, var);
}
#endif


castr
minibuf_read_variable_name (const char *fmt, ...)
{
  #if 0
  Completion *cp = completion_new (false);
  SCM completion_list = scm_call_0 (var_completions_func);

  for (int i = 0; i < scm_to_int (scm_length (completion_list)); i++)
    {
      SCM entry = scm_list_ref (completion_list, scm_from_int (i));
      char *name = scm_to_locale_string (scm_symbol_to_string (entry));
      gl_sortedlist_add (get_completion_completions (cp), completion_strcmp,
                         xstrdup (name));
    }

  va_list ap;
  va_start (ap, fmt);
  castr ms = minibuf_vread_completion (fmt, "", cp, NULL,
                                       "No variable name given",
                                       minibuf_test_in_completions,
                                       "Undefined variable name `%s'", ap);
  va_end (ap);

  return ms;
  #endif
  return (castr) NULL;
}

SCM_DEFINE (G_set_variable, "set-variable", 2, 0, 0, 
	    (SCM gvar, SCM gval), "\
Set a variable value to the user-specified value.")
{
  castr val;
  SCM ok = SCM_BOOL_T;
  char *str = NULL;
  astr var = NULL;
  
  str = guile_to_locale_string_safe (gvar);
  if (str != NULL)
    var = astr_new_cstr (str);
  else
    var = minibuf_read_variable_name ("Set variable: ");
  
  if (var == NULL)
    return SCM_BOOL_F;

  if (SCM_UNBNDP (gval))
    {
      val = minibuf_read ("Set %s to value: ", "",  astr_cstr (var));
      if (val == NULL)
	ok = G_keyboard_quit ();
      else
	set_variable (astr_cstr (var), astr_cstr (val));
    }
  else
    guile_c_define_safe (astr_cstr (var), gval);

  return ok;
}

void
init_guile_variables_procedures (void)
{
#include "variables.x"
  scm_c_export ("set-variable",
		NULL);
  scm_c_define ("setq", scm_c_lookup ("define"));
}
