dnl Process this file with autoconf to produce a configure script.

dnl Every other copy of the package version number gets its value from here
AC_INIT(zile, 1.7-b4, zile-devel@lists.sourceforge.net)
AC_CONFIG_SRCDIR(src/main.c)
AM_INIT_AUTOMAKE

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

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
dnl Check if ncurses header is available as ncurses.h or curses.h
AC_CHECK_HEADER(ncurses.h,
	[AC_DEFINE(HAVE_NCURSES_H, 1, [Define this if you have ncurses.h])],
	[AC_CHECK_HEADER(curses.h,
		[AC_DEFINE(HAVE_CURSES_H, 1, [Define this if you have curses.h instead of ncurses.h])],
		AC_MSG_FAILURE([cannot find neither curses.h or ncurses.h]))])

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
dnl Check if ncurses library is available as libncurses or libcurses
AC_CHECK_LIB(ncurses, getch, [LIBS="$LIBS -lncurses"],
	[AC_CHECK_LIB(curses, getch, [LIBS="$LIBS -lcurses"],
		AC_MSG_FAILURE([cannot find neither libncurses.a or libcurses.a]))])

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
dnl Editing modes
dnl ==================================================================

AC_MSG_CHECKING([if all features should be enabled])
AC_ARG_ENABLE(all-modes,
AC_HELP_STRING([--enable-all-modes], [enable all editing modes]),
[
if test $enableval = yes; then
	allmodes=yes
	AC_MSG_RESULT(yes)
else
	allmodes=no
	AC_MSG_RESULT(no)
fi
], [
allmodes=no
AC_MSG_RESULT(no)
])

AC_MSG_CHECKING([if the C Mode should be included])
AC_ARG_ENABLE(c-mode,
AC_HELP_STRING([--enable-c-mode], [include C Mode]),
[
if test $enableval = yes; then
	enable_c_mode=true
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_C_MODE)
else
	AC_MSG_RESULT(no)
fi
], [
if test $allmodes = yes; then
	enable_c_mode=true
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_C_MODE, 1, [Define this to include C Mode.])
else
	AC_MSG_RESULT(no)
fi
])
AM_CONDITIONAL(ENABLE_C_MODE, test x$enable_c_mode = xtrue)

AC_MSG_CHECKING([if the C++ Mode should be included])
AC_ARG_ENABLE(cpp-mode,
AC_HELP_STRING([--enable-cpp-mode], [include C++ Mode]),
[
if test $enableval = yes; then
	enable_cpp_mode=true
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_CPP_MODE)
else
	AC_MSG_RESULT(no)
fi
], [
if test $allmodes = yes; then
	enable_cpp_mode=true
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_CPP_MODE, 1, [Define this to include C++ Mode.])
else
	AC_MSG_RESULT(no)
fi
])
AM_CONDITIONAL(ENABLE_CPP_MODE, test x$enable_cpp_mode = xtrue)

AC_MSG_CHECKING([if the Shell Mode should be included])
AC_ARG_ENABLE(shell-mode,
AC_HELP_STRING([--enable-shell-mode], [include Shell Mode]),
[
if test $enableval = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_SHELL_MODE)
else
	AC_MSG_RESULT(no)
fi
], [
if test $allmodes = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_SHELL_MODE, 1, [Define this to include Shell Mode.])
else
	AC_MSG_RESULT(no)
fi
])

AC_MSG_CHECKING([if the C Sharp Mode should be included])
AC_ARG_ENABLE(csharp-mode,
AC_HELP_STRING([--enable-csharp-mode], [include C Sharp Mode]),
[
if test $enableval = yes; then
	enable_csharp_mode=true
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_CSHARP_MODE)
else
	AC_MSG_RESULT(no)
fi
], [
if test $allmodes = yes; then
	enable_csharp_mode=true
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_CSHARP_MODE, 1, [Define this to include C# Mode.])
else
	AC_MSG_RESULT(no)
fi
])
AM_CONDITIONAL(ENABLE_CSHARP_MODE, test x$enable_csharp_mode = xtrue)

AC_MSG_CHECKING([if the Java Mode should be included])
AC_ARG_ENABLE(java-mode,
AC_HELP_STRING([--enable-java-mode], [include Java Mode]),
[
if test $enableval = yes; then
	enable_java_mode=true
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_JAVA_MODE)
else
	AC_MSG_RESULT(no)
fi
], [
if test $allmodes = yes; then
	enable_java_mode=true
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_JAVA_MODE, 1, [Define this to include Java Mode.])
else
	AC_MSG_RESULT(no)
fi
])
AM_CONDITIONAL(ENABLE_JAVA_MODE, test x$enable_java_mode = xtrue)

AC_MSG_CHECKING([if the Mail Mode should be included])
AC_ARG_ENABLE(mail-mode,
AC_HELP_STRING([--enable-mail-mode], [include Mail Mode]),
[
if test $enableval = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_MAIL_MODE)
else
	AC_MSG_RESULT(no)
fi
], [
if test $allmodes = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_MAIL_MODE, 1, [Define this to include Mail Mode.])
else
	AC_MSG_RESULT(no)
fi
])

dnl ==================================================================
dnl Lua
dnl ==================================================================

AC_MSG_CHECKING([if Lua scripting should be included])
AC_ARG_ENABLE(lua,
AC_HELP_STRING([--enable-lua], [Lua scripting]),
[
if test $enableval = yes; then
	enable_lua=true
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_LUA, 1, [Define this to enable Lua scripting (EXPERIMENTAL).])
        dnl Really want to check if Lua uses dlopen
        AC_CHECK_LIB(dl, dlopen, LIBS="$LIBS -ldl")
        AC_CHECK_HEADERS(lua.h lualib.h,
            LIBS="$LIBS -llua -llualib",
            AC_MSG_FAILURE([cannot find one of lua.h or lualib.h.]))
else
        enable_lua=false
	AC_MSG_RESULT(no)
fi
],[
enable_lua=false
AC_MSG_RESULT(no)
])
AM_CONDITIONAL(ENABLE_LUA, test x$enable_lua = xtrue)

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

#ifndef HAVE_STRRCHR
extern char *strrchar(const char *, const char *);
#endif

#if defined(ENABLE_C_MODE) || defined(ENABLE_CPP_MODE) || \
    defined(ENABLE_CSHARP_MODE) || defined(ENABLE_JAVA_MODE)
#define ENABLE_CLIKE_MODES 1
#endif

#if defined(ENABLE_CLIKE_MODES) || defined(ENABLE_SHELL_MODE) || \
    defined(ENABLE_MAIL_MODE)
#define ENABLE_NONTEXT_MODES 1
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
	  src/Makefile \
	  src/term_ncurses/Makefile)
