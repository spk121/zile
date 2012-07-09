/* Curses terminal

   Copyright (c) 1997-2012 Free Software Foundation, Inc.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

#include <config.h>

#include <stdlib.h>
#if defined HAVE_NCURSESW_CURSES_H
#  include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
#  include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
#  include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
#  include <ncurses.h>
#elif defined HAVE_CURSES_H
#  include <curses.h>
#else
#  error "SysV or X/Open-compatible Curses header file required"
#endif
#include <term.h>
#include "gl_array_list.h"

#include "main.h"
#include "extern.h"

static gl_list_t key_buf;

static char backspace_code = 0177;

size_t
term_buf_len (void)
{
  return gl_list_size (key_buf);
}

void
term_move (size_t y, size_t x)
{
  move ((int) y, (int) x);
}

void
term_clrtoeol (void)
{
  clrtoeol ();
}

void
term_refresh (void)
{
  refresh ();
}

void
term_clear (void)
{
  clear ();
}

void
term_addch (char c)
{
  addch (c);
}

void
term_addstr (const char *s)
{
  addstr (s);
}

void
term_attrset (size_t attr)
{
  int attrs = 0;
  if (attr & FONT_REVERSE)
    attrs |= A_REVERSE;
  if (attr & FONT_UNDERLINE)
    attrs |= A_UNDERLINE;
  attrset (attrs);
}

void
term_beep (void)
{
  beep ();
}

size_t
term_width (void)
{
  return (size_t) COLS;
}

size_t
term_height (void)
{
  return (size_t) LINES;
}

void
term_init (void)
{
  initscr ();
  noecho ();
  nonl ();
  raw ();
  meta (stdscr, true);
  intrflush (stdscr, false);
  keypad (stdscr, true);
  key_buf = gl_list_create_empty (GL_ARRAY_LIST, NULL, NULL, NULL, true);
  char *kbs = tigetstr("kbs");
  if (kbs && strlen (kbs) == 1)
    backspace_code = *kbs;
}

void
term_close (void)
{
  /* Finish with ncurses. */
  endwin ();
}

static _GL_ATTRIBUTE_PURE size_t
codetokey (int c)
{
  switch (c)
    {
    case '\0':			/* C-@ */
      return KBD_CTRL | '@';
    case '\1':
    case '\2':
    case '\3':
    case '\4':
    case '\5':
    case '\6':
    case '\7':
    case '\10':
    case '\12':
    case '\13':
    case '\14':
    case '\16':
    case '\17':
    case '\20':
    case '\21':
    case '\22':
    case '\23':
    case '\24':
    case '\25':
    case '\26':
    case '\27':
    case '\30':
    case '\31':
    case '\32':			/* C-a ... C-z */
      return KBD_CTRL | ('a' + c - 1);
    case '\11':
      return KBD_TAB;
    case '\15':
      return KBD_RET;
    case '\37':
      return KBD_CTRL | '_';
#ifdef KEY_SUSPEND
    case KEY_SUSPEND:		/* C-z */
      return KBD_CTRL | 'z';
#endif
    case '\33':			/* META */
      return KBD_META;
    case KEY_PPAGE:		/* PGUP */
      return KBD_PGUP;
    case KEY_NPAGE:		/* PGDN */
      return KBD_PGDN;
    case KEY_HOME:
      return KBD_HOME;
    case KEY_END:
      return KBD_END;
    case KEY_DC:		/* DEL */
      return KBD_DEL;
    case KEY_BACKSPACE:		/* Backspace or Ctrl-H */
      return codetokey (backspace_code);
    case 0177:			/* BS */
      return KBD_BS;
    case KEY_IC:		/* INSERT */
      return KBD_INS;
    case KEY_LEFT:
      return KBD_LEFT;
    case KEY_RIGHT:
      return KBD_RIGHT;
    case KEY_UP:
      return KBD_UP;
    case KEY_DOWN:
      return KBD_DOWN;
    case KEY_F (1):
      return KBD_F1;
    case KEY_F (2):
      return KBD_F2;
    case KEY_F (3):
      return KBD_F3;
    case KEY_F (4):
      return KBD_F4;
    case KEY_F (5):
      return KBD_F5;
    case KEY_F (6):
      return KBD_F6;
    case KEY_F (7):
      return KBD_F7;
    case KEY_F (8):
      return KBD_F8;
    case KEY_F (9):
      return KBD_F9;
    case KEY_F (10):
      return KBD_F10;
    case KEY_F (11):
      return KBD_F11;
    case KEY_F (12):
      return KBD_F12;
    default:
      if (c > 0xff || c < 0)
        return KBD_NOKEY;	/* ERR (no key) or undefined behaviour. */
      return c;
    }
}

