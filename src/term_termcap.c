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

/*	$Id: term_termcap.c,v 1.16 2004/10/11 12:57:43 rrt Exp $	*/

/* TODO: signal handler resize_windows(); */

#include "config.h"

#include <assert.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <termcap.h>
#include <termios.h>
#include <unistd.h>

#include "zile.h"
#include "extern.h"

static Terminal thisterm = {
	/* Unitialised screen pointer. */
	NULL,

	/* Uninitialized width and height. */
	-1, -1,
};

typedef struct {
        int curx, cury;  /* cursor x and y. */
        Font font;       /* current font. */
        int *array, *oarray; /* contents of screen (8 low bits is
                                character, rest is Zile font code).
                                array is current, oarray is last
                                displayed contents. */
} Screen;

static Screen screen;

Terminal *termp = &thisterm;

int ZILE_COLS;
int ZILE_LINES;

static char *cm_string;
static char *so_string, *se_string, *mr_string, *me_string;
static char *kl_string, *kr_string, *ku_string, *kd_string;
astr norm_string;
static struct termios ostate, nstate;

void term_move(int y, int x)
{
        screen.curx = x;
        screen.cury = y;
}

void term_clrtoeol(void)
{
        int i, x = screen.curx;
        for (i = screen.curx; i < termp->width; i++)
                term_addch(0);
        screen.curx = x;
}

static const char *getattr(Font f) {
        if (f == ZILE_NORMAL)
                return astr_cstr(norm_string);
        else if (f & ZILE_BOLD)
                return so_string;
        else if (f & ZILE_REVERSE)
                return mr_string;
        assert(0);
        return "";
}

/*
 * Refresh the screen.  This is basically a poor person's ncurses.
 * To minimise the number of system calls, the output is collected in a string.
 * To minimise the time taken to draw the screen, only those
 * characters that have changed are output, and unnecessary cursor
 * positioning and font changes are elided.
 * Because the output is collected in a string, tputs cannot be used,
 * so no padding is done, so this code will not work on slow
 * terminals.  Hopefully there aren't many of those left.
 */
void term_refresh(void)
{
        int i, j, skipped = 0;
        Font of = ZILE_NORMAL;
        astr as = astr_new();

        /* Start at the top left of the screen with no highlighting. */
        astr_cat_cstr(as, tgoto(cm_string, 0, 0));
        astr_cat_cstr(as, getattr(ZILE_NORMAL));

        /* Add the rest of the screen. */
        for (i = 0; i < termp->height; i++)
                for (j = 0; j < termp->width; j++) {
                        int offset = i * termp->width + j;
                        int n = screen.array[offset];

                        if (screen.oarray[offset] != n) {
                                char c = n & 0xff;
                                Font f = n & ~0xff;

                                if (skipped)
                                        astr_cat_cstr(as, tgoto(cm_string, j, i));
                                skipped = 0;
        
                                screen.oarray[offset] = n;

                                if (f != of)
                                        astr_cat_cstr(as, getattr(f));
                                of = f;
                                
                                astr_cat_char(as, c ? c : ' ');
                        } else
                                skipped = 1;
                }

        /* Put the cursor back where it should be. */
        astr_cat_cstr(as, tgoto(cm_string, screen.curx, screen.cury));

        /* Display the output. */
        printf("%s", astr_cstr(as));
        astr_delete(as);
}

void term_clear(void)
{
        int i;
        term_move(0, 0);
        for (i = 0; i < termp->width * termp->height; i++)
                term_addch(0);
}

void term_addch(int c)
{
        screen.array[screen.cury * termp->width + screen.curx] = c | screen.font;
        screen.curx++;
        if (screen.curx == termp->width) {
                if (screen.cury < termp->height - 1) {
                        screen.curx = 0;
                        screen.cury++;
                } else
                        screen.curx--;
        }
}

void term_addnstr(const char *s, int len)
{
        int i;
        for (i = 0; i < len; i++)
                term_addch(*s++);
}

