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

/*	$Id: term_ncurses.c,v 1.8 2004/07/11 00:29:37 rrt Exp $	*/

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
#include "zterm.h"

static Terminal thisterm = {
	/* Unitialised screen pointer */
	NULL,

	/* Uninitialized width and height. */
	-1, -1,
};

Terminal *termp = &thisterm;

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

void term_addch(int c)
{
        addch(c);
}

void term_addnstr(const char *s, int len)
{
        addnstr(s, len);
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

void term_beep(void)
{
	beep();
}

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

        termp->screen = newterm(NULL, stdout, stdin);
	set_term(termp->screen);

	termp->width = ZILE_COLS;
	termp->height = ZILE_LINES;
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

int term_open(void)
{
	int colors = lookup_bool_variable("colors") && has_colors();

	if (colors)
		start_color();
	noecho();
	nonl();
	raw();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	if (colors)
		init_colors();

	return TRUE;
}

int term_close(void)
{
	/* Clear last line.  */
	term_move(ZILE_LINES - 1, 0);
	term_clrtoeol();
	term_refresh();

	/* Free memory and finish with ncurses.  */
	free_rotation_buffers();
	endwin();
	delscreen(termp->screen);
	termp->screen = NULL;

	return TRUE;
}

static int translate_key(int c)
{
	switch (c) {
	case '\0':		/* C-@ */
		return KBD_CTL | '@';
	case '\1':  case '\2':  case '\3':  case '\4':  case '\5':
	case '\6':  case '\7':  case '\10':             case '\12':
	case '\13': case '\14':             case '\16': case '\17':
	case '\20': case '\21': case '\22': case '\23': case '\24':
	case '\25': case '\26': case '\27': case '\30': case '\31':
	case '\32':		/* C-a ... C-z */
		return KBD_CTL | ('a' + c - 1);
	case '\11':
		return KBD_TAB;
	case '\15':
		return KBD_RET;
	case '\37':
		return KBD_CTL | (c ^ 0x40);
#ifdef __linux__
	case 0627:		/* C-z */
		return KBD_CTL | 'z';
#endif
	case '\33':		/* META */
		return KBD_META;
	case KEY_PPAGE:		/* PGUP */
		return KBD_PGUP;
	case KEY_NPAGE:		/* PGDN */
		return KBD_PGDN;
	case KEY_HOME:		/* HOME */
		return KBD_HOME;
	case KEY_END:		/* END */
		return KBD_END;
	case KEY_DC:		/* DEL */
		return KBD_DEL;
	case KEY_BACKSPACE:	/* BS */
	case 0177:		/* BS */
		return KBD_BS;
	case KEY_IC:		/* INSERT */
		return KBD_INS;
	case KEY_LEFT:		/* LEFT */
		return KBD_LEFT;
	case KEY_RIGHT:		/* RIGHT */
		return KBD_RIGHT;
	case KEY_UP:		/* UP */
		return KBD_UP;
	case KEY_DOWN:		/* DOWN */
		return KBD_DOWN;
	case KEY_F(1):
		return KBD_F1;
	case KEY_F(2):
		return KBD_F2;
	case KEY_F(3):
		return KBD_F3;
	case KEY_F(4):
		return KBD_F4;
	case KEY_F(5):
		return KBD_F5;
	case KEY_F(6):
		return KBD_F6;
	case KEY_F(7):
		return KBD_F7;
	case KEY_F(8):
		return KBD_F8;
	case KEY_F(9):
		return KBD_F9;
	case KEY_F(10):
		return KBD_F10;
	case KEY_F(11):
		return KBD_F11;
	case KEY_F(12):
		return KBD_F12;
	default:
		if (c > 255)
			return KBD_NOKEY;	/* Undefined behaviour. */
		return c;
	}
}

#define MAX_UNGETKEY_BUF	16

static int ungetkey_buf[MAX_UNGETKEY_BUF];
static int *ungetkey_p = ungetkey_buf;

int term_getkey(void)
{
	int c, key;

	if (ungetkey_p > ungetkey_buf)
		return *--ungetkey_p;

#ifdef KEY_RESIZE
	for (;;) {
		c = getch();
		if (c != KEY_RESIZE)
			break;
		resize_windows();
	}
#else
	c = getch();
#endif

	if (c == ERR)
		return KBD_NOKEY;

	key = translate_key(c);

	while (key == KBD_META) {
		c = getch();
		key = translate_key(c);
		key |= KBD_META;
	}

	return key;
}

static int xgetkey(int mode, int arg)
{
	int c = 0;
	switch (mode) {
	case GETKEY_NONFILTERED:
		c = getch();
		break;
	case GETKEY_DELAYED:
                wtimeout(stdscr, arg);
		c = term_getkey();
                wtimeout(stdscr, -1);
		break;
	case GETKEY_NONFILTERED|GETKEY_DELAYED:
                wtimeout(stdscr, arg);
		c = getch();
                wtimeout(stdscr, -1);
		break;
	}
	return c;
}

int term_xgetkey(int mode, int arg)
{
	int c;

	if (ungetkey_p > ungetkey_buf)
		return *--ungetkey_p;

#ifdef KEY_RESIZE
	for (;;) {
		c = xgetkey(mode, arg);
		if (c != KEY_RESIZE)
			break;
		resize_windows();
	}
#else
	c = xgetkey(mode, arg);
#endif

	return c;
}

int term_ungetkey(int key)
{
	if (ungetkey_p - ungetkey_buf >= MAX_UNGETKEY_BUF)
		return FALSE;

	*ungetkey_p++ = key;

	return TRUE;
}
