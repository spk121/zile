/*	$Id: fontlock.c,v 1.1 2001/01/19 22:02:09 ssigala Exp $	*/

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

/*
 * In the future these routines may be rewritten using regex pattern
 * matching, simplifying a lot of the work.
 */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "zile.h"
#include "extern.h"

/*
 * Parse the line for comments and strings and add anchors if found.
 */
static void line_set_anchors(linep lp, int *lastanchor)
{
	char *sp, *ap;

	if (lp->size == 0) {
		lp->anchors = NULL;
		return;
	}

	ap = lp->anchors = (char *)xmalloc(lp->size);

	for (sp = lp->text; sp < lp->text + lp->size; sp++)
		if (*lastanchor == ANCHOR_BEGIN_COMMENT) {
			/*
			 * We are in the middle of a comment, so parse
			 * for finding the termination sequence.
			 */
			if (*sp == '*' && sp < lp->text + lp->size - 1 && *(sp+1) == '/') {
				/*
				 * Found a comment termination sequence.
				 * Put an anchor to signal this.
				 */
				++sp;
				*ap++ = ANCHOR_NULL;
				*ap++ = ANCHOR_END_COMMENT;
				*lastanchor = ANCHOR_NULL;
			} else
				*ap++ = ANCHOR_NULL;
		} else if (*lastanchor == ANCHOR_BEGIN_STRING) {
			/*
			 * We are in the middle of a string, so parse for
			 * finding the termination sequence.
			 */
			if (*sp == '\\' && sp < lp->text + lp->size - 1) {
				*ap++ = ANCHOR_NULL;
				*ap++ = ANCHOR_NULL;
				++sp;
			} else if (*sp == '"') {
				/*
				 * Found a string termination.  Put an anchor
				 * to signal this.
				 */
				*ap++ = ANCHOR_END_STRING;
				*lastanchor = ANCHOR_NULL;
			} else
				*ap++ = ANCHOR_NULL;
		} else if (*sp == '/' && sp < lp->text + lp->size - 1 && *(sp+1) == '*') {
			/*
			 * Found a comment termination sequence.  Put an
			 * anchor to signal this.
			 */
			++sp;
			*ap++ = ANCHOR_BEGIN_COMMENT;
			*ap++ = ANCHOR_NULL;
			*lastanchor = ANCHOR_BEGIN_COMMENT;
		} else if (*sp == '\'') {
			/*
			 * Parse a character constant.  No anchors are
			 * required for characters since the characters
			 * cannot be longer than a line.
			 */
			*ap++ = ANCHOR_NULL;
			do {
				*ap++ = ANCHOR_NULL;
				if (*++sp == '\\' && sp < lp->text + lp->size - 1) {
					*ap++ = ANCHOR_NULL;
					++sp;
				}
			} while (sp < lp->text + lp->size - 1 && *sp != '\'');
		} else if (*sp == '"') {
			/*
			 * Found a string beginning sequence.  Put an anchor to
			 * signal this.
			 */
			*ap++ = ANCHOR_BEGIN_STRING;
			*lastanchor = ANCHOR_BEGIN_STRING;
		} else
			*ap++ = ANCHOR_NULL;
}

/*
 * Parse again the line for finding the anchors.  This is called when
 * the line get modified.
 */
void font_lock_reset_anchors(bufferp bp, linep lp)
{
	int lastanchor;

	if (lp->anchors != NULL)
		free(lp->anchors);
	lp->anchors = NULL;
	lastanchor = find_last_anchor(bp, lp->prev);
	line_set_anchors(lp, &lastanchor);
}

/*
 * Find the last set anchor walking backward in the line list.
 */
int find_last_anchor(bufferp bp, linep lp)
{
	char *p;

	for (; lp != bp->limitp; lp = lp->prev)
		if (lp->size > 0)
			for (p = lp->anchors + lp->size - 1; p >= lp->anchors; --p)
				if (*p != ANCHOR_NULL)
					return *p;

	return ANCHOR_NULL;
}

/*
 * Go to the beginning of the buffer and parse for anchors.
 */
static void font_lock_set_anchors(bufferp bp)
{
	linep lp;
	int lastanchor = ANCHOR_NULL;

	minibuf_write("Font Lock: setting anchors in `%s'...", bp->name);
	cur_tp->refresh();

	for (lp = bp->limitp->next; lp != bp->limitp; lp = lp->next)
		line_set_anchors(lp, &lastanchor);

	minibuf_clear();
}

/*
 * Go to the beginning of the buffer and remove the anchors.
 */
static void font_lock_unset_anchors(bufferp bp)
{
	linep lp;

	minibuf_write("Font Lock: removing anchors from `%s'...", bp->name);
	cur_tp->refresh();

	for (lp = bp->limitp->next; lp != bp->limitp; lp = lp->next)
		if (lp->anchors != NULL) {
			free(lp->anchors);
			lp->anchors = NULL;
		}

	minibuf_clear();
}

DEFUN("font-lock-mode", font_lock_mode)
/*+
Toggle Font Lock mode.
When Font Lock mode is enabled, text is fontified as you type it.
+*/
{
	if (cur_bp->flags & BFLAG_FONTLOCK) {
		cur_bp->flags &= ~BFLAG_FONTLOCK;
		font_lock_unset_anchors(cur_bp);
	} else {
		cur_bp->flags |= BFLAG_FONTLOCK;
		font_lock_set_anchors(cur_bp);
	}

	return TRUE;
}

DEFUN("font-lock-refresh", font_lock_refresh)
/*+
Refresh the Font Lock mode internal structures.
This may be called when the fontification looks weird.
+*/
{
	if (cur_bp->flags & BFLAG_FONTLOCK) {
		font_lock_unset_anchors(cur_bp);
		font_lock_set_anchors(cur_bp);
	}

	return TRUE;
}
