/*	$Id: ncurses_minibuf.c,v 1.12 2004/02/08 04:39:26 dacap Exp $	*/

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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>

#include "zile.h"
#include "extern.h"

#include "term_ncurses.h"

static chtype filter_color(unsigned char c)
{
	switch (c) {
	case 'k': case 'K': return COLOR_BLACK;
	case 'r': case 'R': return COLOR_RED;
	case 'g': case 'G': return COLOR_GREEN;
	case 'y': case 'Y': return COLOR_YELLOW;
	case 'b': case 'B': return COLOR_BLUE;
	case 'm': case 'M': return COLOR_MAGENTA;
	case 'c': case 'C': return COLOR_CYAN;
	case 'w': case 'W': return COLOR_WHITE;
	}

	return 0;
}

static size_t astrlen(const char *s)
{
	size_t size = 0;

	while (*s != '\0')
		switch (*s++) {
		case MINIBUF_SET_COLOR:
			++s;
			/* FALLTROUGH */
		case MINIBUF_UNSET_COLOR:
			break;
		default:
			++size;
		}

	return size;
}

void ncurses_minibuf_clear(void)
{
	int y, x;

	getyx(stdscr, y, x);
	move(LINES - 1, 0);
	clrtoeol();
	move(y, x);
	refresh();
}

static void xminibuf_write(const char *fmt)
{
	int y, x, bold;
	const unsigned char *p = fmt;
	chtype attr = 0, oldattr = 0;

	getyx(stdscr, y, x);

	for (; *p != '\0' && x < COLS; p++)
		switch (*p) {
		case MINIBUF_SET_COLOR:
			++p;
			bold = isupper(*p) ? A_BOLD : 0;
			oldattr = attr;
			attr = COLOR_PAIR(filter_color(*p)) | bold;
			break;
		case MINIBUF_UNSET_COLOR:
			attr = oldattr;
			break;
		default:
			addch(attr | *p);
			++x;
		}
}

void ncurses_minibuf_write(const char *fmt)
{
	int y, x;

	/* Save the current cursor position. */
	getyx(stdscr, y, x);

	move(LINES - 1, 0);
	xminibuf_write(fmt);
	clrtoeol();

	/* Restore the previous cursor position. */
	move(y, x);
}

static void draw_minibuf_read(const char *prompt, const char *value, int prompt_len,
			      char *match, int pointo)
{
	move(LINES - 1, 0);
	clrtoeol();
	xminibuf_write(prompt);

	if (prompt_len + pointo + 1 < COLS) {
		attrset(C_FG_RED|A_BOLD);
		addnstr(value, COLS - prompt_len - 1);
		attrset(0);
		addstr(match);
		if ((int)strlen(value) >= COLS - prompt_len - 1)
			mvaddch(LINES - 1, COLS - 1, '>' | C_FG_GREEN|A_BOLD);
		move(LINES - 1, prompt_len + pointo);
	} else {
		int n;
		addch('<' | C_FG_GREEN|A_BOLD);
		attrset(C_FG_RED|A_BOLD);
		n = pointo - pointo % (COLS - prompt_len - 2);
		addnstr(value + n, COLS - prompt_len - 2);
		attrset(0);
		addstr(match);
		if ((int)strlen(value + n) >= COLS - prompt_len - 2)
			mvaddch(LINES - 1, COLS - 1, '>' | C_FG_GREEN|A_BOLD);
		move(LINES - 1, prompt_len + 1 + pointo % (COLS - prompt_len - 2));
	}
}

