## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.2 2004/05/09 18:31:09 rrt Exp $

EXTRA_DIST = README

noinst_LIBRARIES = libterm_ncurses.a
libterm_ncurses_a_SOURCES =			\
	ncurses_key.c				\
	ncurses_misc.c				\
	ncurses_redisplay.c			\
	term_ncurses.c				\
	term_ncurses.h

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/src
