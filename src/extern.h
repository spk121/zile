/* basic.c ---------------------------------------------------------------- */
int get_goalc_bp(Buffer *bp, Point pt);
int get_goalc_wp(Window *wp);
int get_goalc(void);
int backward_char(void);
int forward_char(void);
void goto_line(int to_line);
void gotobob(void);
void gotoeob(void);
int next_line(void);
int ngotodown(int n);
int ngotoup(int n);
int previous_line(void);
int scroll_down(void);
int scroll_up(void);

/* bind.c ----------------------------------------------------------------- */
int do_completion(astr as, int *compl);
int execute_function(char *name, int uniarg);
void free_bindings(void);
char *minibuf_read_function_name(const char *fmt, ...);
char *get_function_by_key_sequence(void);
void init_bindings(void);
void process_key(int c);

/* buffer.c --------------------------------------------------------------- */
int calculate_region(Region *rp);
Buffer *create_buffer(const char *name);
void free_buffer(Buffer *bp);
void free_buffers(void);
void set_buffer_name(Buffer *bp, const char *name);
void set_buffer_filename(Buffer *bp, const char *filename);
Buffer *find_buffer(const char *name, int cflag);
Buffer *get_next_buffer(void);
char *make_buffer_name(const char *filename);
void switch_to_buffer(Buffer *bp);
int zap_buffer_content(void);
int warn_if_readonly_buffer(void);
int warn_if_no_mark(void);
void set_temporary_buffer(Buffer *bp);
int calculate_buffer_size(Buffer *bp);

int transient_mark_mode(void);
void activate_mark(void);
void deactivate_mark(void);
int is_mark_actived(void);

/* file.c ----------------------------------------------------------------- */
int exist_file(const char *filename);
int is_regular_file(const char *filename);
int expand_path(const char *path, const char *cwdir, astr dir, astr fname);
astr compact_path(const char *path);
astr get_current_dir(astr buf, int interactive);
void open_file(char *path, int lineno);
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

/* glue.c ----------------------------------------------------------------- */
void ding(void);
void waitkey(int msecs);
int waitkey_discard(int msecs);
char *copy_text_block(int startn, int starto, size_t size);
astr shorten_string(char *s, int maxlen);
char *replace_string(char *s, char *match, char *subst);
void tabify_string(char *dest, char *src, int scol, int tw);
void untabify_string(char *dest, char *src, int scol, int tw);
void goto_point(Point pt);
void *zmalloc(size_t size);
void *zrealloc(void *ptr, size_t size);
char *zstrdup(const char *s);
#ifdef DEBUG
void ztrace(const char *fmt, ...);
#define ZTRACE(arg)	ztrace arg
#else
#define ZTRACE(arg)	(void)0
#endif
char *getln(FILE *fp);

/* keys.c ----------------------------------------------------------------- */
astr chordtostr(int key);
int strtochord(char *buf, int *len);
int keystrtovec(char *key, int **keyvec);
astr keyvectostr(int *keys, int numkeys);
astr simplify_key(char *key);

/* line.c ----------------------------------------------------------------- */
Line *new_line(int maxsize);
Line *resize_line(Line *lp, int maxsize);
void free_line(Line *lp);
void line_replace_text(Line **lp, int offset, int orgsize, char *newtext, int replace_case);
int insert_char(int c);
int insert_char_in_insert_mode(int c);
int intercalate_char(int c);
int insert_tab(void);
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

/* macro.c ---------------------------------------------------------------- */
void cancel_kbd_macro(void);
void add_kbd_macro(Function func, int set_uniarg, int uniarg);
void add_macro_key_data(int key);
int get_macro_key_data(void);
void free_macros(void);

/* main.c ----------------------------------------------------------------- */
extern Window *cur_wp, *head_wp;
extern Buffer *cur_bp, *head_bp;
extern Terminal *cur_tp;
extern int thisflag, lastflag, last_uniarg;

