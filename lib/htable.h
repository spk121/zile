/* Hash table library
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

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

/*	$Id: htable.h,v 1.3 2004/02/17 20:21:18 ssigala Exp $	*/

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
