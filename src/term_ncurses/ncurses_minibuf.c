/*	$Id: ncurses_minibuf.c,v 1.4 2003/05/06 22:28:42 rrt Exp $	*/

/*
 * Copyright (c) 1997-2002 Sandro Sigala.  All rights reserved.
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

#if HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "zile.h"
#include "pathbuffer.h"
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

static void draw_minibuf_read(char *prompt, char *value, int prompt_len,
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

static char *rot_vminibuf_read(char *prompt, char *value, historyp hp,
			       char **p, int *max)
{
	static int overwrite_mode = 0;
	int c, i, len, prompt_len, thistab, lasttab = -1;
	char *s;

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
		case HISTORY_MATCHEDNONUNIQUE:
			s = " [Complete, but not unique]";
			break;
		case HISTORY_NOTMATCHED:
			s = " [No match]";
			break;
		case HISTORY_MATCHED:
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
			return *p;
		case KBD_CANCEL:
			move(LINES-1, 0);
			clrtoeol();
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
		case KBD_PGUP:
			if (hp == NULL) {
				ding();
				break;
			}

			if (hp->fl_poppedup) {
				hp->scroll_down(hp);
				thistab = lasttab;
			}
			break;
		case KBD_PGDN:
			if (hp == NULL) {
				ding();
				break;
			}

			if (hp->fl_poppedup) {
				hp->scroll_up(hp);
				thistab = lasttab;
			}
			break;
		case KBD_TAB:
		got_tab:
			if (hp == NULL) {
				ding();
				break;
			}

			if (lasttab != -1 && lasttab != HISTORY_NOTMATCHED
			    && hp->fl_poppedup) {
				hp->scroll_up(hp);
				thistab = lasttab;
			} else {
				thistab = hp->try(hp, *p);
				switch (thistab) {
				case HISTORY_NONUNIQUE:
				case HISTORY_MATCHED:
				case HISTORY_MATCHEDNONUNIQUE:
					if (hp->fl_dir)
						len = i = hp->matchsize + strlen(pathbuffer_str(hp->path));
					else
						len = i = hp->matchsize;
					*max = len + 1;
					s = (char *)zmalloc(*max);
					if (hp->fl_dir) {
						strcpy(s, pathbuffer_str(hp->path));
						strncat(s, hp->match, hp->matchsize);
					} else
						strncpy(s, hp->match, hp->matchsize);
					s[len] = '\0';
					if (strncmp(s, *p, len) != 0)
						thistab = -1;
					free(*p);
					*p = s;
					break;
				case HISTORY_NOTMATCHED:
					ding();
				}
			}
			break;
		case ' ':
			if (hp != NULL && !hp->fl_space)
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

char *ncurses_minibuf_read(char *prompt, char *value, historyp hp)
{
	static int max[MAX_ROTATIONS], rot;
	windowp wp, old_wp = cur_wp;
	char **p = rotation_buffers, *s;

	/* Rotate text buffer. */
	if (++rot >= MAX_ROTATIONS)
		rot = 0;
	s = rot_vminibuf_read(prompt, value, hp, &p[rot], &max[rot]);

	if (hp != NULL && hp->fl_poppedup && (wp = find_window("*Completions*")) != NULL) {
		cur_wp = wp;
		cur_bp = wp->bp;
		if (hp->fl_close)
			FUNCALL(delete_window);
		else
			switch_to_buffer(hp->old_bp);
		cur_wp = old_wp;
		cur_bp = old_wp->bp;
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
