## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.15 2004/09/03 02:09:08 dacap Exp $

INCLUDES = -I$(top_srcdir)/lib

bin_PROGRAMS = zile
zile_base_SOURCES =				\
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
	term_allegro.c				\
	term_allegro.h
else
if CURSES
zile_SOURCES = $(zile_base_SOURCES)		\
	term_ncurses.c				\
	term_ncurses.h
else
zile_SOURCES = $(zile_base_SOURCES)		\
	term_termcap.c				\
	term_termcap.h
endif
endif

zile_LDADD =					\
	$(top_srcdir)/lib/libzile.a
man_MANS = zile.1

EXTRA_DIST = zile.1.in

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
