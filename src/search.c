/*	$Id: search.c,v 1.2 2003/04/24 15:11:59 rrt Exp $	*/

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

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"
#include "astr.h"

static char *find_substr(const char *s1, size_t s1size,
			 const char *s2, size_t s2size)
{
	const char *e1 = s1 + s1size - 1, *e2 = s2 + s2size - 1;
	const char *sp1, *sp2;

	for (; s1 <= e1 - s2size + 1; ++s1) {
		if (tolower(*s1) != tolower(*s2))
			continue;
		sp1 = s1; sp2 = s2;
		for (;;) {
			++sp1, ++sp2;
			if (sp2 > e2)
				return (char *)s1;
			else if (tolower(*sp1) != tolower(*sp2))
				break;
		}
	}

	return NULL;
}

static char *rfind_substr(const char *s1, size_t s1size,
			  const char *s2, size_t s2size)
{
	const char *e1 = s1 + s1size - 1, *e2 = s2 + s2size - 1;
	const char *sp1, *sp2;

	for (; e1 >= s1 + s2size - 1; --e1) {
		if (tolower(*e1) != tolower(*e2))
			continue;
		sp1 = e1; sp2 = e2;
		for (;;) {
			--sp1, --sp2;
			if (sp2 < s2)
				return (char *)(sp1 + 1);
			else if (tolower(*sp1) != tolower(*sp2))
				break;
		}
	}

	return NULL;
}

static void goto_linep(linep lp)
{
	cur_wp->pointp = cur_bp->limitp->next;
	cur_wp->pointn = 0;
	cur_wp->pointo = 0;
	resync_redisplay();
	while (cur_wp->pointp != lp)
		next_line();
}

static int search_forward(linep startp, int starto, const char *s)
{
	linep lp;
	const char *sp, *sp2;
	size_t s1size, s2size = strlen(s);

	if (s2size < 1)
		return FALSE;

	for (lp = startp; lp != cur_bp->limitp; lp = lp->next) {
		if (lp == startp) {
			sp = lp->text + starto;
			s1size = lp->size - starto;
		} else {
			sp = lp->text;
			s1size = lp->size;
		}
		if (s1size < 1)
			continue;
		sp2 = find_substr(sp, s1size, s, s2size);
		if (sp2 != NULL) {
			goto_linep(lp);
			cur_wp->pointo = sp2 - lp->text + s2size;
			return TRUE;
		}
	}

	return FALSE;
}

static int search_backward(linep startp, int starto, const char *s)
{
	linep lp;
	const char *sp, *sp2;
	size_t s1size, s2size = strlen(s);

	if (s2size < 1)
		return FALSE;

	for (lp = startp; lp != cur_bp->limitp; lp = lp->prev) {
		sp = lp->text;
		if (lp == startp)
			s1size = starto;
		else
			s1size = lp->size;
		if (s1size < 1)
			continue;
		sp2 = rfind_substr(sp, s1size, s, s2size);
		if (sp2 != NULL) {
			goto_linep(lp);
			cur_wp->pointo = sp2 - lp->text;
			return TRUE;
		}
	}

	return FALSE;
}

static char *last_search = NULL;

DEFUN("search-forward", search_forward)
/*+
Search forward from point for the user specified text.
+*/
{
	char *ms;

	if ((ms = minibuf_read("Search: ", last_search)) == NULL)
		return cancel();
	if (ms[0] == '\0')
		return FALSE;

	if (last_search != NULL)
		free(last_search);
	last_search = zstrdup(ms);

	if (!search_forward(cur_wp->pointp, cur_wp->pointo, ms)) {
		minibuf_error("Search failed: `@v%s@@'", ms);
		return FALSE;
	}

	return TRUE;
}

DEFUN("search-backward", search_backward)
/*+
Search backward from point for the user specified text.
+*/
{
	char *ms;

	if ((ms = minibuf_read("Search backward: ", last_search)) == NULL)
		return cancel();
	if (ms[0] == '\0')
		return FALSE;

	if (last_search != NULL)
		free(last_search);
	last_search = zstrdup(ms);

	if (!search_backward(cur_wp->pointp, cur_wp->pointo, ms)) {
		minibuf_error("Search failed: `@v%s@@'", ms);
		return FALSE;
	}

	return TRUE;
}

