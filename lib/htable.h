/*	$Id: htable.h,v 1.1 2003/04/24 15:11:59 rrt Exp $	*/

/*
 * Copyright (c) 1997-2001 Sandro Sigala.  All rights reserved.
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

#ifndef HTABLE_H
#define HTABLE_H

#include <stdio.h>

#include "alist.h"

typedef unsigned long (*hfunc_t)(const char *data, unsigned long table_size);

typedef struct hpair_s {
	char *key;
	void *data;
} hpair;

typedef struct htable_s *htable;

extern htable htable_new(void);
extern htable htable_new_custom(unsigned long size);
extern void   htable_delete(htable ht);
extern void   htable_set_hash_func(htable ht, hfunc_t hfunc);
extern int    htable_store_key(htable ht, const char *key);
extern int    htable_store_data(htable ht, const char *key, void *data);
extern int    htable_store(htable ht, const char *key, void *data);
extern int    htable_exists(htable ht, const char *key);
extern void * htable_fetch(htable ht, const char *key);
extern int    htable_remove(htable ht, const char *key);
extern void   htable_dump(htable ht, FILE *fout);
extern alist  htable_list(htable ht);

#endif /* !HTABLE_H */
