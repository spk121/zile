/* Miscellaneous functions
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

/*	$Id: ncurses_misc.c,v 1.19 2004/05/09 18:00:33 rrt Exp $	*/

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
#include "extern.h"
#include "term_ncurses.h"

static void resize_windows(int width, int height)
{
	Window *wp;
	int hdelta = height - ncurses_tp->height;

	/* Resize windows horizontally. */
	for (wp = head_wp; wp != NULL; wp = wp->next)
		wp->fwidth = wp->ewidth = width;

	/* Resize windows vertically. */
	if (hdelta > 0) { /* Increase windows height. */
		for (wp = head_wp; hdelta > 0; wp = wp->next) {
			if (wp == NULL)
				wp = head_wp;
			++wp->fheight;
			++wp->eheight;
			--hdelta;
		}
	} else { /* Decrease windows height. */
		int decreased = TRUE;
		while (decreased) {
			decreased = FALSE;
			for (wp = head_wp; wp != NULL && hdelta < 0; wp = wp->next)
				if (wp->fheight > 2) {
					--wp->fheight;
					--wp->eheight;
					++hdelta;
					decreased = TRUE;
				}
		}
	}

	/*
	 * Sometimes Zile cannot reduce the windows height to a certain
	 * value (too small); take care of this case.
	 */
	ncurses_tp->width = width;
	ncurses_tp->height = height - hdelta;
}

void ncurses_resize_windows(void)
{
	/* Resize the Zile windows. */
	resize_windows(ZILE_COLS, ZILE_LINES);
	FUNCALL(recenter);
}

int ncurses_init(void)
{
	ncurses_tp->screen = newterm(NULL, stdout, stdin);
	set_term(ncurses_tp->screen);

	ncurses_tp->width = ZILE_COLS;
	ncurses_tp->height = ZILE_LINES;

	return TRUE;
}

static void init_colors(void)
{
	int fg = COLOR_WHITE;
	int bg = COLOR_BLACK;

#ifdef NCURSES_VERSION
	if (use_default_colors() == OK) {
		fg = -1;
		bg = -1;
	}
#endif

	/* "WHITE" is used as synonym of "DEFAULT". */
	init_pair(ZILE_COLOR_WHITE, fg, bg);

	init_pair(ZILE_COLOR_BLACK,   COLOR_BLACK,   bg);
	init_pair(ZILE_COLOR_RED,     COLOR_RED,     bg);
	init_pair(ZILE_COLOR_GREEN,   COLOR_GREEN,   bg);
	init_pair(ZILE_COLOR_YELLOW,  COLOR_YELLOW,  bg);
	init_pair(ZILE_COLOR_BLUE,    COLOR_BLUE,    bg);
	init_pair(ZILE_COLOR_MAGENTA, COLOR_MAGENTA, bg);
	init_pair(ZILE_COLOR_CYAN,    COLOR_CYAN,    bg);

	init_pair(ZILE_COLOR_BLUEBG,  COLOR_CYAN,    COLOR_BLUE);
}

int ncurses_open(void)
{
	int colors = lookup_bool_variable("colors") && has_colors();

	if (colors)
		start_color();
	noecho();
	nonl();
	raw();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
/*	meta(stdscr, TRUE); */
	if (colors)
		init_colors();

	return TRUE;
}

extern void ncurses_free_rotation_buffers(void);

int ncurses_close(void)
{
	/* Clear last line.  */
	term_move(ZILE_LINES - 1, 0);
	term_clrtoeol();
	refresh();

	/* Free memory and finish with ncurses.  */
	ncurses_free_rotation_buffers();
	endwin();
	delscreen(ncurses_tp->screen);
	ncurses_tp->screen = NULL;

	return TRUE;
}

void ncurses_refresh(void)
{
	refresh();
}

static void show_splash_screen(const char *splash)
{
	int i, bold = 0, red = 0;
	const char *p;

	for (i = 0; i < ZILE_LINES - 2; ++i) {
		term_move(i, 0);
		term_clrtoeol();
	}

	term_move(0, 0);
	for (i = 0, p = splash; *p != '\0'; ++p)
		switch (*p) {
		case '%':
			attrset(bold ? 0: A_BOLD);
			bold ^= 1;
			break;
		case '$':
			attrset(red ? 0: A_BOLD | C_FG_RED);
			red ^= 1;
			break;
		case  '\n':
			term_move(++i, 0);
			break;
		default:
			term_addch(*p);
		}
}

void ncurses_show_about(const char *splash, const char *minibuf)
{
	if (!lookup_bool_variable("novice-level") &&
	    !lookup_bool_variable("skip-splash-screen")) {
		show_splash_screen(splash);
		minibuf_write(minibuf);
		waitkey(20 * 1000);
		minibuf_clear();
	} else
		minibuf_write(minibuf);
}

void ncurses_clear(void)
{
	erase();
}

void ncurses_beep(void)
{
	beep();
}
