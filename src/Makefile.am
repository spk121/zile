## Process this file with automake to produce Makefile.in

if HAVE_REGEX_H
regex =
else
regex = regex.o
endif

TESTS = astr
check_PROGRAMS = astr

astr_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src\""
astr_LDADD = zmalloc.o strrstr.o

AM_CPPFLAGS = -DPATH_DATA="\"$(pkgdatadir)\""

bin_PROGRAMS = zile

zile_base_SOURCES =				\
	list.c					\
	list.h					\
	astr.c					\
	astr.h					\
	vector.c				\
	vector.h				\
	zmalloc.c				\
	eval.c					\
	eval.h					\
	lists.c					\
	lists.h					\
	parser.c				\
	parser.h				\
	vars.c					\
	vars.h					\
	lisp.c					\
	basic.c					\
	bind.c					\
	buffer.c				\
	completion.c				\
	editfns.c				\
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

if EPOCEMX
zile_SOURCES = $(zile_base_SOURCES) term_epocemx.c
else
zile_SOURCES = $(zile_base_SOURCES) term_ncurses.c
endif

zile_LDADD = $(regex) @LIBOBJS@

EXTRA_DIST = regex.c regex.h vasprintf.c strrstr.c getopt.h