#define ISEARCH_FORWARD		1
#define ISEARCH_BACKWARD	2

/*
 * Incremental search engine.
 */
static int isearch(int dir)
{
	int c;
	int last = TRUE;
	astr buf = astr_new();
	astr pattern = astr_new();
	linep startp = cur_wp->pointp;
	int starto = cur_wp->pointo;
	int startn = cur_wp->pointn;
	linep curp = startp;
	int curo = starto;
	int curn = startn;

	for (;;) {
		astr_fmt(buf, "%sI-search%s: %s",
			 last ? "" : "Failing ",
			 (dir == ISEARCH_FORWARD) ? "" : " backward",
			 astr_cstr(pattern));
		minibuf_write("%s", astr_cstr(buf));
		c = cur_tp->getkey();
		if (c == KBD_CANCEL) {
			cur_wp->pointp = startp;
			cur_wp->pointo = starto;
			cur_wp->pointn = startn;
			thisflag |= FLAG_NEED_RESYNC;
			minibuf_write("Quit: I-search");
			break;
		} else if (c == KBD_BS) {
			if (astr_size(pattern) > 0) {
				astr_truncate(pattern, astr_size(pattern) - 1);
				cur_wp->pointp = startp;
				cur_wp->pointo = starto;
				cur_wp->pointn = startn;
				thisflag |= FLAG_NEED_RESYNC;
			} else
				ding();
		} else if (c & KBD_CTL && ((c&255) == 'r' || (c&255) == 's')) {
			/* Invert direction. */
			if ((c & 255) == 'r' && dir == ISEARCH_FORWARD)
				dir = ISEARCH_BACKWARD;
			else if ((c & 255) == 's' && dir == ISEARCH_BACKWARD)
				dir = ISEARCH_FORWARD;
			if (astr_size(pattern) > 0) {
				/* Find next match. */
				curp = cur_wp->pointp;
				curo = cur_wp->pointo;
				curn = cur_wp->pointn;
				/* Save search string. */
				if (last_search != NULL)
					free(last_search);
				last_search = zstrdup(astr_cstr(pattern));
			} else if (last_search != NULL)
				astr_assign_cstr(pattern, last_search);
		} else if (c & KBD_META || c & KBD_CTL || c > KBD_TAB) {
			if (c == KBD_RET && astr_size(pattern) == 0) {
				if (dir == ISEARCH_FORWARD)
					FUNCALL(search_forward);
				else
					FUNCALL(search_backward);
			} else if (astr_size(pattern) > 0) {
				/* Save mark. */
				cur_bp->markp = startp;
				cur_bp->marko = starto;

				/* Save search string. */
				if (last_search != NULL)
					free(last_search);
				last_search = zstrdup(astr_cstr(pattern));
				minibuf_write("Mark saved when search started");
			} else
				minibuf_clear();
			break;
		} else
			astr_append_char(pattern, c);
		if (astr_size(pattern) > 0) {
			if (dir == ISEARCH_FORWARD)
				last = search_forward(curp, curo, astr_cstr(pattern));
			else
				last = search_backward(curp, curo, astr_cstr(pattern));
		} else
			last = TRUE;
		if (thisflag & FLAG_NEED_RESYNC)
			resync_redisplay();
		cur_tp->redisplay();
	}

	astr_delete(buf);
	astr_delete(pattern);

	return TRUE;
}

DEFUN("isearch-forward", isearch_forward)
/*+
Do incremental search forward.
As you type characters, they add to the search string and are found.
Type return to exit, leaving point at location found.
Type C-s to search again forward, C-r to search again backward.
C-g when search is successful aborts and moves point to starting point.
+*/
{
	return isearch(ISEARCH_FORWARD);
}

