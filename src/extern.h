/* Global function prototypes

   Copyright (c) 1997-2012 Free Software Foundation, Inc.

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

#include "gl_xlist.h"
#include <libguile.h>

/* basic.c ---------------------------------------------------------------- */
SCM G_beginning_of_line (void);
SCM G_end_of_line (void);
size_t get_goalc_bp (Buffer * bp, size_t o);
size_t get_goalc (void);
bool previous_line (void);
bool next_line (void);
SCM G_previous_line (SCM gn);
SCM G_next_line (SCM gn);
SCM G_goto_char (SCM gn);
SCM G_goto_line (SCM gn);
SCM G_beginning_of_buffer (void);
SCM G_end_of_buffer (void);
SCM G_backward_char (SCM gn);
SCM G_forward_char (SCM gn);
SCM G_scroll_down (SCM gn);
SCM G_scroll_up (SCM gn);
void init_guile_basic_procedures (void);

/* bind.c ----------------------------------------------------------------- */
_GL_ATTRIBUTE_PURE SCM last_command (void);
void set_this_command (SCM cmd);
size_t do_binding_completion (astr as);
gl_list_t get_key_sequence (void);
SCM get_function_by_keys (gl_list_t keys);
SCM G_self_insert_command (SCM gn);
SCM last_command (void);
void set_this_command (SCM cmd);
SCM call_command (SCM f, int uniarg, bool uniflag);
void get_and_run_command (void);
void init_default_bindings (void);
SCM G_set_key (SCM gkeystr, SCM gname);
SCM G_where_is (SCM sym);
SCM G_describe_bindings (void);
SCM G_key_map (void);
SCM G_mark_word (SCM n);
SCM G_mark_sexp (SCM n);
SCM G_forward_line (SCM n);
void init_guile_bind_procedures (void);

/* buffer.c --------------------------------------------------------------- */
int insert_char (int c);
bool delete_char (void);
bool replace_estr (size_t del, estr es);
bool insert_estr (estr as);
#define FIELD(ty, field)                                \
  ty get_buffer_ ## field (const Buffer *bp);           \
  void set_buffer_ ## field (Buffer *bp, ty field);
#define FIELD_STR(field)                                                \
  char *get_buffer_ ## field (const Buffer *cp);                        \
  void set_buffer_ ## field (Buffer *cp, const char *field);
#include "buffer.h"
#undef FIELD
#undef FIELD_STR
void set_buffer_text (Buffer *bp, estr es);
castr get_buffer_pre_point (Buffer *bp);
castr get_buffer_post_point (Buffer *bp);
_GL_ATTRIBUTE_PURE size_t get_buffer_pt (Buffer *bp);
_GL_ATTRIBUTE_PURE size_t get_buffer_size (Buffer * bp);
_GL_ATTRIBUTE_PURE const char *get_buffer_eol (Buffer *bp);
_GL_ATTRIBUTE_PURE size_t buffer_prev_line (Buffer *bp, size_t o);
_GL_ATTRIBUTE_PURE size_t buffer_next_line (Buffer *bp, size_t o);
_GL_ATTRIBUTE_PURE size_t buffer_start_of_line (Buffer *bp, size_t o);
_GL_ATTRIBUTE_PURE size_t buffer_end_of_line (Buffer *bp, size_t o);
_GL_ATTRIBUTE_PURE size_t buffer_line_len (Buffer *bp, size_t o);
_GL_ATTRIBUTE_CONST size_t get_region_size (const Region r);
_GL_ATTRIBUTE_PURE size_t get_buffer_line_o (Buffer *bp);
_GL_ATTRIBUTE_PURE char get_buffer_char (Buffer *bp, size_t o);
void free_buffer (Buffer * bp);
void init_buffer (Buffer * bp);
void insert_buffer (Buffer * bp);
Buffer * buffer_new (void);
_GL_ATTRIBUTE_PURE const char *get_buffer_filename_or_name (Buffer * bp);
void set_buffer_names (Buffer * bp, const char *filename);
_GL_ATTRIBUTE_PURE Buffer * find_buffer (const char *name);
void switch_to_buffer (Buffer * bp);
int warn_if_no_mark (void);
int warn_if_readonly_buffer (void);
_GL_ATTRIBUTE_CONST Region region_new (size_t o1, size_t o2);
Region calculate_the_region (void);
bool delete_region (const Region r);
_GL_ATTRIBUTE_CONST bool in_region (size_t o, size_t x, Region r);
void set_temporary_buffer (Buffer * bp);
void activate_mark (void);
void deactivate_mark (void);
size_t tab_width (Buffer * bp);
estr get_buffer_region (Buffer *bp, Region r);
Buffer *create_auto_buffer (const char *name);
Buffer *create_scratch_buffer (void);
void kill_buffer (Buffer * kill_bp);
Completion *make_buffer_completion (void);
bool check_modified_buffer (Buffer * bp);
bool move_char (int dir);
bool move_line (int n);
_GL_ATTRIBUTE_PURE size_t offset_to_line (Buffer *bp, size_t offset);
void goto_offset (size_t o);
void init_guile_buffer_procedures (void);