static char *rot_vminibuf_read(const char *prompt, const char *value,
			       Completion *cp, History *hp, char **p, int *max)
{
	static int overwrite_mode = 0;
	int c, i, len, prompt_len, thistab, lasttab = -1;
	char *s, *saved = NULL;

	prompt_len = astrlen(prompt);

	len = i = strlen(value);
	if (*max < i + 10) {
		*max = i + 10;
		if (*p != NULL)
			*p = (char *)zrealloc(*p, *max);
		else
			*p = (char *)zmalloc(*max);
	}
	strcpy(*p, value);

	for (;;) {
		switch (lasttab) {
		case COMPLETION_MATCHEDNONUNIQUE:
			s = " [Complete, but not unique]";
			break;
		case COMPLETION_NOTMATCHED:
			s = " [No match]";
			break;
		case COMPLETION_MATCHED:
			s = " [Sole completion]";
			break;
		default:
			s = "";
		}
		draw_minibuf_read(prompt, *p, prompt_len, s, i);

		thistab = -1;

		switch (c = cur_tp->getkey()) {
		case KBD_NOKEY:
			break;
		case KBD_CTL | 'z':
			FUNCALL(suspend_zile);
			break;
		case KBD_RET:
			move(LINES-1, 0);
			clrtoeol();
			if (saved) free(saved);
			return *p;
		case KBD_CANCEL:
			move(LINES-1, 0);
			clrtoeol();
			if (saved) free(saved);
			return NULL;
		case KBD_CTL | 'a':
		case KBD_HOME:
			i = 0;
			break;
		case KBD_CTL | 'e':
		case KBD_END:
			i = len;
			break;
		case KBD_CTL | 'b':
		case KBD_LEFT:
			if (i > 0)
				--i;
			else
				ding();
			break;
		case KBD_CTL | 'f':
		case KBD_RIGHT:
			if (i < len)
				++i;
			else
				ding();
			break;
		case KBD_CTL | 'k':
			/* XXX no kill-register save is done yet. */
			if (i < len) {
				len -= len - i;
				(*p)[i] = '\0';
			} else
				ding();
			break;
		case KBD_BS:
			if (i > 0) {
				--i, --len;
				strcpy(*p + i, *p + i + 1);
			} else
				ding();
			break;
		case KBD_DEL:
			if (i < len) {
				--len;
				strcpy(*p + i, *p + i + 1);
			} else
				ding();
			break;
		case KBD_INS:
			overwrite_mode = overwrite_mode ? 0 : 1;
			break;
		case KBD_META | 'v':
		case KBD_PGUP:
			if (cp == NULL) {
				ding();
				break;
			}

			if (cp->fl_poppedup) {
				cp->scroll_down(cp);
				thistab = lasttab;
			}
			break;
		case KBD_CTL | 'v':
		case KBD_PGDN:
			if (cp == NULL) {
				ding();
				break;
			}

			if (cp->fl_poppedup) {
				cp->scroll_up(cp);
				thistab = lasttab;
			}
			break;
		case KBD_META | 'p':
			if (hp) {
				const char *elem = previous_history_element(hp);
				if (elem) {
					if (!saved)
						saved = zstrdup(*p);

					len = i = strlen(elem);
					*max = i + 10;
					*p = zrealloc(*p, *max);
					strcpy(*p, elem);
				}
/* 				else { */
/* 					minibuf_error("Beginning of history; no preceding item"); */
/* 				} */
			}
			break;
		case KBD_META | 'n':
			if (hp) {
				const char *elem = next_history_element(hp);
				if (elem) {
					len = i = strlen(elem);
					*max = i + 10;
					*p = zrealloc(*p, *max);
					strcpy(*p, elem);
				}
				else if (saved) {
					len = i = strlen(saved);
					*max = i + 10;
					*p = zrealloc(*p, *max);
					strcpy(*p, saved);

					free(saved);
					saved = NULL;
				}
/* 				else { */
/* 					minibuf_error("End of history; no default item"); */
/* 				} */
			}
			break;
		case KBD_TAB:
		got_tab:
			if (cp == NULL) {
				ding();
				break;
			}

			if (lasttab != -1 && lasttab != COMPLETION_NOTMATCHED
			    && cp->fl_poppedup) {
				cp->scroll_up(cp);
				thistab = lasttab;
			} else {
                                astr as = astr_copy_cstr(*p);
				thistab = cp->try(cp, as);
                                astr_delete(as);
				switch (thistab) {
				case COMPLETION_NONUNIQUE:
				case COMPLETION_MATCHED:
				case COMPLETION_MATCHEDNONUNIQUE:
					if (cp->fl_dir)
						len = i = cp->matchsize + strlen(astr_cstr(cp->path));
					else
						len = i = cp->matchsize;
					*max = len + 1;
					s = (char *)zmalloc(*max);
					if (cp->fl_dir) {
						strcpy(s, astr_cstr(cp->path));
						strncat(s, cp->match, cp->matchsize);
					} else
						strncpy(s, cp->match, cp->matchsize);
					s[len] = '\0';
					if (strncmp(s, *p, len) != 0)
						thistab = -1;
					free(*p);
					*p = s;
					break;
				case COMPLETION_NOTMATCHED:
					ding();
				}
			}
			break;
		case ' ':
			if (cp != NULL && !cp->fl_space)
				goto got_tab;
			/* FALLTROUGH */
		default:
			if (c > 255 || !isprint(c)) {
				ding();
				break;
			}
			if (!overwrite_mode || i == len) {
				char *s;
				if (len >= *max - 1) {
					*max *= 2;
					*p = (char *)zrealloc(*p, *max);
				}
				s = *p;
				for (; *s != '\0'; s++)
					;
				for (++s; s > *p + i; --s)
					*s = *(s - 1);
				*(*p + i) = c;
				++i, ++len;
			} else
				(*p)[i++] = c;
		}

		lasttab = thistab;
	}
}

/*
 * Rotation of text buffer prevents trashing of the returned string
 * when doing less or equal `MAX_ROTATIONS' sequential calls.
 * This means that the returned string by `minibuf_read()' need
 * not to be deallocated by the user.
 */
#define MAX_ROTATIONS	5

static char *rotation_buffers[MAX_ROTATIONS];

char *ncurses_minibuf_read(const char *prompt, const char *value,
			   Completion *cp, History *hp)
{
	static int max[MAX_ROTATIONS], rot;
	Window *wp, *old_wp = cur_wp;
	char **p = rotation_buffers, *s;

	/* Prepare the history.  */
	if (hp) prepare_history(hp);

	/* Rotate text buffer. */
	if (++rot >= MAX_ROTATIONS)
		rot = 0;

	s = rot_vminibuf_read(prompt, value, cp, hp, &p[rot], &max[rot]);

	if (cp != NULL && cp->fl_poppedup && (wp = find_window("*Completions*")) != NULL) {
		set_current_window(wp);
		if (cp->fl_close)
			FUNCALL(delete_window);
		else if (cp->old_bp)
			switch_to_buffer(cp->old_bp);
		set_current_window(old_wp);
	}

	return s;
}

void ncurses_free_rotation_buffers(void)
{
	int i;
	for (i = 0; i < MAX_ROTATIONS; ++i)
		if (rotation_buffers[i] != NULL)
			free(rotation_buffers[i]);
}
