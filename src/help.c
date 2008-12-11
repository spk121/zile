/* Self documentation facility functions

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2003, 2004 Reuben Thomas.

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

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gl_array_list.h"

#include "zile.h"
#include "extern.h"

DEFUN ("zile-version", zile_version)
/*+
Show the version of Zile that is running.
+*/
{
  minibuf_write (ZILE_VERSION_STRING " of " CONFIGURE_DATE " on " CONFIGURE_HOST);

  return leT;
}
END_DEFUN

static le *
show_file (char *filename)
{
  if (!exist_file (filename))
    {
      minibuf_error ("Unable to read file `%s'", filename);
      return leNIL;
    }

  find_file (filename);
  cur_bp->flags = BFLAG_READONLY | BFLAG_NOSAVE | BFLAG_NEEDNAME
    | BFLAG_NOUNDO;

  return leT;
}

DEFUN ("view-zile-FAQ", view_zile_FAQ)
/*+
Show the Zile Frequently Asked Questions (FAQ).
+*/
{
  return show_file (PATH_DATA "/FAQ");
}
END_DEFUN

static void
write_function_description (va_list ap)
{
  const char *name = va_arg (ap, const char *);
  const char *doc = va_arg (ap, const char *);

  bprintf ("Function: %s\n\n" "Documentation:\n%s", name, doc);
}

DEFUN ("describe-function", describe_function)
/*+
Display the full documentation of a function.
+*/
{
  const char *name, *doc;
  astr bufname;

  name = minibuf_read_function_name ("Describe function: ");
  if (name == NULL)
    return leNIL;

  doc = get_function_doc (name);
  if (doc == NULL)
    {
      free ((char *) name);
      return leNIL;
    }

  bufname = astr_new ();
  astr_afmt (bufname, "*Help: function `%s'*", name);
  write_temp_buffer (astr_cstr (bufname), true,
                     write_function_description, name, doc);
  astr_delete (bufname);

  free ((char *) name);
  return leT;
}
END_DEFUN

static void
write_variable_description (va_list ap)
{
  char *name = va_arg (ap, char *);
  char *defval = va_arg (ap, char *);
  char *curval = va_arg (ap, char *);
  char *doc = va_arg (ap, char *);
  bprintf ("Variable: %s\n\n"
           "Default value: %s\n"
           "Current value: %s\n\n"
           "Documentation:\n%s",
           name, defval, curval, doc);
}

DEFUN ("describe-variable", describe_variable)
/*+
Display the full documentation of a variable.
+*/
{
  char *name, *defval;
  const char *doc;
  astr bufname;

  name = minibuf_read_variable_name ("Describe variable: ");
  if (name == NULL)
    return leNIL;

  doc = get_variable_doc (cur_bp, name, &defval);
  if (doc == NULL)
    {
      free (name);
      return leNIL;
    }

  bufname = astr_new ();
  astr_afmt (bufname, "*Help: variable `%s'*", name);
  write_temp_buffer (astr_cstr (bufname), true,
                     write_variable_description,
                     name, defval, get_variable (name), doc);
  free ((char *) name);
  astr_delete (bufname);

  return leT;
}
END_DEFUN

DEFUN ("describe-key", describe_key)
/*+
Display documentation of the command invoked by a key sequence.
+*/
{
  const char *name, *doc;
  astr bufname, binding;
  gl_list_t keys;

  minibuf_write ("Describe key:");
  name = get_function_by_key_sequence (&keys);
  binding = keyvectostr (keys);
  gl_list_free (keys);

  if (name == NULL)
    {
      minibuf_error ("%s is undefined", astr_cstr (binding));
      astr_delete (binding);
      return leNIL;
    }

  minibuf_write ("%s runs the command `%s'", astr_cstr (binding), name);
  astr_delete (binding);

  doc = get_function_doc (name);
  if (doc == NULL)
    return leNIL;

  bufname = astr_new ();
  astr_afmt (bufname, "*Help: function `%s'*", name);
  write_temp_buffer (astr_cstr (bufname), true,
                     write_function_description, name, doc);
  astr_delete (bufname);

  return leT;
}
END_DEFUN