/* marker.c --------------------------------------------------------------- */
Marker *make_marker(void);
void free_marker(Marker *marker);
void unchain_marker(Marker *marker);
void move_marker(Marker *marker, Buffer *bp, Point pt);
Marker *copy_marker(Marker *marker);
Marker *point_marker(void);
Marker *point_min_marker(void);
Marker *point_max_marker(void);
void set_marker_insertion_type(Marker *marker, int type);
int marker_insertion_type(Marker *marker);

/* minibuf.c -------------------------------------------------------------- */
char *minibuf_format(const char *fmt, va_list ap);
void free_minibuf(void);
void minibuf_clear(void);
void minibuf_error(const char *fmt, ...);
void minibuf_write(const char *fmt, ...);
char *minibuf_read(const char *fmt, const char *value, ...);
int minibuf_read_boolean(const char *fmt, ...);
char *minibuf_read_color(const char *fmt, ...);
char *minibuf_read_dir(const char *fmt, const char *value, ...);
char *minibuf_read_completion(const char *fmt, char *value, Completion *cp, History *hp, ...);
int minibuf_read_yesno(const char *fmt, ...);

/* completion.c ----------------------------------------------------------- */
Completion *new_completion(int fileflag);
void free_completion(Completion *cp);

/* history.c -------------------------------------------------------------- */
void free_history_elements(History *hp);
void add_history_element(History *hp, const char *string);
void prepare_history(History *hp);
const char *previous_history_element(History *hp);
const char *next_history_element(History *hp);

/* point.c ---------------------------------------------------------------- */
Point make_point(int lineno, int offset);
int cmp_point(Point pt1, Point pt2);
int point_dist(Point pt1, Point pt2);
int count_lines(Point pt1, Point pt2);
void swap_point(Point *pt1, Point *pt2);
Point point_min(void);
Point point_max(void);
Point line_beginning_position(int count);
Point line_end_position(int count);

/* rc.c ------------------------------------------------------------------- */
void read_rc_file(const char *filename);

/* redisplay.c ------------------------------------------------------------ */
void recenter(Window *wp);
void resync_redisplay(void);

/* search.c --------------------------------------------------------------- */
void free_search_history(void);

/* term.c ----------------------------------------------------------------- */
void term_init(void);
int term_open(void);
int term_close(void);
void term_getyx(int *y, int *x);
void term_move(int y, int x);
void term_clrtoeol(void);
void term_refresh(void);
void term_redisplay(void);
void term_full_redisplay(void);
void term_clear(void);
void term_addch(int c);
void term_addnstr(const char *s, int len);
void term_attrset(Font f);
int term_printw(const char *fmt, ...);
void term_beep(void);
int term_getkey(void);
int term_xgetkey(int mode, int arg);
int term_ungetkey(int c);
void term_minibuf_write(const char *fmt);
char *term_minibuf_read(const char *prompt, const char *value, Completion *cp, History *hp);
void term_minibuf_clear(void);
void term_refresh_cached_variables(void);

/* undo.c ----------------------------------------------------------------- */
extern int undo_nosave;

void undo_start_sequence(void);
void undo_end_sequence(void);
void undo_save(int type, Point pt, int arg1, int arg2);

/* variables.c ------------------------------------------------------------ */
void init_variables(void);
void free_variables(void);
int is_variable_equal(char *var, char *val);
int lookup_bool_variable(char *var);
char *minibuf_read_variable_name(char *msg);
void set_variable(char *var, char *val);
void unset_variable(char *var);
char *get_variable(char *var);

/* window.c --------------------------------------------------------------- */
void create_first_window(void);
Window *new_window(void);
void free_window(Window *wp);
Window *find_window(const char *name);
void free_windows(void);
Window *popup_window(void);
void set_current_window (Window *wp);
Point window_pt(Window *wp);

/*
 * Declare external Zile functions.
 */
#define X0(zile_name, c_name)			\
	extern int F_ ## c_name(int uniarg);
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
 *--------------------------------------------------------------------------*/

#ifndef HAVE_VASPRINTF
int vasprintf(char **ptr, const char *fmt, va_list vargs);
#endif
