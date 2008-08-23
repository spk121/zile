# DejaGnu testsuite Makefile.am
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

AUTOMAKE_OPTIONS = dejagnu

EXTRA_DIST =						\
	config/default.exp				\
	lib/zile.exp					\
	zile.test/make-interactive-test			\
	zile.test/backward_char.exp			\
	zile.test/backward_char.output			\
	zile.test/backward_delete_char.exp		\
	zile.test/backward_delete_char.output		\
	zile.test/backward_kill_word.exp		\
	zile.test/backward_kill_word.output		\
	zile.test/backward_paragraph.exp		\
	zile.test/backward_paragraph.output		\
	zile.test/backward_sexp.exp			\
	zile.test/backward_sexp.output			\
	zile.test/backward_word.exp			\
	zile.test/backward_word.output			\
	zile.test/beginning_of_buffer.exp		\
	zile.test/beginning_of_buffer.output		\
	zile.test/beginning_of_line.exp			\
	zile.test/beginning_of_line.output		\
	zile.test/call_last_kbd_macro.exp		\
	zile.test/call_last_kbd_macro.output		\
	zile.test/capitalize_word.exp			\
	zile.test/capitalize_word.output		\
	zile.test/copy_region_as_kill.exp		\
	zile.test/copy_region_as_kill.output		\
	zile.test/copy_to_register.exp			\
	zile.test/copy_to_register.output		\
	zile.test/delete_blank_lines.exp		\
	zile.test/delete_blank_lines.output		\
	zile.test/delete_char.exp			\
	zile.test/delete_char.output			\
	zile.test/delete_horizontal_space.exp		\
	zile.test/delete_horizontal_space.output	\
	zile.test/delete_region.exp			\
	zile.test/delete_region.output			\
	zile.test/describe_bindings.exp			\
	zile.test/describe_bindings.output		\
	zile.test/describe_function.exp			\
	zile.test/describe_function.output		\
	zile.test/describe_key.exp			\
	zile.test/describe_key.output			\
	zile.test/describe_variable.exp			\
	zile.test/describe_variable.output		\
	zile.test/downcase_region.exp			\
	zile.test/downcase_region.output		\
	zile.test/downcase_word.exp			\
	zile.test/downcase_word.output			\
	zile.test/end_of_buffer.exp			\
	zile.test/end_of_buffer.output			\
	zile.test/end_of_line.exp			\
	zile.test/end_of_line.output			\
	zile.test/exchange_point_and_mark.exp		\
	zile.test/exchange_point_and_mark.output	\
	zile.test/execute_extended_command.exp		\
	zile.test/execute_extended_command.output	\
	zile.test/fill_paragraph.exp			\
	zile.test/fill_paragraph.output			\
	zile.test/find_alternate_file.exp		\
	zile.test/find_alternate_file.output		\
	zile.test/find_file.exp				\
	zile.test/find_file.output			\
	zile.test/find_file_read_only.exp		\
	zile.test/find_file_read_only.output		\
	zile.test/forward_char.exp			\
	zile.test/forward_char.output			\
	zile.test/forward_line.exp			\
	zile.test/forward_line.output			\
	zile.test/forward_paragraph.exp			\
	zile.test/forward_paragraph.output		\
	zile.test/forward_sexp.exp			\
	zile.test/forward_sexp.output			\
	zile.test/forward_word.exp			\
	zile.test/forward_word.output			\
	zile.test/global_set_key.exp			\
	zile.test/global_set_key.output			\
	zile.test/goto_char.exp				\
	zile.test/goto_char.output			\
	zile.test/goto_line.exp				\
	zile.test/goto_line.output			\
	zile.test/indent_for_tab_command.exp		\
	zile.test/indent_for_tab_command.output		\
	zile.test/indent_relative.exp			\
	zile.test/indent_relative.output		\
	zile.test/insert_buffer.exp			\
	zile.test/insert_buffer.output			\
	zile.test/insert_file.exp			\
	zile.test/insert_file.output			\
	zile.test/insert_register.exp			\
	zile.test/insert_register.output		\
	zile.test/isearch_backward.exp			\
	zile.test/isearch_backward.output		\
	zile.test/isearch_backward_regexp.exp		\
	zile.test/isearch_backward_regexp.output	\
	zile.test/isearch_forward.exp			\
	zile.test/isearch_forward.output		\
	zile.test/isearch_forward_regexp.exp		\
	zile.test/isearch_forward_regexp.output		\
	zile.test/just_one_space.exp			\
	zile.test/just_one_space.output			\
	zile.test/keyboard_quit.exp			\
	zile.test/keyboard_quit.output			\
	zile.test/kill_buffer.exp			\
	zile.test/kill_buffer.output			\
	zile.test/kill_line.exp				\
	zile.test/kill_line.output			\
	zile.test/kill_region.exp			\
	zile.test/kill_region.output			\
	zile.test/kill_sexp.exp				\
	zile.test/kill_sexp.output			\
	zile.test/kill_word.exp				\
	zile.test/kill_word.output			\
	zile.test/list_buffers.exp			\
	zile.test/list_buffers.output			\
	zile.test/list_registers.exp			\
	zile.test/list_registers.output			\
	zile.test/mark_paragraph.exp			\
	zile.test/mark_paragraph.output			\
	zile.test/mark_sexp.exp				\
	zile.test/mark_sexp.output			\
	zile.test/mark_whole_buffer.exp			\
	zile.test/mark_whole_buffer.output		\
	zile.test/mark_word.exp				\
	zile.test/mark_word.output			\
	zile.test/name_last_kbd_macro.exp		\
	zile.test/name_last_kbd_macro.output		\
	zile.test/newline.exp				\
	zile.test/newline.output			\
	zile.test/newline_and_indent.exp		\
	zile.test/newline_and_indent.output		\
	zile.test/next_line.exp				\
	zile.test/next_line.output			\
	zile.test/open_line.exp				\
	zile.test/open_line.output			\
	zile.test/overwrite_mode.exp			\
	zile.test/overwrite_mode.output			\
	zile.test/previous_line.exp			\
	zile.test/previous_line.output			\
	zile.test/query_replace.exp			\
	zile.test/query_replace.output			\
	zile.test/quoted_insert.exp			\
	zile.test/quoted_insert.output			\
	zile.test/recenter.exp				\
	zile.test/recenter.output			\
	zile.test/replace_string.exp			\
	zile.test/replace_string.output			\
	zile.test/save_buffer.exp			\
	zile.test/save_buffer.output			\
	zile.test/save_buffers_kill_zile.exp		\
	zile.test/save_buffers_kill_zile.output		\
	zile.test/save_some_buffers.exp			\
	zile.test/save_some_buffers.output		\
	zile.test/scroll_down.exp			\
	zile.test/scroll_down.output			\
	zile.test/scroll_up.exp				\
	zile.test/scroll_up.output			\
	zile.test/search_backward.exp			\
	zile.test/search_backward.output		\
	zile.test/search_backward_regexp.exp		\
	zile.test/search_backward_regexp.output		\
	zile.test/search_forward.exp			\
	zile.test/search_forward.output			\
	zile.test/search_forward_regexp.exp		\
	zile.test/search_forward_regexp.output		\
	zile.test/self_insert_command.exp		\
	zile.test/self_insert_command.output		\
	zile.test/set_fill_column.exp			\
	zile.test/set_fill_column.output		\
	zile.test/set_mark_command.exp			\
	zile.test/set_mark_command.output		\
	zile.test/set_variable.exp			\
	zile.test/set_variable.output			\
	zile.test/shell_command.exp			\
	zile.test/shell_command.output			\
	zile.test/shell_command_on_region.exp		\
	zile.test/shell_command_on_region.output	\
	zile.test/switch_to_buffer.exp			\
	zile.test/switch_to_buffer.output		\
	zile.test/tab_to_tab_stop.exp			\
	zile.test/tab_to_tab_stop.output		\
	zile.test/tabify.exp				\
	zile.test/tabify.output				\
	zile.test/toggle_read_only.exp			\
	zile.test/toggle_read_only.output		\
	zile.test/transient_mark_mode.exp		\
	zile.test/transient_mark_mode.output		\
	zile.test/transpose_chars.exp			\
	zile.test/transpose_chars.output		\
	zile.test/transpose_lines.exp			\
	zile.test/transpose_lines.output		\
	zile.test/transpose_sexps.exp			\
	zile.test/transpose_sexps.output		\
	zile.test/transpose_words.exp			\
	zile.test/transpose_words.output		\
	zile.test/undo.exp				\
	zile.test/undo.output				\
	zile.test/universal_argument.exp		\
	zile.test/universal_argument.output		\
	zile.test/untabify.exp				\
	zile.test/untabify.output			\
	zile.test/upcase_region.exp			\
	zile.test/upcase_region.output			\
	zile.test/upcase_word.exp			\
	zile.test/upcase_word.output			\
	zile.test/write_file.exp			\
	zile.test/write_file.output			\
	zile.test/yank.exp				\
	zile.test/yank.output				\
	zile.test/zile_test.input
