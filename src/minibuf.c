/*	$Id: minibuf.c,v 1.4 2003/05/06 22:28:42 rrt Exp $	*/

/*
 * Copyright (c) 1997-2002 Sandro Sigala.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <assert.h>
#include <dirent.h>
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "zile.h"
#include "extern.h"

/*--------------------------------------------------------------------------
 * Minibuffer wrapper functions.
 *--------------------------------------------------------------------------*/

/*
 * Formatting escapes:
 *  %s  insert string
 *  %d	insert integer
 * Color escapes:
 *  @b  begin buffer name
 *  @f  begin function name
 *  @k  begin key sequence
 *  @r  begin register name
 *  @v  begin variable name
 *  @w  begin window name
 *  @:	start foreground color mode `x', where `x' can be:
 *	   b: blue     B: light blue
 *	   c: cyan     C: light cyan
 *	   g: green    G: light green
 *	   m: magenta  M: light magenta
 *	   r: red      R: light red
 *	   w: white    W: light white
 *	   y: yellow   Y: light yellow
 *	   k: black
 *  @@	end color mode
 */
static char *minibuf_format(const char *fmt, va_list ap, int error)
{
	char *buf, *sp, *dp;
	int maxsize;

	maxsize = max(cur_tp->width, 160);
	buf = (char *)zmalloc(maxsize);

	sp = (char *)fmt;
	dp = buf;

#if 0
	if (error) {
		*dp++ = MINIBUF_SET_COLOR;
		*dp++ = 'C';
	}
#endif

	while (*sp != '\0' && dp < buf + maxsize)
		switch (*sp) {
		case '%':
			switch (*++sp) {
			case 's': {
				char *p = va_arg(ap, char *);
				while (*p != '\0' && dp < buf + maxsize)
					*dp++ = *p++;
				++sp;
				break;
			}
			case 'd': {
				int i = va_arg(ap, int);
				char buf[16], *p = buf;
				sprintf(buf, "%d", i);
				while (*p != '\0' && dp < buf + maxsize)
					*dp++ = *p++;
				++sp;
				break;
			}
			default:
				*dp++ = *sp++;
			}
			break;
		case '@':
			switch (*++sp) {
			case 'b': /* Buffer. */
			case 'w': /* Window. */
			case 'f': /* Function. */
			case 'r': /* Register. */
			case 'v': /* Variable. */
				*dp++ = MINIBUF_SET_COLOR;
				*dp++ = 'Y';
				++sp;
				break;
			case 'k': /* Key. */
				*dp++ = MINIBUF_SET_COLOR;
				*dp++ = 'W';
				++sp;
				break;
			case ':': /* Custom color. */
				*dp++ = MINIBUF_SET_COLOR;
				*dp++ = *++sp;
				++sp;
				break;
			case '@':
				*dp++ = MINIBUF_UNSET_COLOR;
				++sp;
				break;
			default:
				*dp++ = *sp++;
			}
			break;
		default:
			*dp++ = *sp++;
		}

	*dp = '\0';

	return buf;
}

/*
 * Write the specified string in the minibuffer.
 */
void minibuf_write(const char *fmt, ...)
{
	va_list ap;
	char *buf;

	va_start(ap, fmt);
	buf = minibuf_format(fmt, ap, FALSE);
	va_end(ap);

	cur_tp->minibuf_write(buf);
	free(buf);
}

/*
 * Write the specified error string in the minibuffer.
 */
void minibuf_error(const char *fmt, ...)
{
	va_list ap;
	char *buf;

	va_start(ap, fmt);
	buf = minibuf_format(fmt, ap, TRUE);
	va_end(ap);

	cur_tp->minibuf_write(buf);
	free(buf);

	ding();
}

/*
 * Read a string from the minibuffer.
 */
char *minibuf_read(const char *fmt, char *value, ...)
{
	va_list ap;
	char *buf, *p;

	va_start(ap, value);
	buf = minibuf_format(fmt, ap, FALSE);
	va_end(ap);

	if (value != NULL)
		p = cur_tp->minibuf_read(buf, value, NULL);
	else
		p = cur_tp->minibuf_read(buf, "", NULL);
	free(buf);

	return p;
}

