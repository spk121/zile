## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.19 2004/11/15 12:36:13 rrt Exp $

if HAVE_REGEX_H
regex =
else
regex = regex.o
endif

TESTS = alist astr htable
check_PROGRAMS = alist astr htable

alist_CPPFLAGS = -DTEST
alist_LDADD = zmalloc.o

astr_CPPFLAGS = -DTEST
astr_LDADD = zmalloc.o strrstr.o

htable_CPPFLAGS = -DTEST
htable_LDADD = zmalloc.o alist.o

INCLUDES = -I$(top_srcdir)/lib

bin_PROGRAMS = zile

zile_base_SOURCES =				\
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
	strrstr.c				\
	vasprintf.c				\
	zmalloc.c				\
	basic.c					\
	bind.c					\
	buffer.c				\
	completion.c				\
	editfns.c				\
	editfns.h				\
	extern.h				\
	file.c					\
	funcs.c					\
	glue.c					\
	help.c					\
	history.c				\
	keys.c					\
	killring.c				\
	line.c					\
	macro.c					\
	main.c					\
	marker.c				\
	minibuf.c				\
	point.c					\
	rc.c					\
	redisplay.c				\
	registers.c				\
	search.c				\
	tbl_funcs.h				\
	tbl_vars.h				\
	undo.c					\
	variables.c				\
	window.c				\
	term_minibuf.c				\
	term_redisplay.c			\
	zile.h

if ALLEGRO
zile_SOURCES = $(zile_base_SOURCES)		\
	term_allegro.c

zile_LDFLAGS = `allegro-config --libs`
else
zile_SOURCES = $(zile_base_SOURCES)		\
	term_termcap.c
endif

zile_LDADD =					\
	$(regex)

man_MANS = zile.1

EXTRA_DIST = zile.1.in regex.c regex.h

zile.1: zile.1.in
	rm -f $@
	sed s+'@'pkgdatadir@+$(pkgdatadir)+g $< > $@

clean-local:
	rm -f zile.1

distclean-local:
	rm -f paths.h

help.o: paths.h

paths.h:
	@echo "/* Automatically generated file.  NO NOT EDIT! */" >paths.h.tmp
	@echo "" >>paths.h.tmp
	@echo '#define PATH_PREFIX "$(prefix)"' >>paths.h.tmp
	@echo '#define PATH_DATA "$(pkgdatadir)"' >>paths.h.tmp
	if test ! -f paths.h -o				\
	        ! "cmp paths.h paths.h.tmp -s" ; then	\
		mv paths.h.tmp paths.h ;		\
	fi
