/* Default keyboard bindings.

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Reuben Thomas.
   Copyright (c) 2004 David Capello.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

X1 (back_to_indentation, "\\M-m")
X2 (backward_char, "\\C-b", "\\LEFT")
X1 (backward_delete_char, "\\BACKSPACE")
X1 (backward_kill_word, "\\M-\\BACKSPACE")
X1 (backward_paragraph, "\\M-{")
X1 (backward_sexp, "\\C-\\M-b")
X1 (backward_word, "\\M-b")
X1 (beginning_of_buffer, "\\M-<")
X2 (beginning_of_line, "\\C-a", "\\HOME")
X1 (call_last_kbd_macro, "\\C-xe")
X1 (capitalize_word, "\\M-c")
X1 (copy_region_as_kill, "\\M-w")
X2 (copy_to_register, "\\C-xrx", "\\C-xrs")
X1 (delete_blank_lines, "\\C-x\\C-o")
X2 (delete_char, "\\C-d", "\\DELETE")
X1 (delete_horizontal_space, "\\M-\\\\")
X1 (delete_other_windows, "\\C-x1")
X1 (delete_window, "\\C-x0")
X2 (describe_bindings, "\\C-hb", "\\F1b")
X2 (describe_function, "\\C-hf", "\\F1f")
X2 (describe_key, "\\C-hk", "\\F1k")
X2 (describe_variable, "\\C-hv", "\\F1v")
X1 (downcase_region, "\\C-x\\C-l")
X1 (downcase_word, "\\M-l")
X1 (end_kbd_macro, "\\C-x)")
X1 (end_of_buffer, "\\M->")
X2 (end_of_line, "\\C-e", "\\END")
X1 (enlarge_window, "\\C-x^")
X1 (exchange_point_and_mark, "\\C-x\\C-x")
X1 (execute_extended_command, "\\M-x")
X1 (fill_paragraph, "\\M-q")
X1 (find_alternate_file, "\\C-x\\C-v")
X1 (find_file, "\\C-x\\C-f")
X1 (find_file_read_only, "\\C-x\\C-r")
X2 (forward_char, "\\C-f", "\\RIGHT")
X1 (forward_paragraph, "\\M-}")
X1 (forward_sexp, "\\C-\\M-f")
X1 (forward_word, "\\M-f")
X2 (goto_line, "\\M-gg", "\\M-g\\M-g")
X1 (indent_for_tab_command, "\\TAB")
X1 (insert_file, "\\C-xi")
X2 (insert_register, "\\C-xrg", "\\C-xri")
X1 (isearch_backward, "\\C-r")
X1 (isearch_backward_regexp, "\\C-\\M-r")
X1 (isearch_forward, "\\C-s")
X1 (isearch_forward_regexp, "\\C-\\M-s")
X1 (just_one_space, "\\M-\\SPC")
X1 (keyboard_quit, "\\C-g")
X1 (kill_buffer, "\\C-xk")
X1 (kill_line, "\\C-k")
X1 (kill_region, "\\C-w")
X1 (kill_sexp, "\\C-\\M-k")
X1 (kill_word, "\\M-d")
X1 (list_buffers, "\\C-x\\C-b")
X1 (mark_paragraph, "\\M-h")
X1 (mark_sexp, "\\C-\\M-@")
X1 (mark_whole_buffer, "\\C-xh")
X1 (mark_word, "\\M-@")
X1 (newline, "\\RET")
X1 (newline_and_indent, "\\C-j")
X2 (next_line, "\\C-n", "\\DOWN")
X1 (open_line, "\\C-o")
X1 (other_window, "\\C-xo")
X1 (overwrite_mode, "\\INSERT")
X2 (previous_line, "\\C-p", "\\UP")
X1 (query_replace, "\\M-%")
X1 (quoted_insert, "\\C-q")
X1 (recenter, "\\C-l")
X1 (save_buffer, "\\C-x\\C-s")
X1 (save_buffers_kill_zile, "\\C-x\\C-c")
X1 (save_some_buffers, "\\C-xs")
X2 (scroll_down, "\\M-v", "\\PRIOR")
X2 (scroll_up, "\\C-v", "\\NEXT")
X1 (set_fill_column, "\\C-xf")
X1 (set_mark_command, "\\C-@")
X1 (shell_command, "\\M-!")
X1 (shell_command_on_region, "\\M-|")
X1 (split_window, "\\C-x2")
X1 (start_kbd_macro, "\\C-x(")
X2 (suspend_zile, "\\C-x\\C-z", "\\C-z")
X1 (switch_to_buffer, "\\C-xb")
X1 (tab_to_tab_stop, "\\M-i")
X1 (toggle_read_only, "\\C-x\\C-q")
X1 (transpose_chars, "\\C-t")
X1 (transpose_lines, "\\C-x\\C-t")
X1 (transpose_sexps, "\\C-\\M-t")
X1 (transpose_words, "\\M-t")
X2 (undo, "\\C-xu", "\\C-_")
X1 (universal_argument, "\\C-u")
X1 (upcase_region, "\\C-x\\C-u")
X1 (upcase_word, "\\M-u")
X2 (view_zile_FAQ, "\\C-h\\C-f", "\\F1\\C-f")
X2 (where_is, "\\C-hw", "\\F1w")
X1 (write_file, "\\C-x\\C-w")
X1 (yank, "\\C-y")
