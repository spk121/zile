/* Dynamically allocated encoded strings

   Copyright (c) 2011 Free Software Foundation, Inc.

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

#include <stdbool.h>
#include <string.h>
#include "size_max.h"

#include "astr.h"
#include "estr.h"
#include "memrmem.h"


/* Formats of end-of-line. */
const char *coding_eol_lf = "\n";
const char *coding_eol_crlf = "\r\n";
const char *coding_eol_cr = "\r";

/*
 * Determine EOL type from buffer contents.
 */
/* Maximum number of EOLs to check before deciding type. */
#define MAX_EOL_CHECK_COUNT 3
estr
estr_new_astr (astr as)
{
  bool first_eol = true;
  size_t total_eols = 0;
  estr es = (estr) {.as = as};
  for (size_t i = 0; i < astr_len (as) && total_eols < MAX_EOL_CHECK_COUNT; i++)
    {
      char c = astr_get (as, i);
      if (c == '\n' || c == '\r')
        {
          const char *this_eol_type;
          total_eols++;
          if (c == '\n')
            this_eol_type = coding_eol_lf;
          else if (i == astr_len (as) - 1 || astr_get (as, i + 1) != '\n')
            this_eol_type = coding_eol_cr;
          else
            {
              this_eol_type = coding_eol_crlf;
              i++;
            }

          if (first_eol)
            { /* This is the first end-of-line. */
              es.eol = this_eol_type;
              first_eol = false;
            }
          else if (es.eol != this_eol_type)
            { /* This EOL is different from the last; arbitrarily choose LF. */
              es.eol = coding_eol_lf;
              break;
            }
        }
    }
  return es;
}

estr
estr_dup (estr src)
{
  return (estr) {.as = astr_cpy (astr_new (), src.as), .eol = src.eol};
}

size_t
estr_prev_line (estr es, size_t o)
{
  size_t eol_len = strlen (es.eol);
  if (o < eol_len)
    return SIZE_MAX;
  const char *prev = memrmem (astr_cstr (es.as), o - eol_len, es.eol, eol_len);
  return prev ? prev - astr_cstr (es.as) + eol_len : 0;
}

size_t
estr_next_line (estr es, size_t o)
{
  size_t eol_len = strlen (es.eol);
  const char *next = memmem (astr_cstr (es.as) + o, astr_len (es.as) - o,
                             es.eol, eol_len);
  return next ? (next - astr_cstr (es.as) + eol_len) : SIZE_MAX;
}

estr
estr_cat (estr es, estr src)
{
  size_t len = astr_len (src.as), eol_len = strlen (src.eol);

  for (size_t o = 0; o < astr_len (src.as);)
    {
      size_t nexto = estr_next_line (src, o);
      astr_ncat_cstr (es.as, astr_cstr (src.as) + o,
                      ((nexto != SIZE_MAX) ? (size_t) (nexto - eol_len) : len) - o);
      o = nexto;
      if (o != SIZE_MAX)
        astr_cat_cstr (es.as, es.eol);
    }

  return es;
}
