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

/*	$Id: term_epocemx.c,v 1.6 2005/01/18 12:06:15 rrt Exp $	*/

#include "config.h"

#include <assert.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <termcap.h>
#include <emx.h>

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

static char *tcap_ptr;

static Screen screen;

Terminal *termp = &thisterm;

int ZILE_COLS;   /* Current number of columns on screen. */
int ZILE_LINES;  /* Current number of rows on screen. */

static char *cm_string, *ce_string;
static char *so_string, *se_string, *mr_string, *me_string;

astr norm_string;

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
  int i, j, skipped, eol;
  Font of = ZILE_NORMAL;
  astr as = astr_new();

  /* Start at the top left of the screen with no highlighting. */
  astr_cat_cstr(as, tgoto(cm_string, 0, 0));
  astr_cat_cstr(as, getattr(ZILE_NORMAL));

  /* Add the rest of the screen. */
  for (i = 0; i < termp->height; i++) {
    eol = FALSE;
    /*
     * The eol flag may seem unnecessary; it is used (rather than
     * breaking out of the loop when EOL is reached) to allow the
     * rest of the line to be updated in the array (it should be
     * all zeros).
     */
    astr_cat_cstr(as, tgoto(cm_string, 0, i));
    skipped = FALSE;

    for (j = 0; j < termp->width; j++) {
      int offset = i * termp->width + j;
      int n = screen.array[offset];
      char c = n & 0xff;
      Font f = n & ~0xff;

      if (screen.oarray[offset] != n) {
        if (skipped)
          astr_cat_cstr(as, tgoto(cm_string, j, i));
        skipped = FALSE;
        
        screen.oarray[offset] = n;

        if (f != of)
          astr_cat_cstr(as, getattr(f));
        of = f;

        if (c)
          astr_cat_char(as, c);
        else {
          if (!eol) {
            astr_cat_cstr(as, ce_string);
            eol = TRUE;
            skipped = TRUE;
          }
        }
      } else
        skipped = TRUE;
    }
  }

  /* Put the cursor back where it should be. */
  astr_cat_cstr(as, tgoto(cm_string, screen.curx, screen.cury));

  /* Display the output. */
  write(STDOUT_FILENO, astr_cstr(as), astr_len(as));
  astr_delete(as);
}

void term_clear(void)
{
  int i;
  term_move(0, 0);
  for (i = 0; i < termp->width * termp->height; i++) {
    screen.array[i] = 0;
    screen.oarray[i] = -1;
  }
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

void term_beep(void)
{
  putchar('\a');
}

static char *get_tcap(void)
{
  /* termcap buffer, conventionally big enough. */
  char *tcap = (char *)zmalloc(2048);
  char *term = "vt100";   /* This is ignored by epocemx. */
  int res;

  res = tgetent(tcap, term);
  if (res < 0) {
    fprintf(stderr, "Could not access the termcap data base.\n");
    zile_exit(1);
  } else if (res == 0) {
    fprintf(stderr, "Terminal type `%s' is not defined.\n", term);
    zile_exit(1);
  }

  return tcap;
}

static void read_screen_size(void)
{
  char *tcap = get_tcap();
  ZILE_COLS = tgetnum("co");
  ZILE_LINES = tgetnum("li");
  free(tcap);
}

static void term_init_screen(void)
{
  int size = termp->width * termp->height;
        
  screen.array = zmalloc(size * sizeof(int));
  screen.oarray = zmalloc(size * sizeof(int));
  screen.curx = screen.cury = 0;

  term_clear(); /* Ensure the first call to term_refresh will update the screen. */
}

static char *tgetstr_safe(const char *cap, char **tcap)
{
  char *s = tgetstr((char *)cap, tcap);
  return s ? s : "";
}

void term_init(void)
{
  char *tcap;

  tcap_ptr = tcap = get_tcap();

  read_screen_size();
  termp->width = ZILE_COLS;
  termp->height = ZILE_LINES;

  term_init_screen();
  termp->screen = &screen;

  /* Unbuffered I/O so screen is always up-to-date. */
  setvbuf(stdout, NULL, _IONBF, 0);

  /* Extract information we will use. */
  cm_string = tgetstr_safe("cm", &tcap);
  ce_string = tgetstr_safe("ce", &tcap);
  so_string = tgetstr_safe("so", &tcap);
  se_string = tgetstr_safe("se", &tcap);
  mr_string = so_string;  /* Not defined in epocemx termcap */
  me_string = se_string;  /* Not defined in epocemx termcap */

  norm_string = astr_new();
  astr_cat_cstr(norm_string, se_string);
  astr_cat_cstr(norm_string, me_string);
}

void term_close(void)
{
  /* Free memory and finish with termcap. */
  free(screen.array);
  free(screen.oarray);
  termp->screen = NULL;
  free(tcap_ptr);
  astr_delete(norm_string);
  fflush(stdout);
}

/* cursor keys */
#define KUP         4105
#define KDOWN       4106
#define KLEFT       4103
#define KRIGHT      4104
#define KHOME       4098
#define KEND        4099
#define KPGUP       4100
#define KPGDN       4101
#define KMENU       4150

static int translate_key(int code)
{
  switch (code) {
  case '\0':		/* C-@ */
    return KBD_CTL | '@';
  case '\1':  case '\2':  case '\3':  case '\4':  case '\5':
  case '\6':  case '\7':                         case '\12':
  case '\13': case '\14':             case '\16': case '\17':
  case '\20': case '\21': case '\22': case '\23': case '\24':
  case '\25': case '\26': case '\27': case '\30': case '\31':
  case '\32':		/* C-a ... C-z */
    return KBD_CTL | ('a' + code - 1);
  case '\11':
    return KBD_TAB;
  case '\15':
    return KBD_RET;
  case '\33':		/* META */
    return KBD_META;
  case '\37':
    return KBD_CTL | '_';
  case '\10':		/* BS */
    return KBD_BS;
  case KUP:
    return KBD_UP;
  case KDOWN:
    return KBD_DOWN;
  case KLEFT:
    return KBD_LEFT;
  case KRIGHT:
    return KBD_RIGHT;
  case KHOME:
    return KBD_HOME;
  case KEND:
    return KBD_END;
  case KPGUP:
    return KBD_PGUP;
  case KPGDN:
    return KBD_PGDN;
  default:
    if (code < 256)
      return code;
    return KBD_NOKEY;
  }
}

#define MAX_KEY_BUF	16

static int key_buf[MAX_KEY_BUF];
static int *keyp = key_buf;

static int xgetkey(int mode)
{
  int code;

  if (keyp > key_buf)
    return *--keyp;

  code= _read_key(0);
        
  if (mode & GETKEY_NONFILTERED) {
    return code & 0xff;
  } else {
    int key = translate_key(code);
    while (key == KBD_META)
      key = term_getkey() | KBD_META;
    return key;
  }
}

int term_xgetkey(int mode, int arg)
{
  return xgetkey(mode);
}

int term_getkey(void)
{
  return term_xgetkey(0, 0);
}

void term_unget(int key)
{
  if (keyp < key_buf + MAX_KEY_BUF && key != KBD_NOKEY)
    *keyp++ = key;
}
