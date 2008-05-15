dnl configure.ac
dnl
dnl Copyright (c) 2008 Free Software Foundation, Inc.
dnl Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
dnl Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Reuben Thomas.
dnl Copyright (c) 2004 David Capello.
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

dnl ==================================================================
dnl Initialise autoconf and automake
dnl ==================================================================

AC_INIT(Zile, 2.2.59, bug-zile@gnu.org)
AM_INIT_AUTOMAKE(std-options)

dnl ==================================================================
dnl Checks for programs
dnl ==================================================================

AC_GNU_SOURCE
AC_PROG_CC
BFD_CC_FOR_BUILD
AM_PROG_CC_C_O

dnl ==================================================================
dnl Checks for functions
dnl ==================================================================

AC_REPLACE_FUNCS(vasprintf strrstr getopt_long_only)
AC_CHECK_FUNCS(getcwd fchmod fchown euidaccess alloca)
AM_CONDITIONAL(HAVE_GETOPT_LONG_ONLY, test x$have_getopt_long_only = xtrue)

dnl ==================================================================
dnl Checks for header files
dnl ==================================================================

AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h)

dnl ==================================================================
dnl Check for GNU Regex library
dnl ==================================================================

AC_MSG_CHECKING([for GNU regex.h])
AC_TRY_COMPILE([
#include <sys/types.h>
#include <regex.h>
], [
struct re_pattern_buffer pattern;
struct re_registers search_regs;
re_set_syntax(RE_SYNTAX_EMACS);
re_compile_pattern(".*", 2, &pattern);
re_search(&pattern, "test", 4, 0, 4, &search_regs);
], have_regex_h=true, have_regex_h=false)

if test x$have_regex_h = xtrue ; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_REGEX_H, 1, [Define this to use GNU regex.h.])
else
	AC_MSG_RESULT(no)
fi

AM_CONDITIONAL(HAVE_REGEX_H, test x$have_regex_h = xtrue)

dnl ==================================================================
dnl Turn on additional compiler warnings
dnl ==================================================================

if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -Wall -W -Wmissing-prototypes -Wstrict-prototypes -pedantic"
fi

dnl ==================================================================
dnl Curses
dnl ==================================================================

AC_CHECK_HEADER(ncurses.h,AC_DEFINE(HAVE_NCURSES_H, 1, [We have the ncurses.h header.]),
	[AC_CHECK_HEADER(curses.h,AC_DEFINE(HAVE_CURSES_H, 1, [We have the curses.h header.]),
	AC_MSG_FAILURE([cannot find either curses.h or ncurses.h]))])
dnl Check if ncurses library is available as libncurses or libcurses
AC_CHECK_LIB(ncurses, getch, [LIBS="$LIBS -lncurses"],
	[AC_CHECK_LIB(ncurses, getch, [LIBS="$LIBS -lncurses -ldl"],
	[AC_CHECK_LIB(curses, getch, [LIBS="$LIBS -lcurses"],
	[AC_CHECK_LIB(curses, getch, [LIBS="$LIBS -lcurses -ldl"],
	AC_MSG_FAILURE([cannot find either libncurses or libcurses]))])])])

dnl ==================================================================
dnl Extra definitions for config.h
dnl ==================================================================

AH_BOTTOM([
#ifndef HAVE_STRRSTR
extern char *strrstr(const char *, const char *);
#endif
])

AC_DEFINE_UNQUOTED(CONFIGURE_DATE, ["`date '+%a %b %d %Y'`"],
		   [The date of compilation.])
AC_DEFINE_UNQUOTED(CONFIGURE_HOST, ["`hostname`"],
		   [The host of compilation.])

rm -f src/paths.h

dnl ==================================================================
dnl Generate configured files
dnl ==================================================================

AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES(Makefile doc/Makefile src/Makefile)
AC_OUTPUT
