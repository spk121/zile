# Source Makefile.am
#
# Copyright (c) 1997-2011 Free Software Foundation, Inc.
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

doc_DATA += src/dotzile.sample

AM_CFLAGS = $(WERROR_CFLAGS) $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(builddir)/src -I$(srcdir)/src $(ISYSTEM)$(builddir)/lib $(ISYSTEM)$(srcdir)/lib -DPATH_DOCDIR="\"$(docdir)\"" $(LIBGC_CPPFLAGS)
LDADD = $(builddir)/lib/lib$(PACKAGE).a $(LIB_ACL) $(LIB_EACCESS) $(LIBINTL) $(CURSES_LIB)

BUILT_SOURCES =						\
	src/tbl_funcs.h					\
	src/tbl_vars.h

bin_PROGRAMS = src/zile

src_zile_LDADD = $(LDADD) $(LIBOBJS)

src_zile_function_SOURCE_FILES =			\
	src/basic.c					\
	src/bind.c					\
	src/buffer.c					\
	src/eval.c					\
	src/file.c					\
	src/funcs.c					\
	src/help.c					\
	src/killring.c					\
	src/line.c					\
	src/lisp.c					\
	src/macro.c					\
	src/redisplay.c					\
	src/registers.c					\
	src/search.c					\
	src/undo.c					\
	src/variables.c					\
	src/window.c

src_zile_base_SOURCE_FILES =				\
	$(src_zile_function_SOURCE_FILES)		\
	src/memrmem.h					\
	src/memrmem.c					\
	src/astr.c					\
	src/astr.h					\
	src/estr.c					\
	src/estr.h					\
	src/extern.h					\
	src/main.h					\
	src/tbl_vars.h					\
	src/lists.c					\
	src/lists.h					\
	src/buffer.h					\
	src/completion.h				\
	src/completion.c				\
	src/editfns.c					\
	src/getkey.c					\
	src/history.c					\
	src/keycode.c					\
	src/main.c					\
	src/marker.h					\
	src/marker.c					\
	src/minibuf.c					\
	src/window.h					\
	src/term_minibuf.c				\
	src/term_redisplay.c				\
	src/term_curses.c

src_zile_SOURCES =					\
	$(src_zile_base_SOURCE_FILES)			\
	$(BUILT_SOURCES)

src/tbl_funcs.h: $(src_zile_function_SOURCES) build-aux/mkfuncs.pl
	PACKAGE_NAME="$(PACKAGE_NAME)" $(PERL) -I$(srcdir)/build-aux $(srcdir)/build-aux/mkfuncs.pl $(srcdir) $(src_zile_function_SOURCE_FILES)

src/tbl_vars.h src/dotzile.sample: src/tbl_vars.h.in build-aux/mkvars.pl
	PACKAGE="$(PACKAGE)" $(PERL) -I$(srcdir)/build-aux $(srcdir)/build-aux/mkvars.pl $(srcdir)/src/tbl_vars.h.in

DISTCLEANFILES +=					\
	$(BUILT_SOURCES)

check-syntax:
	$(COMPILE) -o /dev/null -S $(CHK_SOURCES)

TESTS_ENVIRONMENT =					\
	srcdir=$(srcdir)/src				\
	builddir=$(builddir)src				\
	$(VALGRIND)

check_PROGRAMS = src/astr

TESTS = $(check_PROGRAMS)

src_astr_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src\"" $(AM_CPPFLAGS)
src_astr_LDADD = $(LDADD) src/memrmem.o

EXTRA_DIST +=						\
	src/dotzile.sample				\
	src/tbl_opts.h.in				\
	src/tbl_vars.h.in
