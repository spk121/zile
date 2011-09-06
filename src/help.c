/* Self documentation facility functions

   Copyright (c) 1997-2004, 2008-2011 Free Software Foundation, Inc.

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
#include <limits.h>
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
  int interactive = get_function_interactive (name);

  bprintf ("%s is %s built-in function in `C source code'.\n\n%s",
           name,
           interactive ? "an interactive" : "a",
           doc);
}

DEFUN_ARGS ("describe-function", describe_function,
            STR_ARG (func))
/*+
Display the full documentation of a function.
+*/
{
  STR_INIT (func)
  else
    {
      func = minibuf_read_function_name ("Describe function: ");
      if (func == NULL)
        ok = leNIL;
    }

  if (ok == leT)
    {
      const char *doc = get_function_doc (astr_cstr (func));
      if (doc == NULL)
        ok = leNIL;
      else
        write_temp_buffer ("*Help*", true,
                           write_function_description, astr_cstr (func), doc);
    }
}
END_DEFUN

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

DEFUN_ARGS ("describe-variable", describe_variable,
            STR_ARG (name))
/*+
Display the full documentation of a variable.
+*/
{
  STR_INIT (name)
  else
    name = minibuf_read_variable_name ("Describe variable: ");

  if (name == NULL)
    ok = leNIL;
  else
    {
      const char *defval;
      const char *doc = get_variable_doc (astr_cstr (name), &defval);
      if (doc == NULL)
        ok = leNIL;
      else
        write_temp_buffer ("*Help*", true,
                           write_variable_description,
                           astr_cstr (name), get_variable (astr_cstr (name)), doc);
    }
}
END_DEFUN

static void
write_key_description (va_list ap)
{
  const char *name = va_arg (ap, const char *);
  const char *doc = va_arg (ap, const char *);
  const char *binding = va_arg (ap, const char *);
  int interactive = get_function_interactive (name);

  assert (interactive != -1);

  bprintf ("%s runs the command %s, which is %s built-in\n"
           "function in `C source code'.\n\n%s",
           binding, name,
           interactive ? "an interactive" : "a",
           doc);
}

DEFUN_ARGS ("describe-key", describe_key,
            STR_ARG (keystr))
/*+
Display documentation of the command invoked by a key sequence.
+*/
{
  const char *name = NULL, *doc;
  astr binding = NULL;

  STR_INIT (keystr);
  if (keystr != NULL)
    {
      gl_list_t keys = keystrtovec (astr_cstr (keystr));

      if (keys != NULL)
        {
          name = get_function_name (get_function_by_keys (keys));
          binding = keyvectodesc (keys);
        }
      else
        ok = leNIL;
    }
  else
    {
      gl_list_t keys;

      minibuf_write ("Describe key:");
      keys = get_key_sequence ();
      name = get_function_name (get_function_by_keys (keys));
      binding = keyvectodesc (keys);

      if (name == NULL)
        {
          minibuf_error ("%s is undefined", astr_cstr (binding));
          ok = leNIL;
        }
    }

  if (ok == leT)
    {
      minibuf_write ("%s runs the command `%s'", astr_cstr (binding), name);

      doc = get_function_doc (name);
      if (doc == NULL)
        ok = leNIL;
      else
        write_temp_buffer ("*Help*", true,
                           write_key_description, name, doc, astr_cstr (binding));
    }
}
END_DEFUN
