dnl Process this file with autoconf to produce a configure script.
AC_INIT([zile], [1.7], [sandro@sigala.it])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER([config.h])

dnl Checks for programs.
AC_PROG_CC
AC_C_INLINE
AC_PROG_INSTALL
AC_PROG_RANLIB

dnl This is enabled by default, since only ncurses is supported yet.
AC_DEFINE(USE_NCURSES, 1, [Default terminal.])

dnl Checks for libraries.
AC_REPLACE_FUNCS(xmalloc xrealloc xstrdup)
dnl The following check should depend on the `USE_NCURSES' definition.
AC_CHECK_LIB(ncurses, initscr,
    AC_DEFINE(HAVE_LIBNCURSES, 1,
	      [Define if you have the ncurses library (-lncurses).])
    LIBS="$LIBS -lncurses",
    AC_CHECK_LIB(curses, curses_version,
	AC_DEFINE(HAVE_LIBNCURSES) LIBS="$LIBS -lcurses"))

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(getcwd strerror strstr vasprintf)

AC_MSG_CHECKING([if the debugging code should be included])
AC_ARG_ENABLE(debug,
AC_HELP_STRING([--enable-debug], [include debugging code and assertions]),
[
if test $enableval = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(DEBUG, 1, [Define this to include debugging code.])
else
	AC_MSG_RESULT(no)
	AC_DEFINE(NDEBUG)
fi
], [
AC_MSG_RESULT(no)
AC_DEFINE(NDEBUG, 1, [Define this to remove the debugging assertions.])
])

AC_MSG_CHECKING([if all features should be enabled])
AC_ARG_ENABLE(all-features,
AC_HELP_STRING([--enable-all-features], [enable all features below]),
[
if test $enableval = yes; then
	allfeatures=yes
	AC_MSG_RESULT(yes)
else
	allfeatures=no
	AC_MSG_RESULT(no)
fi
], [
allfeatures=no
AC_MSG_RESULT(no)
])

AC_MSG_CHECKING([if the C Mode should be included])
AC_ARG_ENABLE(c-mode,
AC_HELP_STRING([--enable-c-mode], [include C Mode]),
[
if test $enableval = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_C_MODE)
else
	AC_MSG_RESULT(no)
fi
], [
if test $allfeatures = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_C_MODE, 1, [Define this to include C Mode.])
else
	AC_MSG_RESULT(no)
fi
])

AC_MSG_CHECKING([if the C++ Mode should be included])
AC_ARG_ENABLE(cpp-mode,
AC_HELP_STRING([--enable-cpp-mode], [include C++ Mode]),
[
if test $enableval = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_CPP_MODE)
else
	AC_MSG_RESULT(no)
fi
], [
if test $allfeatures = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_CPP_MODE, 1, [Define this to include C++ Mode.])
else
	AC_MSG_RESULT(no)
fi
])

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
if test $allfeatures = yes; then
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
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_CSHARP_MODE)
else
	AC_MSG_RESULT(no)
fi
], [
if test $allfeatures = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_CSHARP_MODE, 1, [Define this to include C# Mode.])
else
	AC_MSG_RESULT(no)
fi
])

AC_MSG_CHECKING([if the Java Mode should be included])
AC_ARG_ENABLE(java-mode,
AC_HELP_STRING([--enable-java-mode], [include Java Mode]),
[
if test $enableval = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_JAVA_MODE)
else
	AC_MSG_RESULT(no)
fi
], [
if test $allfeatures = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_JAVA_MODE, 1, [Define this to include Java Mode.])
else
	AC_MSG_RESULT(no)
fi
])

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
if test $allfeatures = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(ENABLE_MAIL_MODE, 1, [Define this to include Mail Mode.])
else
	AC_MSG_RESULT(no)
fi
])

AH_BOTTOM([
#ifdef USE_NCURSES
#ifndef HAVE_LIBNCURSES
#error ncurses is required for compilation.
#endif
#endif

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

AC_CONFIG_FILES([doc/Makefile lib/Makefile etc/Makefile src/Makefile \
          src/term_ncurses/Makefile Makefile])
AC_OUTPUT
