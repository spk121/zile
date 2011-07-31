/* Zile variables handling functions

   Copyright (c) 1997-2011 Free Software Foundation, Inc.

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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "hash.h"

#include "main.h"
#include "extern.h"

/*
 * Variable type.
 */
struct var_entry
{
  char *var;			/* Variable name. */
  char *defval;			/* Default value. */
  char *val;			/* Current value, if any. */
  bool local;			/* If true, becomes local when set. */
  const char *doc;              /* Documentation */
};
typedef struct var_entry var_entry;

static Hash_table *main_vars;

static size_t
var_hash (const void *v, size_t n)
{
  return hash_string (((const var_entry *) v)->var, n);
}

static bool
var_cmp (const void *v, const void *w)
{
  return STREQ (((const var_entry *) v)->var, ((const var_entry *) w)->var);
}

static void
init_builtin_var (const char *var, const char *defval, bool local, const char *doc)
{
  var_entry *p = XZALLOC (var_entry);
  p->var = xstrdup (var);
  p->defval = xstrdup (defval);
  p->val = xstrdup (defval);
  p->local = local;
  p->doc = doc;
  assert (hash_insert (main_vars, p));
}

static Hash_table *
new_varlist (void)
{
  /* Initial size is big enough for default variables and some more */
  return hash_initialize (32, NULL, var_hash, var_cmp, NULL);
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

void
set_variable (const char *var, const char *val)
{
  Hash_table *var_list;
  struct var_entry *ent, *key = XZALLOC (var_entry);
  var_entry *p = XZALLOC (var_entry), *q;

  /* Find whether variable is buffer-local when set, and if needed
     create a buffer-local variable list. */
  key->var = xstrdup (var);
  ent = hash_lookup (main_vars, key);
  if (ent && ent->local && get_buffer_vars (cur_bp) == NULL)
    set_buffer_vars (cur_bp, new_varlist ());
  var_list = (ent && ent->local) ? get_buffer_vars (cur_bp) : main_vars;

  /* Insert variable if it doesn't already exist. */
  p->var = xstrdup (var);
  q = hash_insert (var_list, p);

  /* Update value */
  q->val = xstrdup (val);

  /* If variable is new, initialise other fields. */
  if (q == p)
    {
      if (var_list == main_vars)
        {
          p->defval = xstrdup (val);
          p->local = false;
          p->doc = "";
        }
    }
}

static var_entry *
get_variable_entry (Buffer * bp, const char *var)
{
  var_entry *p = NULL, *key = XZALLOC (var_entry);

  key->var = xstrdup (var);

  if (bp && get_buffer_vars (bp))
    p = hash_lookup (get_buffer_vars (bp), key);

  if (p == NULL)
    p = hash_lookup (main_vars, key);

  return p;
}

const char *
get_variable_doc (const char *var, const char **defval)
{
  var_entry *p = get_variable_entry (NULL, var);
  if (p != NULL)
    {
      *defval = p->defval;
      return p->doc;
    }
  return NULL;
}

static const char *
get_variable_bp (Buffer * bp, const char *var)
{
  var_entry *p = get_variable_entry (bp, var);
  return p ? p->val : NULL;
}

const char *
get_variable (const char *var)
{
  return get_variable_bp (cur_bp, var);
}

long
get_variable_number_bp (Buffer * bp, const char *var)
{
  long t = 0;
  const char *s = get_variable_bp (bp, var);

  if (s)
    t = strtol (s, NULL, 10);
  /* FIXME: Check result and signal error. */

  return t;
}

long
get_variable_number (const char *var)
{
  return get_variable_number_bp (cur_bp, var);
}

bool
get_variable_bool (const char *var)
{
  const char *p = get_variable (var);
  if (p != NULL)
    return !STREQ (p, "nil");

  return false;
}

castr
minibuf_read_variable_name (const char *fmt, ...)
{
  Completion *cp = completion_new (false);

  for (var_entry *p = hash_get_first (main_vars);
       p != NULL;
       p = hash_get_next (main_vars, p))
    {
      gl_sortedlist_add (get_completion_completions (cp), completion_strcmp,
                         xstrdup (p->var));
    }

  va_list ap;
  va_start (ap, fmt);
  castr ms = minibuf_vread_completion (fmt, "", cp, NULL,
                                       "No variable name given",
                                       minibuf_test_in_completions,
                                       "Undefined variable name `%s'", ap);
  va_end (ap);

  return ms;
}

DEFUN_ARGS ("set-variable", set_variable,
            STR_ARG (var)
            STR_ARG (val))
/*+
Set a variable value to the user-specified value.
+*/
{
  STR_INIT (var)
  else
    var = minibuf_read_variable_name ("Set variable: ");
  if (var == NULL)
    return leNIL;
  STR_INIT (val)
  else
    val = minibuf_read ("Set %s to value: ", "", var);
  if (val == NULL)
    ok = FUNCALL (keyboard_quit);

  if (ok == leT)
    set_variable (astr_cstr (var), astr_cstr (val));
}
END_DEFUN
