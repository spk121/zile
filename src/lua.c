/* LUA library interface
   Copyright (c) 2004 Reuben Thomas.  All rights reserved.

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

/*	$Id: lua.c,v 1.3 2004/03/11 13:50:14 rrt Exp $	*/

#include "config.h"

#include "zile.h"
#include "extern.h"

#if ENABLE_LUA
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


/* The global Lua state */
static lua_State *L = NULL;


/*
 * List of Lua libraries to use
 */
static const luaL_reg lualibs[] = {
        {"base", luaopen_base},
        {"table", luaopen_table},
        {"io", luaopen_io},
        {"string", luaopen_string},
        {"math", luaopen_math},
        {"debug", luaopen_debug},
        {"loadlib", luaopen_loadlib},
        {NULL, NULL}
};


/*
 * Open the Lua libraries given in lualibs
 */
static void openlibs(lua_State *l) {
        const luaL_reg *lib = lualibs;
        for (; lib->func; lib++) {
                lib->func(l);  /* open library */
                lua_settop(l, 0);  /* discard any results */
        }
}

/*
 * Open zile's Lua state
 */
void zlua_open(void) {
	if ((L = lua_open()) == NULL) {
		fprintf(stderr, "Cannot open Lua state\n");
		exit(1);
	}
        openlibs(L);
}


/*
 * Close zile's Lua state
 */
void zlua_close(void) {
        lua_close(L);
}


/*
 * Run a Lua command
 */
int zlua_do(const char *cmd, astr *out)
{
	int err, stk;
        const char *res;

        *out = NULL;
        stk = lua_gettop(L);
        if ((err = luaL_loadbuffer(L, cmd, strlen(cmd), NULL)) == 0)
                err = lua_pcall(L, 0, 1, 0);

        res = lua_tostring(L, -1);
        if (res) {
                *out = astr_new();
                astr_assign_cstr(*out, res);
        }
        lua_settop(L, stk);

        return err;
}


DEFUN("lua", lua)
/*+
Read a Lua expression, evaluate it and insert the result at point.
+*/
{
	char *ms;
        int res;
        astr out;

        ms = minibuf_read("Lua command: ", "");
	if (ms == NULL)
		return cancel();
	if (ms[0] == '\0')
		return FALSE;

        if ((res = zlua_do(ms, &out)) != 0)
                minibuf_write("Lua error: %s", out ? astr_cstr(out) :
                              "(no error message)");
        else if (out)
                insert_string(astr_cstr(out));

	return TRUE;
}


#endif
