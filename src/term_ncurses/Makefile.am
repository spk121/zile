## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.1 2004/01/23 01:46:22 dacap Exp $

EXTRA_DIST = README

noinst_LIBRARIES = libterm_ncurses.a
libterm_ncurses_a_SOURCES =			\
	ncurses_key.c				\
	ncurses_minibuf.c			\
	ncurses_misc.c				\
	ncurses_redisplay.c			\
	term_ncurses.c				\
	term_ncurses.h

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/src
