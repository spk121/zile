/* Key sequences functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2007 Reuben Thomas.
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
    astr_cat_cstr(as, "<prior>");
    break;
  case KBD_PGDN:
    astr_cat_cstr(as, "<next>");
    break;
  case KBD_HOME:
    astr_cat_cstr(as, "<home>");
    break;
  case KBD_END:
    astr_cat_cstr(as, "<end>");
    break;
  case KBD_DEL:
    astr_cat_cstr(as, "<delete>");
    break;
  case KBD_BS:
    astr_cat_cstr(as, "<backspace>");
    break;
  case KBD_INS:
    astr_cat_cstr(as, "<insert>");
    break;
  case KBD_LEFT:
    astr_cat_cstr(as, "<left>");
    break;
  case KBD_RIGHT:
    astr_cat_cstr(as, "<right>");
    break;
  case KBD_UP:
    astr_cat_cstr(as, "<up>");
    break;
  case KBD_DOWN:
    astr_cat_cstr(as, "<down>");
    break;
  case KBD_RET:
    astr_cat_cstr(as, "<RET>");
    break;
  case KBD_TAB:
    astr_cat_cstr(as, "<TAB>");
    break;
  case KBD_F1:
    astr_cat_cstr(as, "<f1>");
    break;
  case KBD_F2:
    astr_cat_cstr(as, "<f2>");
    break;
  case KBD_F3:
    astr_cat_cstr(as, "<f3>");
    break;
  case KBD_F4:
    astr_cat_cstr(as, "<f4>");
    break;
  case KBD_F5:
    astr_cat_cstr(as, "<f5>");
    break;
  case KBD_F6:
    astr_cat_cstr(as, "<f6>");
    break;
  case KBD_F7:
    astr_cat_cstr(as, "<f7>");
    break;
  case KBD_F8:
    astr_cat_cstr(as, "<f8>");
    break;
  case KBD_F9:
    astr_cat_cstr(as, "<f9>");
    break;
  case KBD_F10:
    astr_cat_cstr(as, "<f10>");
    break;
  case KBD_F11:
    astr_cat_cstr(as, "<f11>");
    break;
  case KBD_F12:
    astr_cat_cstr(as, "<f12>");
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
 * Array of key names
 */
static const char *keyname[] = {
  "\\BACKSPACE",
  "\\C-",
  "\\DELETE",
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
  "\\INSERT",
  "\\LEFT",
  "\\M-",
  "\\NEXT",
  "\\PAGEDOWN",
  "\\PAGEUP",
  "\\PRIOR",
  "\\RET",
  "\\RIGHT",
  "\\SPC",
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
  KBD_PGDN,
  KBD_PGUP,
  KBD_PGUP,
  KBD_RET,
  KBD_RIGHT,
  ' ',
  KBD_TAB,
  KBD_UP,
  '\\',
};

/*
 * Convert a key string to its key code.
 */
static size_t strtokey(char *buf, size_t *len)
{
  if (*buf == '\\') {
    size_t i;
    char **p = NULL;
    for (i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++)
      if (strncmp(keyname[i], buf, strlen(keyname[i])) == 0)
        p = (char **)&keyname[i];
    if (p == NULL) {
      *len = 0;
      return KBD_NOKEY;
    } else {
      *len = strlen(*p);
      return keycode[p - (char **)keyname];
    }
  } else {
    *len = 1;
    return (size_t)*(unsigned char *)buf;
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
    if (i < numkeys - 1)
      astr_cat_char(as, ' ');
  }

  return as;
}

/*
 * Convert a key like "\\C-xrs" to "C-x r s"
 */
astr simplify_key(char *key)
{
  int i, j;
  size_t *keys;
  astr dest = astr_new();

  if (key == NULL)
    return dest;
  i = keystrtovec(key, &keys);
  for (j = 0; j < i; j++) {
    astr as;
    if (j > 0)
      astr_cat_char(dest, ' ');
    as = chordtostr(keys[j]);
    astr_cat(dest, as);
    astr_delete(as);
  }
  if (i > 0)
    free(keys);

  return dest;
}
