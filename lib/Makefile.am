## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.3 2004/02/17 20:21:17 ssigala Exp $

noinst_MANS = alist.3 astr.3 htable.3

EXTRA_DIST = alist.3 astr.3 htable.3 regex.c regex.h

if HAVE_REGEX_H
regex =
else
regex = regex.o
endif

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
	memcmp.c				\
	vasprintf.c				\
	xmalloc.c				\
	xrealloc.c				\
	xstrdup.c

libzile_a_LIBADD =				\
	$(regex)