/* completion.c ----------------------------------------------------------- */
#define FIELD(ty, field)                                        \
  ty get_completion_ ## field (const Completion *cp);           \
  void set_completion_ ## field (Completion *cp, ty field);
#define FIELD_STR(field)                                               \
  char *get_completion_ ## field (const Completion *cp);               \
  void set_completion_ ## field (Completion *cp, const char *field);
#include "completion.h"
#undef FIELD
#undef FIELD_STR
_GL_ATTRIBUTE_PURE int completion_strcmp (const void *p1, const void *p2);
Completion *completion_new (int fileflag);
void completion_scroll_up (void);
void completion_scroll_down (void);
int completion_try (Completion * cp, astr search, int popup_when_complete);

/* editfns.c -------------------------------------------------------------- */
_GL_ATTRIBUTE_PURE bool is_empty_line (void);
_GL_ATTRIBUTE_PURE bool is_blank_line (void);
_GL_ATTRIBUTE_PURE int following_char (void);
_GL_ATTRIBUTE_PURE int preceding_char (void);
_GL_ATTRIBUTE_PURE bool bobp (void);
_GL_ATTRIBUTE_PURE bool eobp (void);
_GL_ATTRIBUTE_PURE bool bolp (void);
_GL_ATTRIBUTE_PURE bool eolp (void);
void ding (void);

/* eval.c ----------------------------------------------------------------- */
SCM execute_with_uniarg (bool undo, int uniarg, bool (*forward) (void),
                         bool (*backward) (void));
SCM move_with_uniarg (int uniarg, bool (*move) (int dir));
SCM execute_function (const char *name, int uniarg, bool is_uniarg);
#if 0
_GL_ATTRIBUTE_PURE  SCM get_function (const char *name);
_GL_ATTRIBUTE_PURE  const char *get_function_doc (SCM sym);
_GL_ATTRIBUTE_PURE  int get_function_interactive (SCM sym);
_GL_ATTRIBUTE_PURE  const char *get_function_name (SCM p);
#endif
castr minibuf_read_function_name (const char *fmt, ...);
void init_eval (void);
void init_guile_eval_procedures (void);

/* file.c ----------------------------------------------------------------- */
int exist_file (const char *filename);
astr get_home_dir (void);
astr agetcwd (void);
bool expand_path (astr path);
astr compact_path (astr path);
bool find_file (const char *filename);
void _Noreturn zile_exit (int doabort);
void init_guile_file_procedures (void);

/* funcs.c ---------------------------------------------------------------- */
SCM G_keyboard_quit (void);
SCM G_set_mark (void);
SCM G_set_mark_command (void);
SCM G_delete_region (void);
SCM G_exchange_point_and_mark (void);
void set_mark_interactive (void);
void write_temp_buffer (const char *name, bool show, void (*func) (va_list ap), ...);
SCM G_suspend_emacs (void);
SCM G_list_buffers (void);
void init_guile_funcs_procedures (void);

/* getkey.c --------------------------------------------------------------- */
void pushkey (size_t key);
void ungetkey (size_t key);
_GL_ATTRIBUTE_PURE size_t lastkey (void);
size_t getkeystroke (int delay);
size_t getkey (int delay);
size_t getkey_unfiltered (int delay);
void waitkey (void);
void init_getkey (void);
void free_getkey (void);

