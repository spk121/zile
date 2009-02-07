# Source Makefile.am
#
# Copyright (c) 2008, 2009 Free Software Foundation, Inc.
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

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib -DPATH_DATA="\"$(pkgdatadir)\""
LDADD = $(top_builddir)/lib/libgnu.a $(LIB_EACCESS) $(LIBINTL) $(CURSES_LIB)

bin_PROGRAMS = zile

noinst_PROGRAMS = mkfuncs

tbl_funcs.h: $(zile_SOURCES) mkfuncs$(EXEEXT_FOR_BUILD)
	./mkfuncs $(zile_SOURCES) > tbl_funcs.h

zile_LDADD = $(LDADD) $(LIBOBJS)

# Use absolute paths in list below for mkfuncs
zile_SOURCES =					\
	$(top_srcdir)/src/astr.c		\
	$(top_srcdir)/src/astr.h		\
	$(top_srcdir)/src/xalloc_extra.c	\
	$(top_srcdir)/src/xalloc_extra.h	\
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
	$(top_srcdir)/src/term_curses.c

ALLSOURCES = $(zile_SOURCES) mkfuncs.c

BUILT_SOURCES = tbl_funcs.h

CLEANFILES = $(BUILT_SOURCES)

loc:
	sloccount $(ALLSOURCES)

TESTS_ENVIRONMENT = builddir=$(builddir)
# FIXME: Run tests under valgrind
#TESTS_ENVIRONMENT = $(VALGRIND)

SCRIPT_TESTS =						\
	run-lisp-tests					\
	test-bad-argument

TESTS = $(check_PROGRAMS) $(SCRIPT_TESTS)

check_PROGRAMS = astr

astr_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src\"" $(AM_CPPFLAGS)
astr_LDADD = xalloc_extra.o $(LDADD)

LISP_TESTS =						\
	lisp-tests/backward_char.el			\
	lisp-tests/backward_delete_char.el		\
	lisp-tests/backward_kill_word.el		\
	lisp-tests/backward_paragraph.el		\
	lisp-tests/backward_sexp.el			\
	lisp-tests/backward_word.el			\
	lisp-tests/beginning_of_buffer.el		\
	lisp-tests/beginning_of_line.el			\
	lisp-tests/call_last_kbd_macro.el		\
	lisp-tests/capitalize_word.el			\
	lisp-tests/copy_region_as_kill.el		\
	lisp-tests/copy_to_register.el			\
	lisp-tests/delete_blank_lines.el		\
	lisp-tests/delete_char.el			\
	lisp-tests/delete_horizontal_space.el		\
	lisp-tests/delete_region.el			\
	lisp-tests/describe_bindings.el			\
	lisp-tests/describe_function.el			\
	lisp-tests/describe_key.el			\
	lisp-tests/describe_variable.el			\
	lisp-tests/downcase_region.el			\
	lisp-tests/downcase_word.el			\
	lisp-tests/end_of_buffer.el			\
	lisp-tests/end_of_line.el			\
	lisp-tests/exchange_point_and_mark.el		\
	lisp-tests/execute_extended_command.el		\
	lisp-tests/fill_paragraph.el			\
	lisp-tests/forward_char.el			\
	lisp-tests/forward_line.el			\
	lisp-tests/forward_paragraph.el			\
	lisp-tests/forward_sexp.el			\
	lisp-tests/forward_word.el			\
	lisp-tests/global_set_key.el			\
	lisp-tests/goto_char.el				\
	lisp-tests/goto_line.el				\
	lisp-tests/indent_for_tab_command.el		\
	lisp-tests/indent_relative.el			\
	lisp-tests/insert_buffer.el			\
	lisp-tests/insert_char.el			\
	lisp-tests/insert_file.el			\
	lisp-tests/isearch_backward.el			\
	lisp-tests/isearch_backward_regexp.el		\
	lisp-tests/isearch_forward.el			\
	lisp-tests/isearch_forward_regexp.el		\
	lisp-tests/just_one_space.el			\
	lisp-tests/keyboard_quit.el			\
	lisp-tests/kill_buffer.el			\
	lisp-tests/kill_line.el				\
	lisp-tests/kill_region.el			\
	lisp-tests/kill_sexp.el				\
	lisp-tests/kill_word.el				\
	lisp-tests/list_buffers.el			\
	lisp-tests/list_registers.el			\
	lisp-tests/mark_paragraph.el			\
	lisp-tests/mark_sexp.el				\
	lisp-tests/mark_whole_buffer.el			\
	lisp-tests/mark_word.el				\
	lisp-tests/name_last_kbd_macro.el		\
	lisp-tests/newline.el				\
	lisp-tests/newline_and_indent.el		\
	lisp-tests/next_line.el				\
	lisp-tests/open_line.el				\
	lisp-tests/overwrite_mode.el			\
	lisp-tests/previous_line.el			\
	lisp-tests/query_replace.el			\
	lisp-tests/quoted_insert.el			\
	lisp-tests/save_some_buffers.el			\
	lisp-tests/scroll_down.el			\
	lisp-tests/scroll_up.el				\
	lisp-tests/search_backward.el			\
	lisp-tests/search_backward_regexp.el		\
	lisp-tests/search_forward.el			\
	lisp-tests/search_forward_regexp.el		\
	lisp-tests/set_fill_column.el			\
	lisp-tests/set_variable.el			\
	lisp-tests/shell_command.el			\
	lisp-tests/shell_command_on_region.el		\
	lisp-tests/switch_to_buffer.el			\
	lisp-tests/tab_to_tab_stop.el			\
	lisp-tests/tabify.el				\
	lisp-tests/toggle_read_only.el			\
	lisp-tests/transient_mark_mode.el		\
	lisp-tests/transpose_chars.el			\
	lisp-tests/transpose_lines.el			\
	lisp-tests/transpose_sexps.el			\
	lisp-tests/transpose_words.el			\
	lisp-tests/undo.el				\
	lisp-tests/universal_argument.el		\
	lisp-tests/untabify.el				\
	lisp-tests/upcase_region.el			\
	lisp-tests/upcase_word.el			\
	lisp-tests/yank.el				\
	lisp-tests/setq_nonexistent_variable.el

