/* Self documentation facility functions

   Copyright (c) 1997-2004, 2008-2011 Free Software Foundation, Inc.
   Copyright (c) 2012 Michael L. Gran

   This file is part of Michael Gran's unofficial fork ok GNU Zile.

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
#include <limits.h>
#include <libguile.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "gl_array_list.h"

#include "main.h"
#include "extern.h"

static void
write_function_description (va_list ap)
{
  const char *name = va_arg (ap, const char *);
  const char *doc = va_arg (ap, const char *);
  int interactive = guile_get_procedure_interactive_by_name (name);

  bprintf ("%s is %s procedure.\n\n%s",
           name,
           interactive ? "an interactive" : "a",
           doc);
}


SCM_DEFINE (G_describe_function, "describe-function", 0, 1, 0,
	    (SCM gfunc), "\
Display the full documentation of a function.  The optional argument\n\
is a string containing the name of the function.")
{
  char *str = NULL;
  astr func = NULL;
  SCM ok = SCM_BOOL_T;

  // FIXME: interactive logic
  str = guile_to_locale_string_safe (gfunc);
  if (str != NULL)
    func = astr_new_cstr (str);
  if (func == NULL)
    {
      func = minibuf_read_function_name ("Describe function: ");
      if (func == NULL)
        ok = SCM_BOOL_F;
    }

  if (ok == SCM_BOOL_T)
    {
      const char *doc = guile_get_procedure_documentation_by_name (astr_cstr (func));
      if (doc == NULL)
        minibuf_error ("undocumented function: %s", astr_cstr (func));
      else
        write_temp_buffer ("*Help*", true,
                           write_function_description, astr_cstr (func), doc);
    }
  return ok;
}

static void
write_variable_description (va_list ap)
{
  char *name = va_arg (ap, char *);
  char *curval = va_arg (ap, char *);
  char *doc = va_arg (ap, char *);
  bprintf ("%s is a variable defined in `C source code'.\n\n"
           "Its value is %s\n\n%s",
           name, curval, doc);
}

SCM_DEFINE (G_describe_variable, "describe-variable", 0, 1, 0,
	    (SCM gname), "\
Display the full documentation of a variable.")
{
  // GUILE variables don't have documentation
  return SCM_BOOL_F;
}

static void
write_key_description (va_list ap)
{
  const char *name = va_arg (ap, const char *);
  const char *doc = va_arg (ap, const char *);
  const char *binding = va_arg (ap, const char *);
  int interactive = guile_get_procedure_interactive_by_name (name);

  assert (interactive != -1);

  bprintf ("%s runs the command %s, which is %s built-in\n"
           "function.\n\n%s",
           binding, name,
           interactive ? "an interactive" : "a",
           doc);
}

SCM_DEFINE (G_describe_key, "describe-key", 0, 1, 0,
	    (SCM gkeystr), "\
Display documentation of the command invoked by a key sequence.")
{
  SCM ok = SCM_BOOL_T;
  SCM gname;
  const char *name = NULL, *doc;
  astr binding = NULL;
  char *str = NULL;
  astr keystr = NULL;

  // FIXME: need interactive logic
  str = guile_to_locale_string_safe (gkeystr);
  if (str != NULL)
    keystr = astr_new_cstr (str);
  if (keystr != NULL)
    {
      gl_list_t keys = keystrtovec (astr_cstr (keystr));

      if (keys != NULL)
        {
          gname = guile_procedure_name_safe (get_function_by_keys (keys));
          binding = keyvectodesc (keys);
        }
      else
        ok = SCM_BOOL_F;
    }
  else
    {
      gl_list_t keys;

      minibuf_write ("Describe key:");
      keys = get_key_sequence ();
      gname = guile_procedure_name_safe (get_function_by_keys (keys));
      binding = keyvectodesc (keys);

      if (scm_is_true (gname))
        {
          minibuf_error ("%s is undefined", astr_cstr (binding));
          ok = SCM_BOOL_F;
        }
    }

  if (ok == SCM_BOOL_T)
    {
      minibuf_write ("%s runs the command `%s'", astr_cstr (binding), scm_to_locale_string (scm_symbol_to_string (gname)));

      doc = NULL;
      if (doc == NULL)
        ok = SCM_BOOL_F;
      else
        write_temp_buffer ("*Help*", true,
                           write_key_description, name, doc, astr_cstr (binding));
    }
  return ok;
}

void
init_guile_help_procedures (void)
{
#include "help.x"
  scm_c_export ("describe-function",
		"describe-variable",
		"describe-key",
		0);
}
