/* Exported terminal
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
   Copyright (c) 2004 David A. Capello.
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

/*	$Id: term_allegro.c,v 1.3 2004/10/08 13:30:45 rrt Exp $	*/

#include "config.h"

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <allegro.h>
#include <allegro/internal/aintern.h>

#include "zile.h"
#include "extern.h"

/* Font width and height */
#define FW		(8)	/* font_length (font, ...) */
#define FH		(8)	/* font_height (font) */

/* Font codes */
#define ZA_NORMAL		0x0000
#define ZA_REVERSE		0x1000
#define ZA_BOLD			0x2000

static Terminal thisterm = {
	/* Unitialised screen pointer */
	NULL,

	/* Uninitialized width and height. */
	-1, -1,
};

Terminal *termp = &thisterm;

int ZILE_LINES, ZILE_COLS;

/* current position and color */
static int cur_x = 0, cur_y = 0;
static Font cur_color = 0;

static unsigned short *cur_scr = NULL;
static unsigned short *new_scr = NULL;

static void _get_color(int c, int *_fg, int *_bg)
{
	int fg, bg;

	fg = makecol(170, 170, 170);
	bg = makecol(0, 0, 0);

	if (c & ZILE_REVERSE) {
		int aux = fg;
		fg = bg;
		bg = aux;
	}

	if (_fg) *_fg = fg;
	if (_bg) *_bg = bg;
}

static int cursor_state = FALSE;
static volatile int blink_state = 0;

static volatile int cur_time = 0;

static void control_blink_state(void)
{
	blink_state ^= 1;
}

END_OF_STATIC_FUNCTION(control_blink_state);

static void inc_cur_time(void)
{
	cur_time++;
}

END_OF_STATIC_FUNCTION(inc_cur_time);

static void draw_cursor(int state)
{
	if (cursor_state &&
	    cur_x >= 0 && cur_x < ZILE_COLS &&
	    cur_y >= 0 && cur_y < ZILE_LINES) {
		if (state)
			rectfill(screen, cur_x*FW, cur_y*FH,
				 cur_x*FW+FW-1, cur_y*FH+FH-1,
				 makecol(170, 170, 170));
		else {
			int fg, bg, c = new_scr[cur_y*ZILE_COLS+cur_x];
			_get_color(c, &fg, &bg);
			text_mode(bg);
			font->vtable->render_char
				(font, ((c&0xff) < ' ') ? ' ': (c&0xff),
				 fg, bg, screen,
				 cur_x*FW, cur_y*FH);
		}
	}
}

void term_move(int y, int x)
{
	cur_x = x;
	cur_y = y;
}

void term_clrtoeol(void)
{
	if (cur_x >= 0 && cur_x < ZILE_COLS &&
	    cur_y >= 0 && cur_y < ZILE_LINES) {
		int x;
		for (x=cur_x; x<ZILE_COLS; x++)
			new_scr[cur_y*ZILE_COLS+x] = 0;
	}
}

void term_refresh(void)
{
	int x, y, c, i, bg, fg;
	i = 0;
	for (y=0; y<ZILE_LINES; y++)
		for (x=0; x<ZILE_COLS; x++) {
			if (new_scr[i] != cur_scr[i]) {
				c = cur_scr[i] = new_scr[i];
				_get_color(c, &fg, &bg);
				text_mode(bg);
				font->vtable->render_char
					(font,
					 ((c&0xff) < ' ') ? ' ': (c&0xff),
					 fg, bg, screen, x*FW, y*FH);
			}
			i++;
		}
}

void term_clear(void)
{
	memset(new_scr, 0, sizeof(unsigned short)*ZILE_COLS*ZILE_LINES);
}

void term_addch(int c)
{
        if (cur_x >= 0 && cur_x < ZILE_COLS &&
	    cur_y >= 0 && cur_y < ZILE_LINES) {
                int color = 0;

                if (c & 0x0f00)
                        color |= c & 0x0f00;
                else if (cur_color & 0x0f00)
                        color |= cur_color & 0x0f00;
                else
                        color |= C_FG_WHITE;
                
                if (c & 0xf000)
                        color |= c & 0xf000;
                else
                        color |= cur_color & 0xf000;

                new_scr[cur_y*ZILE_COLS+cur_x] = (c & 0x00ff) | color;
	}
	cur_x++;
}

void term_addnstr(const char *s, int len)
{
	int c;
	for (c=0; c<len && *s; c++)
		term_addch(*(s++));
}

void term_attrset(int attrs, ...)
{
	int i;
	unsigned long a = 0;
	va_list valist;
	va_start(valist, attrs);
	for (i = 0; i < attrs; i++)
		a |= va_arg(valist, Font);
	va_end(valist);
        cur_color = a;
}

int term_printw(const char *fmt, ...)
{
	char buf[4096];
        int res;
        va_list valist;
        va_start(valist, fmt);
        res = vsprintf(buf, fmt, valist);
	term_addnstr(buf, res);
        va_end(valist);
        return res;
}

void term_beep(void)
{
	/* nothing */
}

