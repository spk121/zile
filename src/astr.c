/* Dynamically allocated strings

   Copyright (c) 2001, 2002, 2003, 2004, 2005, 2008, 2009 Free Software Foundation, Inc.

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
#include <stdlib.h>
#include <string.h>
#include "xalloc.h"

#include "astr.h"
#include "xalloc_extra.h"

#define ALLOCATION_CHUNK_SIZE	16

/*
 * The implementation of astr.
 */
struct astr
{
  char *text;		/* The string buffer. */
  size_t len;		/* The length of the string. */
  size_t maxlen;	/* The buffer size. */
};

astr
astr_new (void)
{
  astr as;
  as = (astr) XZALLOC (struct astr);
  as->maxlen = ALLOCATION_CHUNK_SIZE;
  as->len = 0;
  as->text = (char *) xzalloc (as->maxlen + 1);
  memset (as->text, 0, as->maxlen + 1);
  return as;
}

void
astr_delete (astr as)
{
  assert (as != NULL);
  free (as->text);
  as->text = NULL;
  free (as);
}

const char *
astr_cstr (astr as)
{
  return (const char *) (as->text);
}

size_t
astr_len (astr as)
{
  return as->len;
}

astr
astr_ncat_cstr (astr as, const char *s, size_t csize)
{
  assert (as != NULL);
  if (as->len + csize > as->maxlen)
    {
      as->maxlen = as->len + csize + ALLOCATION_CHUNK_SIZE;
      as->text = (char *) xrealloc (as->text, as->maxlen + 1);
    }
  memcpy (as->text + as->len, s, csize);
  as->len += csize;
  as->text[as->len] = '\0';
  return as;
}

astr
astr_nreplace_cstr (astr as, size_t pos, size_t size, const char *s,
                    size_t csize)
{
  astr tail;

  assert (as != NULL);
  assert (pos <= as->len);
  if (as->len - pos < size)
    size = as->len - pos;
  tail =
    astr_substr (as, pos + (size_t) size, astr_len (as) - (pos + size));
  astr_truncate (as, pos);
  astr_ncat_cstr (as, s, csize);
  astr_cat (as, tail);
  astr_delete (tail);

  return as;
}

/* Don't define in terms of astr_remove, to avoid endless recursion */
astr
astr_truncate (astr as, size_t pos)
{
  assert (as != NULL);
  assert (pos <= as->len);
  if ((size_t) pos < as->len)
    {
      as->len = pos;
      as->text[pos] = '\0';
    }
  return as;
}


/*
 * Derived functions.
 */

const char *
astr_char (astr as, size_t pos)
{
  assert (pos <= astr_len (as));
  return astr_cstr (as) + pos;
}

static astr
astr_ncpy_cstr (astr as, const char *s, size_t len)
{
  astr_truncate (as, 0);
  return astr_ncat_cstr (as, s, len);
}

astr
astr_new_cstr (const char *s)
{
  return astr_cpy_cstr (astr_new (), s);
}

astr
astr_cpy (astr as, astr src)
{
  return astr_ncpy_cstr (as, astr_cstr (src), astr_len (src));
}

astr
astr_cpy_cstr (astr as, const char *s)
{
  return astr_ncpy_cstr (as, s, strlen (s));
}

astr
astr_cat (astr as, astr src)
{
  return astr_ncat_cstr (as, astr_cstr (src), astr_len (src));
}

astr
astr_cat_cstr (astr as, const char *s)
{
  return astr_ncat_cstr (as, s, strlen (s));
}

astr
astr_cat_char (astr as, int c)
{
  return astr_insert_char (as, astr_len (as), c);
}

astr
astr_substr (astr as, size_t pos, size_t size)
{
  assert (pos + size <= astr_len (as));
  return astr_ncat_cstr (astr_new (), astr_char (as, pos), size);
}

astr
astr_replace_cstr (astr as, size_t pos, size_t size, const char *s)
{
  assert (s != NULL);
  return astr_nreplace_cstr (as, pos, size, s, strlen (s));
}

