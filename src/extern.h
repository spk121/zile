/* Global function prototypes

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.

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

#include <lua.h>
#include "gl_list.h"

#include "xalloc_extra.h"

/* basic.c ---------------------------------------------------------------- */
size_t get_goalc_bp (Buffer * bp, Point pt);
size_t get_goalc (void);
int backward_char (void);
int forward_char (void);
void gotobob (void);
void gotoeob (void);
int next_line (void);
int ngotodown (size_t n);
int ngotoup (size_t n);
int previous_line (void);

/* bind.c ----------------------------------------------------------------- */
extern Binding root_bindings;
size_t do_binding_completion (astr as);
const char *get_function_by_key (size_t key);
void process_key (Binding bindings, size_t key);
Binding init_bindings (void);
void init_default_bindings (void);
Function last_command (void);
void free_bindings (Binding p);

/* buffer.c --------------------------------------------------------------- */
#define FIELD(ty, field)                                \
  ty get_buffer_ ## field (const Buffer *bp);           \
  void set_buffer_ ## field (Buffer *bp, ty field);
#define FIELD_STR(field)                                \
  FIELD(const char *, field)
#include "buffer.h"
#undef FIELD
#undef FIELD_STR
#define FIELD(ty, field)                                \
  ty get_region_ ## field (const Region *cp);           \
  void set_region_ ## field (Region *cp, ty field);
#include "region.h"
#undef FIELD
void free_buffer (Buffer * bp);
void free_buffers (void);
void init_buffer (Buffer * bp);
Buffer * buffer_new (void);
const char *get_buffer_filename_or_name (Buffer * bp);
void set_buffer_names (Buffer * bp, const char *filename);
Buffer * find_buffer (const char *name);
void switch_to_buffer (Buffer * bp);
Region * region_new (void);
int warn_if_readonly_buffer (void);
int calculate_the_region (Region * rp);
bool delete_region (const Region * rp);
bool in_region (size_t lineno, size_t x, Region * rp);
void set_temporary_buffer (Buffer * bp);
size_t calculate_buffer_size (Buffer * bp);
int transient_mark_mode (void);
void activate_mark (void);
void deactivate_mark (void);
size_t tab_width (Buffer * bp);
char *copy_text_block (size_t startn, size_t starto, size_t size);
Buffer *create_scratch_buffer (void);
void kill_buffer (Buffer * kill_bp);
Completion *make_buffer_completion (void);
bool check_modified_buffer (Buffer * bp);

/* completion.c ----------------------------------------------------------- */
#define FIELD(ty, field)                                        \
  ty get_completion_ ## field (const Completion *cp);           \
  void set_completion_ ## field (Completion *cp, ty field);
#define FIELD_STR(field)                        \
  FIELD(const char *, field)
#include "completion.h"
#undef FIELD
#undef FIELD_STR
int completion_strcmp (const void *p1, const void *p2);
Completion *completion_new (int fileflag);
void free_completion (Completion * cp);
void completion_scroll_up (void);
void completion_scroll_down (void);
int completion_try (Completion * cp, astr search, int popup_when_complete);

/* editfns.c -------------------------------------------------------------- */
void push_mark (void);
void pop_mark (void);
void set_mark (void);
int is_empty_line (void);
int is_blank_line (void);
int following_char (void);
int preceding_char (void);
int bobp (void);
int eobp (void);
int bolp (void);
int eolp (void);
void ding (void);

/* eval.c ----------------------------------------------------------------- */
extern le *leNIL, *leT;
size_t countNodes (le * branch);
void leEval (le * list);
le *execute_with_uniarg (bool undo, int uniarg, int (*forward) (void),
                         int (*backward) (void));
le *execute_function (const char *name, int uniarg);
Function get_function (const char *name);
const char *get_function_doc (const char *name);
const char *get_function_name (Function p);
const char *minibuf_read_function_name (const char *fmt, ...);
void init_eval (void);
void free_eval (void);

/* file.c ----------------------------------------------------------------- */
extern char coding_eol_lf[3];
extern char coding_eol_crlf[3];
extern char coding_eol_cr[3];
extern char coding_eol_undecided[3];
int exist_file (const char *filename);
astr get_home_dir (void);
astr agetcwd (void);
bool expand_path (astr path);
astr compact_path (astr path);
bool find_file (const char *filename);
void zile_exit (int doabort);

/* funcs.c ---------------------------------------------------------------- */
void set_mark_interactive (void);
le *universal_argument (int keytype, int xarg);
void write_temp_buffer (const char *name, bool show, void (*func) (va_list ap), ...);

/* getkey.c --------------------------------------------------------------- */
void pushkey (size_t key);
void ungetkey (size_t key);
size_t lastkey (void);
size_t xgetkey (int mode, size_t timeout);
size_t getkey (void);
void waitkey (size_t delay);

/* history.c -------------------------------------------------------------- */
History *history_new (void);
void free_history (History * hp);
void add_history_element (History * hp, const char *string);
void prepare_history (History * hp);
const char *previous_history_element (History * hp);
const char *next_history_element (History * hp);

/* keycode.c -------------------------------------------------------------- */
astr chordtostr (size_t key);
size_t strtochord (const char *buf, size_t * len);
gl_list_t keystrtovec (const char *key);
astr keyvectostr (gl_list_t keys);

/* killring.c ------------------------------------------------------------- */
void free_kill_ring (void);

/* line.c ----------------------------------------------------------------- */
#define FIELD(ty, field)                                \
  ty get_line_ ## field (const Line *wp);               \
  void set_line_ ## field (Line *wp, ty field);
  FIELD(const char *, field)
