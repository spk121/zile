/* Variables handling functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
   All rights reserved.

   This file is part of Zile.

   Zile is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Zile is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with Zile; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id: variables.c,v 1.25 2005/01/22 11:27:17 rrt Exp $	*/

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"
#include "eval.h"
#include "vars.h"

/*
 * Default variables values table.
 */
static struct var_entry {
  char *var;	/* Variable name. */
  char *fmt;	/* Variable format (boolean, etc.). */
  char *val;	/* Default value. */
} def_vars[] = {
#define X(zile_var, fmt, val, doc) { zile_var, fmt, val },
#include "tbl_vars.h"
#undef X
};

void init_variables(void)
{
  struct var_entry *p;

  for (p = &def_vars[0]; p < &def_vars[sizeof(def_vars) / sizeof(def_vars[0])]; p++)
    set_variable(p->var, p->val);
}

void free_variables(void)
{
  variableFree(mainVarList);
}

void set_variable(char *var, char *val)
{
  mainVarList = variableSetString(mainVarList, var, val);
}

char *get_variable(char *var)
{
  return variableGetString(mainVarList, var);
}

int is_variable_equal(char *var, char *val)
{
  char *v = get_variable(var);
  return v != NULL && !strcmp(v, val);
}

int lookup_bool_variable(char *var)
{
  char *p;

  if ((p = get_variable(var)) != NULL)
    return !strcmp(p, "true");

#if DEBUG
  minibuf_error("Warning: used uninitialized variable `%s'", var);
  waitkey(WAITKEY_DEFAULT);
#endif

  return FALSE;
}

char *minibuf_read_variable_name(char *msg)
{
  char *ms;
  Completion *cp = completion_new(FALSE);
  le *lp;
  
  for (lp = mainVarList; lp != NULL; lp = lp->list_next)
    list_append(cp->completions, zstrdup(lp->data));

  for (;;) {
    ms = minibuf_read_completion(msg, "", cp, NULL);

    if (ms == NULL) {
      free_completion(cp);
      cancel();
      return NULL;
    }

    if (ms[0] == '\0') {
      free_completion(cp);
      minibuf_error("No variable name given");
      return NULL;
    } else if (get_variable(ms) == NULL) {
      minibuf_error("Undefined variable name `%s'", ms);
      waitkey(WAITKEY_DEFAULT);
    } else {
      minibuf_clear();
      break;
    }
  }

  free_completion(cp);

  return ms;
}

static char *get_variable_format(char *var)
{
  struct var_entry *p;
  for (p = &def_vars[0]; p < &def_vars[sizeof(def_vars) / sizeof(def_vars[0])]; p++)
    if (!strcmp(p->var, var))
      return p->fmt;

  return "";
}

DEFUN("set-variable", set_variable)
  /*+
    Set a variable value to the user specified value.
    +*/
{
  char *var, *val, *fmt;
  astr as = astr_new();

  var = minibuf_read_variable_name("Set variable: ");
  if (var == NULL)
    return FALSE;

  /* `tab-width' and `fill-column' are local to buffers. */
  if (!strcmp(var, "tab-width"))
    astr_afmt(as, "%d", cur_bp->tab_width);
  else if (!strcmp(var, "fill-column"))
    astr_afmt(as, "%d", cur_bp->fill_column);
  else
    astr_cpy_cstr(as, get_variable(var));
  fmt = get_variable_format(var);
  if (!strcmp(fmt, "b")) {
    int i;
    if ((i = minibuf_read_boolean("Set %s to value: ", var)) == -1)
      return cancel();
    val = (i == TRUE) ? "true" : "false";
  } else { /* Non boolean or such fixed value variable. */
    if ((val = minibuf_read("Set %s to value: ", astr_cstr(as), var)) == NULL)
      return cancel();
  }

  astr_delete(as);

  /*
   * The `tab-width' and `fill-column' variables are local
   * to the buffer (automatically becomes buffer-local when
   * set in any fashion).
   */
  if (!strcmp(var, "tab-width")) {
    int i = atoi(val);
    if (i < 1) {
      minibuf_error("Invalid tab-width value `%s'", val);
      waitkey(WAITKEY_DEFAULT);
    } else
      cur_bp->tab_width = i;

  } else if (!strcmp(var, "fill-column")) {
    int i = atoi(val);
    if (i < 2) {
      minibuf_error("Invalid fill-column value `%s'", val);
      waitkey(WAITKEY_DEFAULT);
    } else
      cur_bp->fill_column = i;
  } else
    set_variable(var, val);

  return TRUE;
}
