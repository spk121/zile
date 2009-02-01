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

TESTS = $(check_PROGRAMS)				\
	run-lisp-tests					\
	test-bad-argument

check_PROGRAMS = astr

astr_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src\"" $(AM_CPPFLAGS)
astr_LDADD = xalloc_extra.o $(LDADD)

LISP_TESTS =						\
	tests/backward_char.el				\
	tests/backward_delete_char.el			\
	tests/backward_kill_word.el			\
	tests/backward_paragraph.el			\
	tests/backward_sexp.el				\
	tests/backward_word.el				\
	tests/beginning_of_buffer.el			\
	tests/beginning_of_line.el			\
	tests/capitalize_word.el			\
	tests/copy_region_as_kill.el			\
	tests/copy_to_register.el			\
	tests/delete_blank_lines.el			\
	tests/delete_char.el				\
	tests/delete_horizontal_space.el		\
	tests/delete_region.el				\
	tests/describe_bindings.el			\
	tests/describe_function.el			\
	tests/describe_key.el				\
	tests/describe_variable.el			\
	tests/downcase_region.el			\
	tests/downcase_word.el				\
	tests/end_of_buffer.el				\
	tests/end_of_line.el				\
	tests/exchange_point_and_mark.el		\
	tests/fill_paragraph.el				\
	tests/forward_char.el				\
	tests/forward_line.el				\
	tests/forward_paragraph.el			\
	tests/forward_sexp.el				\
	tests/forward_word.el				\
	tests/goto_char.el				\
	tests/goto_line.el				\
	tests/indent_for_tab_command.el			\
	tests/indent_relative.el			\
	tests/insert_buffer.el				\
	tests/insert_char.el				\
	tests/insert_file.el				\
	tests/isearch_backward.el			\
	tests/isearch_backward_regexp.el		\
	tests/isearch_forward.el			\
	tests/isearch_forward_regexp.el			\
	tests/just_one_space.el				\
	tests/kill_buffer.el				\
	tests/kill_line.el				\
	tests/kill_region.el				\
	tests/kill_sexp.el				\
	tests/kill_word.el				\
	tests/list_buffers.el				\
	tests/list_registers.el				\
	tests/mark_paragraph.el				\
	tests/mark_sexp.el				\
	tests/mark_whole_buffer.el			\
	tests/mark_word.el				\
	tests/newline.el				\
	tests/newline_and_indent.el			\
	tests/next_line.el				\
	tests/open_line.el				\
	tests/overwrite_mode.el				\
	tests/previous_line.el				\
	tests/save_some_buffers.el			\
	tests/scroll_down.el				\
	tests/scroll_up.el				\
	tests/search_backward.el			\
	tests/search_backward_regexp.el			\
	tests/search_forward.el				\
	tests/search_forward_regexp.el			\
	tests/set_fill_column.el			\
	tests/set_variable.el				\
	tests/shell_command.el				\
	tests/shell_command_on_region.el		\
	tests/switch_to_buffer.el			\
	tests/tab_to_tab_stop.el			\
	tests/tabify.el					\
	tests/toggle_read_only.el			\
	tests/transient_mark_mode.el			\
	tests/transpose_chars.el			\
	tests/transpose_lines.el			\
	tests/transpose_sexps.el			\
	tests/transpose_words.el			\
	tests/undo.el					\
	tests/untabify.el				\
	tests/upcase_region.el				\
	tests/upcase_word.el				\
	tests/yank.el					\
	tests/setq_nonexistent_variable.el

#	tests/call_last_kbd_macro.el			\
#	tests/execute_extended_command.el		\
#	tests/global_set_key.el				\
#	tests/keyboard_quit.el				\
# 	tests/name_last_kbd_macro.el			\
# 	tests/query_replace.el				\
#	tests/quoted_insert.el				\
# 	tests/universal_argument.el			\
#

EXTRA_DIST = $(LISP_TESTS)				\
	tests/backward_char.output			\
	tests/backward_delete_char.output		\
	tests/backward_kill_word.output			\
	tests/backward_paragraph.output			\
	tests/backward_sexp.output			\
	tests/backward_word.output			\
	tests/beginning_of_buffer.output		\
	tests/beginning_of_line.output			\
	tests/call_last_kbd_macro.output		\
	tests/capitalize_word.output			\
	tests/copy_region_as_kill.output		\
	tests/copy_to_register.output			\
	tests/delete_blank_lines.output			\
	tests/delete_char.output			\
	tests/delete_horizontal_space.output		\
	tests/delete_region.output			\
	tests/describe_bindings.output			\
	tests/describe_function.output			\
	tests/describe_key.output			\
	tests/describe_variable.output			\
	tests/downcase_region.output			\
	tests/downcase_word.output			\
	tests/end_of_buffer.output			\
	tests/end_of_line.output			\
	tests/exchange_point_and_mark.output		\
	tests/execute_extended_command.output		\
	tests/fill_paragraph.output			\
	tests/forward_char.output			\
	tests/forward_line.output			\
	tests/forward_paragraph.output			\
	tests/forward_sexp.output			\
	tests/forward_word.output			\
	tests/global_set_key.output			\
	tests/goto_char.output				\
	tests/goto_line.output				\
	tests/indent_for_tab_command.output		\
	tests/indent_relative.output			\
	tests/insert_buffer.output			\
	tests/insert_char.output			\
	tests/insert_file.output			\
	tests/isearch_backward.output			\
	tests/isearch_backward_regexp.output		\
	tests/isearch_forward.output			\
	tests/isearch_forward_regexp.output		\
	tests/just_one_space.output			\
	tests/keyboard_quit.output			\
	tests/kill_buffer.output			\
	tests/kill_line.output				\
	tests/kill_region.output			\
	tests/kill_sexp.output				\
	tests/kill_word.output				\
	tests/list_buffers.output			\
	tests/list_registers.output			\
	tests/mark_paragraph.output			\
	tests/mark_sexp.output				\
	tests/mark_whole_buffer.output			\
	tests/mark_word.output				\
	tests/name_last_kbd_macro.output		\
	tests/newline.output				\
	tests/newline_and_indent.output			\
	tests/next_line.output				\
	tests/open_line.output				\
	tests/overwrite_mode.output			\
	tests/previous_line.output			\
	tests/query_replace.output			\
	tests/quoted_insert.output			\
	tests/save_some_buffers.output			\
	tests/scroll_down.output			\
	tests/scroll_up.output				\
	tests/search_backward.output			\
	tests/search_backward_regexp.output		\
	tests/search_forward.output			\
	tests/search_forward_regexp.output		\
	tests/set_fill_column.output			\
	tests/set_variable.output			\
	tests/shell_command.output			\
	tests/shell_command_on_region.output		\
	tests/switch_to_buffer.output			\
	tests/tab_to_tab_stop.output			\
	tests/tabify.output				\
	tests/toggle_read_only.output			\
	tests/transient_mark_mode.output		\
	tests/transpose_chars.output			\
	tests/transpose_lines.output			\
	tests/transpose_sexps.output			\
	tests/transpose_words.output			\
	tests/undo.output				\
	tests/universal_argument.output			\
	tests/untabify.output				\
	tests/upcase_region.output			\
	tests/upcase_word.output			\
	tests/yank.output				\
	tests/bad_argument.output			\
	tests/setq_nonexistent_variable.output		\
	tests/zile_test.input
