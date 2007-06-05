/* Exported terminal
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2005 Reuben Thomas.
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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

/*	$Id: term_epocemx.c,v 1.19 2007/06/05 14:03:18 rrt Exp $	*/

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

typedef struct {
  size_t curx, cury;  /* cursor x and y. */
  Font font;       /* current font. */
  size_t *array, *oarray; /* contents of screen (8 low bits is
                             character, rest is font code).
                             array is current, oarray is last
                             displayed contents. */
} Screen;

static char *tcap_ptr;

static Screen screen;

static char *cm_string, *ce_string;
static char *so_string, *se_string, *mr_string, *me_string;

astr norm_string;

void term_move(size_t y, size_t x)
{
  screen.curx = x;
  screen.cury = y;
}

void term_clrtoeol(void)
{
  size_t i, x = screen.curx;
  for (i = screen.curx; i < term_width(); i++)
    term_addch(0);
  screen.curx = x;
}

static const char *getattr(Font f) {
  if (f == FONT_NORMAL)
    return astr_cstr(norm_string);
  else if (f & FONT_REVERSE)
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
  int skipped, eol;
  size_t i, j;
  Font of = FONT_NORMAL;
  astr as = astr_new();

  /* Start at the top left of the screen with no highlighting. */
  astr_cat_cstr(as, tgoto(cm_string, 0, 0));
  astr_cat_cstr(as, getattr(FONT_NORMAL));

  /* Add the rest of the screen. */
  for (i = 0; i < term_height(); i++) {
    eol = FALSE;
    /*
     * The eol flag may seem unnecessary; it is used (rather than
     * breaking out of the loop when EOL is reached) to allow the
     * rest of the line to be updated in the array (it should be
     * all zeros).
     */
    astr_cat_cstr(as, tgoto(cm_string, 0, (int)i));
    skipped = FALSE;

    for (j = 0; j < term_width(); j++) {
      size_t offset = i * term_width() + j;
      size_t n = screen.array[offset];
      char c = n & 0xff;
      Font f = n & ~0xffU;

      if (screen.oarray[offset] != n) {
        if (skipped)
          astr_cat_cstr(as, tgoto(cm_string, (int)j, (int)i));
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
  astr_cat_cstr(as, tgoto(cm_string, (int)screen.curx, (int)screen.cury));

  /* Display the output. */
  write(STDOUT_FILENO, astr_cstr(as), astr_len(as));
  astr_delete(as);
}

void term_clear(void)
{
  size_t i;
  term_move(0, 0);
  for (i = 0; i < term_width() * term_height(); i++) {
    screen.array[i] = 0;
    screen.oarray[i] = 1;
  }
}

void term_addch(int c)
{
  screen.array[screen.cury * term_width() + screen.curx] = c | screen.font;
  screen.curx++;
  if (screen.curx == term_width()) {
    if (screen.cury < term_height() - 1) {
      screen.curx = 0;
      screen.cury++;
    } else
      screen.curx--;
  }
}

void term_attrset(size_t attrs, ...)
{
  size_t i;
  va_list valist;
  va_start(valist, attrs);
  for (i = 0; i < attrs; i++) {
    Font f = va_arg(valist, Font);
    switch (f) {
    case FONT_NORMAL:
      screen.font = FONT_NORMAL;
      break;
    case FONT_REVERSE:
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
  char *tcap = zmalloc(2048);
  char *term = "vt100";   /* This is ignored by epocemx. */
  int res;

  res = tgetent(tcap, term);
  if (res < 0) {
    fprintf(stderr, "Could not access the termcap data base.\n");
    return NULL;
  } else if (res == 0) {
    fprintf(stderr, "Terminal type `%s' is not defined.\n", term);
    return NULL;
  }

  return tcap;
}

static void term_init_screen(void)
{
  int size = term_width() * term_height();

  screen.array = zmalloc(size * sizeof(size_t));
  screen.oarray = zmalloc(size * sizeof(size_t));
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
  if (tcap_ptr == NULL)
    exit(1);  

  term_set_size((size_t)tgetnum("co"), (size_t)tgetnum("li"));

  term_init_screen();

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
    return KBD_CTRL | '@';
  case '\1':  case '\2':  case '\3':  case '\4':  case '\5':
  case '\6':  case '\7':                          case '\12':
  case '\13': case '\14':             case '\16': case '\17':
  case '\20': case '\21': case '\22': case '\23': case '\24':
  case '\25': case '\26': case '\27': case '\30': case '\31':
  case '\32':		/* C-a ... C-z */
    return KBD_CTRL | ('a' + code - 1);
  case '\11':
    return KBD_TAB;
  case '\15':
    return KBD_RET;
  case '\33':		/* META */
    return KBD_META;
  case '\37':
    return KBD_CTRL | '_';
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

size_t term_xgetkey(int mode, size_t timeout)
{
  int code = _read_key(0);

  (void)timeout;
  if (mode & GETKEY_UNFILTERED)
    return code & 0xff;
  else {
    int key = translate_key(code);
    while (key == KBD_META)
      key = getkey() | KBD_META;
    return key;
  }
}