void term_attrset(int attrs, ...)
{
	int i;
	va_list valist;
	va_start(valist, attrs);
	for (i = 0; i < attrs; i++) {
 		Font f = va_arg(valist, Font);
                switch (f) {
                case ZILE_NORMAL:
                        screen.font = ZILE_NORMAL;
                        break;
                case ZILE_REVERSE:
                case ZILE_BOLD:
                        screen.font |= f;
                        break;
                }
        }
	va_end(valist);
}

int term_printw(const char *fmt, ...)
{
	char *buf;
        int res = 0;
        va_list ap;
        va_start(ap, fmt);
	res = vasprintf(&buf, fmt, ap);
        va_end(ap);
        term_addnstr(buf, strlen(buf));
        free(buf);
        return res;
}

void term_beep(void)
{
        putchar('\a');
}

static void term_init_screen(void)
{
        int i, size = termp->width * termp->height;
        
        screen.array = zmalloc(size * sizeof(int));
        screen.oarray = zmalloc(size * sizeof(int));
        screen.curx = screen.cury = 0;

        /* Make the first call to term_refresh will update the screen */
        for (i = 0; i < size; i++)
                screen.oarray[i] = 1;
}

void term_init(void)
{
        /* termcap buffer, conventionally big enough. */
        char *tcap = (char *)zmalloc(2048);
        char *term = getenv("TERM");
        int res;

        if (!term) {
                fprintf(stderr, "No terminal type in TERM.\n");
                zile_exit(1);
        }

        res = tgetent(tcap, term);
        if (res < 0) {
                fprintf(stderr, "Could not access the termcap data base.\n");
                zile_exit(1);
        } else if (res == 0) {
                fprintf(stderr, "Terminal type `%s' is not defined.\n", term);
                zile_exit(1);
        }

	ZILE_COLS = termp->width = tgetnum("co");
	ZILE_LINES = termp->height = tgetnum("li");
        
        term_init_screen();
        termp->screen = &screen;

        /* Extract information we will use. */
        cm_string = tgetstr("cm", &tcap);
        so_string = tgetstr("so", &tcap);
        se_string = tgetstr("se", &tcap);
	mr_string = tgetstr("mr", &tcap);
	me_string = tgetstr("me", &tcap);
        kl_string = tgetstr("kl", &tcap);
        kr_string = tgetstr("ku", &tcap);
        ku_string = tgetstr("kr", &tcap);
        kd_string = tgetstr("kd", &tcap);
        /* TODO: Use km, mm, mo for Meta key. */

        norm_string = astr_new();
        astr_cat_cstr(norm_string, se_string);
        astr_cat_cstr(norm_string, me_string);

        /* Save terminal flags. */
        if ((tcgetattr(0, &ostate) < 0) || (tcgetattr(0, &nstate) < 0)) {
                fprintf(stderr, "Can't read terminal capabilites\n");
                zile_exit(1);
        }

        /* Set up terminal. */
        nstate.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                            |INLCR|IGNCR|ICRNL|IXON);
        nstate.c_oflag &= ~OPOST;
        nstate.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
        nstate.c_cflag &= ~(CSIZE|PARENB);
        nstate.c_cc[VMIN] = 1;
        nstate.c_cflag |= CS8;
        if (tcsetattr(0, TCSADRAIN, &nstate) < 0) {
                fprintf(stderr, "Can't set terminal mode\n");
                zile_exit(1);
        }

       setvbuf(stdout, NULL, _IONBF, 0);
}

void term_close(void)
{
	/* Clear last line. */
	term_move(ZILE_LINES - 1, 0);
	term_clrtoeol();
	term_refresh();
        printf(getattr(ZILE_NORMAL));

	/* Free memory and finish with termcap. */
	free_rotation_buffers();
        free(screen.array);
        free(screen.oarray);
	termp->screen = NULL;
        tcdrain(0);
        fflush(stdout);
        if (tcsetattr(0, TCSADRAIN, &ostate) < 0) {
                fprintf(stderr, "Can't restore terminal flags\n");
                zile_exit(1);
        }
}