DEFUN("isearch-backward", isearch_backward)
/*+
Do incremental search backward.
As you type characters, they add to the search string and are found.
Type return to exit, leaving point at location found.
Type C-r to search again backward, C-s to search again forward.
C-g when search is successful aborts and moves point to starting point.
+*/
{
	return isearch(ISEARCH_BACKWARD);
}

void free_search_history(void)
{
	if (last_search != NULL)
		free(last_search);
}

DEFUN("replace-string", replace_string)
/*+
Replace occurrences of a string with other text.
+*/
{
	char *find, *repl;
	int count = 0;

	if ((find = minibuf_read("Replace string: ", "")) == NULL)
		return cancel();
	if (find[0] == '\0')
		return FALSE;

	if ((repl = minibuf_read("Replace `@v%s@@' with: ", "", find)) == NULL)
		return cancel();

	if (!strcmp(find, repl)) {
		minibuf_error("The two strings cannot match");
		return FALSE;
	}

	while (search_forward(cur_wp->pointp, cur_wp->pointo, find)) {
		++count;
		undo_save(UNDO_REPLACE_BLOCK, cur_wp->pointn, cur_wp->pointo - strlen(find),
			  strlen(find), strlen(repl));
		line_replace_text(&cur_wp->pointp, cur_wp->pointo - strlen(find),
				  strlen(find), repl);
	}

	if (thisflag & FLAG_NEED_RESYNC)
		resync_redisplay();
	cur_tp->redisplay();

	minibuf_write("Replaced %d occurrences", count);

	return TRUE;
}

DEFUN("query-replace", query_replace)
/*+
Replace occurrences of a string with other text.
As each match is found, the user must type a character saying
what to do with it.
+*/
{
	char *find, *repl;
	int count = 0, noask = FALSE, exitloop = FALSE;

	if ((find = minibuf_read("Query replace string: ", "")) == NULL)
		return cancel();
	if (find[0] == '\0')
		return FALSE;

	if ((repl = minibuf_read("Query replace `@v%s@@' with: ", "", find)) == NULL)
		return cancel();

	if (!strcmp(find, repl)) {
		minibuf_error("The two strings cannot match");
		return FALSE;
	}

	/*
	 * Spaghetti code follows... :-(
	 */
	while (search_forward(cur_wp->pointp, cur_wp->pointo, find)) {
		if (!noask) {
			int c;
			if (thisflag & FLAG_NEED_RESYNC)
				resync_redisplay();
			cur_tp->redisplay();
			for (;;) {
				minibuf_write("Query replacing `@v%s@@' with `@v%s@@' (y, n, !, ., q)? ", find, repl);
				c = cur_tp->getkey();
				switch (c) {
				case KBD_CANCEL:
				case KBD_RET:
				case ' ':
				case 'y':
				case 'n':
				case 'q':
				case '.':
				case '!':
					goto exitloop;
				}
				minibuf_error("Please answer y, n, !, . or q.");
				waitkey(2 * 1000);
			}
		exitloop:
			minibuf_clear();

			switch (c) {
			case KBD_CANCEL: /* C-g */
				return cancel();
			case 'q': /* Quit immediately. */
				goto endoffunc;
			case '.': /* Replace and quit. */
				exitloop = TRUE;
				goto replblock;
			case '!': /* Replace all without asking. */
				noask = TRUE;
				goto replblock;
			case ' ': /* Replace. */
			case 'y':
				goto replblock;
				break;
			case 'n': /* Do not replace. */
			case KBD_RET:
			case KBD_DEL:
				goto nextmatch;
			}
		}

	replblock:
		++count;
		undo_save(UNDO_REPLACE_BLOCK, cur_wp->pointn, cur_wp->pointo - strlen(find),
			  strlen(find), strlen(repl));
		line_replace_text(&cur_wp->pointp, cur_wp->pointo - strlen(find),
				  strlen(find), repl);
	nextmatch:
		if (exitloop)
			break;
	}

endoffunc:
	if (thisflag & FLAG_NEED_RESYNC)
		resync_redisplay();
	cur_tp->redisplay();

	minibuf_write("Replaced %d occurrences", count);

	return TRUE;
}
