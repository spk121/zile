## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.9 2004/10/14 00:49:37 rrt Exp $

EXTRA_DIST = regex.c regex.h

if HAVE_REGEX_H
regex =
else
regex = regex.o
endif

TESTS = alist astr htable
check_PROGRAMS = alist astr htable

alist_LDADD = -lzile
alist_CPPFLAGS = -DTEST

astr_LDADD = -lzile
astr_CPPFLAGS = -DTEST

htable_LDADD = -lzile
htable_CPPFLAGS = -DTEST

noinst_LIBRARIES = libzile.a
libzile_a_SOURCES =				\
	agetcwd.c				\
	agetcwd.h				\
	alist.c					\
	alist.h					\
	astr.c					\
	astr.h					\
	htable.c				\
	htable.h				\
	vector.c				\
	vector.h				\
	memcmp.c				\
	strrstr.c				\
	vasprintf.c				\
	xmalloc.c				\
	xrealloc.c				\
	xstrdup.c

libzile_a_LIBADD =				\
	$(regex)
