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

/*	$Id: astr.h,v 1.7 2004/02/17 23:20:33 rrt Exp $	*/

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
extern size_t astr_maxsize(castr as);
extern int    astr_isempty(castr as);
extern astr   astr_fill(astr as, int c, size_t size);
extern int    astr_cmp(castr s1, castr s2);
extern int    astr_cmp_cstr(castr s1, const char *s2);
extern int    astr_eq(castr s1, castr s2);
extern int    astr_eq_cstr(castr as, const char *s);
extern astr   astr_assign(astr as, castr src);
extern astr   astr_assign_cstr(astr as, const char *s);
extern astr   astr_assign_char(astr as, int c);
extern astr   astr_insert(astr as, int pos, castr src);
extern astr   astr_insert_cstr(astr as, int pos, const char *s);
extern astr   astr_insert_char(astr as, int pos, int c);
extern astr   astr_prepend(astr as, castr src);
extern astr   astr_prepend_cstr(astr as, const char *s);
extern astr   astr_prepend_char(astr as, int c);
extern astr   astr_append(astr as, castr src);
extern astr   astr_append_cstr(astr as, const char *s);
extern astr   astr_append_char(astr as, int c);
extern astr   astr_remove(astr as, int pos, size_t size);
extern astr   astr_truncate(astr as, size_t size);
extern astr   astr_substr(castr as, int pos, size_t size);
extern astr   astr_left(castr as, size_t size);
extern astr   astr_right(castr as, size_t size);
extern char   astr_first_char(castr as);
extern char   astr_last_char(castr as);
extern int    astr_find(castr as, castr src);
extern int    astr_find_cstr(castr as, const char *s);
extern int    astr_find_char(castr as, int c);
extern int    astr_rfind(castr as, castr src);
extern int    astr_rfind_cstr(castr as, const char *s);
extern int    astr_rfind_char(castr as, int c);
extern astr   astr_replace(astr as, int pos, size_t size, castr src);
extern astr   astr_replace_cstr(astr as, int pos, size_t size, const char *s);
extern astr   astr_replace_char(astr as, int pos, size_t size, int c);
extern astr   astr_fgets(astr as, FILE *f);
extern void   astr_fputs(castr as, FILE *f);
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
#define astr_maxsize(as)	((const int)as->maxsize)
#define astr_isempty(as)	(as->size == 0)
#define astr_first_char(as)	(as->text[0])
#define astr_last_char(as)	(as->text[as->size - 1])
#define astr_cmp(s1, s2)	(strcmp(s1->text, s2->text))
#define astr_cmp_cstr(s1, s2)	(strcmp(s1->text, s2))
#define astr_eq(s1, s2)		(!strcmp(s1->text, s2->text))
#define astr_eq_cstr(s1, s2)	(!strcmp(s1->text, s2))

#endif /* !ASTR_NO_MACRO_DEFS */

#endif /* !ASTR_H */
