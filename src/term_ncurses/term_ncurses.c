/* Exported terminal
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

/*	$Id: term_ncurses.c,v 1.12 2004/05/10 16:02:13 rrt Exp $	*/

/*
 * This module exports only the `ncurses_tp' pointer.
 */

#include "config.h"

#include <stddef.h>
#include <stdarg.h>
#if HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "zile.h"
#include "extern.h"
#include "term_ncurses.h"

static Terminal thisterm = {
	/* Unitialised screen pointer */
	NULL,

	/* Uninitialized width and height. */
	-1, -1,

	/* Pointers to ncurses terminal functions. */
	ncurses_init,
	ncurses_open,
        ncurses_close,
	ncurses_getkey,
	ncurses_xgetkey,
	ncurses_ungetkey,
	ncurses_refresh_cached_variables,
	ncurses_refresh,
	ncurses_redisplay,
	ncurses_full_redisplay,
	ncurses_show_about,
	ncurses_clear,
	ncurses_beep,
	ncurses_minibuf_write,
	ncurses_minibuf_read,
	ncurses_minibuf_clear,
};

Terminal *ncurses_tp = &thisterm;

Font ZILE_REVERSE = A_REVERSE, ZILE_BOLD = A_BOLD;

Font C_FG_BLACK;
Font C_FG_RED;
Font C_FG_GREEN;
Font C_FG_YELLOW;
Font C_FG_BLUE;
Font C_FG_MAGENTA;
Font C_FG_CYAN;
Font C_FG_WHITE;
Font C_FG_WHITE_BG_BLUE;

void term_init(void)
{
        C_FG_BLACK = COLOR_PAIR(ZILE_COLOR_BLACK);
        C_FG_RED = COLOR_PAIR(ZILE_COLOR_RED);
        C_FG_GREEN = COLOR_PAIR(ZILE_COLOR_GREEN);
        C_FG_YELLOW = COLOR_PAIR(ZILE_COLOR_YELLOW);
        C_FG_BLUE = COLOR_PAIR(ZILE_COLOR_BLUE);
        C_FG_MAGENTA = COLOR_PAIR(ZILE_COLOR_MAGENTA);
        C_FG_CYAN = COLOR_PAIR(ZILE_COLOR_CYAN);
        C_FG_WHITE = COLOR_PAIR(ZILE_COLOR_WHITE);
        C_FG_WHITE_BG_BLUE = COLOR_PAIR(ZILE_COLOR_BLUEBG);
}

void term_getyx(int *y, int *x)
{
        getyx(stdscr, *y, *x);
}

void term_move(int y, int x)
{
        move(y, x);
}

void term_clrtoeol(void)
{
        clrtoeol();
}

void term_refresh(void)
{
        refresh();
}

void term_clear(void)
{
        clear();
}

void term_addch(char c)
{
        addch(c);
}

void term_addnstr(const char *s, int len)
{
        addnstr(s, len);
}

void term_addstr(const char *s)
{
        addstr(s);
}

void term_mvaddch(int y, int x, char c)
{
        mvaddch(y, x, c);
}

void term_mvaddstr(int y, int x, char *s)
{
        mvaddstr(y, x, s);
}

void term_attrset(Font f)
{
        attrset(f);
}

int term_printw(const char *fmt, ...)
{
        int res;
        va_list valist;
        va_start(valist, fmt);
        res = vw_printw(stdscr, fmt, valist);
        va_end(valist);
        return res;
}
