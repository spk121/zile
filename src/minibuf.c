/*	$Id: minibuf.c,v 1.1 2001/01/19 22:02:42 ssigala Exp $	*/

/*
 * Copyright (c) 1997-2001 Sandro Sigala.  All rights reserved.
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

#include <sys/stat.h>
#include <assert.h>
#include <dirent.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "zile.h"
#include "extern.h"

/*--------------------------------------------------------------------------
 * Minibuffer wrapper functions.
 *--------------------------------------------------------------------------*/

/*
 * Formatting escapes:
 *  %s	 insert string
 *  %d	 insert integer
 *  %Ff	 start foreground color mode `f', where `f' can be one of:
 *	   b: blue     B: light blue
 *	   c: cyan     C: light cyan
 *	   g: green    G: light green
 *	   m: magenta  M: light magenta
 *	   r: red      R: light red
 *	   w: white    W: light white
 *	   y: yellow   Y: light yellow
 *	   k: black
 *  %Bb	 start background color mode `b', where `b' can be one of the
 *       characters described above.
 *  %Sfb start color mode `fb', where `f' is the foreground color,
 *       and `b' the background color (see above).
 *  %E	 end any color mode
 */
static char *minibuf_format(const char *fmt, va_list ap)
{
	char *buf, *sp, *dp;
	int maxsize;

	maxsize = max(cur_tp->width, 160);
	buf = (char *)xmalloc(maxsize);

	sp = (char *)fmt;
	dp = buf;

	while (*sp != '\0' && dp < buf + maxsize)
		if (*sp == '%')
			switch (*++sp) {
			case 'S':
				*dp++ = MINIBUF_SET_FGBG;
				*dp++ = *++sp;
				*dp++ = *++sp;
				++sp;
				break;
			case 'F':
				*dp++ = MINIBUF_SET_FG;
				*dp++ = *++sp;
				++sp;
				break;
			case 'B':
				*dp++ = MINIBUF_SET_BG;
				*dp++ = *++sp;
				++sp;
				break;
			case 'E':
				*dp++ = MINIBUF_UNSET;
				++sp;
				break;
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
		else
			*dp++ = *sp++;

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
	buf = minibuf_format(fmt, ap);
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
	buf = minibuf_format(fmt, ap);
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
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	if (value != NULL)
		p = cur_tp->minibuf_read(buf, value, NULL);
	else
		p = cur_tp->minibuf_read(buf, "", NULL);
	free(buf);

	return p;
}

char *minibuf_read_dir(const char *fmt, char *value, ...)
{
	va_list ap;
	char *buf, *p;
	historyp hp;
	static char rbuf[PATH_MAX];
	char dir[PATH_MAX], fname[PATH_MAX];

	va_start(ap, value);
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	getcwd(rbuf, PATH_MAX);
	expand_path(value, rbuf, dir, fname);
	strcat(dir, fname);
	compact_path(rbuf, dir);

	hp = new_history(0, TRUE);
	p = cur_tp->minibuf_read(buf, rbuf, hp);
	free_history(hp);
	free(buf);

	if (p == NULL)
		return p;

	getcwd(rbuf, PATH_MAX);
	expand_path(p, rbuf, dir, fname);
	strcpy(rbuf, dir);
	strcat(rbuf, fname);

	return rbuf;
}

int minibuf_read_forced(const char *fmt, const char *errmsg,
			historyp hp, ...)
{
	va_list ap;
	char *buf, *p;

	va_start(ap, hp);
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	for (;;) {
		p = cur_tp->minibuf_read(buf, "", hp);
		if (p == NULL) { /* Cancelled. */
			free(buf);
			return -1;
		} else {
			int i;
			/* Complete partial words if possible. */
			if (hp->try(hp, p) == HISTORY_MATCHED)
				p = hp->match;
			for (i = 0; i < hp->size; ++i)
				if (!strcmp(p, hp->completions[i])) {
					free(buf);
					return i;
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
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	hp = new_history(2, FALSE);
	hp->completions[0] = xstrdup("yes");
	hp->completions[1] = xstrdup("no");

	retvalue = minibuf_read_forced(buf, "%FCPlease answer yes or no.%E", hp);
	if (retvalue != -1) {
		/* The completions may be sorted by the minibuf completion
		   routines. */
		if (!strcmp(hp->completions[retvalue], "yes"))
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
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	hp = new_history(2, FALSE);
	hp->completions[0] = xstrdup("true");
	hp->completions[1] = xstrdup("false");

	retvalue = minibuf_read_forced(buf, "%FCPlease answer true or false.%E", hp);
	if (retvalue != -1) {
		/* The completions may be sorted by the minibuf completion
		   routines. */
		if (!strcmp(hp->completions[retvalue], "true"))
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
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	hp = new_history(sizeof valid / sizeof (char *), FALSE);
	for (i = 0; i < sizeof valid / sizeof (char *); ++i)
		hp->completions[i] = xstrdup(valid[i]);

	retvalue = minibuf_read_forced(buf, "%FCInvalid color name.%E", hp);
	if (retvalue != -1) {
		/* The completions may be sorted by the minibuf completion
		   routines. */
		for (i = 0; i < sizeof valid / sizeof (char *); ++i)
			if (!strcmp(hp->completions[retvalue], valid[i])) {
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
	buf = minibuf_format(fmt, ap);
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
historyp new_history(int size, int fileflag)
{
	historyp hp;

	hp = (historyp)xmalloc(sizeof *hp);
	memset(hp, 0, sizeof *hp);

	hp->size = size;
	hp->maxsize = max(size, 1);
	hp->completions = (char **)xmalloc(hp->maxsize * sizeof(char *));
	hp->matches = (char **)xmalloc(hp->maxsize * sizeof(char *));
	hp->try = default_history_try;
	hp->scroll_up = default_history_scroll_up;
	hp->scroll_down = default_history_scroll_down;
	hp->reread = default_history_reread;

	if (fileflag) {
		hp->path = (char *)xmalloc(PATH_MAX);
		hp->fl_dir = TRUE;
	}

	return hp;
}

/*
 * Dispose an history structure.
 */
void free_history(historyp hp)
{
	int i;
	for (i = 0; i < hp->size; ++i)
		free(hp->completions[i]);
	free(hp->completions);
	free(hp->matches);
	if (hp->fl_dir)
		free(hp->path);
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
static int calculate_max_length(char **vec, int size)
{
	int i, j, max = 0;

	for (i = 0; i < size; ++i)
		if ((j = strlen(vec[i])) > max)
			max = j;

	return max;
}

/*
 * Print the list of completions in a set of columns.
 */
static void history_print(char **vec, int size)
{
	int i, j, k, max, numcols;

	max = calculate_max_length(vec, size) + 5;
	numcols = (cur_wp->ewidth - 1) / max;

	bprintf("Possible completions are:\n");
	for (i = 0; i < size; ) {
		for (j = 0; i < size && j < numcols; ++i, ++j) {
			insert_string(vec[i]);
			for (k = max - strlen(vec[i]); k > 0; --k)
				insert_char(' ');
		}
		insert_newline();
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
		history_print(hp->completions, hp->size);
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
	char c, search[PATH_MAX];

	strcpy(search, s);
	if (hp->fl_dir)
		if (!hp->reread(hp, search))
			return HISTORY_NOTMATCHED;

	if (!hp->fl_sorted) {
		qsort(hp->completions, hp->size, sizeof(char *), hcompar);
		hp->fl_sorted = 1;
	}

	ssize = strlen(search);

#if 0
	/* For debugging... */
	minibuf_write(":%s:%s:%s:", hp->path, search, s);
	waitkey(3000);
#endif

	if (ssize == 0) {
		if (hp->size > 1) {
			hp->match = hp->completions[0];
			hp->matchsize = 0;
			popup_history(hp, TRUE, 0);
			return HISTORY_NONUNIQUE;
		} else {
			hp->match = hp->completions[0];
			hp->matchsize = strlen(hp->completions[0]);
			return HISTORY_MATCHED;
		}
	}

	for (i = 0; i < hp->size; ++i)
		if (!strncmp(hp->completions[i], search, ssize)) {
			hp->matches[partmatches++] = hp->completions[i];
			if (!strcmp(hp->completions[i], search))
				++fullmatches;
		}

	if (partmatches == 0)
		return HISTORY_NOTMATCHED;
	else if (partmatches == 1) {
		hp->match = hp->matches[0];
		hp->matchsize = strlen(hp->matches[0]);
		return HISTORY_MATCHED;
	}

	if (fullmatches == 1 && partmatches > 1) {
		hp->match = hp->matches[0];
		hp->matchsize = strlen(hp->matches[0]);
		popup_history(hp, FALSE, partmatches);
		return HISTORY_MATCHEDNONUNIQUE;
	}

	for (j = ssize; ; ++j) {
		c = hp->matches[0][j];
		for (i = 1; i < partmatches; ++i)
			if (hp->matches[i][j] != c) {
				hp->match = hp->matches[0];
				hp->matchsize = j;
				popup_history(hp, FALSE, partmatches);
				return HISTORY_NONUNIQUE;
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
	char buf[PATH_MAX], pdir[PATH_MAX], fname[PATH_MAX];
	DIR *dir;
	struct dirent *d;
	struct stat st;
	int i;

	for (i = 0; i < hp->size; ++i)
		free(hp->completions[i]);

	hp->size = 0;
	hp->fl_sorted = 0;

	getcwd(buf, PATH_MAX);
	if (!expand_path(s, buf, pdir, fname))
		return FALSE;

	if ((dir = opendir(pdir)) == NULL)
		return FALSE;

	strcpy(s, fname);

	while ((d = readdir(dir)) != NULL) {
		if (hp->size + 1 >= hp->maxsize) {
			hp->maxsize += 10;
			hp->completions = (char **)xrealloc(hp->completions, hp->maxsize * sizeof(char *));
			hp->matches = (char **)xrealloc(hp->matches, hp->maxsize * sizeof(char *));
		}
		strcpy(buf, pdir);
		strcat(buf, d->d_name);
		if (stat(buf, &st) != -1) {
			strcpy(buf, d->d_name);
			if (S_ISDIR(st.st_mode))
				strcat(buf, "/");
		} else
			strcpy(buf, d->d_name);
		hp->completions[hp->size] = xstrdup(buf);
		++hp->size;
	}
	closedir(dir);

	compact_path(hp->path, pdir);

	return TRUE;
}
