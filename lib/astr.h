/*	$Id: astr.h,v 1.1 2003/04/24 15:11:59 rrt Exp $	*/

/*
 * Copyright (c) 2001 Sandro Sigala.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ASTR_H
#define ASTR_H

#include <stdio.h>

typedef struct astr_s *astr;
typedef const struct astr_s *castr;

extern astr   astr_new(void);
extern astr   astr_copy(castr as);
extern astr   astr_copy_cstr(const char *s);
extern void   astr_delete(astr as);
extern void   astr_clear(astr as);
extern char * astr_cstr(castr as);
extern size_t astr_size(castr as);
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
#define astr_cmp(s1, s2)	(strcmp(s1->text, s2->text))
#define astr_cmp_cstr(s1, s2)	(strcmp(s1->text, s2))
#define astr_eq(s1, s2)		(!strcmp(s1->text, s2->text))
#define astr_eq_cstr(s1, s2)	(!strcmp(s1->text, s2))

#endif /* !ASTR_NO_MACRO_DEFS */

#endif /* !ASTR_H */
