/*	$Id: ncurses_key.c,v 1.6 2004/01/28 09:24:23 rrt Exp $	*/

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
#if 0
	case '\34': case '\35': case '\36': case '\37':
	case '\40': case '\41': case '\42': case '\43': case '\44':
	case '\45': case '\46': case '\47': case '\50': case '\51':
	case '\52': case '\53': case '\54': case '\55': case '\56':
	case '\57': case '\60': case '\61': case '\62': case '\63':
	case '\64': case '\65': case '\66': case '\67': case '\70':
	case '\71': case '\72': case '\73': case '\74': case '\75':
	case '\76': case '\77': case '\100':
		return KBD_CTL | (c ^ 0x40);
#endif
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

extern void ncurses_resize_windows(void);

int ncurses_getkey(void)
{
	int c, key;

	if (ungetkey_p > ungetkey_buf)
		return *--ungetkey_p;

#ifdef KEY_RESIZE
	for (;;) {
		c = getch();
		if (c != KEY_RESIZE)
			break;
		ncurses_resize_windows();
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
		c = ncurses_getkey();
                wtimeout(stdscr, -1);
		break;
	case GETKEY_NONFILTERED|GETKEY_DELAYED:
                wtimeout(stdscr, arg);
		c = getch();
                wtimeout(stdscr, -1);
		break;
	case GETKEY_NONBLOCKING:
		nodelay(stdscr, TRUE);
		c = ncurses_getkey();
		nodelay(stdscr, FALSE);
		break;
	case GETKEY_NONFILTERED|GETKEY_NONBLOCKING:
		nodelay(stdscr, TRUE);
		c = getch();
		nodelay(stdscr, FALSE);
		break;
	}
	return c;
}

int ncurses_xgetkey(int mode, int arg)
{
	int c;

	if (ungetkey_p > ungetkey_buf)
		return *--ungetkey_p;

#ifdef KEY_RESIZE
	for (;;) {
		c = xgetkey(mode, arg);
		if (c != KEY_RESIZE)
			break;
		ncurses_resize_windows();
	}
#else
	c = xgetkey(mode, arg);
#endif

	return c;
}

int ncurses_ungetkey(int key)
{
	if (ungetkey_p - ungetkey_buf >= MAX_UNGETKEY_BUF)
		return FALSE;

	*ungetkey_p++ = key;

	return TRUE;
}
