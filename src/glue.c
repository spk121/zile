/*	$Id: glue.c,v 1.6 2004/01/28 10:47:33 rrt Exp $	*/

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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

/*
 * Emit an error sound only when the appropriate variable
 * is set.
 */
void ding(void)
{
	if (thisflag & FLAG_DEFINING_MACRO)
		cancel_kbd_macro();

	if (lookup_bool_variable("beep"))
		cur_tp->beep();

	thisflag |= FLAG_GOT_ERROR;
}

/*
 * Wait for `msecs' milliseconds or until a key is pressed.
 * The key is then available with cur_tp->getkey().
 */
void waitkey(int msecs)
{
	int c;

	if ((c = cur_tp->xgetkey(GETKEY_DELAYED, msecs)) != KBD_NOKEY)
		cur_tp->ungetkey(c);
}

/*
 * Wait for `msecs' milliseconds or until a key is pressed.
 * The key is returned (the editor doesn't see it).
 */
int waitkey_discard(int msecs)
{
	return cur_tp->xgetkey(GETKEY_DELAYED, msecs);
}

/*
 * Copy a region of text into an allocated buffer.
 */
char *copy_text_block(int startn, int starto, size_t size)
{
	char *buf, *sp, *dp;
	int max_size, n;
	linep lp;

	max_size = 10;
	dp = buf = (char *)zmalloc(max_size);

	lp = cur_wp->pointp;
	n = cur_wp->pointn;
	if (n > startn)
		do
			lp = lp->prev;
		while (--n > startn);
	else if (n < startn)
		do
			lp = lp->next;
		while (++n < startn);

	sp = lp->text + starto;

	while (dp - buf < (int)size) {
		if (dp - buf + 1 > max_size) {
			int save_off = dp - buf;
			max_size += 10;
			buf = (char *)zrealloc(buf, max_size);
			dp = buf + save_off;
		}
		if (sp < lp->text + lp->size)
			*dp++ = *sp++;
		else {
			*dp++ = '\n';
			lp = lp->next;
			sp = lp->text;
		}
	}

	return buf;
}

/*
 * Return a string of maximum length `maxlen' beginning with a `...'
 * sequence if a cut is need.
 */
char *shorten_string(char *dest, char *s, int maxlen)
{
	int len;

	if ((len = strlen(s)) <= maxlen)
		strcpy(dest, s);
	else {
		strcpy(dest, "...");
		strcpy(dest + 3, s + len - maxlen + 3);
	}

	return dest;
}

/*
 * Replace the matches of `match' into `s' with the string `subst'.  The
 * two strings `match' and `subst' must have the same length.
 */
char *replace_string(char *s, char *match, char *subst)
{
	char *sp = s, *p;
	size_t slen = strlen(subst);

	if (strlen(match) != slen)
		return NULL;
	while ((p = strstr(sp, match)) != NULL) {
		strncpy(p, subst, slen);
		sp = p + slen;
	}

	return s;
}

/*
 * Get the current column number taking into account also the tabulations.
 */
int get_text_goalc(windowp wp)
{
	int col = 0, t = wp->bp->tab_width;
	char *sp = wp->pointp->text, *p = sp;

	while (p < sp + wp->pointo) {
		if (*p == '\t')
			col |= t - 1;
		++col, ++p;
	}

	return col;
}

/*
 * Compact the spaces into tabulations according to the `tw' tab width.
 */
void tabify_string(char *dest, char *src, int scol, int tw)
{
	char *sp, *dp;
	int dcol = scol, ocol = 0;

	for (sp = src, dp = dest;; ++sp)
		switch (*sp) {
		case ' ':
			++dcol;
			break;
		case '\t':
			dcol += tw;
			dcol -= dcol % tw;
			break;
		default:
			while (((ocol + tw) - (ocol + tw) % tw) <= dcol) {
				if (ocol + 1 == dcol)
					break;
				*dp++ = '\t';
				ocol += tw;
				ocol -= ocol % tw;
			}
			while (ocol < dcol) {
				*dp++ = ' ';
				ocol++;
			}
			*dp++ = *sp;
			if (*sp == '\0')
				return;
			++ocol;
			++dcol;
		}
}

/*
 * Expand the tabulations into spaces according to the `tw' tab width.
 * The output buffer should be big enough to contain the expanded string.
 * To be sure, sizeof(dest) should be >= strlen(src)*tw + 1.
 */
void untabify_string(char *dest, char *src, int scol, int tw)
{
	char *sp, *dp;
	int col = scol, w;

	for (sp = src, dp = dest; *sp != '\0'; ++sp)
		if (*sp == '\t')
			for (w = tw - col % tw; w > 0; --w)
				*dp++ = ' ', ++col;
		else
			*dp++ = *sp, ++col;
	*dp = '\0';
}

/*
 * Calculate the mark line number.
 */
int calculate_mark_lineno(windowp wp)
{
	int n;
	linep lp;

	if (wp->bp->markp == NULL)
		return -1;

	if (wp->bp->markp == wp->pointp)
		return wp->pointn;

	n = wp->pointn;
	for (lp = wp->pointp->prev; lp != wp->bp->limitp; lp = lp->prev) {
		--n;
		if (lp == wp->bp->markp)
			return n;
	}

	n = wp->pointn;
	for (lp = wp->pointp->next; lp != wp->bp->limitp; lp = lp->next) {
		++n;
		if (lp == wp->bp->markp)
			return n;
	}

	assert(0); /* Cannot happen */
	return 0;
}

/*
 * Jump to the specified line number and offset.
 */
void goto_point (int pointn, int pointo)
{
	if (cur_wp->pointn > pointn)
		do
			FUNCALL(previous_line);
		while (cur_wp->pointn > pointn);
	else if (cur_wp->pointn < pointn)
		do
			FUNCALL(next_line);
		while (cur_wp->pointn < pointn);
	if (cur_wp->pointo > pointo)
		do
			FUNCALL(backward_char);
		while (cur_wp->pointo > pointo);
	else if (cur_wp->pointo < pointo)
		do
			FUNCALL(forward_char);
		while (cur_wp->pointo < pointo);
}

/*
 * Return an allocated memory area.
 */
void *zmalloc(size_t size)
{
	void *ptr;

	assert(size > 0);

	if ((ptr = malloc(size)) == NULL) {
		fprintf(stderr, "zile: cannot allocate memory\n");
		zile_exit(1);
	}

	return ptr;
}

/*
 * Resize an allocated memory area.
 */
void *zrealloc(void *ptr, size_t size)
{
	void *newptr;

	assert(size > 0);

	if ((newptr = realloc(ptr, size)) == NULL) {
		fprintf(stderr, "zile: cannot allocate memory\n");
		zile_exit(1);
	}

	return newptr;
}

/*
 * Duplicate a string.
 */
char *zstrdup(const char *s)
{
	return strcpy(zmalloc(strlen(s) + 1), s);
}

#ifdef DEBUG
/*
 * Append a debug message to `zile.dbg'.
 */
void ztrace(const char *fmt, ...)
{
	static FILE *dbgfile = NULL;
	va_list ap;
	if (dbgfile == NULL) {
		if ((dbgfile = fopen("zile.dbg", "w")) == NULL)
			return;
	}
	va_start(ap, fmt);
	vfprintf(dbgfile, fmt, ap);
	va_end(ap);
}
#endif
