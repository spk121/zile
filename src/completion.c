/*	$Id: completion.c,v 1.1 2004/02/08 04:39:26 dacap Exp $	*/

/*
 * Copyright (c) 1997-2003 Sandro Sigala.  All rights reserved.
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "zile.h"
#include "extern.h"
#include "agetcwd.h"

/***********************************************************************
			 Completion functions
 ***********************************************************************/

/* Forward declarations. */
static void default_completion_scroll_up(Completion *cp);
static void default_completion_scroll_down(Completion *cp);
static int default_completion_try(Completion *cp, astr as);
static int default_completion_reread(Completion *cp, astr as);

/*
 * Allocate a new completion structure.
 */
Completion *new_completion(int fileflag)
{
	Completion *cp;

	cp = (Completion *)zmalloc(sizeof(Completion));
	memset(cp, 0, sizeof(Completion));

	cp->completions = alist_new();
	cp->matches = alist_new();
	cp->try = default_completion_try;
	cp->scroll_up = default_completion_scroll_up;
	cp->scroll_down = default_completion_scroll_down;
	cp->reread = default_completion_reread;

	if (fileflag) {
		cp->path = astr_new();
		cp->fl_dir = TRUE;
	}

	return cp;
}

/*
 * Dispose an completion structure.
 */
void free_completion(Completion *cp)
{
	char *p;
	for (p = alist_first(cp->completions); p != NULL; p = alist_next(cp->completions))
		free(p);
	alist_delete(cp->completions);
	alist_delete(cp->matches);
	if (cp->fl_dir)
		astr_delete(cp->path);
	free(cp);
}

/*
 * The default completion scrolling function.
 */
static void default_completion_scroll_up(Completion *cp)
{
	Window *wp, *old_wp = cur_wp;

	wp = find_window("*Completions*");
	assert(wp != NULL);
	set_current_window(wp);
	if (cur_bp->pt.n == cur_bp->num_lines || !FUNCALL(scroll_up))
		gotobob();
	set_current_window(old_wp);

	cur_tp->redisplay();
}

/*
 * The default completion scrolling function.
 */
static void default_completion_scroll_down(Completion *cp)
{
	Window *wp, *old_wp = cur_wp;

	wp = find_window("*Completions*");
	assert(wp != NULL);
	set_current_window(wp);
	if (cur_bp->pt.n == 0 || !FUNCALL(scroll_down)) {
		gotoeob();
		resync_redisplay();
	}
	set_current_window(old_wp);

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
static void completion_print(alist al, int size)
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
 * Popup the completion window.
 */
static void popup_completion(Completion *cp, int allflag, int num)
{
	Window *wp, *old_wp = cur_wp;
	Buffer *bp;

	cp->fl_poppedup = 1;

	if ((wp = find_window("*Completions*")) == NULL) {
		if (head_wp->next == NULL)
			cp->fl_close = 1;
		set_current_window(popup_window ());
		if (!cp->fl_close)
			cp->old_bp = cur_bp;
		bp = find_buffer("*Completions*", TRUE);
		switch_to_buffer(bp);
	} else {
		set_current_window(wp);
	}

	zap_buffer_content();
	cur_bp->flags = BFLAG_NEEDNAME | BFLAG_NOSAVE | BFLAG_NOUNDO
		| BFLAG_NOEOB;
	set_temporary_buffer(cur_bp);

	if (allflag)
		completion_print(cp->completions, alist_count(cp->completions));
	else
		completion_print(cp->matches, num);

	gotobob();

	cur_bp->flags |= BFLAG_READONLY;

	set_current_window(old_wp);

	cur_tp->redisplay();
}

static int hcompar(const void *p1, const void *p2)
{
	return strcmp(*(const char **)p1, *(const char **)p2);
}

/*
 * The default completion matching function.
 */
static int default_completion_try(Completion *cp, astr search)
{
	int i, j, ssize, fullmatches = 0, partmatches = 0;
	char *p, c;

	alist_clear(cp->matches);

	if (cp->fl_dir)
		if (!cp->reread(cp, search))
			return COMPLETION_NOTMATCHED;

	if (!cp->fl_sorted) {
		alist_sort(cp->completions, hcompar);
		cp->fl_sorted = 1;
	}

	ssize = astr_size(search);

	if (ssize == 0) {
		if (alist_count(cp->completions) > 1) {
			cp->match = alist_first(cp->completions);
			cp->matchsize = 0;
			popup_completion(cp, TRUE, 0);
			return COMPLETION_NONUNIQUE;
		} else {
			cp->match = alist_first(cp->completions);
			cp->matchsize = strlen(cp->match);
			return COMPLETION_MATCHED;
		}
	}

	for (p = alist_first(cp->completions); p != NULL;
	     p = alist_next(cp->completions))
		if (!strncmp(p, astr_cstr(search), ssize)) {
			++partmatches;
			alist_append(cp->matches, p);
			if (!strcmp(p, astr_cstr(search)))
				++fullmatches;
		}

	if (partmatches == 0)
		return COMPLETION_NOTMATCHED;
	else if (partmatches == 1) {
		cp->match = alist_first(cp->matches);
		cp->matchsize = strlen(cp->match);
		return COMPLETION_MATCHED;
	}

	if (fullmatches == 1 && partmatches > 1) {
		cp->match = alist_first(cp->matches);
		cp->matchsize = strlen(cp->match);
		popup_completion(cp, FALSE, partmatches);
		return COMPLETION_MATCHEDNONUNIQUE;
	}

	for (j = ssize; ; ++j) {
		char *s = alist_first(cp->matches);
		c = s[j];
		for (i = 1; i < partmatches; ++i) {
			s = alist_at(cp->matches, i);
			if (s[j] != c) {
				cp->match = alist_first(cp->matches);
				cp->matchsize = j;
				popup_completion(cp, FALSE, partmatches);
				return COMPLETION_NONUNIQUE;
			}
		}
	}

	assert(0);
	return COMPLETION_NOTMATCHED;
}

/*
 * The default completion directory re-reading function.
 */
static int default_completion_reread(Completion *cp, astr as)
{
	astr buf, pdir, fname;
	DIR *dir;
	struct dirent *d;
	struct stat st;
	char *p;

	for (p = alist_first(cp->completions); p != NULL; p = alist_next(cp->completions))
		free(p);

	alist_clear(cp->completions);
	cp->fl_sorted = 0;

	buf = astr_new();
	agetcwd(buf);
	pdir = astr_new();
	fname = astr_new();
	if (!expand_path(astr_cstr(as), astr_cstr(buf), pdir, fname))
		return FALSE;

	if ((dir = opendir(astr_cstr(pdir))) == NULL)
		return FALSE;

	astr_assign(as, fname);

	while ((d = readdir(dir)) != NULL) {
		astr_assign_cstr(buf, astr_cstr(pdir));
		astr_append_cstr(buf, d->d_name);
		if (stat(astr_cstr(buf), &st) != -1) {
			astr_assign_cstr(buf, d->d_name);
			if (S_ISDIR(st.st_mode))
				astr_append_cstr(buf, "/");
		} else
			astr_assign_cstr(buf, d->d_name);
		alist_append(cp->completions, zstrdup(astr_cstr(buf)));
	}
	closedir(dir);

	compact_path(cp->path, astr_cstr(pdir));

	astr_delete(buf);
	astr_delete(pdir);
	astr_delete(fname);

	return TRUE;
}
