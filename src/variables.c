/* Zile variables handling functions

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.

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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gl_list.h"

#include "main.h"
#include "extern.h"

static void
set_variable_in_list (const char *var, const char *val,
                      const char *defval, bool local, const char *doc)
{
  /* Variable list is on Lua stack. */
  lua_newtable (L);

  lua_pushstring (L, val);
  lua_setfield (L, -2, "val");

  if (defval != NULL)
    {
      lua_pushstring (L, defval);
      lua_setfield (L, -2, "defval");
    }

  lua_pushboolean (L, (int) local);
  lua_setfield (L, -2, "local");

  if (doc != NULL)
    {
      lua_pushstring (L, doc);
      lua_setfield (L, -2, "doc");
    }

  lua_setfield (L, -2, var);
}

void
set_variable (const char *var, const char *val)
{
  bool local = false;

  lua_getglobal (L, "main_vars");
  lua_getfield (L, -1, var);
  if (lua_istable (L, -1))
    {
      lua_getfield (L, -1, "local");
      local = (bool) lua_toboolean (L, -1);
      lua_pop (L, 1);
    }
  lua_pop (L, 1);
  if (local)
    {
      lua_pop (L, 1);
      if (get_buffer_vars (cur_bp) == 0)
        {
          lua_newtable (L);
          set_buffer_vars (cur_bp, luaL_ref (L, LUA_REGISTRYINDEX));
        }
      lua_rawgeti (L, LUA_REGISTRYINDEX, get_buffer_vars (cur_bp));
    }

  set_variable_in_list (var, val, NULL, true, NULL);
  lua_pop (L, 1);
}

void
init_variables (void)
{
  lua_newtable (L);
#define X(var, defval, local, doc)              \
  set_variable_in_list (var, defval, defval, local, doc);
#include "tbl_vars.h"
#undef X
  lua_setglobal (L, "main_vars");
}

void
free_variable_list (int ref)
{
  lua_unref (L, ref);
}

static bool
get_variable_entry (Buffer * bp, const char *var)
{
  bool found = false;

  if (bp && get_buffer_vars (bp))
    {
      lua_rawgeti (L, LUA_REGISTRYINDEX, get_buffer_vars (bp));
      lua_getfield (L, -1, var);
      found = lua_istable (L, -1);
      if (found)
        lua_remove (L, -2);
      else
        lua_pop (L, 2);
    }

  if (!found)
    {
      lua_getglobal (L, "main_vars");
      lua_getfield (L, -1, var);
      found = lua_istable (L, -1);
      if (found)
        lua_remove (L, -2);
      else
        lua_pop (L, 2);
    }

  return found;
}

const char *
get_variable_doc (const char *var, char **defval)
{
  char *ret = NULL;

  if (get_variable_entry (NULL, var))
    {
      lua_getfield (L, -1, "defval");
      *defval = (char *) lua_tostring (L, -1);
      lua_pop (L, 1);
      lua_getfield (L, -1, "doc");
      ret = (char *) lua_tostring (L, -1);
      lua_pop (L, 2);
    }

  return ret;
}

const char *
get_variable_bp (Buffer * bp, const char *var)
{
  char *ret = NULL;

  if (get_variable_entry (bp, var))
    {
      lua_getfield (L, -1, "val");
      ret = (char *) lua_tostring (L, -1);
      lua_pop (L, 2);
    }

  return ret;
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
    return strcmp (p, "nil") != 0;

  return false;
}

char *
minibuf_read_variable_name (char *fmt, ...)
{
  va_list ap;
  char *ms;
  Completion *cp = completion_new (false);

  lua_getglobal (L, "main_vars");
  lua_pushnil (L);
  while (lua_next (L, -2) != 0) {
    char *s = (char *) lua_tostring (L, -2);
    assert (s);
    gl_sortedlist_add (get_completion_completions (cp), completion_strcmp,
                       xstrdup (s));
    lua_pop (L, 1);
  }
  lua_pop (L, 1);

  va_start (ap, fmt);
  ms = minibuf_vread_completion (fmt, "", cp, NULL,
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
    set_variable (var, val);

  STR_FREE (var);
  STR_FREE (val);
}
END_DEFUN
