dnl Process this file with autoconf to produce a configure script.

AC_INIT(src/main.c)
dnl Every other copy of the package version number gets its value from here
AM_INIT_AUTOMAKE(zile, 1.7b3)
dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_CC
AC_C_INLINE
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PATH_PROG(MAKEINFO, makeinfo)
AC_PATH_PROG(GPERF, gperf)

dnl Checks for functions.
AC_REPLACE_FUNCS(xmalloc xrealloc xstrdup vasprintf)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h)
AC_CHECK_HEADER(curses.h,
         LIBS="$LIBS -lcurses",
         AC_MSG_FAILURE([cannot find curses.h]))

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(getcwd strerror strstr)

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


AH_BOTTOM([
#include <stddef.h>

#ifndef HAVE_XMALLOC
extern void *xmalloc(size_t);
#endif

#ifndef HAVE_XREALLOC
extern void *xrealloc(void *, size_t);
#endif

#ifndef HAVE_XSTRDUP
extern char *xstrdup(const char *);
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

AC_OUTPUT(Makefile \
	  doc/Makefile \
	  lib/Makefile \
	  etc/Makefile \
	  src/Makefile \
	  src/term_ncurses/Makefile)