/* guile.c ---------------------------------------------------------------- */
SCM F_universal_argument (void);
SCM F_undo (void);
SCM F_self_insert_command (void);
SCM F_next_line (void);
SCM F_previous_line (void);
SCM F_mark_paragraph (void);
SCM F_kill_region (void);
SCM F_procedure_completions (void);
SCM guile_c_define_safe (const char *name, SCM value);
SCM guile_variable_ref_safe (SCM var);
SCM guile_c_resolve_module_safe (const char *name);
SCM guile_from_locale_string_safe (const char *str);
SCM guile_lookup_safe (SCM name);
char *guile_to_locale_string_safe (SCM x);
SCM guile_procedure_documentation_safe (SCM var);
SCM guile_procedure_name_safe (SCM var);
SCM guile_use_zile_module (void *unused);
const char * guile_get_procedure_documentation_by_name (const char *proc);
bool guile_get_procedure_interactive_by_name (const char *name);
const char *guile_get_procedure_name_by_name (const char *name);
SCM guile_error_handler (void *data, SCM key, SCM exception);
void guile_error (const char *function_name, const char *msg);
void guile_beginning_of_buffer_error (const char *function_name);
void guile_end_of_buffer_error (const char *function_name);
void guile_overflow_error (const char *function_name, int position, SCM variable);
void guile_quit_error (const char *function_name);
void guile_read_only_error (const char *function_name, const char *buffer_name);
void guile_void_function_error (const char *function_name);
void guile_wrong_number_of_arguments_error (const char *function_name);
void guile_wrong_type_argument_error (const char *function_name, 
				      int position, SCM variable, const char *type);
long guile_to_long_or_error (const char *function_name, int position, SCM n);
void guile_procedure_set_uniarg_integer (const char *func);
void guile_procedure_set_uniarg_boolean (const char *func);
void guile_procedure_set_uniarg_none (const char *func);
bool guile_procedure_takes_uniarg_integer (SCM proc);
bool guile_procedure_takes_uniarg_boolean (SCM proc);
bool guile_procedure_takes_uniarg_none (SCM proc);
SCM guile_call_procedure (SCM proc);
SCM guile_call_procedure_with_long (SCM proc, long uniarg);
SCM guile_call_procedure_with_boolean (SCM proc, bool uniarg);
bool guile_symbol_is_name_of_defined_function (SCM sym);
void set_guile_error_port_to_minibuffer (void);
void set_guile_error_port_to_stderr (void);
void guile_load (const char *filename);
bool guile_get_procedure_arity (SCM proc, int *required, int *optional);
void init_guile_guile_procedures (void);

/* help.c ----------------------------------------------------------------- */
void init_guile_help_procedures (void);

/* history.c -------------------------------------------------------------- */
History *history_new (void);
void add_history_element (History * hp, const char *string);
void prepare_history (History * hp);
const char *previous_history_element (History * hp);
const char *next_history_element (History * hp);

/* keycode.c -------------------------------------------------------------- */
astr chordtodesc (size_t key);
gl_list_t keystrtovec (const char *key);
astr keyvectodesc (gl_list_t keys);
void init_guile_keycode_procedures (void);

/* killring.c ------------------------------------------------------------- */
void init_guile_killring_procedures (void);

/* line.c ----------------------------------------------------------------- */
SCM G_tab_to_tab_stop (SCM n);
bool insert_newline (void);
bool intercalate_newline (void);
bool fill_break_line (void);
SCM G_znewline (SCM n);
SCM G_openline (SCM n);
bool insert_newline (void);
void bprintf (const char *fmt, ...);
SCM G_delete_char (SCM n);
SCM G_backward_delete_char (SCM n);
SCM G_delete_horizontal_space (void);
SCM G_just_one_space (void);
SCM G_indent_relative (void);
SCM G_indent_for_tab_command (void);
void init_guile_line_procedures (void);

/* lisp.c ----------------------------------------------------------------- */
void init_lisp (void);
void lisp_loadstring (astr as);
bool lisp_loadfile (const char *file);
void init_guile_lisp_procedures (void);

/* macro.c ---------------------------------------------------------------- */
void cancel_kbd_macro (void);
void add_cmd_to_macro (void);
void add_key_to_cmd (size_t key);
void remove_key_from_cmd (void);
void init_guile_macro_procedures (void);

/* main.c ----------------------------------------------------------------- */
extern char *prog_name;
extern Window *cur_wp, *head_wp;
extern Buffer *cur_bp, *head_bp;
extern int thisflag, lastflag, last_uniarg;
extern bool interactive;

/* marker.c --------------------------------------------------------------- */
#define FIELD(ty, field)                                \
  ty get_marker_ ## field (const Marker *cp);           \
  void set_marker_ ## field (Marker *cp, ty field);
