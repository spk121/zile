/* Font Lock Mode
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
   All rights reserved.

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

/*	$Id: fontlock.c,v 1.9 2004/02/17 23:19:42 rrt Exp $	*/

/*
 * In the future these routines may be rewritten using regex pattern
 * matching, simplifying a lot of the work.
 */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "zile.h"
#include "extern.h"

#if ENABLE_NONTEXT_MODES
#if ENABLE_CLIKE_MODES
/*
 * Parse the line for comments and strings and add anchors if found.
 * C/C++/C#/Java Mode.
 */
static void cpp_set_anchors(Line *lp, Anchor *lastanchor)
{
	char *sp;
	Anchor *ap;

	if (lp->size == 0) {
		lp->anchors = NULL;
		return;
	}

	ap = lp->anchors = (Anchor *)zmalloc(sizeof(Anchor) * lp->size);

	for (sp = lp->text; sp < lp->text + lp->size; ++sp)
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
		} else if (*sp == '/' && sp < lp->text + lp->size - 1 && *(sp+1) == '/') {
			/* // C++ style comment; ignore to end of line. */
			for (; sp < lp->text + lp->size; ++sp)
				*ap++ = ANCHOR_NULL;
		} else if (*sp == '\'') {
			/*
			 * Parse a character constant.  No anchors are
			 * required for characters since the characters
			 * cannot be longer than a line.
			 */
			*ap++ = ANCHOR_NULL;
			++sp;
			while (sp < lp->text + lp->size - 1 && *sp != '\'') {
				*ap++ = ANCHOR_NULL;
				if (*++sp == '\\' && sp < lp->text + lp->size - 1) {
					*ap++ = ANCHOR_NULL;
					++sp;
				}
			}
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
#endif /* ENABLE_CLIKE_MODES */

#if ENABLE_SHELL_MODE
static char *heredoc_string = NULL;

static int make_heredoc_string(Line *lp, char *sp, Anchor *ap)
{
	char *start, *end;

	if (heredoc_string) {
		free(heredoc_string);
		heredoc_string = NULL;
	}

	for (; sp < lp->text + lp->size; ++sp) {
		if (isspace(*sp)) {
			if (ap)
				*ap++ = ANCHOR_NULL;
			continue;
		}

		start = end = sp;
		for (; sp < lp->text + lp->size; ++sp) {
			if (!isalpha(*sp) && *sp != '_') {
				if (start == end)
					return FALSE;
				break;
			}

			if (ap) {
				if (sp == start)
					*ap++ = ANCHOR_BEGIN_HEREDOC;
				else
					*ap++ = ANCHOR_NULL;
			}
			end = sp;
		}

		heredoc_string = zmalloc(end - start + 2);
		strncpy(heredoc_string, start, end - start + 1);
		heredoc_string[end - start + 1] = '\0';
		ZTRACE(("special string = `%s'\n", heredoc_string));

		return TRUE;
	}

	return FALSE;
}

/*
 * Parse the line for comments and strings and add anchors if found.
 * Shell Mode.
 */
static void shell_set_anchors(Line *lp, Anchor *lastanchor)
{
	char *sp;
	Anchor *ap;
	static int laststrchar;

	if (lp->size == 0) {
		lp->anchors = NULL;
		return;
	}

	ap = lp->anchors = (Anchor *)zmalloc(sizeof(Anchor) * lp->size);

	for (sp = lp->text; sp < lp->text + lp->size; sp++) {
		if (*lastanchor == ANCHOR_BEGIN_STRING) {
			/*
			 * We are in the middle of a string, so parse for
			 * finding the termination sequence.
			 */
			if (*sp == '\\' && sp < lp->text + lp->size - 1) {
				*ap++ = ANCHOR_NULL;
				*ap++ = ANCHOR_NULL;
				++sp;
			} else if (*sp == laststrchar) {
				/*
				 * Found a string termination.  Put an anchor
				 * to signal this.
				 */
				*ap++ = ANCHOR_END_STRING;
				*lastanchor = ANCHOR_NULL;
			} else {
				*ap++ = ANCHOR_NULL;
			}
		} else if (*lastanchor == ANCHOR_BEGIN_HEREDOC) {
			/* Special chunks "<< EOF \n ... \n " */
			if ((sp == lp->text) && heredoc_string &&
			    (*sp == *heredoc_string) &&
			    (lp->size == (int)strlen(heredoc_string)) &&
			    (strncmp(sp, heredoc_string, lp->size) == 0)) {
				int i, l = strlen(heredoc_string);
				for (i = 0; i < l - 1; ++i)
					*ap++ = ANCHOR_NULL;
				sp += l;
				*ap++ = ANCHOR_END_HEREDOC;
				*lastanchor = ANCHOR_NULL;
			} else {
				*ap++ = ANCHOR_NULL;
			}
		} else if (*sp == '"' || *sp == '`' || *sp == '\'') {
			/* Found just a string character (\', \" or \`) */
			if ((sp > lp->text) && (*(sp-1) == '\\')) {
				laststrchar = *sp;
				ap--;
				*ap++ = ANCHOR_BEGIN_STRING;
				*ap++ = ANCHOR_END_STRING;
				*lastanchor = ANCHOR_NULL;
			} else {
				/*
				 * Found a string beginning sequence.
				 * Put an anchor to signal this.
				 */
				laststrchar = *sp;
				*ap++ = ANCHOR_BEGIN_STRING;
				*lastanchor = ANCHOR_BEGIN_STRING;
			}
		} else if ((*sp == '<') && (sp > lp->text) && (*(sp-1) == '<')) {
			if (make_heredoc_string(lp, sp+1, ap+1)) {
				*lastanchor = ANCHOR_BEGIN_HEREDOC;
				break;
			} else
				*ap++ = ANCHOR_NULL;
		}
		/* Comment; ignore to end of line. */
		else if ((*sp == '#') && ((sp == lp->text) || (*(sp-1) != '$'))) {
			for (; sp < lp->text + lp->size; sp++)
				*ap++ = ANCHOR_NULL;
		} else
			*ap++ = ANCHOR_NULL;
	}
}
#endif /* ENABLE_SHELL_MODE */

static void line_set_anchors(Buffer *bp, Line *lp, Anchor *lastanchor)
{
	if (lp->anchors) {
		free(lp->anchors);
		lp->anchors = NULL;
	}

	switch (bp->mode) {
	case BMODE_C:
	case BMODE_CPP:
	case BMODE_CSHARP:
	case BMODE_JAVA:
#if ENABLE_CLIKE_MODES
		cpp_set_anchors(lp, lastanchor);
#endif
		break;
	case BMODE_SHELL:
#if ENABLE_SHELL_MODE
		shell_set_anchors(lp, lastanchor);
#endif
		break;
	}
}

/*
 * Parse again the line for finding the anchors.  This is called when
 * the line get modified.
 */
void font_lock_reset_anchors(Buffer *bp, Line *lp)
{
	Anchor lastanchor;
	lastanchor = find_last_anchor(bp, lp->prev);
	line_set_anchors(bp, lp, &lastanchor);
}

/*
 * Find the last set anchor walking backward in the line list.
 */
Anchor find_last_anchor(Buffer *bp, Line *lp)
{
	Anchor *ap;

	for (; lp != bp->limitp; lp = lp->prev) {
		if (lp->anchors == NULL || lp->size == 0)
			continue;
		for (ap = lp->anchors + lp->size - 1; ap >= lp->anchors; --ap)
			if (*ap != ANCHOR_NULL) {
#if ENABLE_SHELL_MODE
				if ((bp->mode == BMODE_SHELL) && (*ap == ANCHOR_BEGIN_HEREDOC))
					make_heredoc_string(lp, lp->text + (ap - lp->anchors), NULL);
#endif
				return *ap;
			}
	}

	return ANCHOR_NULL;
}

/*
 * Go to the beginning of the buffer and parse for anchors.
 */
static void font_lock_set_anchors(Buffer *bp)
{
	Line *lp;
	Anchor lastanchor = ANCHOR_NULL;

	minibuf_write("Font Lock: setting anchors in `%s'...", bp->name);
	cur_tp->refresh();

	for (lp = bp->limitp->next; lp != bp->limitp; lp = lp->next)
		line_set_anchors(bp, lp, &lastanchor);

	minibuf_clear();
}

/*
 * Go to the beginning of the buffer and remove the anchors.
 */
static void font_lock_unset_anchors(Buffer *bp)
{
	Line *lp;

	minibuf_write("Font Lock: removing anchors from `%s'...", bp->name);
	cur_tp->refresh();

	for (lp = bp->limitp->next; lp != bp->limitp; lp = lp->next)
		if (lp->anchors != NULL) {
			free(lp->anchors);
			lp->anchors = NULL;
		}

	minibuf_clear();
}
#endif /* ENABLE_NONTEXT_MODES */

DEFUN("font-lock-mode", font_lock_mode)
/*+
Toggle Font Lock mode.
When Font Lock mode is enabled, text is fontified as you type it.
+*/
{
	if (cur_bp->flags & BFLAG_FONTLOCK) {
		cur_bp->flags &= ~BFLAG_FONTLOCK;
#if ENABLE_NONTEXT_MODES
		font_lock_unset_anchors(cur_bp);
#endif
	} else {
		cur_bp->flags |= BFLAG_FONTLOCK;
#if ENABLE_NONTEXT_MODES
		font_lock_set_anchors(cur_bp);
#endif
	}

	return TRUE;
}

DEFUN("font-lock-refresh", font_lock_refresh)
/*+
Refresh the Font Lock mode internal structures.
This may be called when the fontification looks weird.
+*/
{
#if ENABLE_NONTEXT_MODES
	if (cur_bp->flags & BFLAG_FONTLOCK) {
		font_lock_unset_anchors(cur_bp);
		font_lock_set_anchors(cur_bp);
	}
#endif

	return TRUE;
}
