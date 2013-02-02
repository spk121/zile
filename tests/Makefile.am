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
	VALGRIND="$(VALGRIND)"				\
	GUILE_LOAD_PATH=$(abs_srcdir)/src			\
	GUILE_AUTO_COMPILE=0

EXTRA_DIST +=						\
	$(LISP_ALL_TESTS)				\
	$(LISP_TESTS_OUTPUTS)				\
	tests/test.input				\
	tests/run-lisp-tests.pl				\
	tests/elisp_to_scm.sh

ZILE_GUILE_TESTS = \
	$(srcdir)/tests/backward-char.el \
	$(srcdir)/tests/backward-delete-char.el \
	$(srcdir)/tests/backward-kill-word.el \
	$(srcdir)/tests/backward-paragraph.el \
	$(srcdir)/tests/backward-sexp.el \
	$(srcdir)/tests/backward-word.el \
	$(srcdir)/tests/beginning-of-buffer.el \
	$(srcdir)/tests/beginning-of-line.el \
	$(srcdir)/tests/capitalize-word.el \
	$(srcdir)/tests/copy-region-as-kill.el \
	$(srcdir)/tests/copy-to-register.el \
	$(srcdir)/tests/delete-blank-lines.el \
	$(srcdir)/tests/delete-char.el \
	$(srcdir)/tests/delete-horizontal-space.el \
	$(srcdir)/tests/delete-region.el \
	$(srcdir)/tests/describe-bindings.el \
	$(srcdir)/tests/downcase-region.el \
	$(srcdir)/tests/downcase-word.el \
	$(srcdir)/tests/end-of-buffer.el \
	$(srcdir)/tests/end-of-line.el \
	$(srcdir)/tests/exchange-point-and-mark.el \
	$(srcdir)/tests/find-file.el \
	$(srcdir)/tests/find-file-read-only.el \
	$(srcdir)/tests/forward-char.el \
	$(srcdir)/tests/forward-line.el \
	$(srcdir)/tests/forward-paragraph.el \
	$(srcdir)/tests/forward-sexp.el \
	$(srcdir)/tests/forward-word.el \
	$(srcdir)/tests/goto-char.el \
	$(srcdir)/tests/goto-line.el \
	$(srcdir)/tests/indent-for-tab-command.el \
	$(srcdir)/tests/indent-relative.el \
	$(srcdir)/tests/insert-buffer.el \
	$(srcdir)/tests/insert-char.el \
	$(srcdir)/tests/insert-file.el \
	$(srcdir)/tests/just-one-space.el \
	$(srcdir)/tests/kill-buffer.el \
	$(srcdir)/tests/kill-line.el \
	$(srcdir)/tests/kill-region.el \
	$(srcdir)/tests/kill-sexp.el \
	$(srcdir)/tests/kill-word.el \
	$(srcdir)/tests/list-registers.el \
	$(srcdir)/tests/mark-paragraph.el \
	$(srcdir)/tests/mark-sexp.el \
	$(srcdir)/tests/mark-whole-buffer.el \
	$(srcdir)/tests/mark-word.el \
	$(srcdir)/tests/newline-and-indent.el \
	$(srcdir)/tests/newline.el \
	$(srcdir)/tests/next-line.el \
	$(srcdir)/tests/open-line.el \
	$(srcdir)/tests/previous-line.el \
	$(srcdir)/tests/quit.el \
	$(srcdir)/tests/search-backward.el \
	$(srcdir)/tests/search-backward-regexp.el \
	$(srcdir)/tests/search-forward.el \
	$(srcdir)/tests/search-forward-regexp.el \
	$(srcdir)/tests/shell-command.el \
	$(srcdir)/tests/shell-command-on-region.el \
	$(srcdir)/tests/switch-to-buffer.el \
	$(srcdir)/tests/tab-to-tab-stop.el \
	$(srcdir)/tests/toggle-read-only.el \
	$(srcdir)/tests/transpose-chars.el \
	$(srcdir)/tests/transpose-lines.el \
	$(srcdir)/tests/transpose-sexps.el \
	$(srcdir)/tests/transpose-words.el \
	$(srcdir)/tests/upcase-region.el \
	$(srcdir)/tests/upcase-word.el \
	$(srcdir)/tests/yank.el

ZILE_GUILE_TESTS_NOT_RUN = \
	$(srcdir)/tests/describe-function.el \
	$(srcdir)/tests/describe-variable.el \
	$(srcdir)/tests/describe-key.el \
	$(srcdir)/tests/fill-paragraph.el \
	$(srcdir)/tests/set-fill-column.el \
	$(srcdir)/tests/set-variable.el \
	$(srcdir)/tests/setq-nonexistent-variable.el


check-local: $(builddir)/src/zile-on-guile$(EXEEXT)
	echo $(ZILE_GUILE_TESTS) | $(LISP_TESTS_ENVIRONMENT) EMACSPROG="$(EMACSPROG)" xargs $(RUNLISPTESTS)
	$(LISP_TESTS_ENVIRONMENT) $(builddir)/src/zile-on-guile$(EXEEXT) --unknown-flag --load $(srcdir)/tests/quit.el

#echo $(LISP_TESTS_ZILE_ONLY) | $(LISP_TESTS_ENVIRONMENT) EMACSPROG= xargs $(RUNLISPTESTS)
#echo $(LISP_TESTS_ZILE_ONLY_FIXED_SCREEN) | COLUMNS=80 LINES=24 $(LISP_TESTS_ENVIRONMENT) EMACSPROG= xargs $(RUNLISPTESTS)
