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

/*	$Id: term_termcap.c,v 1.4 2004/07/11 00:29:37 rrt Exp $	*/

#include "config.h"

#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <term.h>

/* Avoid clash with zile function. */
#undef newline

#include "zile.h"
#include "extern.h"
#include "zterm.h"

static Terminal thisterm = {
	/* Unitialised screen pointer. */
	NULL,

	/* Uninitialized width and height. */
	-1, -1,
};

Terminal *termp = &thisterm;

/* XXX TODO */
Font ZILE_REVERSE = 0, ZILE_BOLD = 0;

Font C_FG_BLACK;
Font C_FG_RED;
Font C_FG_GREEN;
Font C_FG_YELLOW;
Font C_FG_BLUE;
Font C_FG_MAGENTA;
Font C_FG_CYAN;
Font C_FG_WHITE;
Font C_FG_WHITE_BG_BLUE;

static char *cl_string, *cm_string, *ce_string, *se_string, *so_string;
static int auto_wrap;
static struct termios ostate, nstate;

static char PC;   /* For tputs. */
static char *BC;  /* For tgoto. */
static char *UP;

void term_move(int y, int x)
{
        tputs(tgoto(cm_string, x, y), 1, putchar);
}

void term_clrtoeol(void)
{
        tputs(ce_string, 1, putchar);
}

void term_refresh(void)
{
        /* Termcap has no refresh function; all updates are immediate. */
}

void term_clear(void)
{
        tputs(cl_string, 1, putchar);
}

void term_addch(int c)
{
        putchar(c);
}

void term_addnstr(const char *s, int len)
{
        printf("%.*s", len, s);
}

void term_attrset(Font f)
{
        /* XXX TODO */
}

int term_printw(const char *fmt, ...)
{
        int res = 0;
        va_list valist;
        va_start(valist, fmt);
        res = vprintf(fmt, valist);
        va_end(valist);
        return res;
}

void term_beep(void)
{
        putchar('\a');
}

void term_init(void)
{
        /* termcap buffer, conventionally big enough. */
        char *tcap = (char *)malloc(2048);
        char *term = getenv("TERM");
        int res;
        char *temp;

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

	termp->width = tgetnum("co");
	termp->height = tgetnum("li");

        /* Extract information we will use. */
        cl_string = tgetstr("cl", &tcap);
        cm_string = tgetstr("cm", &tcap);
        auto_wrap = tgetflag("am");
        ce_string = tgetstr("ce", &tcap);
        se_string = tgetstr("se", &tcap);
        so_string = tgetstr("so", &tcap);

        /* Extract information that termcap functions use. */
        temp = tgetstr("pc", &tcap);
        PC = temp ? *temp : 0;
        BC = tgetstr("le", &tcap);
        UP = tgetstr("up", &tcap);

        /* Save terminal flags. */
        if ((tcgetattr(0, &ostate) < 0) || (tcgetattr(0, &nstate) < 0)) {
                fprintf(stderr, "Can't read terminal capabilites\n");
                zile_exit(1);
        }
        nstate.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                            |INLCR|IGNCR|ICRNL|IXON);
        nstate.c_oflag &= ~OPOST;
        nstate.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
        nstate.c_cflag &= ~(CSIZE|PARENB);
        nstate.c_cflag |= CS8;
        nstate.c_cc[VMIN] = 1;
        nstate.c_cc[VTIME] = 0;	/* Block indefinitely for a single char. */
        if (tcsetattr(0, TCSADRAIN, &nstate) < 0) {
                fprintf(stderr, "Can't set terminal mode\n");
                zile_exit(1);
        }
        /* Provide a smaller terminal output buffer so that the type-ahead
           detection works better (more often). */
/*         setbuffer (stdout, &tobuf[0], TBUFSIZ); */
        signal (SIGTSTP, SIG_DFL);
}

/* static void init_colors(void) */
/* { */
/* } */

int term_open(void)
{

	return TRUE;
}

int term_close(void)
{
	/* Clear last line. */
	term_move(ZILE_LINES - 1, 0);
	term_clrtoeol();
	term_refresh();

	/* Free memory and finish with termcap. */
	free_rotation_buffers();
	termp->screen = NULL;
        tcdrain (0);
        fflush (stdout);
        if (tcsetattr(0, TCSADRAIN, &ostate) < 0)
        {
                fprintf(stderr, "Can't restore terminal flags\n");
                zile_exit(1);
        }

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
		c = getchar();
		if (c != KEY_RESIZE)
			break;
		resize_windows();
	}
#else
	c = getchar();
#endif

/*	if (c == ERR) */
/*		return KBD_NOKEY; */

	key = translate_key(c);

	while (key == KBD_META) {
		c = getchar();
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
		c = getchar();
		break;
	case GETKEY_DELAYED:
/*                 wtimeout(stdscr, arg); */
		c = term_getkey();
/*                 wtimeout(stdscr, -1); */
		break;
	case GETKEY_NONFILTERED|GETKEY_DELAYED:
/*                 wtimeout(stdscr, arg); */
		c = getchar();
/*                 wtimeout(stdscr, -1); */
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
