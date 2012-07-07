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
#include <sys/time.h>

#include "main.h"
#include "extern.h"

/* Maximum time to avoid screen updates when catching up with buffered
   input, in milliseconds. */
#define MAX_RESYNC_MS 500

/* These are not required for POSIX.1-2001, and not always defined in
   the system headers. */
#if !defined(timeradd)
#  define timeradd(_a, _b, _result)				\
   do {								\
	(_result)->tv_sec  = (_a)->tv_sec + (_b)->tv_sec;	\
	(_result)->tv_usec = (_a)->tv_usec + (_b)->tv_usec;	\
	if ((_result)->tv_usec > 1000000) {			\
	  ++(_result)->tv_sec;					\
	  (_result)->tv_usec -= 1000000;			\
	}							\
   while (0)
#endif

#if !defined(timercmp)
#  define timercmp(_a, _b, _CMP)				\
   (((_a)->tv_sec == (_b)->tv_sec)				\
    ? ((_a)->tv_usec _CMP (_b)->tv_usec)			\
    : ((_a)->tv_sec  _CMP (_b)->tv_sec))
#endif

static size_t _last_key;

/* Return last key pressed */
size_t
lastkey (void)
{
  return _last_key;
}

/*
 * Get a keystroke, waiting for up to GETKEY_DELAYED ms, and translate
 * it into a keycode.
 */
size_t
getkey (int mode)
{
  static struct timeval next_refresh = { 0, 0 };
  static struct timeval refresh_wait = {
    MAX_RESYNC_MS / 1000, (MAX_RESYNC_MS % 1000) * 1000 };
  static struct timeval now;

  gettimeofday (&now, NULL);
  _last_key = term_getkey (0);

  if (_last_key == KBD_NOKEY || !timercmp (&now, &next_refresh, <))
    {
      term_redisplay ();
      term_refresh ();
      timeradd (&now, &refresh_wait, &next_refresh);
    }

  if (_last_key == KBD_NOKEY)
    _last_key = term_getkey (mode);

  if (thisflag & FLAG_DEFINING_MACRO)
    add_key_to_cmd (_last_key);

  return _last_key;
}

size_t
getkey_unfiltered (int mode)
{
  int key = term_getkey_unfiltered (mode);
  assert (key >= 0);

  _last_key = (size_t) key;
  if (thisflag & FLAG_DEFINING_MACRO)
    add_key_to_cmd (key);

  return key;
}

/*
 * Wait for GETKEY_DELAYED ms or until a key is pressed.  The key is
 * then available with [x]getkey.
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
