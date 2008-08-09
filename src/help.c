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

#include <assert.h>
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
Show the zile version.
+*/
{
  minibuf_write ("Zile " VERSION " of " CONFIGURE_DATE " on " CONFIGURE_HOST);

  return true;
}
END_DEFUN

static int
show_file (char *filename)
{
  if (!exist_file (filename))
    {
      minibuf_error ("Unable to read file `%s'", filename);
      return false;
    }

  find_file (filename);
  cur_bp->flags = BFLAG_READONLY | BFLAG_NOSAVE | BFLAG_NEEDNAME
    | BFLAG_NOUNDO;

  return true;
}

DEFUN ("help", help)
/*+
Show a help window.
+*/
{
  return show_file (PATH_DATA "/HELP");
}
END_DEFUN

DEFUN ("view-zile-FAQ", view_zile_FAQ)
/*+
Show the Zile Frequently Asked Questions (FAQ).
+*/
{
  return show_file (PATH_DATA "/FAQ");
}
END_DEFUN

DEFUN ("help-with-tutorial", help_with_tutorial)
/*+
Show a tutorial window.
+*/
{
  if (show_file (PATH_DATA "/TUTORIAL"))
    {
      astr buf = get_home_dir ();
      if (buf == NULL)
        buf = agetcwd ();
      cur_bp->flags = 0;
      astr_cat_cstr (buf, "/TUTORIAL");
      set_buffer_filename (cur_bp, astr_cstr (buf));
      astr_delete (buf);

      return true;
    }

  return false;
}
END_DEFUN

/*
 * Fetch the documentation of a function or variable from the
 * AUTODOC automatically generated file.
 */
static astr
get_funcvar_doc (const char *name, astr defval, int isfunc)
{
  astr buf, match, doc;
  int reading_doc = 0;
  FILE *fp = fopen (PATH_DATA "/AUTODOC", "r");

  if (fp == NULL)
    {
      minibuf_error ("Unable to read file `%s'", PATH_DATA "/AUTODOC");
      return NULL;
    }

  match = astr_new ();
  if (isfunc)
    astr_afmt (match, "\fF_%s", name);
  else
    astr_afmt (match, "\fV_%s", name);

  doc = astr_new ();
  while ((buf = astr_fgets (fp)) != NULL)
    {
      if (reading_doc)
	{
	  if (*astr_char (buf, 0) == '\f')
	    {
	      astr_delete (buf);
	      break;
	    }
	  if (isfunc || astr_len (defval) > 0)
	    {
	      astr_cat (doc, buf);
	      astr_cat_char (doc, '\n');
	    }
	  else
	    astr_cpy (defval, buf);
	}
      else if (!astr_cmp (buf, match))
	reading_doc = 1;
      astr_delete (buf);
    }

  fclose (fp);
  astr_delete (match);

  if (!reading_doc)
    {
      minibuf_error ("Cannot find documentation for `%s'", name);
      astr_delete (doc);
      return NULL;
    }

  return doc;
}

static void
write_function_description (va_list ap)
{
  const char *name = va_arg (ap, const char *);
  astr doc = va_arg (ap, astr);

  bprintf ("Function: %s\n\n" "Documentation:\n%s", name, astr_cstr (doc));
}

DEFUN ("describe-function", describe_function)
/*+
Display the full documentation of a function.
+*/
{
  const char *name;
  astr bufname, doc;

  name = minibuf_read_function_name ("Describe function: ");
  if (name == NULL)
    return false;

  doc = get_funcvar_doc (name, NULL, true);
  if (doc == NULL)
    return false;

  bufname = astr_new ();
  astr_afmt (bufname, "*Help: function `%s'*", name);
  write_temp_buffer (astr_cstr (bufname), write_function_description,
		     name, doc);
  free ((char *) name);
  astr_delete (bufname);
  astr_delete (doc);

  return true;
}
END_DEFUN

static void
write_variable_description (va_list ap)
{
  char *name = va_arg (ap, char *);
  astr defval = va_arg (ap, astr);
  char *curval = va_arg (ap, char *);
  astr doc = va_arg (ap, astr);
  bprintf ("Variable: %s\n\n"
	   "Default value: %s\n"
	   "Current value: %s\n\n"
	   "Documentation:\n%s",
	   name, astr_cstr (defval), curval, astr_cstr (doc));
}

DEFUN ("describe-variable", describe_variable)
/*+
Display the full documentation of a variable.
+*/
{
  char *name;
  astr bufname, defval, doc;

  name = minibuf_read_variable_name ("Describe variable: ");
  if (name == NULL)
    return false;

  defval = astr_new ();
  doc = get_funcvar_doc (name, defval, false);
  if (doc == NULL)
    {
      astr_delete (defval);
      return false;
    }

  bufname = astr_new ();
  astr_afmt (bufname, "*Help: variable `%s'*", name);
  write_temp_buffer (astr_cstr (bufname), write_variable_description,
		     name, defval, get_variable (name), doc);
  astr_delete (bufname);
  astr_delete (doc);
  astr_delete (defval);

  return true;
}
END_DEFUN

DEFUN ("describe-key", describe_key)
/*+
Display documentation of the command invoked by a key sequence.
+*/
{
  const char *name;
  astr bufname, doc, binding;
  gl_list_t keys;

  minibuf_write ("Describe key:");
  name = get_function_by_key_sequence (&keys);
  binding = keyvectostr (keys);
  gl_list_free (keys);

  if (name == NULL)
    {
      minibuf_error ("%s is undefined", astr_cstr (binding));
      astr_delete (binding);
      return false;
    }

  minibuf_write ("%s runs the command `%s'", astr_cstr (binding), name);
  astr_delete (binding);

  doc = get_funcvar_doc (name, NULL, true);
  if (doc == NULL)
    return false;

  bufname = astr_new ();
  astr_afmt (bufname, "*Help: function `%s'*", name);
  write_temp_buffer (astr_cstr (bufname), write_function_description,
		     name, doc);
  astr_delete (bufname);
  astr_delete (doc);

  return true;
}
END_DEFUN
