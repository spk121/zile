/* Zile variables handling functions

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2003, 2004, 2005, 2006, 2007 Reuben Thomas.

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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "gl_list.h"

#include "zile.h"
#include "extern.h"

/*
 * Variable type.
 */
struct var_entry
{
  const char *var;		/* Variable name. */
  const char *defval;		/* Default value. */
  const char *val;		/* Current value, if any. */
  bool local;			/* If true, becomes local when set. */
  const char *doc;              /* Documentation */
};
typedef struct var_entry var_entry;

static Hash_table *main_vars;

static size_t
var_hash (const void *v, size_t n)
{
  return hash_string (((var_entry *) v)->var, n);
}

static bool
var_cmp (const void *v, const void *w)
{
  return strcmp (((var_entry *) v)->var, ((var_entry *) w)->var) == 0;
}

static void
var_free (void *v)
{
  var_entry *p = (var_entry *) v;
  free ((char *) p->var);
  if (p->defval != p->val)
    free ((char *) p->defval);
  free ((char *) p->val);
  free (p);
}

static void
init_builtin_var (const char *var, const char *defval, bool local, const char *doc)
{
  var_entry *p = XZALLOC (var_entry);
  p->var = xstrdup (var);
  p->defval = p->val = xstrdup (defval);
  p->local = local;
  p->doc = doc;
  hash_insert (main_vars, p);
}

static Hash_table *
new_varlist (void)
{
  /* Initial size of 32 is big enough for default variables and some
     more */
  return hash_initialize (32, NULL, var_hash, var_cmp, var_free);
}

void
init_variables (void)
{
  main_vars = new_varlist ();
#define X(var, defval, local, doc)              \
  init_builtin_var (var, defval, local, doc);
#include "tbl_vars.h"
#undef X
}

static void
set_variable_in_list (Hash_table *var_list, const char *var, const char *val)
{
  var_entry *p = XZALLOC (var_entry), *q;

  /* Insert variable if it doesn't already exist */
  p->var = xstrdup (var);
  q = hash_insert (var_list, p);

  /* Update value */
  q->val = xstrdup (val);

  /* If variable is new, initialise other fields */
  if (q == p)
    {
      p->defval = xstrdup (val);
      p->local = false;
      p->doc = "";
    }
  else
    var_free (p);
}

void
set_variable (const char *var, const char *val)
{
  struct var_entry *ent, *key = XZALLOC (var_entry);
  key->var = var;
  ent = hash_lookup (main_vars, key);
  free (key);
  if (ent && ent->local && cur_bp->vars == NULL)
    cur_bp->vars = new_varlist ();
  set_variable_in_list ((ent && ent->local) ? cur_bp->vars : main_vars, var, val);
}

void
free_variables (void)
{
  hash_free (main_vars);
}

static var_entry *
get_variable_entry (Buffer * bp, char *var)
{
  var_entry *p = NULL, *key = XZALLOC (var_entry);

  key->var = var;

  if (bp && bp->vars)
    p = hash_lookup (bp->vars, key);

  if (p == NULL)
    p = hash_lookup (main_vars, key);

  free (key);

  return p;
}

const char *
get_variable_doc (Buffer * bp, char *var, char **defval)
{
  var_entry *p = get_variable_entry (bp, var);
  if (p != NULL)
    {
      *defval = (char *) p->defval;
      return p->doc;
    }
  return NULL;
}

const char *
get_variable_bp (Buffer * bp, char *var)
{
  var_entry *p = get_variable_entry (bp, var);
  return p ? p->val : NULL;
}

const char *
get_variable (char *var)
{
  return get_variable_bp (cur_bp, var);
}

int
get_variable_number_bp (Buffer * bp, char *var)
{
  int t = 0;
  const char *s = get_variable_bp (bp, var);

  if (s)
    t = atoi (s);

  return t;
}

int
get_variable_number (char *var)
{
  return get_variable_number_bp (cur_bp, var);
}

bool
get_variable_bool (char *var)
{
  const char *p = get_variable (var);
  if (p != NULL)
    return strcmp (p, "nil") != 0;

  return false;
}

char *
minibuf_read_variable_name (char *msg)
{
  char *ms;
  Completion *cp = completion_new (false);
  var_entry *p;

  for (p = hash_get_first (main_vars);
       p != NULL;
       p = hash_get_next (main_vars, p))
    {
      gl_sortedlist_add (cp->completions, completion_strcmp,
                         xstrdup (p->var));
    }

  ms = vminibuf_read_completion (msg, "", cp, NULL,
                            "No variable name given",
                            minibuf_test_in_completions,
                            "Undefined variable name `%s'", NULL);

  return ms;
}

DEFUN ("set-variable", set_variable)
/*+
Set a variable value to the user-specified value.
+*/
{
  char *var, *val;
  size_t argc = countNodes (arglist);

  if (arglist && argc >= 3)
    set_variable (arglist->next->data, arglist->next->next->data);
  else
    {
      var = minibuf_read_variable_name ("Set variable: ");
      if (var == NULL)
        return leNIL;

      val = minibuf_read ("Set %s to value: ", "", var);
      if (val == NULL)
        {
          free (var);
          return FUNCALL (keyboard_quit);
        }

      set_variable (var, val);
      free (var);
      free (val);
    }

  return leT;
}
END_DEFUN