/* The returned buffer must be freed by the caller. */
char *minibuf_read_dir(const char *fmt, char *value, ...)
{
	va_list ap;
	char *buf, *p;
	historyp hp;
	pathbuffer_t *dir, *fname;
	pathbuffer_t *rbuf;

	rbuf = pathbuffer_create(0);
	va_start(ap, value);
	buf = minibuf_format(fmt, ap, FALSE);
	va_end(ap);

	while (getcwd(pathbuffer_str(rbuf), pathbuffer_size(rbuf)) == NULL) {
		pathbuffer_realloc_larger(rbuf);
	}
	dir = pathbuffer_create(0);
	fname = pathbuffer_create(0);
	expand_path(value, pathbuffer_str(rbuf), dir, fname);
	pathbuffer_append(dir, pathbuffer_str(fname));
	compact_path(rbuf, pathbuffer_str(dir));

	hp = new_history(TRUE);
	p = cur_tp->minibuf_read(buf, pathbuffer_str(rbuf), hp);
	free_history(hp);
	free(buf);

	if (p == NULL) {
		pathbuffer_free(dir);
		pathbuffer_free(fname);
		pathbuffer_free(rbuf);
		return p;
	}

	while (getcwd(pathbuffer_str(rbuf), pathbuffer_size(rbuf)) == NULL) {
		pathbuffer_realloc_larger(rbuf);
	}
	expand_path(p, pathbuffer_str(rbuf), dir, fname);
	pathbuffer_put(rbuf, pathbuffer_str(dir));
	pathbuffer_append(rbuf, pathbuffer_str(fname));

	pathbuffer_free(dir);
	pathbuffer_free(fname);
	return pathbuffer_free_struct_only(rbuf);
}

int minibuf_read_forced(const char *fmt, const char *errmsg,
			historyp hp, ...)
{
	va_list ap;
	char *buf, *p;

	va_start(ap, hp);
	buf = minibuf_format(fmt, ap, FALSE);
	va_end(ap);

	for (;;) {
		p = cur_tp->minibuf_read(buf, "", hp);
		if (p == NULL) { /* Cancelled. */
			free(buf);
			return -1;
		} else {
			char *s;
			/* Complete partial words if possible. */
			if (hp->try(hp, p) == HISTORY_MATCHED)
				p = hp->match;
			for (s = alist_first(hp->completions); s != NULL;
			     s = alist_next(hp->completions))
				if (!strcmp(p, s)) {
					free(buf);
					return alist_current_idx(hp->completions);
				}
			minibuf_error(errmsg);
			waitkey(2 * 1000);
		}
	}

	assert(0);
	return FALSE;
}

int minibuf_read_yesno(const char *fmt, ...)
{
	va_list ap;
	char *buf;
	historyp hp;
	int retvalue;

	va_start(ap, fmt);
	buf = minibuf_format(fmt, ap, FALSE);
	va_end(ap);

	hp = new_history(FALSE);
	alist_append(hp->completions, zstrdup("yes"));
	alist_append(hp->completions, zstrdup("no"));

	retvalue = minibuf_read_forced(buf, "Please answer yes or no.", hp);
	if (retvalue != -1) {
		/* The completions may be sorted by the minibuf completion
		   routines. */
		if (!strcmp(alist_at(hp->completions, retvalue), "yes"))
			retvalue = TRUE;
		else
			retvalue = FALSE;
	}
	free_history(hp);
	free(buf);

	return retvalue;
}

int minibuf_read_boolean(const char *fmt, ...)
{
	va_list ap;
	char *buf;
	historyp hp;
	int retvalue;

	va_start(ap, fmt);
	buf = minibuf_format(fmt, ap, FALSE);
	va_end(ap);

	hp = new_history(FALSE);
	alist_append(hp->completions, zstrdup("true"));
	alist_append(hp->completions, zstrdup("false"));

	retvalue = minibuf_read_forced(buf, "Please answer true or false.", hp);
	if (retvalue != -1) {
		/* The completions may be sorted by the minibuf completion
		   routines. */
		if (!strcmp(alist_at(hp->completions, retvalue), "true"))
			retvalue = TRUE;
		else
			retvalue = FALSE;
	}
	free_history(hp);
	free(buf);

	return retvalue;	
}

char *minibuf_read_color(const char *fmt, ...)
{
	va_list ap;
	char *buf;
	historyp hp;
	int retvalue;
	unsigned int i;
	char *valid[] = {
		"black", "blue", "cyan", "green",
		"magenta", "red", "white", "yellow",
		"light-black", "light-blue", "light-cyan", "light-green",
		"light-magenta", "light-red", "light-white", "light-yellow"
	};

	va_start(ap, fmt);
	buf = minibuf_format(fmt, ap, FALSE);
	va_end(ap);

	hp = new_history(FALSE);
	for (i = 0; i < sizeof valid / sizeof(char *); ++i)
		alist_append(hp->completions, zstrdup(valid[i]));

	retvalue = minibuf_read_forced(buf, "Invalid color name.", hp);
	if (retvalue != -1) {
		/* The completions may be sorted by the minibuf completion
		   routines. */
		for (i = 0; i < sizeof valid / sizeof (char *); ++i)
			if (!strcmp(alist_at(hp->completions, retvalue), valid[i])) {
				retvalue = i;
				break;
			}
	}
	free_history(hp);
	free(buf);

	if (retvalue == -1)
		return NULL;
	return valid[retvalue];
}

