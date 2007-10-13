/* Miscellaneous functions
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

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

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

/*
 * Emit an error sound only when the appropriate variable
 * is set.
 */
void ding(void)
{
  if (thisflag & FLAG_DEFINING_MACRO)
    cancel_kbd_macro();

  if (lookup_bool_variable("ring-bell"))
    term_beep();
}

#define MAX_KEY_BUF	16

static int key_buf[MAX_KEY_BUF];
static int *keyp = key_buf;
static size_t _last_key;

/* Return last key pressed */
size_t lastkey(void)
{
  return _last_key;
}

/*
 * Get a keystroke, waiting for up to timeout 10ths of a second if
 * mode contains GETKEY_DELAYED, and translating it into a
 * keycode unless mode contains GETKEY_UNFILTERED.
 */
size_t xgetkey(int mode, size_t timeout)
{
  if (keyp > key_buf)
    _last_key = *--keyp;
  else
    _last_key = term_xgetkey(mode, timeout);

  if (thisflag & FLAG_DEFINING_MACRO)
    add_key_to_cmd(_last_key);

  return _last_key;
}

/*
 * Wait for a keystroke indefinitely, and return the
 * corresponding keycode.
 */
size_t getkey(void)
{
  return xgetkey(0, 0);
}

/*
 * Wait for timeout 10ths if a second or until a key is pressed.
 * The key is then available with [x]getkey().
 */
void waitkey(size_t timeout)
{
  ungetkey(xgetkey(GETKEY_DELAYED, timeout));
}

void ungetkey(size_t key)
{
  if (keyp < key_buf + MAX_KEY_BUF && key != KBD_NOKEY)
    *keyp++ = key;
}

/*
 * Copy a region of text into an allocated buffer.
 */
char *copy_text_block(size_t startn, size_t starto, size_t size)
{
  char *buf, *dp;
  size_t max_size, n, i;
  Line *lp;

  max_size = 10;
  dp = buf = (char *)zmalloc(max_size);

  lp = cur_bp->pt.p;
  n = cur_bp->pt.n;
  if (n > startn)
    do
      lp = list_prev(lp);
    while (--n > startn);
  else if (n < startn)
    do
      lp = list_next(lp);
    while (++n < startn);

  for (i = starto; dp - buf < (int)size;) {
    if (dp >= buf + max_size) {
      int save_off = dp - buf;
      max_size += 10;
      buf = (char *)zrealloc(buf, max_size);
      dp = buf + save_off;
    }
    if (i < astr_len(lp->item))
      *dp++ = *astr_char(lp->item, (ptrdiff_t)(i++));
    else {
      *dp++ = '\n';
      lp = list_next(lp);
      i = 0;
    }
  }

  return buf;
}

/*
 * Return a string of maximum length `maxlen' beginning with a `...'
 * sequence if a cut is need.
 */
astr shorten_string(char *s, int maxlen)
{
  int len;
  astr as = astr_new();

  if ((len = strlen(s)) <= maxlen)
    astr_cpy_cstr(as, s);
  else {
    astr_cpy_cstr(as, "...");
    astr_cat_cstr(as, s + len - maxlen + 3);
  }

  return as;
}

/*
 * Replace the matches of `match' into `s' with the string `subst'.  The
 * two strings `match' and `subst' must have the same length.
 */
char *replace_string(char *s, char *match, char *subst)
{
  char *sp = s, *p;
  size_t slen = strlen(subst);

  if (strlen(match) != slen)
    return NULL;
  while ((p = strstr(sp, match)) != NULL) {
    strncpy(p, subst, slen);
    sp = p + slen;
  }

  return s;
}

/*
 * Compact the spaces into tabulations according to the `tw' tab width.
 */
void tabify_string(char *dest, char *src, size_t scol, size_t tw)
{
  char *sp, *dp;
  size_t dcol = scol, ocol = scol;

  for (sp = src, dp = dest;; ++sp)
    switch (*sp) {
    case ' ':
      ++dcol;
      break;
    case '\t':
      dcol += tw;
      dcol -= dcol % tw;
      break;
    default:
      while (((ocol + tw) - (ocol + tw) % tw) <= dcol) {
        if (ocol + 1 == dcol)
          break;
        *dp++ = '\t';
        ocol += tw;
        ocol -= ocol % tw;
      }
      while (ocol < dcol) {
        *dp++ = ' ';
        ocol++;
      }
      *dp++ = *sp;
      if (*sp == '\0')
        return;
      ++ocol;
      ++dcol;
    }
}

/*
 * Expand the tabulations into spaces according to the `tw' tab width.
 * The output buffer should be big enough to contain the expanded string.
 * To be sure, sizeof(dest) should be >= strlen(src)*tw + 1.
 */
void untabify_string(char *dest, char *src, size_t scol, size_t tw)
{
  char *sp, *dp;
  int col = scol;

  for (sp = src, dp = dest; *sp != '\0'; ++sp)
    if (*sp == '\t') {
      do
        *dp++ = ' ', ++col;
      while ((col%tw) > 0);
    }
    else
      *dp++ = *sp, ++col;
  *dp = '\0';
}

/*
 * Jump to the specified line number and offset.
 */
void goto_point(Point pt)
{
  if (cur_bp->pt.n > pt.n)
    do
      FUNCALL(previous_line);
    while (cur_bp->pt.n > pt.n);
  else if (cur_bp->pt.n < pt.n)
    do
      FUNCALL(next_line);
    while (cur_bp->pt.n < pt.n);

  if (cur_bp->pt.o > pt.o)
    do
      FUNCALL(backward_char);
    while (cur_bp->pt.o > pt.o);
  else if (cur_bp->pt.o < pt.o)
    do
      FUNCALL(forward_char);
    while (cur_bp->pt.o < pt.o);
}
