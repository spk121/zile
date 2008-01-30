/* strrstr implementation
   Copyright (c) 2004, 2007, 2008 Reuben Thomas.  All rights reserved.

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

#ifndef HAVE_STRRSTR

#include <stddef.h>
#include <string.h>

char *strrstr(const char *s, const char *t);

char *strrstr(const char *s, const char *t)
{
  size_t i, j, tlen = strlen(t);

  for (i = strlen(s); i >= tlen; i--) {
    for (j = 0; j < tlen && s[i - tlen + j] == t[j]; j++)
      ;
    if (j == tlen)
      return (char *)(s + i - tlen);
  }

  return NULL;
}

#endif