static int translate_key(char *s, int bytes)
{
	switch (*s) {
	case '\0':		/* C-@ */
		return KBD_CTL | '@';
	case '\1':  case '\2':  case '\3':  case '\4':  case '\5':
	case '\6':  case '\7':  case '\10':             case '\12':
	case '\13': case '\14':             case '\16': case '\17':
	case '\20': case '\21': case '\22': case '\23': case '\24':
	case '\25': case '\26': case '\27': case '\30': case '\31':
	case '\32':		/* C-a ... C-z */
		return KBD_CTL | ('a' + *s - 1);
	case '\11':
		return KBD_TAB;
	case '\15':
		return KBD_RET;
	case '\37':
		return KBD_CTL | (*s ^ 0x40);
	case '\33':		/* META */
		return KBD_META;
/*	case KEY_PPAGE:		/\* PGUP *\/ */
/*		return KBD_PGUP; */
/*	case KEY_NPAGE:		/\* PGDN *\/ */
/*		return KBD_PGDN; */
/*	case KEY_HOME:		/\* HOME *\/ */
/*		return KBD_HOME; */
/*	case KEY_END:		/\* END *\/ */
/*		return KBD_END; */
/*	case KEY_DC:		/\* DEL *\/ */
/*		return KBD_DEL; */
/*	case KEY_BACKSPACE:	/\* BS *\/ */
	case 0177:		/* BS */
		return KBD_BS;
/*	case KEY_IC:		/\* INSERT *\/ */
/*		return KBD_INS; */
/*	case KEY_LEFT:		/\* LEFT *\/ */
/*		return KBD_LEFT; */
/*	case KEY_RIGHT:		/\* RIGHT *\/ */
/*		return KBD_RIGHT; */
/*	case KEY_UP:		/\* UP *\/ */
/*		return KBD_UP; */
/*	case KEY_DOWN:		/\* DOWN *\/ */
/*		return KBD_DOWN; */
/*	case KEY_F(1): */
/*		return KBD_F1; */
/*	case KEY_F(2): */
/*		return KBD_F2; */
/*	case KEY_F(3): */
/*		return KBD_F3; */
/*	case KEY_F(4): */
/*		return KBD_F4; */
/*	case KEY_F(5): */
/*		return KBD_F5; */
/*	case KEY_F(6): */
/*		return KBD_F6; */
/*	case KEY_F(7): */
/*		return KBD_F7; */
/*	case KEY_F(8): */
/*		return KBD_F8; */
/*	case KEY_F(9): */
/*		return KBD_F9; */
/*	case KEY_F(10): */
/*		return KBD_F10; */
/*	case KEY_F(11): */
/*		return KBD_F11; */
/*	case KEY_F(12): */
/*		return KBD_F12; */
	default:
		return *s;
	}
}

#define MAX_KEY_CHARS 16 /* Hopefully more than the longest keycode sequence. */

static int xgetkey(int mode, int arg)
{
        int ret = 0;
        size_t nbytes;
        fd_set rfds;
        char keys[MAX_KEY_CHARS];

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);

        if (mode & GETKEY_DELAYED)
                nstate.c_cc[VTIME] = 0;	/* Wait indefinitely for at least one character. */
        else {
                if (arg == 0) /* If we want to wait no time at all, wait the minimum. */
                        arg = 1;
                nstate.c_cc[VTIME] = arg / 100; /* Wait up to arg/100 10ths of a second. */
        }                

        nbytes = read(STDIN_FILENO, keys, MAX_KEY_CHARS);
        
	if (ret < 0)
		return KBD_NOKEY;

        if (mode & GETKEY_NONFILTERED)
                return keys[nbytes - 1];
        else {
                int key = translate_key(keys, nbytes);
                while (key == KBD_META) {
                        char c = getchar();
                        key = translate_key(&c, 1);
                        key |= KBD_META;
                }
                return key;
        }
}

#define MAX_UNGETKEY_BUF	16

static int ungetkey_buf[MAX_UNGETKEY_BUF];
static int *ungetkey_p = ungetkey_buf;

int term_xgetkey(int mode, int arg)
{
	if (ungetkey_p > ungetkey_buf)
		return *--ungetkey_p;

	return xgetkey(mode, arg);
}

int term_getkey(void)
{
        return term_xgetkey(0, 0);
}

int term_ungetkey(int key)
{
	if (ungetkey_p - ungetkey_buf >= MAX_UNGETKEY_BUF)
		return FALSE;

	*ungetkey_p++ = key;

	return TRUE;
}
