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

AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib -DPATH_DATA="\"$(pkgdatadir)\"" $(LIBGC_CPPFLAGS)
LDADD = $(top_builddir)/lib/libgnu.a $(LIB_EACCESS) $(LIBINTL) $(CURSES_LIB)

bin_PROGRAMS = zile

zile_LDADD = $(LDADD) $(LIBOBJS)

zile_function_SOURCES =					\
	basic.c						\
	bind.c						\
	buffer.c					\
	eval.c						\
	file.c						\
	funcs.c						\
	help.c						\
	killring.c					\
	line.c						\
	lisp.c						\
	macro.c						\
	redisplay.c					\
	registers.c					\
	search.c					\
	undo.c						\
	variables.c					\
	window.c

zile_base_SOURCES =					\
	$(zile_function_SOURCES)			\
	memrmem.h					\
	memrmem.c					\
	astr.c						\
	astr.h						\
	estr.c						\
	estr.h						\
	extern.h					\
	main.h						\
	tbl_vars.h					\
	lists.c						\
	lists.h						\
	buffer.h					\
	completion.h					\
	completion.c					\
	editfns.c					\
	getkey.c					\
	history.c					\
	keycode.c					\
	main.c						\
	marker.h					\
	marker.c					\
	minibuf.c					\
	point.c						\
	window.h					\
	term_minibuf.c					\
	term_redisplay.c				\
	term_curses.c

zile_SOURCES =						\
	$(zile_base_SOURCES)				\
	tbl_funcs.h					\
	tbl_opts.h.in

PERL_BUILDTIME =					\
	Zile.pm						\
	mkfuncs.pl					\
	mkvars.pl

PRODUCTIONSOURCES =					\
	$(zile_base_SOURCES)				\
	tbl_opts.h.in					\
	$(PERL_BUILDTIME)				\
	Makefile.am ../Makefile.am ../configure.ac	\
	*.el

$(zile_OBJECTS): tbl_funcs.h

tbl_funcs.h: $(zile_function_SOURCES) mkfuncs.pl
	PACKAGE_NAME="$(PACKAGE_NAME)" $(PERL) -I$(srcdir) $(srcdir)/mkfuncs.pl $(srcdir) $(zile_function_SOURCES)

tbl_vars.h: tbl_vars.h.in mkvars.pl
	PACKAGE="$(PACKAGE)" $(PERL) -I$(srcdir) $(srcdir)/mkvars.pl $(srcdir)/tbl_vars.h.in

edit = sed \
	-e 's|@pkgdatadir[@]|$(pkgdatadir)|g'

zile.1: zile.1.in Makefile
	rm -f $@ $@.tmp
	$(edit) '$(srcdir)/$@.in' >$@.tmp
	mv $@.tmp $@

zile.1.in: main.c man-extras zile-help2man-wrapper
	builddir=$(builddir) $(HELP2MAN) --output=$@ --no-info --name="Zile Is Lossy Emacs" --include $(srcdir)/man-extras $(srcdir)/zile-help2man-wrapper

dist_pkgdata_DATA = dotzile-extra.el

dist_man_MANS = zile.1

DISTCLEANFILES =					\
	tbl_funcs.h					\
	tbl_vars.h					\
	zile.1

CLOC = cloc --force-lang="C",h --force-lang="C",in

TESTSOURCES = $(LISP_TESTS) $(LISP_TESTS_ZILE_ONLY)

loc:
	$(CLOC) $(PRODUCTIONSOURCES)

loc-all:
	$(CLOC) $(PRODUCTIONSOURCES) $(TESTSOURCES)

check-syntax:
	$(COMPILE) -o /dev/null -S $(CHK_SOURCES)

TESTS_VARS = srcdir=$(srcdir) builddir=$(builddir)
TESTS_ENVIRONMENT = $(TESTS_VARS) $(VALGRIND)

TESTS = $(check_PROGRAMS)

check-local:
	echo $(LISP_TESTS) | $(TESTS_VARS) EMACSPROG="$(EMACSPROG)" VALGRIND="$(VALGRIND)" xargs $(srcdir)/run-lisp-tests
	echo $(LISP_TESTS_ZILE_ONLY) | $(TESTS_VARS) EMACSPROG= VALGRIND="$(VALGRIND)" xargs $(srcdir)/run-lisp-tests

check_PROGRAMS = astr

astr_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src\"" $(AM_CPPFLAGS)
astr_LDADD = $(LDADD) memrmem.o

# FIXME: Find a way of making these work
#	lisp-tests/disabled/*.el

# FIXME: Make these work in Emacs too
LISP_TESTS_ZILE_ONLY = $(srcdir)/lisp-tests/zile-only/*.el

LISP_TESTS = $(srcdir)/lisp-tests/*.el

LISP_TESTS_OUTPUTS = $(srcdir)/lisp-tests/*.output $(srcdir)/lisp-tests/zile-only/*.output

EXTRA_DIST =						\
	$(PERL_BUILDTIME)				\
	$(LISP_TESTS)					\
	$(LISP_TESTS_ZILE_ONLY)				\
	$(LISP_TESTS_OUTPUTS)				\
	lisp-tests/test.input				\
	run-lisp-tests					\
	test-bad-argument				\
	quit.el						\
	tbl_opts.h.in					\
	tbl_vars.h.in					\
	man-extras					\
	zile.1.in					\
	zile-help2man-wrapper
