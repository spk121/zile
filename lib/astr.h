/* Dynamically allocated strings
   Copyright (c) 2001-2004 Sandro Sigala.
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

/*	$Id: astr.h,v 1.13 2004/03/13 16:31:20 rrt Exp $	*/

#ifndef ASTR_H
#define ASTR_H

#include <stdio.h>
#include <stdarg.h>

/*
 * The astr library provides dynamically allocated null-terminated C
 * strings.
 *
 * The string type, astr, is a pointer type.
 *
 * String positions start at zero, as with ordinary C strings.
 * Negative values are also allowed, and count from the end of the
 * string. In particular, -1 refers to the last character of the
 * string.
 */
typedef struct astr_s *astr;

/*
 * Allocate a new string with zero length.
 */
extern astr   astr_new(void);

/*
 * Deallocate the previously allocated string as.
 */
extern void   astr_delete(astr as);

/*
 * Convert as into a C null-terminated string.
 * as[0] to as[astr_size(as) - 1] inclusive may be read and modified.
 */
#define astr_cstr(as)		((const char *)((as)->text))

/*
 * Return the length of the argument string as.
 */
#define astr_size(as)		((const int)((as)->size))

/*
 * Return the address of the pos'th character of as. If pos is >= 0,
 * than 0, count from the left; if less than zero count from the
 * right.
 */
extern char * astr_char(const astr as, int pos);

/*
 * Return a new astr consisting of size characters from string as
 * starting from position pos.
 */
extern astr   astr_substr(const astr as, int pos, size_t size);

/*
 * Do strcmp on the contents of s1 and s2
 */
#define astr_cmp(s1, s2)	(strcmp(s1->text, s2->text))

/*
 * Assign the contents of the argument string or to the string as.
 */
extern astr   astr_assign(astr as, const astr src);
extern astr   astr_assign_cstr(astr as, const char *s);

/* Append the contents of the argument string or character at the end
 * of the string as.
 */
extern astr   astr_append(astr as, const astr src);
extern astr   astr_append_cstr(astr as, const char *s);
extern astr   astr_append_char(astr as, int c);

/*
 * Truncate as to given length.
 */
extern astr   astr_truncate(astr as, size_t size);

/* .Pp */
/* The */
/* .Fn astr_find */
/* and */
/* .Fn astr_find_cstr */
/* functions find the first occurrence of the argument string or character */
/* into the argument */
/* .Fa as , */
/* returning the position starting from the beginning of the string. */
/* .Pp */
/* The */
/* .Fn astr_rfind */
/* and */
/* .Fn astr_rfind_cstr */
/* functions find the first occurrence of the argument string or character */
/* into the argument */
/* .Fa as , */
/* returning the position starting from the end of the string. */
/* .Pp */
/* The */
/* .Fn astr_replace , */
/* and */
/* .Fn astr_replace_cstr */
/* functions replace up to */
/* .Fa size */
/* characters of the argument string */
/* .Fa as , */
/* starting from the position */
/* .Fa pos , */
/* with the argument string or character. */
extern int    astr_find(const astr as, const astr src);
extern int    astr_find_cstr(const astr as, const char *s);
extern int    astr_rfind(const astr as, const astr src);
extern int    astr_rfind_cstr(const astr as, const char *s);
extern astr   astr_replace(astr as, int pos, size_t size, const astr src);
extern astr   astr_replace_cstr(astr as, int pos, size_t size, const char *s);

/*
 * Read a string from the stream f and return it. The trailing newline
 * is removed from the string. If the stream is at eof when astr_fgets
 * is called, it returns NULL.
 */
extern astr   astr_fgets(FILE *f);

/*
 * Append formatted text to the argument string
 */
extern astr   astr_vafmt(astr as, const char *fmt, va_list ap);
extern astr   astr_afmt(astr as, const char *fmt, ...);

/*
 * Internal data structures
 *
 * You should never access directly to the following data.
 */

struct astr_s {
	char *	text;
	size_t	size;
	size_t	maxsize;
};

#endif /* !ASTR_H */
