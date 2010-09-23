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
AC_INIT(Zile, 2.3.20, bug-zile@gnu.org)
AM_INIT_AUTOMAKE([std-options check-news])

dnl Checks for programs
AC_GNU_SOURCE
AC_PROG_CC
AX_CFLAGS_WARN_ALL
if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -W -Wmissing-prototypes -Wstrict-prototypes -D_FORTIFY_SOURCE=2"
fi
gl_EARLY
AM_PROG_CC_C_O

dnl help2man
AX_WITH_PROG(HELP2MAN, help2man)

dnl Code complexity reports
AC_PATH_PROG([PMCCABE], [pmccabe], [false])

dnl Valgrind
AX_WITH_PROG(VALGRIND, valgrind)
if test -n "$VALGRIND"; then
  VALGRIND="$VALGRIND --error-exitcode=1 --leak-check=full"
fi

dnl Checks for functions and headers
AC_HEADER_STDC
dnl If system lacks RE_PLAIN, force --with-included-regex
AC_MSG_CHECKING([whether system regex.h has RE_PLAIN])
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM(
    [AC_INCLUDES_DEFAULT[
     #include <regex.h>
     ]],
    [[reg_syntax_t syn = RE_PLAIN;]])],
 [AC_MSG_RESULT([yes])],
 [AC_MSG_RESULT([no])
 with_included_regex=yes],
 dnl When crosscompiling, force included regex.
 [AC_MSG_RESULT([no])
 with_included_regex=yes])
gl_INIT
AC_CHECK_FUNCS_ONCE(fchmod)

dnl Curses
AX_WITH_CURSES
if test "$ax_cv_curses" != "yes"; then
  AC_MSG_ERROR([cannot find curses])
fi
AC_ARG_VAR(CURSES_LIB, [linker flags for curses library])

dnl Emacs
AX_WITH_PROG(EMACS, emacs)
if test -n "$EMACS"; then
  emacs_min_version=23.1
  emacs_version=$($EMACS --version 2>&1 | head -n 1 | cut -d' ' -f3)
  AC_MSG_CHECKING([Emacs version $emacs_version >= $emacs_min_version])
  AX_COMPARE_VERSION("$emacs_version", ge, $emacs_min_version,
    AC_MSG_RESULT([yes]),
    [AC_MSG_RESULT([no])
    unset EMACS])
fi

dnl Perl
AX_WITH_PROG(PERL, perl)

dnl Extra definitions for config.h
AC_DEFINE_UNQUOTED(CONFIGURE_DATE, ["`date '+%a %b %d %Y'`"],
                   [The date of compilation.])
AC_DEFINE_UNQUOTED(CONFIGURE_HOST, ["${ac_hostname}"],
                   [The compilation host.])

dnl Generate output
AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES(Makefile lib/Makefile src/Makefile src/tbl_opts.h)
AC_OUTPUT
