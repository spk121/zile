dnl Process this file with autoconf to produce a configure script.

dnl Every other copy of the package version number gets its value from here
AC_INIT(zile, 1.8-a1, zile-devel@lists.sourceforge.net)
AC_CONFIG_SRCDIR(src/main.c)
AM_INIT_AUTOMAKE

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

AC_SUBST(VERSION)

dnl Allow GNU functions to be used if present (e.g. vasprintf)
AC_GNU_SOURCE

dnl ==================================================================
dnl Checks for programs.
dnl ==================================================================

AC_PROG_CC
AC_C_INLINE
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PATH_PROG(MAKEINFO, makeinfo)
AC_PATH_PROG(GPERF, gperf)

dnl ==================================================================
dnl Checks for functions.
dnl ==================================================================

AC_REPLACE_FUNCS(xmalloc xrealloc xstrdup vasprintf strrstr)

dnl ==================================================================
dnl Checks for header files.
dnl ==================================================================

AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h)

dnl ==================================================================
dnl Checks for typedefs, structures, and compiler characteristics.
dnl ==================================================================

AC_C_CONST
AC_TYPE_SIZE_T

dnl ==================================================================
dnl Checks for library functions.
dnl ==================================================================

AC_FUNC_MEMCMP
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(getcwd strerror strstr)

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
  CFLAGS="$CFLAGS -Wall -Wmissing-prototypes -Wstrict-prototypes"
fi

dnl ==================================================================
dnl Debugging
dnl ==================================================================

AC_MSG_CHECKING([if the debugging code should be included])
AC_ARG_ENABLE(debug,
AC_HELP_STRING([--enable-debug], [include debugging code]),
[
if test $enableval = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(DEBUG, 1, [Define this to include debugging code.])
else
	AC_MSG_RESULT(no)
fi
], [
AC_MSG_RESULT(no)
])

dnl ==================================================================
dnl Allegro
dnl ==================================================================

AC_DEFUN(zile_CHECK_ALLEGRO,
[
        AC_CHECK_HEADER(allegro.h,
			AC_DEFINE(HAVE_ALLEGRO_H, 1, [We have the allegro.h header.]),
			AC_MSG_FAILURE([cannot find allegro.h]))
        AC_DEFINE(ALLEGRO, 1, [Define this to use Allegro library.])
])

AC_MSG_CHECKING([use allegro library])
AC_ARG_ENABLE(allegro,
AC_HELP_STRING([--enable-allegro], [use allegro interface]),
[
if test $enableval = yes; then
        allegro=true
        AC_MSG_RESULT(yes)
	zile_CHECK_ALLEGRO
else
        allegro=false
        AC_MSG_RESULT(no)
fi
],[
        allegro=false
        AC_MSG_RESULT(no)
])

AM_CONDITIONAL(ALLEGRO, test x$allegro = xtrue)

dnl ==================================================================
dnl Curses
dnl ==================================================================

AC_DEFUN(zile_CHECK_CURSES,
[
        AC_MSG_RESULT(yes)
        AC_DEFINE(CURSES, 1, [Define this to use curses.])
        AC_CHECK_HEADER(ncurses.h,AC_DEFINE(HAVE_NCURSES_H, 1, [We have the ncurses.h header.]),
	[AC_CHECK_HEADER(curses.h,AC_DEFINE(HAVE_CURSES_H, 1, [We have the curses.h header.]),
	        AC_MSG_FAILURE([cannot find either curses.h or ncurses.h]))])
        dnl Check if ncurses library is available as libncurses or libcurses
        AC_CHECK_LIB(ncurses, getch, [LIBS="$LIBS -lncurses"],
	        [AC_CHECK_LIB(curses, getch, [LIBS="$LIBS -lcurses"],
		AC_MSG_FAILURE([cannot find either libncurses or libcurses]))])
])

if test x$allegro = xfalse; then
	AC_MSG_CHECKING([if curses is required])
	AC_ARG_ENABLE(curses,
	AC_HELP_STRING([--disable-curses], [don't use curses interface]),
	[
	if test $enableval = yes; then
	        curses=true
	        zile_CHECK_CURSES
	else
	        curses=false
	        AC_MSG_RESULT(no)
	        AC_CHECK_HEADERS([termios.h] [term.h])
	        AC_CHECK_LIB(termcap, tgetstr, [LIBS="$LIBS -ltermcap"],
			AC_MSG_FAILURE([cannot find libtermcap]))
	fi
	],[
	        curses=true
	        zile_CHECK_CURSES
	])
fi

AM_CONDITIONAL(CURSES, test x$curses = xtrue)

dnl ==================================================================
dnl Extra definitions for config.h
dnl ==================================================================

AH_BOTTOM([
#include <stddef.h>
#include <stdarg.h>

#ifndef HAVE_XMALLOC
extern void *xmalloc(size_t);
#endif

#ifndef HAVE_XREALLOC
extern void *xrealloc(void *, size_t);
#endif

#ifndef HAVE_XSTRDUP
extern char *xstrdup(const char *);
#endif

#ifndef HAVE_VASPRINTF
extern int asprintf(char **strp, const char *fmt, ...);
extern int vasprintf(char **strp, const char *fmt, va_list ap);
#endif

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
dnl Generate makefiles
dnl ==================================================================

AC_OUTPUT(Makefile \
	  doc/Makefile \
	  lib/Makefile \
	  etc/Makefile \
	  src/Makefile)
