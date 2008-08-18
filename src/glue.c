/* Miscellaneous functions

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.

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

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

/*
 * Ring the bell if ring-bell is set.
 */
void
ding (void)
{
  if (thisflag & FLAG_DEFINING_MACRO)
    cancel_kbd_macro ();

  if (get_variable_bool ("ring-bell"))
    term_beep ();
}

/* FIXME: Use an array list */
#define MAX_KEY_BUF	16
static int key_buf[MAX_KEY_BUF];
static int *keyp = key_buf;
static size_t _last_key;

/* Return last key pressed */
size_t
lastkey (void)
{
  return _last_key;
}

/*
 * Get a keystroke, waiting for up to timeout 10ths of a second if
 * mode contains GETKEY_DELAYED, and translating it into a
 * keycode unless mode contains GETKEY_UNFILTERED.
 */
size_t
xgetkey (int mode, size_t timeout)
{
  if (keyp > key_buf)
    _last_key = *--keyp;
  else
    _last_key = term_xgetkey (mode, timeout);

  if (thisflag & FLAG_DEFINING_MACRO)
    add_key_to_cmd (_last_key);

  return _last_key;
}

/*
 * Wait for a keystroke indefinitely, and return the
 * corresponding keycode.
 */
size_t
getkey (void)
{
  return xgetkey (0, 0);
}

/*
 * Wait for timeout 10ths if a second or until a key is pressed.
 * The key is then available with [x]getkey().
 */
void
waitkey (size_t timeout)
{
  ungetkey (xgetkey (GETKEY_DELAYED, timeout));
}

void
ungetkey (size_t key)
{
  if (keyp < key_buf + MAX_KEY_BUF && key != KBD_NOKEY)
    *keyp++ = key;
}

/*
 * Copy a region of text into an allocated buffer.
 */
char *
copy_text_block (size_t startn, size_t starto, size_t size)
{
  char *buf, *dp;
  size_t max_size, n, i;
  Line *lp;

  max_size = 10;
  dp = buf = (char *) xzalloc (max_size);

  lp = cur_bp->pt.p;
  n = cur_bp->pt.n;
  if (n > startn)
    do
      lp = lp->prev;
    while (--n > startn);
  else if (n < startn)
    do
      lp = lp->next;
    while (++n < startn);

  for (i = starto; dp - buf < (int) size;)
    {
      if (dp >= buf + max_size)
	{
	  int save_off = dp - buf;
	  max_size += 10;
	  buf = (char *) xrealloc (buf, max_size);
	  dp = buf + save_off;
	}
      if (i < astr_len (lp->text))
	*dp++ = *astr_char (lp->text, (ptrdiff_t) (i++));
      else
	{
	  *dp++ = '\n';
	  lp = lp->next;
	  i = 0;
	}
    }

  return buf;
}

/*
 * Return a string of maximum length `maxlen' beginning with a `...'
 * sequence if a cut is need.
 */
astr
shorten_string (char *s, int maxlen)
{
  astr as = astr_new ();
  int len = strlen (s);

  if (len <= maxlen)
    astr_cpy_cstr (as, s);
  else
    {
      astr_cpy_cstr (as, "...");
      astr_cat_cstr (as, s + len - maxlen + 3);
    }

  return as;
}

/*
 * Jump to the specified line number and offset.
 */
void
goto_point (Point pt)
{
  if (cur_bp->pt.n > pt.n)
    do
      FUNCALL (previous_line);
    while (cur_bp->pt.n > pt.n);
  else if (cur_bp->pt.n < pt.n)
    do
      FUNCALL (next_line);
    while (cur_bp->pt.n < pt.n);

  if (cur_bp->pt.o > pt.o)
    do
      FUNCALL (backward_char);
    while (cur_bp->pt.o > pt.o);
  else if (cur_bp->pt.o < pt.o)
    do
      FUNCALL (forward_char);
    while (cur_bp->pt.o < pt.o);
}
