/* Zile configuration file reader
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id: rc.c,v 1.21 2005/01/22 12:32:04 rrt Exp $	*/

#include "config.h"

#include "zile.h"
#include "extern.h"

void read_rc_file(void)
{
  astr buf = get_home_dir();
  astr_cat_cstr(buf, "/.zile");
  lisp_read_file(astr_cstr(buf));
  astr_delete(buf);
}
