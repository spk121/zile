/* astr based getcwd()
   Copyright (c) 2003-2004 Reuben Thomas.  All rights reserved.

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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id: agetcwd.c,v 1.1 2004/11/15 00:47:12 rrt Exp $	*/

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "config.h"
#include "agetcwd.h"
#include "zile.h"
#include "extern.h"

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

astr agetcwd(astr as)
{
        size_t len = PATH_MAX;
        char *buf = (char *)zmalloc(len);
        char *res;
        while ((res = getcwd(buf, len)) == NULL && errno == ERANGE) {
                len *= 2;
                buf = zrealloc(buf, len);
        }
        /* If there was an error, return the empty string */
        if (res == NULL)
                *buf = '\0';
	astr_cpy_cstr(as, buf);
	free(buf);
        return as;
}
