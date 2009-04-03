dnl configure.ac
dnl
dnl Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
AC_INIT(Zile, 2.4.0, bug-zile@gnu.org)
AM_INIT_AUTOMAKE(std-options)

dnl Checks for programs
AC_GNU_SOURCE
AC_PROG_CC
if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -Wall -W -Wmissing-prototypes -Wstrict-prototypes -pedantic -D_FORTIFY_SOURCE=2"
fi
gl_EARLY
AM_PROG_CC_C_O

dnl help2man
AC_PATH_PROG(HELP2MAN, help2man)
if test "$ac_cv_path_HELP2MAN" = ""; then
  AC_MSG_FAILURE([cannot find help2man])
fi

dnl Valgrind
AC_ARG_ENABLE(valgrind,
  [AS_HELP_STRING([--disable-valgrind],
    [do not run tests with Valgrind])],
  [],
  [enable_valgrind=yes])
AS_IF([test "$enable_valgrind" != "no"],
  [AC_PATH_PROG(VALGRIND, valgrind)
  if test -n "$VALGRIND"; then
    VALGRIND="$VALGRIND --error-exitcode=1 --leak-check=full"
  fi])

dnl Checks for functions and headers
gl_INIT
AC_HEADER_STDC
AC_CHECK_FUNCS(fchmod)

dnl Curses
MP_WITH_CURSES
if test "$mp_cv_curses" != "yes" -a "$mp_cv_ncurses" != "yes"; then
  AC_MSG_FAILURE([cannot find curses])
fi
AC_SUBST(CURSES_LIB)

dnl Lua
AX_WITH_LUA
if test -z "$LUA"; then
  AC_MSG_FAILURE([Lua not found])
fi
AX_LUA_VERSION(501, 502)
AX_LUA_HEADERS
AX_LUA_LIBS
AX_LUA_LIB_VERSION(501, 502)
AC_SUBST(LUA)
AC_SUBST(LUA_INCLUDE)
AC_SUBST(LUA_LIB)

dnl Extra definitions for config.h
AC_DEFINE_UNQUOTED(CONFIGURE_DATE, ["`date '+%a %b %d %Y'`"],
                   [The date of compilation.])
AC_DEFINE_UNQUOTED(CONFIGURE_HOST, ["${ac_hostname}"],
                   [The host of compilation.])

dnl Generate output
AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES(Makefile lib/Makefile src/Makefile src/tbl_opts.h)
AC_OUTPUT
