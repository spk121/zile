/* basic.c ---------------------------------------------------------------- */
size_t get_goalc_bp(Buffer *bp, Point pt);
size_t get_goalc_wp(Window *wp);
size_t get_goalc(void);
int backward_char(void);
int forward_char(void);
void goto_line(size_t to_line);
void gotobob(void);
void gotoeob(void);
int next_line(void);
int ngotodown(size_t n);
int ngotoup(size_t n);
int previous_line(void);
int scroll_down(void);
int scroll_up(void);

/* bind.c ----------------------------------------------------------------- */
size_t do_completion(astr as);
char *minibuf_read_function_name(const char *fmt, ...);
char *get_function_by_key_sequence(void);
void process_key(size_t key);
void init_bindings(void);
Function last_command(void);
void free_bindings(void);
Function get_function(char *name);
char *get_function_name(Function p);

/* buffer.c --------------------------------------------------------------- */
void calculate_region(Region *rp, Point from, Point to);
int calculate_the_region(Region *rp);
Buffer *create_buffer(const char *name);
void free_buffer(Buffer *bp);
void free_buffers(void);
void set_buffer_name(Buffer *bp, const char *name);
void set_buffer_filename(Buffer *bp, const char *filename);
Buffer *find_buffer(const char *name, int cflag);
Buffer *get_next_buffer(void);
char *make_buffer_name(const char *filename);
void switch_to_buffer(Buffer *bp);
int warn_if_readonly_buffer(void);
int warn_if_no_mark(void);
void set_temporary_buffer(Buffer *bp);
size_t calculate_buffer_size(Buffer *bp);
int transient_mark_mode(void);
void activate_mark(void);
void deactivate_mark(void);
int is_mark_actived(void);
size_t tab_width(Buffer *bp);

/* completion.c ----------------------------------------------------------- */
Completion *completion_new(int fileflag);
void free_completion(Completion *cp);
void completion_scroll_up(void);
void completion_scroll_down(void);
int completion_try(Completion *cp, astr search, int popup_when_complete);

/* editfns.c -------------------------------------------------------------- */
void push_mark(void);
void pop_mark(void);
void set_mark(void);
int is_empty_line(void);
int is_blank_line(void);
int char_after(Point *pt);
int char_before(Point *pt);
int following_char(void);
int preceding_char(void);
int bobp(void);
int eobp(void);
int bolp(void);
int eolp(void);

/* file.c ----------------------------------------------------------------- */
int exist_file(const char *filename);
int is_regular_file(const char *filename);
astr agetcwd(void);
astr get_home_dir(void);
int expand_path(const char *path, const char *cwdir, astr dir, astr fname);
astr compact_path(const char *path);
astr get_current_dir(int interactive);
void open_file(char *path, size_t lineno);
void read_from_disk(const char *filename);
int find_file(const char *filename);
Completion *make_buffer_completion(void);
int check_modified_buffer(Buffer *bp);
void kill_buffer(Buffer *kill_bp);
void zile_exit(int exitcode);

/* funcs.c ---------------------------------------------------------------- */
int cancel(void);
int set_mark_command(void);
int exchange_point_and_mark(void);
int universal_argument(int keytype, int xarg);
void write_temp_buffer(const char *name, void (*func)(va_list ap), ...);
int forward_sexp(void);
int backward_sexp(void);

/* glue.c ----------------------------------------------------------------- */
void ding(void);
size_t xgetkey(int mode, size_t timeout);
size_t getkey(void);
void waitkey(size_t delay);
char *copy_text_block(size_t startn, size_t starto, size_t size);
astr shorten_string(char *s, int maxlen);
char *replace_string(char *s, char *match, char *subst);
void tabify_string(char *dest, char *src, size_t scol, size_t tw);
void untabify_string(char *dest, char *src, size_t scol, size_t tw);
void goto_point(Point pt);
char *getln(FILE *fp);

/* history.c -------------------------------------------------------------- */
void free_history_elements(History *hp);
void add_history_element(History *hp, const char *string);
void prepare_history(History *hp);
const char *previous_history_element(History *hp);
const char *next_history_element(History *hp);

/* keys.c ----------------------------------------------------------------- */
astr chordtostr(size_t key);
size_t strtochord(char *buf, size_t *len);
int keystrtovec(char *key, size_t **keyvec);
astr keyvectostr(size_t *keys, size_t numkeys);
astr simplify_key(char *key);

/* line.c ----------------------------------------------------------------- */
void line_replace_text(Line **lp, size_t offset, size_t oldlen, char *newtext, size_t newlen, int replace_case);
int insert_char(int c);
int insert_char_in_insert_mode(int c);
int intercalate_char(int c);
int insert_tab(void);
void fill_break_line(void);
int insert_newline(void);
int intercalate_newline(void);
void insert_string(const char *s);
void insert_nstring(const char *s, size_t size);
int self_insert_command(int c);
void bprintf(const char *fmt, ...);
int delete_char(void);
int backward_delete_char(void);
void free_registers(void);
void free_kill_ring(void);

/* lisp.c ----------------------------------------------------------------- */
void lisp_init(void);
void lisp_finalise(void);
le *lisp_read(getcCallback getcp, ungetcCallback ungetcp);
le *lisp_read_string(const char *string);
le *lisp_read_file(const char *file);
astr lisp_dump(le *list);

