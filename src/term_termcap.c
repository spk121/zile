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

/*	$Id: term_termcap.c,v 1.45 2005/01/16 13:04:58 rrt Exp $	*/

#include "config.h"

#include <assert.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
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

static char *tcap_ptr;

static Screen screen;

Terminal *termp = &thisterm;

static int max_key_chars = 0; /* Length of longest key code. */

int ZILE_COLS;   /* Current number of columns on screen. */
int ZILE_LINES;  /* Current number of rows on screen. */

static char *ks_string, *ke_string, *cm_string, *ce_string;
static char *so_string, *se_string, *mr_string, *me_string;

static int key_code[] = {
  KBD_LEFT, KBD_RIGHT, KBD_UP, KBD_DOWN,
  KBD_HOME, KBD_END, KBD_PGUP, KBD_PGDN,
  KBD_BS, KBD_DEL, KBD_INS,
  /* KBD_F1, KBD_F2, KBD_F3, KBD_F4,
     KBD_F5, KBD_F6, KBD_F7, KBD_F8,
     KBD_F9, KBD_F10, KBD_F11, KBD_F12, */
};

#define KEYS (sizeof(key_code) / sizeof(key_code[0]))

static char *key_cap_name[KEYS] = {
  "kl", "kr", "ku", "kd",
  "kh", "kH", "kP", "kN",
  "kb", "kD", "kI",
};

static char *key_cap[KEYS];
static int key_len[KEYS];

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
  screen.array[screen.cury * termp->width + screen.curx] = (c & 0xff) | screen.font;
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

static char *get_tcap(void)
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
  char *s = tgetstr(cap, tcap);
  return s ? s : "";
}

static char *tgetstr_note_len(const char *cap, char **tcap)
{
  char *s = tgetstr_safe(cap, tcap);
  max_key_chars = max(max_key_chars, strlen(s));
  return s;
}

static void setattr(int flags)
{
  if (tcsetattr(0, flags, &nstate) < 0) {
    fprintf(stderr, "Can't set terminal mode\n");
    zile_exit(1);
  }
}

void term_init(void)
{
  int i;
  char *tcap;

  tcap_ptr = tcap = get_tcap();

  read_screen_size();
  termp->width = ZILE_COLS;
  termp->height = ZILE_LINES;

  term_init_screen();
  termp->screen = &screen;

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
  nstate.c_cflag |= CS8;
  setattr(TCSADRAIN);

  /* Unbuffered I/O so screen is always up-to-date. */
  setvbuf(stdout, NULL, _IONBF, 0);

  /* Extract information we will use. */
  ks_string = tgetstr_safe("ks", &tcap);
  ke_string = tgetstr_safe("ke", &tcap);
  cm_string = tgetstr_safe("cm", &tcap);
  ce_string = tgetstr_safe("ce", &tcap);
  so_string = tgetstr_safe("so", &tcap);
  se_string = tgetstr_safe("se", &tcap);
  mr_string = tgetstr_safe("mr", &tcap);
  me_string = tgetstr_safe("me", &tcap);

  for (i = 0; i < KEYS; i++) {
    key_cap[i] = tgetstr_note_len(key_cap_name[i], &tcap);
    key_len[i] = strlen(key_cap[i]);
  }
  /* Assume the Meta key is present and on, and generates ESC */

  norm_string = astr_new();
  astr_cat_cstr(norm_string, se_string);
  astr_cat_cstr(norm_string, me_string);

  printf("%s", ks_string); /* Activate keypad (including cursor keys). */
}

void term_close(void)
{
  printf("%s", ke_string); /* Put keypad back in normal mode. */

  /* Free memory and finish with termcap. */
  free(screen.array);
  free(screen.oarray);
  termp->screen = NULL;
  free(tcap_ptr);
  astr_delete(norm_string);
  tcdrain(0);
  fflush(stdout);
  if (tcsetattr(0, TCSADRAIN, &ostate) < 0) {
    fprintf(stderr, "Can't restore terminal flags\n");
    zile_exit(1);
  }
}