/*
 * Read a string from the minibuffer using a history.
 */
char *minibuf_read_history(const char *fmt, char *value, historyp hp, ...)
{
	va_list ap;
	char *buf, *p;

	va_start(ap, hp);
	buf = minibuf_format(fmt, ap, FALSE);
	va_end(ap);

	p = cur_tp->minibuf_read(buf, value, hp);
	free(buf);

	return p;
}

/*
 * Clear the minibuffer.
 */
void minibuf_clear(void)
{
	cur_tp->minibuf_clear();
}

/*--------------------------------------------------------------------------
 * History functions.
 *--------------------------------------------------------------------------*/

/* Forward declarations. */
static void default_history_scroll_up(historyp hp);
static void default_history_scroll_down(historyp hp);
static int default_history_try(historyp hp, char *s);
static int default_history_reread(historyp hp, char *s);

/*
 * Allocate a new history structure.
 */
historyp new_history(int fileflag)
{
	historyp hp;

	hp = (historyp)zmalloc(sizeof *hp);
	memset(hp, 0, sizeof *hp);

	hp->completions = alist_new();
	hp->matches = alist_new();
	hp->try = default_history_try;
	hp->scroll_up = default_history_scroll_up;
	hp->scroll_down = default_history_scroll_down;
	hp->reread = default_history_reread;

	if (fileflag) {
		hp->path = pathbuffer_create(0);
		hp->fl_dir = TRUE;
	}

	return hp;
}

/*
 * Dispose an history structure.
 */
void free_history(historyp hp)
{
	char *p;
	for (p = alist_first(hp->completions); p != NULL; p = alist_next(hp->completions))
		free(p);
	alist_delete(hp->completions);
	alist_delete(hp->matches);
	if (hp->fl_dir)
		pathbuffer_free(hp->path);
	free(hp);
}

/*
 * The default history scrolling function.
 */
static void default_history_scroll_up(historyp hp)
{
	windowp wp, old_wp = cur_wp;

	wp = find_window("*Completions*");
	assert(wp != NULL);
	cur_wp = wp;
	cur_bp = wp->bp;
	if (cur_wp->pointn == cur_bp->num_lines || !FUNCALL(scroll_up))
		gotobob();
	cur_wp = old_wp; 
	cur_bp = old_wp->bp;

	cur_tp->redisplay();
}

/*
 * The default history scrolling function.
 */
static void default_history_scroll_down(historyp hp)
{
	windowp wp, old_wp = cur_wp;

	wp = find_window("*Completions*");
	assert(wp != NULL);
	cur_wp = wp;
	cur_bp = wp->bp;
	if (cur_wp->pointn == 0 || !FUNCALL(scroll_down)) {
		gotoeob();
		resync_redisplay();
	}
	cur_wp = old_wp; 
	cur_bp = old_wp->bp;

	cur_tp->redisplay();
}

/*
 * Calculate the maximum length of the completions.
 */
static int calculate_max_length(alist al, int size)
{
	int i, max = 0;
	char *p;

	for (p = alist_first(al); p != NULL; p = alist_next(al)) {
		if (alist_current_idx(al) >= size)
			break;
		if ((i = strlen(p)) > max)
			max = i;
	}

	return max;
}

/*
 * Print the list of completions in a set of columns.
 */
static void history_print(alist al, int size)
{
	int i, col, max, numcols;
	char *p;

	max = calculate_max_length(al, size) + 5;
	numcols = (cur_wp->ewidth - 1) / max;

	bprintf("Possible completions are:\n");
	for (p = alist_first(al), col = 0; p != NULL; p = alist_next(al)) {
		if (alist_current_idx(al) >= size)
			break;
		if (col >= numcols) {
			col = 0;
			insert_newline();
		}
		insert_string(p);
		for (i = max - strlen(p); i > 0; --i)
			insert_char(' ');
		++col;
	}
}

/*
 * Popup the history window.
 */
static void popup_history(historyp hp, int allflag, int num)
{
	windowp wp, old_wp = cur_wp;
	bufferp bp;

	hp->fl_poppedup = 1;

	if ((wp = find_window("*Completions*")) == NULL) {
		if (head_wp->next == NULL)
			hp->fl_close = 1;
		cur_wp = popup_window();
		cur_bp = cur_wp->bp;
		if (!hp->fl_close)
			hp->old_bp = cur_bp;
		bp = find_buffer("*Completions*", TRUE);
		switch_to_buffer(bp);
	} else {
		cur_wp = wp;
		cur_bp = wp->bp;
	}

	zap_buffer_content();
	cur_bp->flags = BFLAG_NEEDNAME | BFLAG_NOSAVE | BFLAG_NOUNDO
		| BFLAG_NOEOB;
	set_temporary_buffer(cur_bp);

	if (allflag)
		history_print(hp->completions, alist_count(hp->completions));
	else
		history_print(hp->matches, num);

	gotobob();

	cur_bp->flags |= BFLAG_READONLY;

	cur_wp = old_wp;
	cur_bp = old_wp->bp;

	cur_tp->redisplay();
}

