/* Useful editing functions
   Copyright (c) 2004 David A. Capello.  All rights reserved.

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

/*	$Id: editfns.h,v 1.2 2004/02/17 20:21:18 ssigala Exp $	*/

#ifndef EDITFNS_H
#define EDITFNS_H

/* Mark-ring.  */

void push_mark(void);
void pop_mark(void);
void set_mark(void);

/* What type of line.  */

int is_empty_line(void);
int is_blank_line(void);

/* Examining text near point.  */

int char_after(Point *pt);
int char_before(Point *pt);
int following_char(void);
int preceding_char(void);

int bobp(void);
int eobp(void);
int bolp(void);
int eolp(void);

#endif /* !EDITFNS_H */