/* macro.c ---------------------------------------------------------------- */
void cancel_kbd_macro(void);
void add_cmd_to_macro(void);
void add_key_to_cmd(size_t key);
int call_macro(Macro *mp);
void free_macros(void);
Macro *get_macro(char *name);

/* main.c ----------------------------------------------------------------- */
extern Window *cur_wp, *head_wp;
extern Buffer *cur_bp, *head_bp;
extern Terminal *cur_tp;
extern int thisflag, lastflag, last_uniarg;

/* marker.c --------------------------------------------------------------- */
Marker *make_marker(void);
void free_marker(Marker *marker);
void move_marker(Marker *marker, Buffer *bp, Point pt);
Marker *copy_marker(Marker *marker);
Marker *point_marker(void);
Marker *point_min_marker(void);
Marker *point_max_marker(void);

/* minibuf.c -------------------------------------------------------------- */
char *minibuf_format(const char *fmt, va_list ap);
void free_minibuf(void);
void minibuf_error(const char *fmt, ...);
void minibuf_write(const char *fmt, ...);
char *minibuf_read(const char *fmt, const char *value, ...);
int minibuf_read_yesno(const char *fmt, ...);
int minibuf_read_boolean(const char *fmt, ...);
char *minibuf_read_dir(const char *fmt, const char *value, ...);
char *minibuf_read_completion(const char *fmt, char *value, Completion *cp, History *hp, ...);
void minibuf_clear(void);

/* point.c ---------------------------------------------------------------- */
Point make_point(size_t lineno, size_t offset);
int cmp_point(Point pt1, Point pt2);
int point_dist(Point pt1, Point pt2);
int count_lines(Point pt1, Point pt2);
void swap_point(Point *pt1, Point *pt2);
Point point_min(void);
Point point_max(void);
Point line_beginning_position(int count);
Point line_end_position(int count);

/* redisplay.c ------------------------------------------------------------ */
void resync_redisplay(void);
void resize_windows(void);
void recenter(Window *wp);

/* search.c --------------------------------------------------------------- */
void free_search_history(void);

/* term_minibuf.c --------------------------------------------------------- */
void term_minibuf_write(const char *fmt);
char *term_minibuf_read(const char *prompt, const char *value, Completion *cp, History *hp);
void free_rotation_buffers(void);

/* term_redisplay.c ------------------------------------------------------- */
void term_redisplay(void);
void term_full_redisplay(void);
void show_splash_screen(const char *splash);
void term_tidy(void);
void term_addnstr(const char *s, size_t len);
int term_printw(const char *fmt, ...);

/* term_{allegro,epocemx,termcap}.c --------------------------------------- */
void term_init(void);
void term_close(void);
void term_suspend(void);
void term_resume(void);
void term_move(size_t y, size_t x);
void term_clrtoeol(void);
void term_refresh(void);
void term_redraw_cursor(void);
void term_clear(void);
void term_addch(int c);
void term_attrset(size_t attrs, ...);
void term_beep(void);
size_t term_xgetkey(int mode, size_t timeout);
void term_ungetkey(size_t key);

/* undo.c ----------------------------------------------------------------- */
extern int undo_nosave;

void undo_start_sequence(void);
void undo_end_sequence(void);
void undo_save(int type, Point pt, size_t arg1, size_t arg2);

/* variables.c ------------------------------------------------------------ */
void init_variables(void);
int is_variable_equal(char *var, char *val);
int lookup_bool_variable(char *var);
char *minibuf_read_variable_name(char *msg);
void set_variable(char *var, char *val);
char *get_variable_bp(Buffer *bp, char *var);
char *get_variable(char *var);
int get_variable_number_bp(Buffer *bp, char *var);
int get_variable_number(char *var);

/* window.c --------------------------------------------------------------- */
void create_first_window(void);
void free_window(Window *wp);
Window *find_window(const char *name);
void free_windows(void);
Window *popup_window(void);
void set_current_window (Window *wp);
Point window_pt(Window *wp);

/* zmalloc.c -------------------------------------------------------------- */
void *zmalloc(size_t size);
void *zrealloc(void *ptr, size_t size);
char *zstrdup(const char *s);
int zvasprintf(char **ptr, const char *fmt, va_list vargs);
int zasprintf(char **ptr, const char *fmt, ...);


/*
 * Declare external Zile functions.
 */
#define X0(zile_name, c_name)			\
	extern int F_ ## c_name(int argc, le *branch);
#define X1(zile_name, c_name, k1)		\
	X0(zile_name, c_name)
#define X2(zile_name, c_name, k1, k2)		\
	X0(zile_name, c_name)
#define X3(zile_name, c_name, k1, k2, k3)	\
	X0(zile_name, c_name)
#include "tbl_funcs.h"
#undef X0
#undef X1
#undef X2
#undef X3

/*--------------------------------------------------------------------------
 * Missing functions.
 *
 * The functions here are used in tools, where config.h is not available.
 *--------------------------------------------------------------------------*/

#ifndef HAVE_VASPRINTF
#include <stdarg.h>

int vasprintf(char **ptr, const char *fmt, va_list vargs);
#endif
