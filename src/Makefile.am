# Source Makefile.am
#
# Copyright (c) 2008 Free Software Foundation, Inc.
# Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
# Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Reuben Thomas.
# Copyright (c) 2004 David Capello.
#
# This file is part of GNU Zile.
#
# GNU Zile is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Zile is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Zile; see the file COPYING.  If not, write to the
# Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
# MA 02111-1301, USA.

TESTS = astr $(top_srcdir)/src/zile-startup-test
check_PROGRAMS = astr

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib -DPATH_DATA="\"$(pkgdatadir)\""
LDADD = $(top_builddir)/lib/libgnu.a $(LIB_EACCESS) $(LIBINTL)

astr_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src\"" $(AM_CPPFLAGS)
astr_LDADD = xalloc.o $(LDADD)

bin_PROGRAMS = zile

zile_SOURCES =				\
	list.c				\
	list.h				\
	astr.c				\
	astr.h				\
	xalloc.c			\
	eval.c				\
	eval.h				\
	lists.c				\
	lists.h				\
	parser.c			\
	parser.h			\
	lisp.c				\
	basic.c				\
	bind.c				\
	buffer.c			\
	completion.c			\
	editfns.c			\
	extern.h			\
	zile.h				\
	file.c				\
	funcs.c				\
	glue.c				\
	help.c				\
	history.c			\
	keys.c				\
	killring.c			\
	line.c				\
	macro.c				\
	main.c				\
	marker.c			\
	minibuf.c			\
	point.c				\
	redisplay.c			\
	registers.c			\
	search.c			\
	tbl_funcs.h			\
	tbl_vars.h			\
	undo.c				\
	variables.c			\
	window.c			\
	term_minibuf.c			\
	term_redisplay.c		\
	term_ncurses.c

zile_LDADD = $(LDADD) $(LIBOBJS)

EXTRA_DIST = zile-startup-test

loc:
	sloccount $(zile_SOURCES)
