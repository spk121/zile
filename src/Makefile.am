# Source Makefile.am
#
# Copyright (c) 1997-2011 Free Software Foundation, Inc.
# Copyright (c) 2012 Michael L. Gran
#
# This file is part of Michael Gran's unofficial fork of GNU Zile.
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

# doc_DATA += src/dotzile.sample
AM_CFLAGS = $(WERROR_CFLAGS) $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(builddir)/src -I$(srcdir)/src -I$(builddir)/lib -I$(srcdir)/lib -DPATH_DOCDIR="\"$(docdir)\"" $(LIBGC_CPPFLAGS) $(GUILE_CFLAGS)
LDADD = $(builddir)/lib/libzile.a $(LIB_ACL) $(LIB_EACCESS) $(LIBINTL) $(CURSES_LIB) $(GUILE_LDFLAGS)

BUILT_SOURCES =					\
	src/basic.x					\
	src/buffer.x					\
	src/bind.x					\
	src/eval.x					\
	src/file.x					\
	src/guile.x                                     \
	src/funcs.x					\
	src/help.x					\
	src/killring.x					\
	src/line.x					\
	src/lisp.x					\
	src/macro.x					\
	src/redisplay.x				\
	src/registers.x				\
	src/search.x					\
	src/undo.x					\
	src/variables.x				\
	src/window.x

bin_PROGRAMS = src/zile-on-guile

src_zile_on_guile_LDADD = $(LDADD) $(LIBOBJS)

src_zile_on_guile_function_SOURCE_FILES =		\
	src/basic.c					\
	src/bind.c					\
	src/buffer.c					\
	src/eval.c					\
	src/file.c					\
	src/funcs.c					\
	src/guile.c					\
	src/help.c					\
	src/killring.c					\
	src/line.c					\
	src/lisp.c					\
	src/macro.c					\
	src/redisplay.c				\
	src/registers.c				\
	src/search.c					\
	src/undo.c					\
	src/variables.c				\
	src/window.c

src_zile_on_guile_base_SOURCE_FILES =			\
	$(src_zile_on_guile_function_SOURCE_FILES)	\
	src/memrmem.h					\
	src/memrmem.c					\
	src/astr.c					\
	src/astr.h					\
	src/estr.c					\
	src/estr.h					\
	src/extern.h					\
	src/main.h					\
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

src_zile_on_guile_SOURCES =					\
	$(src_zile_on_guile_base_SOURCE_FILES)			\
	$(BUILT_SOURCES)



snarfcppopts = -I. $(AM_CPPFLAGS) $(AM_CFLAGS)

.c.x:
	guile-snarf -o $@ $< $(snarfcppopts)

src/basic.x:	src/basic.c
src/buffer.x:	src/buffer.c
src/bind.x:	src/bind.c
src/eval.x:	src/eval.c
src/file.x:	src/file.c
src/funcs.x:	src/funcs.c
src/guile.x:    src/guile.c
src/help.x:	src/help.c
src/killring.x:	src/killring.c
src/line.x:	src/line.c
src/lisp.x:	src/lisp.c
src/macro.x:	src/macro.c
src/redisplay.x: src/redisplay.c
src/registers.x: src/registers.c
src/search.x:	src/search.c
src/undo.x:	src/undo.c
src/variables.x: src/variables.c
src/window.x:	src/window.c



DISTCLEANFILES +=					\
	$(BUILT_SOURCES)

check-syntax:
	$(COMPILE) -o /dev/null -S $(CHK_SOURCES)

TESTS_ENVIRONMENT =					\
	srcdir=$(srcdir)/src				\
	builddir=$(builddir)src			\
	$(VALGRIND)

check_PROGRAMS = src/astr

TESTS = $(check_PROGRAMS)

src_astr_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src\"" $(AM_CPPFLAGS)
src_astr_LDADD = $(LDADD) src/memrmem.o

EXTRA_DIST +=						\
	src/tbl_opts.h.in


guilemodule_DATA = src/zile.scm
ETAGS_ARGS = src/zile.scm
