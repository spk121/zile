/* Dynamically allocated strings
   Copyright (c) 2001-2004 Sandro Sigala.
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

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "astr.h"
#include "zile.h"
#include "extern.h"

#define ALLOCATION_CHUNK_SIZE	16

astr astr_new(void)
{
  astr as;
  as = (astr)zmalloc(sizeof *as);
  as->maxlen = ALLOCATION_CHUNK_SIZE;
  as->len = 0;
  as->text = (char *)zmalloc(as->maxlen + 1);
  memset(as->text, 0, as->maxlen + 1);
  return as;
}

static void astr_resize(astr as, size_t reqsize)
{
  assert(as != NULL);
  if (reqsize > as->maxlen) {
    as->maxlen = reqsize + ALLOCATION_CHUNK_SIZE;
    as->text = (char *)zrealloc(as->text, as->maxlen + 1);
  }
}

static int astr_pos(astr as, ptrdiff_t pos)
{
  assert(as != NULL);
  if (pos < 0)
    pos = as->len + pos;
  assert(pos >=0 && pos <= (int)as->len);
  return pos;
}

char *astr_char(const astr as, ptrdiff_t pos)
{
  assert(as != NULL);
  pos = astr_pos(as, pos);
  return as->text + pos;
}

void astr_delete(astr as)
{
  assert(as != NULL);
  free(as->text);
  as->text = NULL;
  free(as);
}

static astr astr_cpy_x(astr as, const char *s, size_t csize)
{
  astr_resize(as, csize);
  memcpy(as->text, s, csize);
  as->len = csize;
  as->text[csize] = '\0';
  return as;
}

astr astr_cpy(astr as, const astr src)
{
  assert(src != NULL);
  return astr_cpy_x(as, src->text, src->len);
}

astr astr_cpy_cstr(astr as, const char *s)
{
  assert(s != NULL);
  return astr_cpy_x(as, s, strlen(s));
}

static astr astr_cat_x(astr as, const char *s, size_t csize)
{
  astr_resize(as, as->len + csize);
  memcpy(as->text + as->len, s, csize);
  as->len += csize;
  as->text[as->len] = '\0';
  return as;
}

astr astr_cat(astr as, const astr src)
{
  assert(src != NULL);
  return astr_cat_x(as, src->text, src->len);
}

astr astr_ncat_cstr(astr as, const char *s, size_t len)
{
  return astr_cat_x(as, s, len);
}

astr astr_cat_cstr(astr as, const char *s)
{
  return astr_ncat_cstr(as, s, strlen(s));
}

astr astr_cat_char(astr as, int c)
{
  assert(as != NULL);
  astr_resize(as, as->len + 1);
  as->text[as->len] = (char)c;
  as->text[++as->len] = '\0';
  return as;
}

astr astr_cat_delete(astr as, const astr src)
{
  assert(src != NULL);
  astr_cat_x(as, src->text, src->len);
  astr_delete(src);
  return as;
}

astr astr_substr(const astr as, ptrdiff_t pos, size_t size)
{
  assert(as != NULL);
  pos = astr_pos(as, pos);
  assert(pos + size <= as->len);
  return astr_ncat_cstr(astr_new(), astr_char(as, pos), size);
}

int astr_find(const astr as, const astr src)
{
  return astr_find_cstr(as, src->text);
}

int astr_find_cstr(const astr as, const char *s)
{
  char *sp;
  assert(as != NULL && s != NULL);
  sp = strstr(as->text, s);
  return (sp == NULL) ? -1 : sp - as->text;
}

int astr_rfind(const astr as, const astr src)
{
  return astr_rfind_cstr(as, src->text);
}

int astr_rfind_cstr(const astr as, const char *s)
{
  char *sp;
  assert(as != NULL && s != NULL);
  sp = strrstr(as->text, s);
  return (sp == NULL) ? -1 : sp - as->text;
}

static astr astr_replace_x(astr as, ptrdiff_t pos, size_t size, const char *s, size_t csize)
{
  astr tail;

  assert(as != NULL);

  pos = astr_pos(as, pos);
  if (as->len - pos < size)
    size = as->len - pos;
  tail = astr_substr(as, pos + (ptrdiff_t)size, astr_len(as) - (pos + size));
  astr_truncate(as, pos);
  astr_ncat_cstr(as, s, csize);
  astr_cat(as, tail);
  astr_delete(tail);

  return as;
}

astr astr_replace(astr as, ptrdiff_t pos, size_t size, const astr src)
{
  assert(src != NULL);
  return astr_replace_x(as, pos, size, src->text, src->len);
}

astr astr_replace_cstr(astr as, ptrdiff_t pos, size_t size, const char *s)
{
  assert(s != NULL);
  return astr_replace_x(as, pos, size, s, strlen(s));
}

astr astr_replace_char(astr as, ptrdiff_t pos, size_t size, int c)
{
  return astr_replace_x(as, pos, size, (const char *)&c, (size_t)1);
}

astr astr_insert(astr as, ptrdiff_t pos, const astr src)
{
  assert(src != NULL);
  return astr_replace_x(as, pos, (size_t)0, src->text, src->len);
}

astr astr_insert_cstr(astr as, ptrdiff_t pos, const char *s)
{
  assert(s != NULL);
  return astr_replace_x(as, pos, (size_t)0, s, strlen(s));
}

astr astr_insert_char(astr as, ptrdiff_t pos, int c)
{
  char ch = (char)c;
  return astr_replace_x(as, pos, (size_t)0, &ch, (size_t)1);
}

astr astr_remove(astr as, ptrdiff_t pos, size_t size)
{
  return astr_replace_x(as, pos, size, "", (size_t)0);
}

/* Don't define in terms of astr_remove, to avoid endless recursion */
astr astr_truncate(astr as, ptrdiff_t pos)
{
  assert(as != NULL);
  pos = astr_pos(as, pos);
  if ((size_t)pos < as->len) {
    as->len = pos;
    as->text[pos] = '\0';
  }
  return as;
}