EXTRA_DIST = $(LISP_TESTS) $(SCRIPT_TESTS)		\
	quit.el						\
	lisp-tests/backward_char.output			\
	lisp-tests/backward_delete_char.output		\
	lisp-tests/backward_kill_word.output		\
	lisp-tests/backward_paragraph.output		\
	lisp-tests/backward_sexp.output			\
	lisp-tests/backward_word.output			\
	lisp-tests/beginning_of_buffer.output		\
	lisp-tests/beginning_of_line.output		\
	lisp-tests/call_last_kbd_macro.output		\
	lisp-tests/capitalize_word.output		\
	lisp-tests/copy_region_as_kill.output		\
	lisp-tests/copy_to_register.output		\
	lisp-tests/delete_blank_lines.output		\
	lisp-tests/delete_char.output			\
	lisp-tests/delete_horizontal_space.output	\
	lisp-tests/delete_region.output			\
	lisp-tests/describe_bindings.output		\
	lisp-tests/describe_function.output		\
	lisp-tests/describe_key.output			\
	lisp-tests/describe_variable.output		\
	lisp-tests/downcase_region.output		\
	lisp-tests/downcase_word.output			\
	lisp-tests/end_of_buffer.output			\
	lisp-tests/end_of_line.output			\
	lisp-tests/exchange_point_and_mark.output	\
	lisp-tests/execute_extended_command.output	\
	lisp-tests/fill_paragraph.output		\
	lisp-tests/forward_char.output			\
	lisp-tests/forward_line.output			\
	lisp-tests/forward_paragraph.output		\
	lisp-tests/forward_sexp.output			\
	lisp-tests/forward_word.output			\
	lisp-tests/global_set_key.output		\
	lisp-tests/goto_char.output			\
	lisp-tests/goto_line.output			\
	lisp-tests/indent_for_tab_command.output	\
	lisp-tests/indent_relative.output		\
	lisp-tests/insert_buffer.output			\
	lisp-tests/insert_char.output			\
	lisp-tests/insert_file.output			\
	lisp-tests/isearch_backward.output		\
	lisp-tests/isearch_backward_regexp.output	\
	lisp-tests/isearch_forward.output		\
	lisp-tests/isearch_forward_regexp.output	\
	lisp-tests/just_one_space.output		\
	lisp-tests/keyboard_quit.output			\
	lisp-tests/kill_buffer.output			\
	lisp-tests/kill_line.output			\
	lisp-tests/kill_region.output			\
	lisp-tests/kill_sexp.output			\
	lisp-tests/kill_word.output			\
	lisp-tests/list_buffers.output			\
	lisp-tests/list_registers.output		\
	lisp-tests/mark_paragraph.output		\
	lisp-tests/mark_sexp.output			\
	lisp-tests/mark_whole_buffer.output		\
	lisp-tests/mark_word.output			\
	lisp-tests/name_last_kbd_macro.output		\
	lisp-tests/newline.output			\
	lisp-tests/newline_and_indent.output		\
	lisp-tests/next_line.output			\
	lisp-tests/open_line.output			\
	lisp-tests/overwrite_mode.output		\
	lisp-tests/previous_line.output			\
	lisp-tests/query_replace.output			\
	lisp-tests/quoted_insert.output			\
	lisp-tests/save_some_buffers.output		\
	lisp-tests/scroll_down.output			\
	lisp-tests/scroll_up.output			\
	lisp-tests/search_backward.output		\
	lisp-tests/search_backward_regexp.output	\
	lisp-tests/search_forward.output		\
	lisp-tests/search_forward_regexp.output		\
	lisp-tests/set_fill_column.output		\
	lisp-tests/set_variable.output			\
	lisp-tests/shell_command.output			\
	lisp-tests/shell_command_on_region.output	\
	lisp-tests/switch_to_buffer.output		\
	lisp-tests/tab_to_tab_stop.output		\
	lisp-tests/tabify.output			\
	lisp-tests/toggle_read_only.output		\
	lisp-tests/transient_mark_mode.output		\
	lisp-tests/transpose_chars.output		\
	lisp-tests/transpose_lines.output		\
	lisp-tests/transpose_sexps.output		\
	lisp-tests/transpose_words.output		\
	lisp-tests/undo.output				\
	lisp-tests/universal_argument.output		\
	lisp-tests/untabify.output			\
	lisp-tests/upcase_region.output			\
	lisp-tests/upcase_word.output			\
	lisp-tests/yank.output				\
	lisp-tests/setq_nonexistent_variable.output	\
	lisp-tests/zile_test.input
