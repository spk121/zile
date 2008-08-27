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

SUBDIRS = testsuite

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib -DPATH_DATA="\"$(pkgdatadir)\""
LDADD = $(top_builddir)/lib/libgnu.a $(LIB_EACCESS) $(LIBINTL)

TESTS = astr
check_PROGRAMS = astr

astr_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src\"" $(AM_CPPFLAGS)
astr_LDADD = xalloc.o $(LDADD)

bin_PROGRAMS = zile

EXTRA_PROGRAMS = mkfuncs

tbl_funcs.h: $(zile_SOURCES) mkfuncs$(EXEEXT_FOR_BUILD)
	./mkfuncs $(zile_SOURCES) > tbl_funcs.h

zile_LDADD = $(LDADD) $(LIBOBJS)

zile_SOURCES =					\
	$(top_srcdir)/src/astr.c		\
	$(top_srcdir)/src/astr.h		\
	$(top_srcdir)/src/xalloc.c		\
	$(top_srcdir)/src/extern.h		\
	$(top_srcdir)/src/zile.h		\
	$(top_srcdir)/src/tbl_bind.h		\
	$(top_srcdir)/src/tbl_vars.h		\
	$(top_srcdir)/src/eval.c		\
	$(top_srcdir)/src/lists.c		\
	$(top_srcdir)/src/lists.h		\
	$(top_srcdir)/src/lisp.c		\
	$(top_srcdir)/src/basic.c		\
	$(top_srcdir)/src/bind.c		\
	$(top_srcdir)/src/buffer.c		\
	$(top_srcdir)/src/completion.c		\
	$(top_srcdir)/src/editfns.c		\
	$(top_srcdir)/src/file.c		\
	$(top_srcdir)/src/funcs.c		\
	$(top_srcdir)/src/getkey.c		\
	$(top_srcdir)/src/help.c		\
	$(top_srcdir)/src/history.c		\
	$(top_srcdir)/src/keycode.c		\
	$(top_srcdir)/src/killring.c		\
	$(top_srcdir)/src/line.c		\
	$(top_srcdir)/src/macro.c		\
	$(top_srcdir)/src/main.c		\
	$(top_srcdir)/src/marker.c		\
	$(top_srcdir)/src/minibuf.c		\
	$(top_srcdir)/src/point.c		\
	$(top_srcdir)/src/redisplay.c		\
	$(top_srcdir)/src/registers.c		\
	$(top_srcdir)/src/search.c		\
	$(top_srcdir)/src/undo.c		\
	$(top_srcdir)/src/variables.c		\
	$(top_srcdir)/src/window.c		\
	$(top_srcdir)/src/term_minibuf.c	\
	$(top_srcdir)/src/term_redisplay.c	\
	$(top_srcdir)/src/term_ncurses.c

ALLSOURCES = $(zile_SOURCES) mkdoc.c

BUILT_SOURCES = tbl_funcs.h

CLEANFILES = $(BUILT_SOURCES) $(EXTRA_PROGRAMS)

loc:
	sloccount $(ALLSOURCES)