#include "marker.h"
#undef FIELD
Marker * marker_new (void);
void unchain_marker (const Marker * marker);
void move_marker (Marker * marker, Buffer * bp, size_t o);
Marker *copy_marker (const Marker * marker);
Marker *point_marker (void);
void push_mark (void);
void pop_mark (void);
void set_mark (void);

/* minibuf.c -------------------------------------------------------------- */
void init_minibuf (void);
_GL_ATTRIBUTE_PURE int minibuf_no_error (void);
void minibuf_refresh (void);
void minibuf_write (const char *fmt, ...);
void minibuf_error (const char *fmt, ...);
castr minibuf_read (const char *fmt, const char *value, ...);
long minibuf_read_number (const char *fmt, ...);
bool minibuf_test_in_completions (const char *ms, gl_list_t completions);
int minibuf_read_yn (const char *fmt, ...);
int minibuf_read_yesno (const char *fmt, ...);
castr minibuf_read_completion (const char *fmt, const char *value, Completion * cp,
                               History * hp, ...);
castr minibuf_vread_completion (const char *fmt, const char *value, Completion * cp,
                                      History * hp, const char *empty_err,
                                      bool (*test) (const char *s, gl_list_t completions),
                                      const char *invalid_err, va_list ap);
castr minibuf_read_filename (const char *fmt, const char *value,
                             const char *file, ...);
void minibuf_clear (void);

/* redisplay.c ------------------------------------------------------------ */
void resync_redisplay (Window * wp);
void resize_windows (void);
void recenter (Window * wp);
SCM G_recenter (void);
void init_guile_redisplay_procedures (void);

/* registers.c ------------------------------------------------------------ */
void init_guile_registers_procedures (void);

/* search.c --------------------------------------------------------------- */
void init_search (void);
void init_guile_search_procedures (void);

/* term_curses.c ---------------------------------------------------------- */
size_t term_buf_len (void);
void term_init (void);
void term_close (void);
void term_move (size_t y, size_t x);
void term_clrtoeol (void);
void term_refresh (void);
void term_clear (void);
void term_addch (char c);
void term_addstr (const char *s);
void term_attrset (size_t attr);
void term_beep (void);
_GL_ATTRIBUTE_PURE size_t term_width (void);
_GL_ATTRIBUTE_PURE size_t term_height (void);
size_t term_getkey (int delay);
int term_getkey_unfiltered (int delay);
int term_getch (void);
void term_ungetkey (size_t key);

/* term_minibuf.c --------------------------------------------------------- */
void term_minibuf_write (const char *fmt);
castr term_minibuf_read (const char *prompt, const char *value, size_t pos,
                         Completion * cp, History * hp);

/* term_redisplay.c ------------------------------------------------------- */
void term_redraw_cursor (void);
void term_redisplay (void);
void term_finish (void);

/* undo.c ----------------------------------------------------------------- */
extern int undo_nosave;
void undo_start_sequence (void);
void undo_end_sequence (void);
void undo_save_block (size_t o, size_t osize, size_t size);
void undo_set_unchanged (Undo *up);
void init_guile_undo_procedures (void);

/* variables.c ------------------------------------------------------------ */
void init_variables (void);
castr minibuf_read_variable_name (const char *fmt, ...);
void set_variable (const char *var, const char *val);
const char *get_variable_doc (const char *var, const char **defval);
const char *get_variable (const char *var);
long get_variable_number (const char *var);
bool get_variable_bool (const char *var);
char *get_variable_string (const char *var);
void set_variable_bool (const char *var, int val);
void set_variable_number (const char *var, long val);
void set_variable_string (const char *var, const char *str);
void init_guile_variables_procedures (void);


/* window.c --------------------------------------------------------------- */
#define FIELD(ty, field)                                \
  ty get_window_ ## field (const Window *wp);           \
  void set_window_ ## field (Window *wp, ty field);
#include "window.h"
#undef FIELD
void create_scratch_window (void);
Window *find_window (const char *name);
Window *popup_window (void);
void set_current_window (Window * wp);
void delete_window (Window * del_wp);
size_t window_o (Window * wp);
bool window_top_visible (Window * wp);
bool window_bottom_visible (Window * wp);
SCM G_split_window (void);
SCM G_other_window (void);
SCM G_delete_window (void);
void window_resync (Window * wp);
void init_guile_window_procedures (void);
