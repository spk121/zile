/* basic.c ---------------------------------------------------------------- */
extern int	backward_char(void);
extern int	forward_char(void);
extern void	goto_line(int to_line);
extern void	gotobob(void);
extern void	gotoeob(void);
extern int	next_line(void);
extern int	ngotodown(int n);
extern int	ngotoup(int n);
extern int	previous_line(void);
extern int	scroll_down(void);
extern int	scroll_up(void);

/* bind.c ----------------------------------------------------------------- */
extern void	bind_key(char *key, funcp func);
extern int	do_completion(char *s, int *compl);
extern int	execute_function(char *name, int uniarg);
extern void	free_bindings(void);
extern char *	get_function_by_key_sequence(void);
extern void	init_bindings(void);
extern char *	minibuf_read_function_name(char *msg);
extern void	process_key(int c);

/* buffer.c --------------------------------------------------------------- */
extern int	calculate_region(regionp rp);
extern bufferp	create_buffer(const char *name);
extern bufferp	new_buffer(void);
extern void	free_buffer(bufferp bp);
extern void	free_buffers(void);
extern void	set_buffer_name(bufferp bp, const char *name);
extern void	set_buffer_filename(bufferp bp, char *filename);
extern bufferp	find_buffer(const char *name, int cflag);
extern bufferp	get_next_buffer(void);
extern char *	make_buffer_name(char *filename);
extern void	switch_to_buffer(bufferp bp);
extern int	zap_buffer_content(void);
extern int	warn_if_readonly_buffer(void);
extern int	warn_if_no_mark(void);
extern void	set_temporary_buffer(bufferp bp);

/* file.c ----------------------------------------------------------------- */
extern int	exist_file(char *filename);
extern int	is_regular_file(char *filename);
extern int	expand_path(char *path, char *cwdir, pathbuffer_t *dir,
			    pathbuffer_t *fname);
extern pathbuffer_t *	compact_path(pathbuffer_t *buf, char *path);
extern pathbuffer_t *	get_current_dir(pathbuffer_t *buf);
extern void	open_file(char *path, int lineno);
extern void	read_from_disk(char *filename);
extern int	find_file(char *filename);
extern historyp	make_buffer_history(void);
extern int	check_modified_buffer(bufferp bp);
extern void	kill_buffer(bufferp kill_bp);
extern void	zile_exit(int exitcode);

/* fontlock.c ------------------------------------------------------------- */
extern void	font_lock_reset_anchors(bufferp bp, linep lp);
extern int	find_last_anchor(bufferp bp, linep lp);

/* fontlock_c.c ----------------------------------------------------------- */
extern char *	is_c_keyword(const char *str, int len);

/* fontlock_cpp.c --------------------------------------------------------- */
extern char *	is_cpp_keyword(const char *str, int len);

/* funcs.c ---------------------------------------------------------------- */
extern int	cancel(void);
extern int	set_mark_command(void);
extern int	universal_argument(int keytype, int xarg);
extern void	write_temp_buffer(const char *name, void (*func)(va_list ap), ...);

/* glue.c ----------------------------------------------------------------- */
extern void	ding(void);
extern void	waitkey(int msecs);
extern int	waitkey_discard(int msecs);
extern char *	copy_text_block(int startn, int starto, size_t size);
extern char *	shorten_string(char *dest, char *s, int maxlen);
extern char *	replace_string(char *s, char *match, char *subst);
extern void	tabify_string(char *dest, char *src, int scol, int tw);
extern void	untabify_string(char *dest, char *src, int scol, int tw);
extern int	get_text_goalc(windowp wp);
extern int	calculate_mark_lineno(windowp wp);
extern void *	zmalloc(size_t size);
extern void *	zrealloc(void *ptr, size_t size);
extern char *	zstrdup(const char *s);

/* keys.c ----------------------------------------------------------------- */
extern char *	keytostr(char *buf, int key, int *len);
extern char *	keytostr_nobs(char *buf, int key, int *len);
extern int	strtokey(char *buf, int *len);
extern int	keytovec(char *key, int *keyvec);
extern char *	simplify_key(char *dest, char *key);

/* line.c ----------------------------------------------------------------- */
extern linep	new_line(int maxsize);
extern linep	resize_line(windowp wp, linep lp, int maxsize);
extern void	free_line(linep lp);
extern void	line_replace_text(linep *lp, int offset, int orgsize, char *newtext);
extern int	insert_char(int c);
extern int      insert_char_in_insert_mode(int c);
extern int      intercalate_char(int c);
extern int	insert_tab(void);
extern int	insert_newline(void);
extern int      intercalate_newline(void);
extern void	insert_string(char *s);
extern void	insert_nstring(char *s, size_t size);
extern int	self_insert_command(int c);
extern void	bprintf(const char *fmt, ...);
extern int	delete_char(void);
extern int	backward_delete_char(void);
extern void	free_registers(void);
extern void	free_kill_ring(void);

/* macro.c ---------------------------------------------------------------- */
extern void	cancel_kbd_macro(void);
extern void	add_kbd_macro(funcp func, int uniarg);
extern void	free_macros(void);

/* main.c ----------------------------------------------------------------- */
extern windowp	cur_wp, head_wp;
extern bufferp	cur_bp, prev_bp, head_bp;
extern terminalp cur_tp;
extern int	thisflag, lastflag, last_uniarg;

/* minibuf.c -------------------------------------------------------------- */
extern void	minibuf_clear(void);
extern void	minibuf_error(const char *fmt, ...);
extern void	minibuf_write(const char *fmt, ...);
extern char *	minibuf_read(const char *fmt, char *value, ...);
extern int	minibuf_read_boolean(const char *fmt, ...);
extern char *	minibuf_read_color(const char *fmt, ...);
extern char *	minibuf_read_dir(const char *fmt, char *value, ...);
extern char *	minibuf_read_history(const char *fmt, char *value, historyp hp, ...);
extern int	minibuf_read_yesno(const char *fmt, ...);
extern historyp new_history(int fileflag);
extern void	free_history(historyp hp);

/* rc.c ------------------------------------------------------------------- */
extern void	read_rc_file(void);

/* redisplay.c ------------------------------------------------------------ */
extern void	recenter(windowp wp);
extern void	resync_redisplay(void);

/* search.c --------------------------------------------------------------- */
extern void	free_search_history(void);

/* undo.c ----------------------------------------------------------------- */
extern int	undo_nosave;
extern void	undo_save(int type, int startn, int starto, int arg1, int arg2);

/* variables.c ------------------------------------------------------------ */
extern void	init_variables(void);
extern void	free_variables(void);
extern int	is_variable_equal(char *var, char *val);
extern int	lookup_bool_variable(char *var);
extern char *	minibuf_read_variable_name(char *msg);
extern void	set_variable(char *var, char *val);
extern void	unset_variable(char *var);
extern char *	get_variable(char *var);

/* window.c --------------------------------------------------------------- */
extern void	create_first_window(void);
extern windowp	new_window(void);
extern void	free_window(windowp wp);
extern windowp	find_window(const char *name);
extern void	free_windows(void);
extern windowp	popup_window(void);

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