static size_t
keytocodes (size_t key, int ** codevec)
{
  if (key == KBD_NOKEY)
    return 0;

  int *p = *codevec = XCALLOC (2, int);

  if (key & KBD_META)				/* META */
    *p++ = '\33';
  key &= ~KBD_META;

  switch (key)
    {
    case KBD_CTRL | '@':			/* C-@ */
      *p++ = '\0';
      break;
    case KBD_CTRL | 'a':
    case KBD_CTRL | 'b':
    case KBD_CTRL | 'c':
    case KBD_CTRL | 'd':
    case KBD_CTRL | 'e':
    case KBD_CTRL | 'f':
    case KBD_CTRL | 'g':
    case KBD_CTRL | 'h':
    case KBD_CTRL | 'j':
    case KBD_CTRL | 'k':
    case KBD_CTRL | 'l':
    case KBD_CTRL | 'n':
    case KBD_CTRL | 'o':
    case KBD_CTRL | 'p':
    case KBD_CTRL | 'q':
    case KBD_CTRL | 'r':
    case KBD_CTRL | 's':
    case KBD_CTRL | 't':
    case KBD_CTRL | 'u':
    case KBD_CTRL | 'v':
    case KBD_CTRL | 'w':
    case KBD_CTRL | 'x':
    case KBD_CTRL | 'y':
    case KBD_CTRL | 'z':	/* C-a ... C-z */
      *p++ = (key & ~KBD_CTRL) + 1 - 'a';
      break;
    case KBD_TAB:
      *p++ = '\11';
      break;
    case KBD_RET:
      *p++ = '\15';
      break;
    case KBD_CTRL | '_':
      *p++ = '\37';
      break;
    case KBD_PGUP:		/* PGUP */
      *p++ = KEY_PPAGE;
      break;
    case KBD_PGDN:		/* PGDN */
      *p++ = KEY_NPAGE;
      break;
    case KBD_HOME:
      *p++ = KEY_HOME;
      break;
    case KBD_END:
      *p++ = KEY_END;
      break;
    case KBD_DEL:		/* DEL */
      *p++ = KEY_DC;
      break;
    case KBD_BS:		/* BS */
      *p++ = 0177;
      break;
    case KBD_INS:		/* INSERT */
      *p++ = KEY_IC;
      break;
    case KBD_LEFT:
      *p++ = KEY_LEFT;
      break;
    case KBD_RIGHT:
      *p++ = KEY_RIGHT;
      break;
    case KBD_UP:
      *p++ = KEY_UP;
      break;
    case KBD_DOWN:
      *p++ = KEY_DOWN;
      break;
    case KBD_F1:
      *p++ = KEY_F (1);
      break;
    case KBD_F2:
      *p++ = KEY_F (2);
      break;
    case KBD_F3:
      *p++ = KEY_F (3);
      break;
    case KBD_F4:
      *p++ = KEY_F (4);
      break;
    case KBD_F5:
      *p++ = KEY_F (5);
      break;
    case KBD_F6:
      *p++ = KEY_F (6);
      break;
    case KBD_F7:
      *p++ = KEY_F (7);
      break;
    case KBD_F8:
      *p++ = KEY_F (8);
      break;
    case KBD_F9:
      *p++ = KEY_F (9);
      break;
    case KBD_F10:
      *p++ = KEY_F (10);
      break;
    case KBD_F11:
      *p++ = KEY_F (11);
      break;
    case KBD_F12:
      *p++ = KEY_F (12);
      break;
    default:
      if ((key & 0xff) == key)
        *p++ = key;
      break;
    }

  return p - *codevec;
}

static int
get_char (int delay)
{
  int c;

  size_t size = term_buf_len ();
  if (size > 0)
    {
      c = (ptrdiff_t) gl_list_get_at (key_buf, size - 1);
      gl_list_remove_at (key_buf, size - 1);
    }
  else
    {
      timeout (delay);

#ifdef KEY_RESIZE
      do {
#endif
        c = getch ();

#ifdef KEY_RESIZE
        if (c == KEY_RESIZE)
          resize_windows ();
      } while (c == KEY_RESIZE);
#endif
    }

  return c;
}

size_t
term_getkey (int delay)
{
  size_t key = codetokey (get_char (delay));
  while (key == KBD_META)
    key = codetokey (get_char (GETKEY_DEFAULT)) | KBD_META;
  return key;
}

int
term_getkey_unfiltered (int delay)
{
  keypad (stdscr, false);
  int key = get_char (delay);
  keypad (stdscr, true);
  return key;
}

void
term_ungetkey (size_t key)
{
  int * codes = NULL;
  for (size_t i = keytocodes (key, &codes); i > 0; i--)
    gl_list_add_last (key_buf, (void *)(ptrdiff_t) codes[i - 1]);
}