#include "line.h"
#undef FIELD
Line *line_new (void);
void line_delete (Line *l);
Line *line_insert (Line *l, astr i);
void line_remove (Line *l);
void line_replace_text (Line ** lp, size_t offset, size_t oldlen,
                        char *newtext, int replace_case);
int insert_char (int c);
int insert_char_in_insert_mode (int c);
bool fill_break_line (void);
int insert_newline (void);
void insert_nstring (const char *s, size_t len);
void insert_astr (astr as);
void bprintf (const char *fmt, ...);
int delete_char (void);
void free_registers (void);

/* lisp.c ----------------------------------------------------------------- */
void init_lisp (void);
void free_lisp (void);
void lisp_loadstring (astr as);
bool lisp_loadfile (const char *file);

/* macro.c ---------------------------------------------------------------- */
void cancel_kbd_macro (void);
void add_cmd_to_macro (void);
void add_key_to_cmd (size_t key);
void remove_key_from_cmd (void);
void call_macro (Macro * mp);
void free_macros (void);
Macro *get_macro (const char *name);
void add_macros_to_list (gl_list_t l, gl_listelement_compar_fn f);

/* main.c ----------------------------------------------------------------- */
CLUE_DECLS(L);
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
void free_marker (Marker * marker);
void move_marker (Marker * marker, Buffer * bp, Point pt);
Marker *copy_marker (Marker * marker);
Marker *point_marker (void);
Marker *point_min_marker (void);

/* minibuf.c -------------------------------------------------------------- */
extern char *minibuf_contents;
void init_minibuf (void);
void free_minibuf (void);
void minibuf_error (const char *fmt, ...);
void minibuf_write (const char *fmt, ...);
char *minibuf_read (const char *fmt, const char *value, ...);
unsigned long minibuf_read_number (const char *fmt, ...);
bool minibuf_test_in_completions (const char *ms, gl_list_t completions);
int minibuf_read_yn (const char *fmt, ...);
int minibuf_read_yesno (const char *fmt, ...);
char *minibuf_read_completion (const char *fmt, char *value, Completion * cp,
                               History * hp, ...);
char *minibuf_vread_completion (const char *fmt, char *value, Completion * cp,
                                History * hp, const char *empty_err,
                                bool (*test) (const char *s, gl_list_t completions),
                                const char *invalid_err, va_list ap);
char *minibuf_read_filename (const char *fmt, const char *value,
                             const char *file, ...);
void minibuf_clear (void);

/* point.c ---------------------------------------------------------------- */
Point make_point (size_t lineno, size_t offset);
int cmp_point (Point pt1, Point pt2);
int point_dist (Point pt1, Point pt2);
int count_lines (Point pt1, Point pt2);
void swap_point (Point * pt1, Point * pt2);
Point point_min (void);
Point point_max (void);
Point line_beginning_position (int count);
Point line_end_position (int count);
void goto_point (Point pt);

/* redisplay.c ------------------------------------------------------------ */
void resync_redisplay (void);
void resize_windows (void);
void recenter (Window * wp);

/* search.c --------------------------------------------------------------- */
void init_search (void);
void free_search (void);

/* term_minibuf.c --------------------------------------------------------- */
void term_minibuf_write (const char *fmt);
char *term_minibuf_read (const char *prompt, const char *value, size_t pos,
                         Completion * cp, History * hp);

/* term_redisplay.c ------------------------------------------------------- */
size_t term_width (void);
size_t term_height (void);
void term_set_size (size_t cols, size_t rows);
void term_redisplay (void);
void term_full_redisplay (void);
void show_splash_screen (const char *splash);
void term_tidy (void);
void term_finish (void);
void term_addnstr (const char *s, size_t len);

/* term_curses.c ---------------------------------------------------------- */
void term_init (void);
void term_close (void);
void term_suspend (void);
void term_resume (void);
void term_move (size_t y, size_t x);
void term_clrtoeol (void);
void term_refresh (void);
void term_clear (void);
void term_addch (int c);
void term_attrset (size_t attrs, ...);
void term_beep (void);
size_t term_xgetkey (int mode, size_t timeout);
void term_ungetkey (size_t key);

/* undo.c ----------------------------------------------------------------- */
extern int undo_nosave;
void undo_start_sequence (void);
void undo_end_sequence (void);
void undo_save (int type, Point pt, size_t arg1, size_t arg2);
void free_undo (Undo *up);
void undo_set_unchanged (Undo *up);

/* variables.c ------------------------------------------------------------ */
void init_variables (void);
void free_variable_list (int ref);
char *minibuf_read_variable_name (char *fmt, ...);
void set_variable (const char *var, const char *val);
const char *get_variable_doc (const char *var, char **defval);
const char *get_variable_bp (Buffer * bp, const char *var);
const char *get_variable (const char *var);
long get_variable_number_bp (Buffer * bp, const char *var);
long get_variable_number (const char *var);
bool get_variable_bool (const char *var);

/* window.c --------------------------------------------------------------- */
#define FIELD(ty, field)                                \
  ty get_window_ ## field (const Window *wp);           \
  void set_window_ ## field (Window *wp, ty field);
  FIELD(const char *, field)
#include "window.h"
#undef FIELD
void create_first_window (void);
Window *find_window (const char *name);
void free_windows (void);
Window *popup_window (void);
void set_current_window (Window * wp);
void delete_window (Window * del_wp);
Point window_pt (Window * wp);


/*
 * Declare external Zile functions.
 */
#define X(zile_name, c_name, interactive, doc)   \
  le *F_ ## c_name (long uniarg, le * l);
#include "tbl_funcs.h"
#undef X