void term_init(void)
{
	allegro_init();
	install_timer();
	install_keyboard();
	set_color_depth(8);
	if (set_gfx_mode(GFX_SAFE, 640, 480, 0, 0) < 0) {
		/* XXX */
		exit(1);
	}

	LOCK_VARIABLE(blink_state);
	LOCK_FUNCTION(control_blink_state);
	install_int(control_blink_state, 150);

	LOCK_VARIABLE(cur_time);
	LOCK_FUNCTION(inc_cur_time);
	install_int_ex(inc_cur_time, BPS_TO_TIMER(1000));

	ZILE_COLS = SCREEN_W/FW;
	ZILE_LINES = SCREEN_H/FH;

        termp->screen = screen;
	termp->width = ZILE_COLS;
	termp->height = ZILE_LINES;

	cur_scr = calloc(1, sizeof(unsigned short)*ZILE_COLS*ZILE_LINES);
	new_scr = calloc(1, sizeof(unsigned short)*ZILE_COLS*ZILE_LINES);
}

int term_open(void)
{
	return TRUE;
}

void term_close(void)
{
	/* Clear last line. */
	term_move(ZILE_LINES - 1, 0);
	term_clrtoeol();
	term_refresh();

	/* Free memory and finish with allegro. */
	free_rotation_buffers();
	termp->screen = NULL;
	free(cur_scr);
	free(new_scr);

	set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);
	allegro_exit ();
}

static int translate_key(int c)
{
	int ascii = c & 0xff;
	int scancode = c >> 8;

	if (!ascii && key_shifts & KB_ALT_FLAG) {
		ascii = scancode_to_ascii (scancode);
		if (ascii)
			return KBD_META | ascii |
				((key_shifts & KB_CTRL_FLAG) ? KBD_CTL: 0);
		else
			return KBD_NOKEY;
	}

	switch (scancode) {
	case KEY_F1: return KBD_F1;
	case KEY_F2: return KBD_F2;
	case KEY_F3: return KBD_F3;
	case KEY_F4: return KBD_F4;
	case KEY_F5: return KBD_F5;
	case KEY_F6: return KBD_F6;
	case KEY_F7: return KBD_F7;
	case KEY_F8: return KBD_F8;
	case KEY_F9: return KBD_F9;
	case KEY_F10: return KBD_F10;
	case KEY_F11: return KBD_F11;
	case KEY_F12: return KBD_F12;
	case KEY_ESC: return KBD_META;
	case KEY_TAB: return KBD_TAB;
	case KEY_ENTER:
	case KEY_ENTER_PAD: return KBD_RET;
	case KEY_BACKSPACE: return KBD_BS;
	case KEY_INSERT: return KBD_INS;
	case KEY_DEL: return KBD_DEL;
	case KEY_HOME: return KBD_HOME;
	case KEY_END: return KBD_END;
	case KEY_PGUP: return KBD_PGUP;
	case KEY_PGDN: return KBD_PGDN;
	case KEY_LEFT: return KBD_LEFT;
	case KEY_RIGHT: return KBD_RIGHT;
	case KEY_UP: return KBD_UP;
	case KEY_DOWN: return KBD_DOWN;
	case KEY_SPACE: 
		if (key_shifts & KB_CTRL_FLAG)
			return '@' | KBD_CTL;
		else
			return ' ';
	default:
		if ((scancode >= KEY_0 && scancode <= KEY_9) &&
		    (ascii < 32 || ascii == 127)) {
			return ('0'+scancode-KEY_0) | KBD_CTL;
		}
		else if (ascii >= 1 && ascii <= 'z'-'a'+1) {
			if ('a'+ascii-1 == 'g')
				return KBD_CANCEL;
			else
				return ('a'+ascii-1) | KBD_CTL;
		}
		else if (ascii >= ' ') {
			return ascii;
		}
	}

	return KBD_NOKEY;
}

#define MAX_UNGETKEY_BUF	16

static int ungetkey_buf[MAX_UNGETKEY_BUF];
static int *ungetkey_p = ungetkey_buf;

static int hooked_readkey(int timeout)
{
	int beg_time = cur_time;
	term_refresh();

	cursor_state = TRUE;
	while (!keypressed() && (!timeout || cur_time-beg_time < timeout))
		draw_cursor(blink_state);

	draw_cursor(FALSE);
	cursor_state = FALSE;

	return readkey();
}

static int _term_getkey(int timeout)
{
	int key;

	if (ungetkey_p > ungetkey_buf)
		return *--ungetkey_p;

	key = translate_key(hooked_readkey(timeout));

	while (key == KBD_META) {
		key = translate_key(hooked_readkey(0));
		key |= KBD_META;
	}

	return key;
}

int term_getkey(void)
{
	return _term_getkey(0);
}

static int xgetkey(int mode, int arg)
{
	int c = 0;
	switch (mode) {
	case GETKEY_NONFILTERED:
		c = hooked_readkey(0) & 0xff;
		break;
	case GETKEY_DELAYED:
		c = _term_getkey(arg);
		break;
	case GETKEY_NONFILTERED|GETKEY_DELAYED:
		c = hooked_readkey(arg) & 0xff;
		break;
	}
	return c;
}

int term_xgetkey(int mode, int arg)
{
	int c;

	if (ungetkey_p > ungetkey_buf)
		return *--ungetkey_p;

	c = xgetkey(mode, arg);

	return c;
}

int term_ungetkey(int key)
{
	if (ungetkey_p - ungetkey_buf >= MAX_UNGETKEY_BUF)
		return FALSE;

	*ungetkey_p++ = key;

	return TRUE;
}