astr astr_fgets(FILE *f)
{
  int c;
  astr as;

  if (feof(f))
    return NULL;
  as = astr_new();
  while ((c = fgetc(f)) != EOF && c != '\n')
    astr_cat_char(as, c);
  return as;
}

astr astr_vafmt(astr as, const char *fmt, va_list ap)
{
  char *buf;
  zvasprintf(&buf, fmt, ap);
  astr_cat_cstr(as, buf);
  free(buf);
  return as;
}

astr astr_afmt(astr as, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  astr_vafmt(as, fmt, ap);
  va_end(ap);
  return as;
}

#ifdef TEST

#include <stdio.h>

static void assert_eq(astr as, const char *s)
{
  if (astr_cmp_cstr(as, s))
    printf("test failed: \"%s\" != \"%s\"\n", as->text, s);
}

/*
 * Stub to make zmalloc &c. happy.
 */
void zile_exit(int doabort)
{
  (void)doabort;
  exit(2);
}

int main(void)
{
  astr as1, as2, as3;
  int i;
  FILE *fp;

  as1 = astr_new();
  astr_cpy_cstr(as1, "hello world");
  astr_cat_char(as1, '!');
  assert_eq(as1, "hello world!");

  as3 = astr_substr(as1, 6, 5);
  assert_eq(as3, "world");

  as2 = astr_new();
  astr_cpy_cstr(as2, "The ");
  astr_cat(as2, as3);
  astr_cat_char(as2, '.');
  assert_eq(as2, "The world.");

  astr_delete(as3);
  as3 = astr_substr(as1, -6, 5);
  assert_eq(as3, "world");

  astr_cpy_cstr(as1, "12345");
  astr_delete(as2);

  astr_cpy_cstr(as1, "12345");
  astr_insert_cstr(as1, 3, "mid");
  astr_insert_cstr(as1, 0, "begin");
  astr_cat_cstr(as1, "end");
  assert_eq(as1, "begin123mid45end");

  astr_cpy_cstr(as1, "12345");
  astr_insert_char(as1, -2, 'x');
  astr_insert_char(as1, -6, 'y');
  astr_insert_char(as1, 7, 'z');
  assert_eq(as1, "y123x45z");

  astr_cpy_cstr(as1, "1234567");
  astr_replace_cstr(as1, -4, 2, "foo");
  assert_eq(as1, "123foo67");

  astr_cpy_cstr(as1, "1234567");
  astr_replace_cstr(as1, 1, 3, "foo");
  assert_eq(as1, "1foo567");

  astr_cpy_cstr(as1, "1234567");
  astr_replace_cstr(as1, -1, 5, "foo");
  assert_eq(as1, "123456foo");

  astr_cpy_cstr(as1, "1234567");
  astr_remove(as1, 4, 10);
  assert_eq(as1, "1234");

  astr_cpy_cstr(as1, "abc def de ab cd ab de fg");
  while ((i = astr_find_cstr(as1, "de")) >= 0)
    astr_replace_cstr(as1, i, 2, "xxx");
  assert_eq(as1, "abc xxxf xxx ab cd ab xxx fg");
  while ((i = astr_find_cstr(as1, "ab")) >= 0)
    astr_remove(as1, i, 2);
  assert_eq(as1, "c xxxf xxx  cd  xxx fg");
  while ((i = astr_find_cstr(as1, "  ")) >= 0)
    astr_replace_char(as1, i, 2, ' ');
  assert_eq(as1, "c xxxf xxx cd xxx fg");

  astr_cpy_cstr(as1, "12345");
  as2 = astr_substr(as1, -2, 2);
  assert_eq(as2, "45");

  astr_cpy_cstr(as1, "12345");
  astr_delete(as2);
  as2 = astr_substr(as1, -5, 5);
  assert_eq(as2, "12345");

  astr_cpy_cstr(as1, "1234567");
  astr_replace_cstr(as1, -4, 2, "foo");
  assert_eq(as1, "123foo67");

  astr_cpy_cstr(as1, "1234567");
  astr_replace_cstr(as1, 1, 3, "foo");
  assert_eq(as1, "1foo567");

  astr_cpy_cstr(as1, "1234567");
  astr_replace_cstr(as1, -1, 5, "foo");
  assert_eq(as1, "123456foo");

  astr_cpy_cstr(as1, "abc def de ab cd ab de fg");
  while ((i = astr_find_cstr(as1, "de")) >= 0)
    astr_replace_cstr(as1, i, 2, "xxx");
  assert_eq(as1, "abc xxxf xxx ab cd ab xxx fg");

  astr_cpy_cstr(as1, "");
  astr_afmt(as1, "%s * %d = ", "5", 3);
  astr_afmt(as1, "%d", 15);
  assert_eq(as1, "5 * 3 = 15");
  astr_delete(as1);

  assert(fp = fopen(SRCPATH "/astr.c", "r"));
  as1 = astr_fgets(fp);
  printf("The first line of astr.c is: \"%s\"\n", astr_cstr(as1));
  assert(fclose(fp) == 0);

  astr_delete(as1);
  astr_delete(as2);
  astr_delete(as3);
  printf("astr test successful.\n");

  return 0;
}

#endif /* TEST */
