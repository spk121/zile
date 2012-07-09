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

/* basic.c ---------------------------------------------------------------- */
size_t get_goalc_bp (Buffer * bp, size_t o);
size_t get_goalc (void);
bool previous_line (void);
bool next_line (void);

/* bind.c ----------------------------------------------------------------- */
_GL_ATTRIBUTE_PURE Function last_command (void);
void set_this_command (Function cmd);
size_t do_binding_completion (astr as);
gl_list_t get_key_sequence (void);
Function get_function_by_keys (gl_list_t keys);
le *call_command (Function f, int uniarg, bool uniflag, le *branch);
void get_and_run_command (void);
void init_default_bindings (void);

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
extern le *leNIL, *leT;
_GL_ATTRIBUTE_PURE size_t countNodes (le * branch);
void leEval (le * list);
le *execute_with_uniarg (bool undo, int uniarg, bool (*forward) (void),
                         bool (*backward) (void));
le *move_with_uniarg (int uniarg, bool (*move) (int dir));
le *execute_function (const char *name, int uniarg, bool is_uniarg);
_GL_ATTRIBUTE_PURE Function get_function (const char *name);
_GL_ATTRIBUTE_PURE const char *get_function_doc (const char *name);
_GL_ATTRIBUTE_PURE int get_function_interactive (const char *name);
_GL_ATTRIBUTE_PURE const char *get_function_name (Function p);
castr minibuf_read_function_name (const char *fmt, ...);
void init_eval (void);

/* file.c ----------------------------------------------------------------- */
int exist_file (const char *filename);
astr get_home_dir (void);
astr agetcwd (void);
bool expand_path (astr path);
astr compact_path (astr path);
bool find_file (const char *filename);
void _Noreturn zile_exit (int doabort);

/* funcs.c ---------------------------------------------------------------- */
void set_mark_interactive (void);
void write_temp_buffer (const char *name, bool show, void (*func) (va_list ap), ...);

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

/* line.c ----------------------------------------------------------------- */
bool insert_newline (void);
bool intercalate_newline (void);
bool fill_break_line (void);
void bprintf (const char *fmt, ...);

/* lisp.c ----------------------------------------------------------------- */
void init_lisp (void);
void lisp_loadstring (astr as);
bool lisp_loadfile (const char *file);

/* macro.c ---------------------------------------------------------------- */
void cancel_kbd_macro (void);
void add_cmd_to_macro (void);
void add_key_to_cmd (size_t key);
void remove_key_from_cmd (void);

/* main.c ----------------------------------------------------------------- */
extern char *prog_name;
extern Window *cur_wp, *head_wp;
extern Buffer *cur_bp, *head_bp;
extern int thisflag, lastflag, last_uniarg;

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
void resize_windows (void);
void recenter (Window * wp);

/* search.c --------------------------------------------------------------- */
void init_search (void);

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

/* variables.c ------------------------------------------------------------ */
void init_variables (void);
castr minibuf_read_variable_name (const char *fmt, ...);
void set_variable (const char *var, const char *val);
const char *get_variable_doc (const char *var, const char **defval);
const char *get_variable (const char *var);
long get_variable_number_bp (Buffer * bp, const char *var);
long get_variable_number (const char *var);
bool get_variable_bool (const char *var);

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
_GL_ATTRIBUTE_PURE bool window_bottom_visible (Window * wp);
void window_resync (Window * wp);


/*
 * Declare external Zile functions.
 */
#define X(zile_name, c_name, interactive, doc)   \
  le *F_ ## c_name (long uniarg, bool is_uniarg, le * l);
#include "tbl_funcs.h"
#undef X