astr
astr_insert_char (astr as, size_t pos, int c)
{
  char ch = (char) c;
  return astr_nreplace_cstr (as, pos, (size_t) 0, &ch, (size_t) 1);
}

astr
astr_remove (astr as, size_t pos, size_t size)
{
  return astr_nreplace_cstr (as, pos, size, "", (size_t) 0);
}

astr astr_fread (FILE * fp)
{
  int c;
  astr as = astr_new ();

  while ((c = getc (fp)) != EOF)
    astr_cat_char (as, c);

  return as;
}

astr
astr_vafmt (astr as, const char *fmt, va_list ap)
{
  char *buf;
  xvasprintf (&buf, fmt, ap);
  astr_cat_cstr (as, buf);
  free (buf);
  return as;
}

astr
astr_afmt (astr as, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  astr_vafmt (as, fmt, ap);
  va_end (ap);
  return as;
}

#ifdef TEST

#include <stdio.h>

static void
assert_eq (astr as, const char *s)
{
  if (strcmp (astr_cstr (as), s))
    printf ("test failed: \"%s\" != \"%s\"\n", as->text, s);
}

/*
 * Stubs to make xalloc_die happy.
 */
char *prog_name = "astr";

void
zile_exit (int doabort)
{
  exit (2);
}


int
main (void)
{
  astr as1, as2, as3;

  as1 = astr_new ();
  astr_cpy_cstr (as1, "hello world");
  astr_cat_char (as1, '!');
  assert_eq (as1, "hello world!");

  as3 = astr_substr (as1, 6, 5);
  assert_eq (as3, "world");

  as2 = astr_new ();
  astr_cpy_cstr (as2, "The ");
  astr_cat (as2, as3);
  astr_cat_char (as2, '.');
  assert_eq (as2, "The world.");

  astr_delete (as3);
  as3 = astr_substr (as1, astr_len (as1) - 6, 5);
  assert_eq (as3, "world");

  astr_cpy_cstr (as1, "12345");
  astr_delete (as2);

  astr_cpy_cstr (as1, "12345");
  astr_insert_char (as1, astr_len (as1) - 2, 'x');
  astr_insert_char (as1, astr_len (as1) - 6, 'y');
  astr_insert_char (as1, 7, 'z');
  assert_eq (as1, "y123x45z");

  astr_cpy_cstr (as1, "1234567");
  astr_replace_cstr (as1, astr_len (as1) - 4, 2, "foo");
  assert_eq (as1, "123foo67");

  astr_cpy_cstr (as1, "1234567");
  astr_replace_cstr (as1, 1, 3, "foo");
  assert_eq (as1, "1foo567");

  astr_cpy_cstr (as1, "1234567");
  astr_replace_cstr (as1, astr_len (as1) - 1, 5, "foo");
  assert_eq (as1, "123456foo");

  astr_cpy_cstr (as1, "1234567");
  astr_remove (as1, 4, 10);
  assert_eq (as1, "1234");

  astr_cpy_cstr (as1, "12345");
  as2 = astr_substr (as1, astr_len (as1) - 2, 2);
  assert_eq (as2, "45");

  astr_cpy_cstr (as1, "12345");
  astr_delete (as2);
  as2 = astr_substr (as1, astr_len (as1) - 5, 5);
  assert_eq (as2, "12345");

  astr_cpy_cstr (as1, "1234567");
  astr_replace_cstr (as1, astr_len (as1) - 4, 2, "foo");
  assert_eq (as1, "123foo67");

  astr_cpy_cstr (as1, "1234567");
  astr_replace_cstr (as1, 1, 3, "foo");
  assert_eq (as1, "1foo567");

  astr_cpy_cstr (as1, "1234567");
  astr_replace_cstr (as1, astr_len (as1) - 1, 5, "foo");
  assert_eq (as1, "123456foo");

  astr_cpy_cstr (as1, "");
  astr_afmt (as1, "%s * %d = ", "5", 3);
  astr_afmt (as1, "%d", 15);
  assert_eq (as1, "5 * 3 = 15");
  astr_delete (as1);

  astr_delete (as2);
  astr_delete (as3);
  printf ("astr test successful.\n");

  return 0;
}

#endif /* TEST */
