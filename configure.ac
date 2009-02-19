dnl configure.ac
dnl
dnl Copyright (c) 2008, 2009 Free Software Foundation, Inc.
dnl Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
dnl Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Reuben Thomas.
dnl
dnl This file is part of GNU Zile.
dnl
dnl GNU Zile is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published
dnl by the Free Software Foundation; either version 3, or (at your
dnl option) any later version.
dnl
dnl GNU Zile is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with GNU Zile; see the file COPYING.  If not, write to the
dnl Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
dnl MA 02111-1301, USA.

dnl Initialise autoconf and automake
AC_INIT(Zile, 2.3.1, bug-zile@gnu.org)
AM_INIT_AUTOMAKE(std-options)

dnl Checks for programs
AC_GNU_SOURCE
AC_PROG_CC
gl_EARLY
AM_PROG_CC_C_O

dnl Checks for functions and headers
gl_INIT
AC_HEADER_STDC
AC_CHECK_FUNCS(fchmod)

dnl Check for valgrind
AC_PATH_PROG([VALGRIND], [valgrind])
AC_SUBST([VALGRIND])

dnl Curses
MP_WITH_CURSES
if test "$mp_cv_curses" = "no" -a "$mp_cv_ncurses" = "no"; then
  AC_MSG_FAILURE([cannot find curses])
fi
AC_SUBST([CURSES_LIB])

dnl Turn on additional compiler warnings
if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -Wall -W -Wmissing-prototypes -Wstrict-prototypes -pedantic -D_FORTIFY_SOURCE=2"
fi

dnl ==================================================================
dnl Lua
dnl ==================================================================
dnl FIXME: Farm out to a separate file

AC_SUBST([LUA])
AC_SUBST([LUAC])
AC_SUBST([LUA_INCLUDES])
AC_SUBST([LUA_LIBS])

dnl Arguments
AC_ARG_WITH([lua-prefix],
            [AS_HELP_STRING([--with-lua-prefix=DIR],
                            [Lua files are in DIR])])
AC_ARG_WITH([lua-includes],
            [AS_HELP_STRING([--with-lua-includes=DIR],
                            [Lua include files are in DIR])])
AC_ARG_WITH([lua-libraries],
            [AS_HELP_STRING([--with-lua-libraries=DIR],
                            [Lua library files are in DIR])])
AC_ARG_WITH([lua-suffix],
            [AS_HELP_STRING([--with-lua-suffix=ARG],
                            [Lua binary and library files are suffixed with
                             ARG])])

dnl LUA
if test "x$with_lua_prefix" = x; then
    lua_search_path="$PATH"
else
    lua_search_path="$with_lua_prefix/bin"
fi
if test "x$LUA" = x; then
    AC_PATH_PROG([LUA], [lua$with_lua_suffix], [], [$lua_search_path])
fi

dnl lua_version
AC_MSG_CHECKING([for Lua version = 5.1.x])
lua_version=$($LUA -v 2>&1 | head -n 1 | cut -d' ' -f2)
case $lua_version in
5.1*)
    AC_MSG_RESULT([yes (found $lua_version)])
    ;;
*)
    AC_MSG_RESULT([no (found $lua_version)])
    ;;
esac

dnl LUA_INCLUDES
if test "x$with_lua_includes" != x; then
    LUA_INCLUDES="-I$with_lua_includes"
elif test "x$with_lua_prefix" != x; then
    LUA_INCLUDES="-I$with_lua_prefix/include"
fi
CPPFLAGS="$CPPFLAGS $LUA_INCLUDES"
AC_CHECK_HEADERS([lua.h lualib.h])

dnl LUA_LIBS
if test "x$with_lua_libraries" != x; then
    LUA_LIBS="-L$with_lua_libraries"
elif test "x$with_lua_prefix" != x; then
    LUA_LIBS="-L$with_lua_prefix/lib"
fi
AC_CHECK_LIB([m], [exp], [lua_extra_libs="$lua_extra_libs -lm"], [])
AC_CHECK_LIB([dl], [dlopen], [lua_extra_libs="$lua_extra_libs -ldl"], [])
AC_CHECK_LIB([lua$with_lua_suffix],
             [lua_call],
             [LUA_LIBS="$LUA_LIBS -llua$with_lua_suffix $lua_extra_libs"],
             [],
             [$LUA_LIBS $lua_extra_libs])

dnl liblua_version
AC_MSG_CHECKING([for liblua version 5.1.x])
LIBS="$LIBS $LUA_LIBS"
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <lua.h>
#include <stdlib.h>
#include <stdio.h>
int main()
{
    printf("(found %s, %d)... ", LUA_VERSION, LUA_VERSION_NUM);
    if (LUA_VERSION_NUM == 501)
        exit(EXIT_SUCCESS);
    exit(EXIT_FAILURE);
}

]])],
              [AC_MSG_RESULT([yes])],
              [AC_MSG_RESULT([no])])


dnl Extra definitions for config.h
AC_DEFINE_UNQUOTED(CONFIGURE_DATE, ["`date '+%a %b %d %Y'`"],
                   [The date of compilation.])
AC_DEFINE_UNQUOTED(CONFIGURE_HOST, ["`hostname`"],
                   [The host of compilation.])

dnl Generate configured files
AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES(Makefile lib/Makefile src/Makefile)
AC_OUTPUT
