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

LISP_TESTS =						\
	$(srcdir)/tests/*.el				\
	$(srcdir)/tests/interactive/*.el
LISP_TESTS_ZILE_ONLY =					\
	$(srcdir)/tests/zile-only/*.el

# FIXME: Find a way of making these work too
#	tests/disabled/*.el

LISP_TESTS_OUTPUTS =					\
	$(srcdir)/tests/*.output			\
	$(srcdir)/tests/interactive/*.output		\
	$(srcdir)/tests/zile-only/*.output

LISP_ALL_TESTS =					\
	$(LISP_TESTS)					\
	$(LISP_TESTS_ZILE_ONLY)

EXTRA_DIST +=						\
	$(LISP_ALL_TESTS)				\
	$(LISP_TESTS_OUTPUTS)				\
	tests/test.input				\
	tests/run-lisp-tests.pl				\
	tests/test-bad-argument

LISP_TESTS_ENVIRONMENT =				\
	abs_srcdir=$(abs_srcdir)			\
	srcdir=$(srcdir)				\
	VALGRIND="$(VALGRIND)"

check-local: src/zile
	echo $(LISP_TESTS) | $(LISP_TESTS_ENVIRONMENT) EMACSPROG="$(EMACSPROG)" xargs $(RUNLISPTESTS)
	echo $(LISP_TESTS_ZILE_ONLY) | $(LISP_TESTS_ENVIRONMENT) EMACSPROG= xargs $(RUNLISPTESTS)
