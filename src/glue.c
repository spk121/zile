/* Miscellaneous functions
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

   This file is part of Zile.

   Zile is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Zile is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with Zile; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id: glue.c,v 1.16 2004/11/14 23:57:54 rrt Exp $	*/

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
		term_beep();

	thisflag |= FLAG_GOT_ERROR;
}

/*
 * Wait for two seconds or until a key is pressed.
 * The key is then available with term_getkey().
 */
void waitkey(void)
{
	int c;

	if ((c = term_xgetkey(GETKEY_DELAYED, 20)) != KBD_NOKEY)
		term_unget(c);
}

/*
 * Copy a region of text into an allocated buffer.
 */
char *copy_text_block(int startn, int starto, size_t size)
{
	char *buf, *dp;
	int max_size, n, i;
	Line *lp;

	max_size = 10;
	dp = buf = (char *)zmalloc(max_size);

	lp = cur_bp->pt.p;
	n = cur_bp->pt.n;
	if (n > startn)
		do
			lp = lp->prev;
		while (--n > startn);
	else if (n < startn)
		do
			lp = lp->next;
		while (++n < startn);

	for (i = starto; dp - buf < (int)size;) {
		if (dp - buf + 1 > max_size) {
			int save_off = dp - buf;
			max_size += 10;
			buf = (char *)zrealloc(buf, max_size);
			dp = buf + save_off;
		}
		if (i < astr_len(lp->text))
			*dp++ = *astr_char(lp->text, i++);
		else {
			*dp++ = '\n';
			lp = lp->next;
			i = 0;
		}
	}

	return buf;
}

/*
 * Return a string of maximum length `maxlen' beginning with a `...'
 * sequence if a cut is need.
 */
astr shorten_string(char *s, int maxlen)
{
	int len;
        astr as = astr_new();

	if ((len = strlen(s)) <= maxlen)
		astr_cpy_cstr(as, s);
	else {
		astr_cpy_cstr(as, "...");
		astr_cat_cstr(as, s + len - maxlen + 3);
	}

	return as;
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
 * Compact the spaces into tabulations according to the `tw' tab width.
 */
void tabify_string(char *dest, char *src, int scol, int tw)
{
	char *sp, *dp;
	int dcol = scol, ocol = scol;

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
	int col = scol;

	for (sp = src, dp = dest; *sp != '\0'; ++sp)
		if (*sp == '\t') {
			do
				*dp++ = ' ', ++col;
			while ((col%tw) > 0);
		}
		else
			*dp++ = *sp, ++col;
	*dp = '\0';
}

/*
 * Jump to the specified line number and offset.
 */
void goto_point(Point pt)
{
	if (cur_bp->pt.n > pt.n)
		do
			FUNCALL(previous_line);
		while (cur_bp->pt.n > pt.n);
	else if (cur_bp->pt.n < pt.n)
		do
			FUNCALL(next_line);
		while (cur_bp->pt.n < pt.n);

	if (cur_bp->pt.o > pt.o)
		do
			FUNCALL(backward_char);
		while (cur_bp->pt.o > pt.o);
	else if (cur_bp->pt.o < pt.o)
		do
			FUNCALL(forward_char);
		while (cur_bp->pt.o < pt.o);
}

/*
 * Return a zeroed allocated memory area.
 */
void *zmalloc(size_t size)
{
	void *ptr;

	assert(size > 0);

	if ((ptr = calloc(size, 1)) == NULL) {
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

/*
 * Read an arbitrary length string.
 */
char *
getln(FILE *fp)
{
        size_t len = 256;
        int c;
        char *l = zmalloc(len), *s = l;

        for (c = getc(fp); c != '\n' && c != EOF; c = getc(fp)) {
                if (s == l + len)
                        zrealloc(l, len *= 2);
                *s++ = c;
        }
        *s = '\0';

        return l;
}
