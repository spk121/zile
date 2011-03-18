/* Self documentation facility functions

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.

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

DEFUN ("zile-version", zile_version)
/*+
Show the version of Zile that is running.
+*/
{
  minibuf_write (ZILE_VERSION_STRING " of " CONFIGURE_DATE " on " CONFIGURE_HOST);
}
END_DEFUN

static le *
show_file (const char *filename)
{
  if (!exist_file (filename))
    {
      minibuf_error ("Unable to read file `%s'", filename);
      return leNIL;
    }

  find_file (filename);
  set_buffer_readonly (cur_bp, true);
  set_buffer_noundo (cur_bp, true);
  set_buffer_needname (cur_bp, true);
  set_buffer_nosave (cur_bp, true);

  return leT;
}

DEFUN ("view-emacs-FAQ", view_emacs_FAQ)
/*+
Display the Zile Frequently Asked Questions (FAQ) file.
+*/
{
  ok = show_file (PATH_DATA "/FAQ");
}
END_DEFUN

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
  const char *doc;

  STR_INIT (func)
  else
    {
      func = xstrdup (minibuf_read_function_name ("Describe function: "));
      if (func == NULL)
        ok = leNIL;
    }

  if (ok == leT)
    {
      doc = get_function_doc (func);
      if (doc == NULL)
        ok = leNIL;
      else
        write_temp_buffer ("*Help*", true,
                           write_function_description, func, doc);
    }

  STR_FREE (func);
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
      const char *doc = get_variable_doc (name, &defval);
      if (doc == NULL)
        ok = leNIL;
      else
        write_temp_buffer ("*Help*", true,
                           write_variable_description,
                           name, get_variable (name), doc);
    }

  STR_FREE (name);
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
      gl_list_t keys = keystrtovec (keystr);

      if (keys != NULL)
        {
          name = get_function_name (get_function_by_keys (keys));
          binding = keyvectostr (keys);
          gl_list_free (keys);
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
      binding = keyvectostr (keys);
      gl_list_free (keys);

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

  if (binding)
    astr_delete (binding);

  STR_FREE (keystr);
}
END_DEFUN
