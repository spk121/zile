/*	$Id: search.c,v 1.1 2001/01/19 22:02:45 ssigala Exp $	*/

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

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "zile.h"
#include "extern.h"

/*
 * XXX need to write an Emacs incremental search equivalent.
 */

static int search_forward(char *s)
{
	linep lp = cur_wp->pointp;
	int i = 0;
	char *sp, *dp, *dp2;

	while (lp != cur_bp->limitp) {
		if (lp == cur_wp->pointp)
			dp = lp->text + cur_wp->pointo;
		else
			dp = lp->text;
		for (; dp < lp->text + lp->size; ++dp) {
			if (tolower(*dp) != tolower(*s))
				continue;
			for (sp = s + 1, dp2 = dp + 1;; ++sp, ++dp2)
				if (*sp == '\0') {
					cur_wp->pointp = lp;
					cur_wp->pointn += i;
					cur_wp->pointo = dp2 - lp->text;
					thisflag |= FLAG_NEED_RESYNC;

					return TRUE;
				} else if (tolower(*sp) != tolower(*dp2))
					break;
		}
		++i;
		lp = lp->next;
	}

	return FALSE;
}

static int search_backward(char *s)
{
	linep lp = cur_wp->pointp;
	int i = 0;
	char *sp, *sp2, *dp, *dp2;

	if (s[0] != '\0')
		sp = s + strlen(s) - 1;
	else
		sp = s;

	while (lp != cur_bp->limitp) {
		if (lp == cur_wp->pointp)
			dp = lp->text + cur_wp->pointo;
		else
			dp = lp->text + lp->size;
		for (; dp >= lp->text; --dp) {
			if (tolower(*dp) != tolower(*sp))
				continue;
			for (sp2 = sp - 1, dp2 = dp - 1;; --sp2, --dp2)
				if (sp2 < s) {
					cur_wp->pointp = lp;
					cur_wp->pointn -= i;
					cur_wp->pointo = dp2 - lp->text + 1;
					thisflag |= FLAG_NEED_RESYNC;

					return TRUE;
				} else if (tolower(*sp2) != tolower(*dp2))
					break;
		}
		++i;
		lp = lp->prev;
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

	if ((ms = minibuf_read("Search forward: ", last_search)) == NULL)
		return cancel();
	if (ms[0] == '\0')
		return FALSE;

	if (last_search != NULL)
		free(last_search);
	last_search = xstrdup(ms);

	if (!search_forward(ms)) {
		minibuf_error("%FCFailed forward search for %FY`%s'%E", ms);
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
	last_search = xstrdup(ms);

	if (!search_backward(ms)) {
		minibuf_error("%FCFailed backward search for %FY`%s'%E", ms);
		return FALSE;
	}

	return TRUE;
}

void free_search_history(void)
{
	if (last_search != NULL)
		free(last_search);
}
