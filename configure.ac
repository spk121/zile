dnl configure.ac
dnl
dnl Copyright (c) 1997-2011 Free Software Foundation, Inc.
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
AC_INIT(Zile, 2.3.24, bug-zile@gnu.org)
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror std-options check-news])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Checks for programs
AC_GNU_SOURCE
AC_PROG_CC
gl_EARLY
AM_PROG_CC_C_O
gl_MANYWARN_ALL_GCC([warnings])
# Set up the list of undesired warnings.
nw=
nw="$nw -Wsystem-headers"   # Don’t let system headers trigger warnings
nw="$nw -Wundef"            # All compiler preprocessors support #if UNDEF
nw="$nw -Wtraditional"      # All compilers nowadays support ANSI C
nw="$nw -Wstrict-overflow"  # Use a lower level (see below).
nw="$nw -Wconversion"       # These warnings usually don’t point to mistakes.
nw="$nw -Wsign-conversion"  # Likewise.
nw="$nw -Waggregate-return" # We do this on purpose.
nw="$nw -Wbad-function-cast" # FIXME: We do this on purpose (non-pointer list elements).
nw="$nw -Wtraditional-conversion" # Don't care.
nw="$nw -Wpadded"           # Don't care.
nw="$nw -Wc++-compat"       # Don't care.
nw="$nw -Woverlength-strings" # Don't care.
nw="$nw -Wmissing-format-attribute" # Don't care.
nw="$nw -Wunreachable-code" # Seems buggy.
# Enable all GCC warnings not in this list.
gl_MANYWARN_COMPLEMENT([warnings], [$warnings], [$nw])
for w in $warnings; do
  gl_WARN_ADD([$w])
done
# Add an extra warning
gl_WARN_ADD([-Wstrict-overflow=1], [WARN_FLAGS])
# Add some more safety measures
gl_WARN_ADD([-D_FORTIFY_SOURCE=2], [WARN_FLAGS])
gl_WARN_ADD([-fmudflap], [WARN_FLAGS])

dnl help2man
AX_WITH_PROG(HELP2MAN, help2man)

dnl Code complexity reports
AC_PATH_PROG([PMCCABE], [pmccabe], [false])

dnl Valgrind
AX_WITH_PROG(VALGRIND, valgrind)
if test -n "$VALGRIND"; then
  VALGRIND="$VALGRIND -q --error-exitcode=1 --leak-check=full"
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
AC_DEFINE_UNQUOTED([CONFIGURE_DATE], ["`date '+%a %b %d %Y'`"],
                   [The date of compilation.])
AC_DEFINE_UNQUOTED([CONFIGURE_HOST], ["${ac_hostname}"],
                   [The compilation host.])

dnl Generate output
AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES(Makefile lib/Makefile src/Makefile src/tbl_opts.h)
AC_OUTPUT
