/*	$Id: lua.c,v 1.1 2004/01/29 10:40:22 rrt Exp $	*/

/*
 * Copyright (c) 2004 Reuben Thomas.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
        if (res)
                *out = astr_copy_cstr(res);
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
