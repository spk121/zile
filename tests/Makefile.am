# Tests Makefile.am
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

RUNLISPTESTS = $(PERL) $(srcdir)/tests/run-lisp-tests.pl

TERM ?= vt100

LISP_TESTS =						\
	$(srcdir)/tests/*.el				\
	$(srcdir)/tests/interactive/*.el

LISP_TESTS_ZILE_ONLY =					\
	$(srcdir)/tests/regression/*.el			\
	$(srcdir)/tests/zile-only/*.el			\
	$(srcdir)/tests/zile-only/interactive/*.el

LISP_TESTS_ZILE_ONLY_FIXED_SCREEN =			\
	$(srcdir)/tests/zile-only/fixed-screen/*.el

LISP_TESTS_OUTPUTS =					\
	$(srcdir)/tests/*.output			\
	$(srcdir)/tests/interactive/*.output		\
	$(srcdir)/tests/regression/*.output		\
	$(srcdir)/tests/zile-only/*.output		\
	$(srcdir)/tests/zile-only/interactive/*.output	\
	$(srcdir)/tests/zile-only/fixed-screen/*.output

LISP_ALL_TESTS =					\
	$(LISP_TESTS)					\
	$(LISP_TESTS_ZILE_ONLY)				\
	$(LISP_TESTS_ZILE_ONLY_FIXED_SCREEN)

LISP_TESTS_ENVIRONMENT =				\
	abs_srcdir=$(abs_srcdir)			\
	srcdir=$(srcdir)				\
	TERM=$(TERM)					\
	VALGRIND="$(VALGRIND)"

EXTRA_DIST +=						\
	$(LISP_ALL_TESTS)				\
	$(LISP_TESTS_OUTPUTS)				\
	tests/test.input				\
	tests/run-lisp-tests.pl

check-local: $(builddir)/src/zile$(EXEEXT)
	echo $(LISP_TESTS) | $(LISP_TESTS_ENVIRONMENT) EMACSPROG="$(EMACSPROG)" xargs $(RUNLISPTESTS)
	echo $(LISP_TESTS_ZILE_ONLY) | $(LISP_TESTS_ENVIRONMENT) EMACSPROG= xargs $(RUNLISPTESTS)
	echo $(LISP_TESTS_ZILE_ONLY_FIXED_SCREEN) | COLUMNS=80 LINES=24 $(LISP_TESTS_ENVIRONMENT) EMACSPROG= xargs $(RUNLISPTESTS)
	$(LISP_TESTS_ENVIRONMENT) $(builddir)/src/zile$(EXEEXT) --unknown-flag --load $(srcdir)/tests/quit.el
