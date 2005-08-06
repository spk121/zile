/* Key sequences functions
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

/*	$Id: keys.c,v 1.21 2005/08/06 16:23:30 rrt Exp $	*/

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "zile.h"
#include "extern.h"

/*
 * Convert a key chord into its ASCII representation
 */
astr chordtostr(size_t key)
{
  astr as = astr_new();

  if (key & KBD_CTRL)
    astr_cat_cstr(as, "C-");
  if (key & KBD_META)
    astr_cat_cstr(as, "M-");
  key &= ~(KBD_CTRL | KBD_META);

  switch (key) {
  case KBD_PGUP:
    astr_cat_cstr(as, "PGUP");
    break;
  case KBD_PGDN:
    astr_cat_cstr(as, "PGDN");
    break;
  case KBD_HOME:
    astr_cat_cstr(as, "HOME");
    break;
  case KBD_END:
    astr_cat_cstr(as, "END");
    break;
  case KBD_DEL:
    astr_cat_cstr(as, "DEL");
    break;
  case KBD_BS:
    astr_cat_cstr(as, "BS");
    break;
  case KBD_INS:
    astr_cat_cstr(as, "INS");
    break;
  case KBD_LEFT:
    astr_cat_cstr(as, "LEFT");
    break;
  case KBD_RIGHT:
    astr_cat_cstr(as, "RIGHT");
    break;
  case KBD_UP:
    astr_cat_cstr(as, "UP");
    break;
  case KBD_DOWN:
    astr_cat_cstr(as, "DOWN");
    break;
  case KBD_RET:
    astr_cat_cstr(as, "RET");
    break;
  case KBD_TAB:
    astr_cat_cstr(as, "TAB");
    break;
  case KBD_F1:
    astr_cat_cstr(as, "F1");
    break;
  case KBD_F2:
    astr_cat_cstr(as, "F2");
    break;
  case KBD_F3:
    astr_cat_cstr(as, "F3");
    break;
  case KBD_F4:
    astr_cat_cstr(as, "F4");
    break;
  case KBD_F5:
    astr_cat_cstr(as, "F5");
    break;
  case KBD_F6:
    astr_cat_cstr(as, "F6");
    break;
  case KBD_F7:
    astr_cat_cstr(as, "F7");
    break;
  case KBD_F8:
    astr_cat_cstr(as, "F8");
    break;
  case KBD_F9:
    astr_cat_cstr(as, "F9");
    break;
  case KBD_F10:
    astr_cat_cstr(as, "F10");
    break;
  case KBD_F11:
    astr_cat_cstr(as, "F11");
    break;
  case KBD_F12:
    astr_cat_cstr(as, "F12");
    break;
  case ' ':
    astr_cat_cstr(as, "SPC");
    break;
  default:
    if (isgraph(key))
      astr_cat_char(as, (int)(key & 0xff));
    else
      astr_afmt(as, "<%x>", key);
  }

  return as;
}

/*
 * Array of key names in lexical order
 */
static const char *keyname[] = {
  "\\BS",
  "\\C-",
  "\\DEL",
  "\\DOWN",
  "\\END",
  "\\F1",
  "\\F10",
  "\\F11",
  "\\F12",
  "\\F2",
  "\\F3",
  "\\F4",
  "\\F5",
  "\\F6",
  "\\F7",
  "\\F8",
  "\\F9",
  "\\HOME",
  "\\INS",
  "\\LEFT",
  "\\M-",
  "\\PGDN",
  "\\PGUP",
  "\\RET",
  "\\RIGHT",
  "\\TAB",
  "\\UP",
  "\\\\",
};

/*
 * Array of key codes in the same order as keyname above
 */
static int keycode[] = {
  KBD_BS,
  KBD_CTRL,
  KBD_DEL,
  KBD_DOWN,
  KBD_END,
  KBD_F1,
  KBD_F10,
  KBD_F11,
  KBD_F12,
  KBD_F2,
  KBD_F3,
  KBD_F4,
  KBD_F5,
  KBD_F6,
  KBD_F7,
  KBD_F8,
  KBD_F9,
  KBD_HOME,
  KBD_INS,
  KBD_LEFT,
  KBD_META,
  KBD_PGDN,
  KBD_PGUP,
  KBD_RET,
  KBD_RIGHT,
  KBD_TAB,
  KBD_UP,
  '\\',
};

/*
 * String prefix comparison.
 */
static int bstrcmp_prefix(const void *s, const void *t)
{
  return strncmp(*(const char **)s, *(const char **)t,
                 strlen(*(const char **)t));
}

/*
 * Convert a key string to its key code.
 */
static size_t strtokey(char *buf, size_t *len)
{
  if (*buf == '\\') {
    char **p = bsearch(&buf, keyname,
                       sizeof(keyname) / sizeof(keyname[0]),
                       sizeof(char *),
                       bstrcmp_prefix);
    if (p == NULL) {
      *len = 0;
      return KBD_NOKEY;
    } else {
      *len = strlen(*p);
      return keycode[p - (char **)keyname];
    }
  } else {
    *len = 1;
    return (size_t)*buf;
  }
}

/*
 * Convert a key chord string to its key code.
 */
size_t strtochord(char *buf, size_t *len)
{
  size_t key, l;

  key = strtokey(buf, &l);
  if (key == KBD_NOKEY) {
    *len = 0;
    return KBD_NOKEY;
  }

  *len = l;

  if (key == KBD_CTRL || key == KBD_META) {
    size_t k = strtochord(buf + l, &l);
    if (k == KBD_NOKEY) {
      *len = 0;
      return KBD_NOKEY;
    }
    *len += l;
    key |= k;
  }

  return key;
}

/*
 * Convert a key sequence string into a key code sequence.
 */
int keystrtovec(char *key, size_t **keys)
{
  vector *v = vec_new(sizeof(size_t));
  size_t size;

  for (size = 0; *key != '\0'; size++) {
    size_t len, code = strtochord(key, &len);
    vec_item(v, size, size_t) = code;
    if ((vec_item(v, size, size_t) = code) == KBD_NOKEY) {
      vec_delete(v);
      return -1;
    }
    key += len;
  }

  *keys = vec_toarray(v);
  return size;
}

/*
 * Convert a key code sequence into a key code sequence string.
 */
astr keyvectostr(size_t *keys, size_t numkeys)
{
  size_t i;
  astr as = astr_new();

  for (i = 0; i < numkeys; i++) {
    astr key = chordtostr(keys[i]);
    astr_cat(as, key);
    astr_delete(key);
    if (i > 0)
      astr_cat_char(as, ' ');
  }

  return as;
}

/*
 * Convert a key like "\\C-xrs" to "C-x r s"
 */
astr simplify_key(char *key)
{
  size_t i, j, *keys;
  astr dest = astr_new();

  if (key == NULL)
    return dest;
  i = keystrtovec(key, &keys);
  for (j = 0; j < i; j++) {
    astr as;
    if (j > 0)
      astr_cat_cstr(dest, " ");
    as = chordtostr(keys[j]);
    astr_cat(dest, as);
    astr_delete(as);
  }
  if (i > 0)
    free(keys);

  return dest;
}
