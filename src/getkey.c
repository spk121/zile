/* Getting and ungetting key strokes

   Copyright (c) 1997-2004, 2008-2011 Free Software Foundation, Inc.

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

#include <assert.h>

#include "main.h"
#include "extern.h"

static size_t _last_key;

/* Return last key pressed */
size_t
lastkey (void)
{
  return _last_key;
}

/*
 * Get a keystroke, waiting for up to WAITKEY_DEFAULT 10ths of a
 * second if mode contains GETKEY_DELAYED, and translating it into a
 * keycode.
 */
size_t
getkey (int mode)
{
  _last_key = term_getkey (mode);

  if (thisflag & FLAG_DEFINING_MACRO)
    add_key_to_cmd (_last_key);

  return _last_key;
}

size_t
getkey_unfiltered (int mode, int **codes)
{
  size_t n = term_getkey_unfiltered (mode, codes);

  assert (n > 0);
  _last_key = *codes[n - 1];
  if (thisflag & FLAG_DEFINING_MACRO)
    for (size_t i = 0; i < n; i++)
      add_key_to_cmd (*codes[i]);

  return n;
}

/*
 * Wait for WAITKEY_DEFAULT 10ths if a second or until a key is
 * pressed. The key is then available with [x]getkey.
 */
void
waitkey (void)
{
  ungetkey (getkey (GETKEY_DELAYED));
}

/*
 * Push a key into the input buffer.
 */
void
pushkey (size_t key)
{
  term_ungetkey (key);
}

/*
 * Unget a key as if it had not been fetched.
 */
void ungetkey (size_t key)
{
  pushkey (key);

  if (thisflag & FLAG_DEFINING_MACRO)
    remove_key_from_cmd ();
}
