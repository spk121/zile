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

AC_INIT(Zile, 2.2.60, bug-zile@gnu.org)
AM_INIT_AUTOMAKE(std-options)

dnl ==================================================================
dnl Checks for programs
dnl ==================================================================

AC_GNU_SOURCE
AC_PROG_CC
gl_EARLY
BFD_CC_FOR_BUILD
AM_PROG_CC_C_O
AC_PROG_RANLIB

dnl ==================================================================
dnl Checks for functions
dnl ==================================================================

gl_INIT
AC_REPLACE_FUNCS(vasprintf strrstr)
AC_CHECK_FUNCS(basename getcwd fchmod fchown euidaccess)

dnl ==================================================================
dnl Checks for header files
dnl ==================================================================

AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h libgen.h)

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
AC_CONFIG_FILES(Makefile doc/Makefile lib/Makefile src/Makefile)
AC_OUTPUT