static int hcompar(const void *p1, const void *p2)
{
	return strcmp(*(const char **)p1, *(const char **)p2);
}

/*
 * The default history matching function.
 */
static int default_history_try(historyp hp, char *s)
{
	int i, j, ssize, fullmatches = 0, partmatches = 0;
	char *p, c;
	pathbuffer_t *search;

	alist_clear(hp->matches);

	search = pathbuffer_create(0);
	pathbuffer_put(search, s);
	if (hp->fl_dir)
		if (!hp->reread(hp, pathbuffer_str(search))) {
			pathbuffer_free(search);
			return HISTORY_NOTMATCHED;
		}

	if (!hp->fl_sorted) {
		alist_sort(hp->completions, hcompar);
		hp->fl_sorted = 1;
	}

	ssize = strlen(pathbuffer_str(search));

	if (ssize == 0) {
		if (alist_count(hp->completions) > 1) {
			hp->match = alist_first(hp->completions);
			hp->matchsize = 0;
			popup_history(hp, TRUE, 0);
			pathbuffer_free(search);
			return HISTORY_NONUNIQUE;
		} else {
			hp->match = alist_first(hp->completions);
			hp->matchsize = strlen(hp->match);
			pathbuffer_free(search);
			return HISTORY_MATCHED;
		}
	}

	for (p = alist_first(hp->completions); p != NULL;
	     p = alist_next(hp->completions))
		if (!strncmp(p, pathbuffer_str(search), ssize)) {
			++partmatches;
			alist_append(hp->matches, p);
			if (!strcmp(p, pathbuffer_str(search)))
				++fullmatches;
		}

	pathbuffer_free(search);
	search = NULL;

	if (partmatches == 0)
		return HISTORY_NOTMATCHED;
	else if (partmatches == 1) {
		hp->match = alist_first(hp->matches);
		hp->matchsize = strlen(hp->match);
		return HISTORY_MATCHED;
	}

	if (fullmatches == 1 && partmatches > 1) {
		hp->match = alist_first(hp->matches);
		hp->matchsize = strlen(hp->match);
		popup_history(hp, FALSE, partmatches);
		return HISTORY_MATCHEDNONUNIQUE;
	}

	for (j = ssize; ; ++j) {
		char *s = alist_first(hp->matches);
		c = s[j];
		for (i = 1; i < partmatches; ++i) {
			s = alist_at(hp->matches, i);
			if (s[j] != c) {
				hp->match = alist_first(hp->matches);
				hp->matchsize = j;
				popup_history(hp, FALSE, partmatches);
				return HISTORY_NONUNIQUE;
			}
		}
	}

	assert(0);
	return HISTORY_NOTMATCHED;
}

/*
 * The default history directory re-reading function.
 */
static int default_history_reread(historyp hp, char *s)
{
	pathbuffer_t *buf, *pdir, *fname;
	DIR *dir;
	struct dirent *d;
	struct stat st;
	char *p;

	for (p = alist_first(hp->completions); p != NULL; p = alist_next(hp->completions))
		free(p);

	alist_clear(hp->completions);
	hp->fl_sorted = 0;

	buf = pathbuffer_create(0);
	while ((getcwd(pathbuffer_str(buf), pathbuffer_size(buf))) == NULL) {
		pathbuffer_realloc_larger(buf);
	}
	pdir = pathbuffer_create(0);
	fname = pathbuffer_create(0);
	if (!expand_path(s, pathbuffer_str(buf), pdir, fname))
		return FALSE;

	if ((dir = opendir(pathbuffer_str(pdir))) == NULL)
		return FALSE;

	strcpy(s, pathbuffer_str(fname));

	while ((d = readdir(dir)) != NULL) {
		pathbuffer_put(buf, pathbuffer_str(pdir));
		pathbuffer_append(buf, d->d_name);
		if (stat(pathbuffer_str(buf), &st) != -1) {
			pathbuffer_put(buf, d->d_name);
			if (S_ISDIR(st.st_mode))
				pathbuffer_append(buf, "/");
		} else
			pathbuffer_put(buf, d->d_name);
		alist_append(hp->completions, zstrdup(pathbuffer_str(buf)));
	}
	closedir(dir);

	compact_path(hp->path, pathbuffer_str(pdir));

	pathbuffer_free(buf);
	pathbuffer_free(pdir);
	pathbuffer_free(fname);

	return TRUE;
}
