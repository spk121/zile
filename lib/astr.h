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

/*	$Id: astr.h,v 1.10 2004/03/10 15:15:46 rrt Exp $	*/

#ifndef ASTR_H
#define ASTR_H

#include <stdio.h>
#include <stdarg.h>

typedef struct astr_s *astr;
typedef const struct astr_s *castr;

extern astr   astr_new(void);
extern void   astr_resize(astr as, size_t reqsize);
extern astr   astr_copy(castr as);
extern astr   astr_copy_cstr(const char *s);
extern void   astr_delete(astr as);
extern void   astr_clear(astr as);
extern const char * astr_cstr(castr as);
extern size_t astr_size(castr as);
extern int    astr_cmp(castr s1, castr s2);
extern astr   astr_assign(astr as, castr src);
extern astr   astr_assign_cstr(astr as, const char *s);
extern astr   astr_insert(astr as, int pos, castr src);
extern astr   astr_insert_cstr(astr as, int pos, const char *s);
extern astr   astr_append(astr as, castr src);
extern astr   astr_append_cstr(astr as, const char *s);
extern astr   astr_append_char(astr as, int c);
extern astr   astr_remove(astr as, int pos, size_t size);
extern astr   astr_truncate(astr as, size_t size);
extern astr   astr_substr(castr as, int pos, size_t size);
extern char   astr_last_char(castr as);
extern int    astr_find(castr as, castr src);
extern int    astr_find_cstr(castr as, const char *s);
extern int    astr_rfind(castr as, castr src);
extern int    astr_rfind_cstr(castr as, const char *s);
extern astr   astr_replace(astr as, int pos, size_t size, castr src);
extern astr   astr_replace_cstr(astr as, int pos, size_t size, const char *s);
extern astr   astr_fgets(astr as, FILE *f);
extern astr   astr_vfmt(astr as, const char *fmt, va_list ap);
extern astr   astr_vafmt(astr as, const char *fmt, va_list ap);
extern astr   astr_fmt(astr as, const char *fmt, ...);
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

#ifndef ASTR_NO_MACRO_DEFS

/*
 * Fast macro definitions.
 */

#define astr_cstr(as)		((const char *)as->text)
#define astr_size(as)		((const int)as->size)
#define astr_last_char(as)	(as->text[as->size - 1])
#define astr_cmp(s1, s2)	(strcmp(s1->text, s2->text))

#endif /* !ASTR_NO_MACRO_DEFS */

#endif /* !ASTR_H */
