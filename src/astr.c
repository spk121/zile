/* Dynamically allocated strings

   Copyright (c) 2001-2005, 2008-2011 Free Software Foundation, Inc.

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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "xalloc.h"
#include "xvasprintf.h"
#include "minmax.h"

#include "astr.h"

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

castr
castr_new_nstr (const char *s, size_t n)
{
  astr as;
  as = (astr) XZALLOC (struct astr);
  as->len = n;
  as->text = s;
  return as;
}

const char *
astr_cstr (castr as)
{
  return (const char *) (as->text);
}

size_t
astr_len (castr as)
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
      as->text = xrealloc (as->text, as->maxlen + 1);
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
  tail = astr_substr (as, pos + (size_t) size,
                      astr_len (as) - (pos + size));
  as->len = pos;
  as->text[pos] = '\0';
  astr_ncat_cstr (as, s, csize);
  astr_cat (as, tail);

  return as;
}


/*
 * Derived functions.
 */

char
astr_get (castr as, size_t pos)
{
  assert (pos <= astr_len (as));
  return (astr_cstr (as))[pos];
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
astr_cpy (astr as, castr src)
{
  return astr_ncpy_cstr (as, astr_cstr (src), astr_len (src));
}

astr
astr_cpy_cstr (astr as, const char *s)
{
  return astr_ncpy_cstr (as, s, strlen (s));
}

astr
astr_cat (astr as, castr src)
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
astr_substr (castr as, size_t pos, size_t size)
{
  assert (pos + size <= astr_len (as));
  return astr_ncat_cstr (astr_new (), astr_cstr (as) + pos, size);
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

astr
astr_truncate (astr as, size_t pos)
{
  return astr_remove (as, pos, astr_len (as) - pos);
}

astr
astr_readf (const char *filename)
{
  astr as = NULL;
  struct stat st;
  if (stat (filename, &st) == 0)
    {
      size_t size = st.st_size;
      int fd = open (filename, O_RDONLY);
      if (fd >= 0)
        {
          char buf[BUFSIZ];
          as = astr_new ();
          while ((size = read (fd, buf, BUFSIZ)) > 0)
            astr_ncat_cstr (as, buf, size);
          close (fd);
        }
    }
  return as;
}

astr
astr_vfmt (const char *fmt, va_list ap)
{
  return astr_new_cstr (xvasprintf (fmt, ap));
}

astr
astr_fmt (const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  astr as = astr_vfmt (fmt, ap);
  va_end (ap);
  return as;
}

/*
 * Recase str according to newcase.
 */
astr
astr_recase (astr as, enum casing newcase)
{
  astr bs = astr_new ();

  if (newcase == case_capitalized || newcase == case_upper)
    astr_cat_char (bs, toupper (astr_get (as, 0)));
  else
    astr_cat_char (bs, tolower (astr_get (as, 0)));

  for (size_t i = 1, len = astr_len (as); i < len; i++)
    astr_cat_char (bs, ((newcase == case_upper) ? toupper : tolower) (astr_get (as, i)));

  astr_cpy (as, bs);

  return as;
}


#ifdef TEST

#include <config.h>

#include <stdio.h>
#include "progname.h"

#include "main.h"

static void
assert_eq (astr as, const char *s)
{
  if (STRNEQ (astr_cstr (as), s))
    {
      printf ("test failed: \"%s\" != \"%s\"\n", astr_cstr (as), s);
      exit (EXIT_FAILURE);
    }
}

/* Stub to make xalloc_die happy. */
void zile_exit (int doabort) _GL_ATTRIBUTE_NORETURN;

void
zile_exit (int doabort _GL_UNUSED_PARAMETER)
{
  exit (EXIT_CRASH);
}


int
main (int argc _GL_UNUSED_PARAMETER, char **argv)
{
  astr as1, as2, as3;

  set_program_name (argv[0]);

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

  as3 = astr_substr (as1, astr_len (as1) - 6, 5);
  assert_eq (as3, "world");

  astr_cpy_cstr (as1, "12345");

  astr_cpy_cstr (as1, "12345");
  astr_insert_char (as1, astr_len (as1) - 2, 'x');
  astr_insert_char (as1, astr_len (as1) - 6, 'y');
  astr_insert_char (as1, 7, 'z');
  assert_eq (as1, "y123x45z");

  astr_cpy_cstr (as1, "1234567");
  astr_nreplace_cstr (as1, astr_len (as1) - 4, 2, "foo", 3);
  assert_eq (as1, "123foo67");

  astr_cpy_cstr (as1, "1234567");
  astr_nreplace_cstr (as1, 1, 3, "foo", 3);
  assert_eq (as1, "1foo567");

  astr_cpy_cstr (as1, "1234567");
  astr_nreplace_cstr (as1, astr_len (as1) - 1, 5, "foo", 3);
  assert_eq (as1, "123456foo");

  astr_cpy_cstr (as1, "1234567");
  astr_remove (as1, 4, 10);
  assert_eq (as1, "1234");

  astr_cpy_cstr (as1, "12345");
  as2 = astr_substr (as1, astr_len (as1) - 2, 2);
  assert_eq (as2, "45");

  astr_cpy_cstr (as1, "12345");
  as2 = astr_substr (as1, astr_len (as1) - 5, 5);
  assert_eq (as2, "12345");

  astr_cpy_cstr (as1, "1234567");
  astr_nreplace_cstr (as1, astr_len (as1) - 4, 2, "foo", 3);
  assert_eq (as1, "123foo67");

  astr_cpy_cstr (as1, "1234567");
  astr_nreplace_cstr (as1, 1, 3, "foo", 3);
  assert_eq (as1, "1foo567");

  astr_cpy_cstr (as1, "1234567");
  astr_nreplace_cstr (as1, astr_len (as1) - 1, 5, "foo", 3);
  assert_eq (as1, "123456foo");

  as1 = astr_fmt ("%s * %d = ", "5", 3);
  astr_cat (as1, astr_fmt ("%d", 15));
  assert_eq (as1, "5 * 3 = 15");

  astr_cpy_cstr (as1, "some text");
  astr_recase (as1, case_capitalized);
  assert_eq (as1, "Some text");
  astr_recase (as1, case_upper);
  assert_eq (as1, "SOME TEXT");
  astr_recase (as1, case_lower);
  assert_eq (as1, "some text");

  printf ("astr test successful.\n");

  return EXIT_SUCCESS;
}

#endif /* TEST */