static int translate_key(char *s, int nbytes)
{
  int key = KBD_NOKEY, i, used = 0;

  if (nbytes == 1) {
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
    case '\33':		/* META */
      return KBD_META;
    case '\37':
      return KBD_CTL | '_';
    case 0177:		/* BS */
      return KBD_BS;
    default:
      return *s;
    }
  } else {
    for (i = 0; i < KEYS; i++) {
      if (key_len[i] > 0 && key_len[i] <= nbytes &&
          strncmp(s, key_cap[i], key_len[i]) == 0) {
        key = key_code[i];
        used = key_len[i];
        break;
      }
    }
    if (used == 0 && nbytes > 1) {
#ifdef DEBUG
      int i;
      fprintf(stderr, "key code ");
      for (i = 0; i < nbytes; i++)
        fprintf(stderr, "%d ", s[i]);
      fprintf(stderr, "\n");
#endif
      key = translate_key(s, 1);
      used = 1;
    }
  }

  for (i = nbytes - 1; i >= used; i--)
    term_unget(s[i]);

  return key;
}

#define MAX_KEY_BUF	64

static char key_buf[MAX_KEY_BUF];
static char *keyp = key_buf + MAX_KEY_BUF;

static int xgetkey(int mode, int dsecs)
{
  size_t nbytes;
  char *keys = zmalloc(MAX_KEY_BUF + max_key_chars);
  int len = MAX_KEY_BUF - (keyp - key_buf);
  int key = KBD_NOKEY;

  if (mode & GETKEY_DELAYED) {
    nstate.c_cc[VTIME] = dsecs;  /* Wait up to arg deciseconds... */
    nstate.c_cc[VMIN] = 0;       /* ...but don't require any characters. */
  } else {
    nstate.c_cc[VTIME] = 0;	 /* Wait indefinitely... */
    nstate.c_cc[VMIN] = 1;   /* ...for at least one character. */
  }

  if (keyp < key_buf + MAX_KEY_BUF) {
    nstate.c_cc[VMIN] = 0; /* We already have characters; don't wait for more. */
    nstate.c_cc[VTIME] = 0;
    memcpy(keys, keyp, len);
    keyp = key_buf + MAX_KEY_BUF;
  }

  setattr(TCSANOW); /* Set waiting period and number of characters. */

  nbytes = len;
  if (len < max_key_chars)  /* Don't get more chars if we already have the maximum for a keystroke. */
    nbytes += read(STDIN_FILENO, keys + len, max_key_chars);

  if (nbytes >= 0) {
    if (mode & GETKEY_NONFILTERED) {
      int i;
      for (i = nbytes - 1; i >= 0; i--)
        term_unget(keys[i]);
      key = keys[nbytes - 1];
    } else {
      key = translate_key(keys, nbytes);
      while (key == KBD_META)
        key = term_getkey() | KBD_META;
    }
  }

  free(keys);
  return key;
}

static void winch_sig_handler(int signo)
{
  assert(signo == SIGWINCH);
  read_screen_size();
  resize_windows();
  termp->width = ZILE_COLS;
  termp->height = ZILE_LINES;
  resync_redisplay();
  term_refresh();
}

int term_xgetkey(int mode, int arg)
{
  int key;
  struct sigaction winch_sig;

  winch_sig.sa_handler = winch_sig_handler;
  sigemptyset(&winch_sig.sa_mask);
  winch_sig.sa_flags = SA_RESTART;
  sigaction(SIGWINCH, &winch_sig, NULL);

  key = xgetkey(mode, arg);

  winch_sig.sa_handler = SIG_DFL;
  sigaction(SIGWINCH, &winch_sig, NULL);

  return key;
}

int term_getkey(void)
{
  return term_xgetkey(0, 0);
}

void term_unget(int c)
{
  if (keyp == key_buf || c == KBD_NOKEY)
    return;

  if (c & KBD_CTL)
    switch (c & 0xff) {
    case '@':
      *--keyp = '\0';
      break;
    case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    case 'f':  case 'g':  case 'h':             case 'j':
    case 'k':  case 'l':             case 'n':  case 'o':
    case 'p':  case 'q':  case 'r':  case 's':  case 't':
    case 'u':  case 'v':  case 'w':  case 'x':  case 'y':
    case 'z':
      *--keyp = (c & 0xff) - 'a' + 1;
      break;
    case '_':
      *--keyp = '\37';
      break;
    }
  else
    *--keyp = c & 0xff;
  if (c & KBD_META)
    *--keyp = '\033';
}
