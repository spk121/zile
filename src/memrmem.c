/* memrmem

   Copyright (c) 2011-2012 Free Software Foundation, Inc.

   Based on strrstr.c:

     Copyright (c) 2004 Reuben Thomas.
     Copyright (c) Kent Irwin (kent.irwin@nist.gov).

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

#include <string.h>

#include "memrmem.h"


const char *
memrmem (const char *s, size_t slen, const char *t, size_t tlen)
{
  if (slen >= tlen)
    {
      size_t i = slen - tlen;
      do
        if (memcmp (s + i, t, tlen) == 0)
          return s + i;
      while (i-- != 0);
    }
  return NULL;
}
