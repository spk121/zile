/*	$Id: minibuf.c,v 1.12 2004/02/14 10:17:19 dacap Exp $	*/

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
#include "agetcwd.h"

static History files_history;

/*--------------------------------------------------------------------------
 * Minibuffer wrapper functions.
 *--------------------------------------------------------------------------*/

static char *minibuf_format(const char *fmt, va_list ap)
{
	char *buf, *sp, *dp;
	vasprintf(&buf, fmt, ap);
	vsprintf(buf, fmt, ap);
	return buf;
}

void free_minibuf(void)
{
	free_history_elements(&files_history);
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
char *minibuf_read(const char *fmt, const char *value, ...)
{
	va_list ap;
	char *buf, *p;

	va_start(ap, value);
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	if (value != NULL)
		p = cur_tp->minibuf_read(buf, value, NULL, NULL);
	else
		p = cur_tp->minibuf_read(buf, "", NULL, NULL);
	free(buf);

	return p;
}

/* The returned buffer must be freed by the caller. */
char *minibuf_read_dir(const char *fmt, const char *value, ...)
{
	va_list ap;
	char *buf, *p;
	Completion *cp;
	astr dir, fname;
	astr rbuf;

	va_start(ap, value);
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	rbuf = astr_new();
	agetcwd(rbuf);
	dir = astr_new();
	fname = astr_new();
	expand_path(value, astr_cstr(rbuf), dir, fname);
	astr_append_cstr(dir, astr_cstr(fname));
	compact_path(rbuf, astr_cstr(dir));

	cp = new_completion(TRUE);
	p = cur_tp->minibuf_read(buf, astr_cstr(rbuf), cp, &files_history);
	free_completion(cp);
	free(buf);

	if (p == NULL) {
		astr_delete(dir);
		astr_delete(fname);
		astr_delete(rbuf);
		return p;
	}

	/* Add history element.  */
	add_history_element(&files_history, p);

	agetcwd(rbuf);
        astr_clear(dir);
        astr_clear(fname);
	expand_path(p, astr_cstr(rbuf), dir, fname);
	astr_assign_cstr(rbuf, astr_cstr(dir));
	astr_append_cstr(rbuf, astr_cstr(fname));

	astr_delete(dir);
	astr_delete(fname);
        p = zstrdup(astr_cstr(rbuf));
        astr_delete(rbuf);
        return p;
}

int minibuf_read_forced(const char *fmt, const char *errmsg,
			Completion *cp, ...)
{
	va_list ap;
	char *buf, *p;

	va_start(ap, cp);
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	for (;;) {
		p = cur_tp->minibuf_read(buf, "", cp, NULL);
		if (p == NULL) { /* Cancelled. */
			free(buf);
			return -1;
		} else {
			char *s;
                        astr as;
			/* Complete partial words if possible. */
                        as = astr_copy_cstr(p);
			if (cp->try(cp, as) == COMPLETION_MATCHED)
				p = cp->match;
                        astr_delete(as);
			for (s = alist_first(cp->completions); s != NULL;
			     s = alist_next(cp->completions))
				if (!strcmp(p, s)) {
					free(buf);
					return alist_current_idx(cp->completions);
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
	Completion *cp;
	int retvalue;

	va_start(ap, fmt);
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	cp = new_completion(FALSE);
	alist_append(cp->completions, zstrdup("yes"));
	alist_append(cp->completions, zstrdup("no"));

	retvalue = minibuf_read_forced(buf, "Please answer yes or no.", cp);
	if (retvalue != -1) {
		/* The completions may be sorted by the minibuf completion
		   routines. */
		if (!strcmp(alist_at(cp->completions, retvalue), "yes"))
			retvalue = TRUE;
		else
			retvalue = FALSE;
	}
	free_completion(cp);
	free(buf);

	return retvalue;
}

int minibuf_read_boolean(const char *fmt, ...)
{
	va_list ap;
	char *buf;
	Completion *cp;
	int retvalue;

	va_start(ap, fmt);
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	cp = new_completion(FALSE);
	alist_append(cp->completions, zstrdup("true"));
	alist_append(cp->completions, zstrdup("false"));

	retvalue = minibuf_read_forced(buf, "Please answer true or false.", cp);
	if (retvalue != -1) {
		/* The completions may be sorted by the minibuf completion
		   routines. */
		if (!strcmp(alist_at(cp->completions, retvalue), "true"))
			retvalue = TRUE;
		else
			retvalue = FALSE;
	}
	free_completion(cp);
	free(buf);

	return retvalue;
}

char *minibuf_read_color(const char *fmt, ...)
{
	va_list ap;
	char *buf;
	Completion *cp;
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

	cp = new_completion(FALSE);
	for (i = 0; i < sizeof valid / sizeof(char *); ++i)
		alist_append(cp->completions, zstrdup(valid[i]));

	retvalue = minibuf_read_forced(buf, "Invalid color name.", cp);
	if (retvalue != -1) {
		/* The completions may be sorted by the minibuf completion
		   routines. */
		for (i = 0; i < sizeof valid / sizeof (char *); ++i)
			if (!strcmp(alist_at(cp->completions, retvalue), valid[i])) {
				retvalue = i;
				break;
			}
	}
	free_completion(cp);
	free(buf);

	if (retvalue == -1)
		return NULL;
	return valid[retvalue];
}

/*
 * Read a string from the minibuffer using a completion.
 */
char *minibuf_read_completion(const char *fmt, char *value, Completion *cp, History *hp, ...)
{
	va_list ap;
	char *buf, *p;

	va_start(ap, hp);
	buf = minibuf_format(fmt, ap);
	va_end(ap);

	p = cur_tp->minibuf_read(buf, value, cp, hp);
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
