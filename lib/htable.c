/* Hash table library
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004 Reuben Thomas.
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

/*	$Id: htable.c,v 1.5 2004/03/13 20:07:00 rrt Exp $	*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "config.h"
#include "htable.h"

typedef struct hbucket_s *hbucket;

struct hbucket_s {
	hpair	pair;
	hbucket	next;
};

struct htable_s {
	unsigned long size;
	hbucket *table;
};

/*
 * Hashing function.
 */
static unsigned long hash_func(const char *p, unsigned long size)
{
	unsigned long hash = 0, g;

	while (*p != '\0') {
		hash = (hash << 4) + *p++;
		if ((g = hash & 0xF0000000) != 0)
			hash ^= g >> 24;
		hash &= ~g;
	}

	return hash % size;
}

static hbucket build_bucket(const char *key)
{
	hbucket ptr = (hbucket)xmalloc(sizeof(*ptr));

	memset(ptr, 0, sizeof(*ptr));
	ptr->pair.key = xstrdup(key);

	return ptr;
}

static void free_bucket(hbucket ptr)
{
	if (ptr->pair.key != NULL)
		free(ptr->pair.key);

	free(ptr);
}

htable htable_new(unsigned long size)
{
	htable ptr = (htable)xmalloc(sizeof(*ptr));

	ptr->size = size;
	ptr->table = (hbucket *)xmalloc(ptr->size * sizeof(hbucket));
	memset(ptr->table, 0, ptr->size * sizeof(hbucket));

	return ptr;
}

void htable_delete(htable ht)
{
	hbucket bucket, next;
	unsigned i;

	for (i = 0; i < ht->size; i++) {
		bucket = ht->table[i];
		while (bucket != NULL) {
			next = bucket->next;
			free_bucket(bucket);
			bucket = next;
		}
	}

	free(ht->table);
	free(ht);
}

static int store_key(htable ht, const char *key)
{
	unsigned long hash;

	if (htable_fetch(ht, key) != NULL)
		return -1;

	hash = hash_func(key, ht->size);

	if (ht->table[hash] == NULL)
		ht->table[hash] = build_bucket(key);
	else {
		hbucket bucket = build_bucket(key);
		bucket->next = ht->table[hash];
		ht->table[hash] = bucket;
	}

	return 0;
}

static int store_value(htable ht, const char *key, void *val)
{
	hbucket bucket;
	unsigned long hash;

	hash = hash_func(key, ht->size);

	for (bucket = ht->table[hash]; bucket != NULL; bucket = bucket->next)
		if (!strcmp(bucket->pair.key, key)) {
			bucket->pair.val = val;
			return 0;
		}

	return -1;
}

int htable_store(htable ht, const char *key, void *val)
{
        int exists;

	if (htable_fetch(ht, key) != NULL)
		htable_remove(ht, key);

	exists = store_key(ht, key);
	assert(store_value(ht, key, val) == 0);

	return exists;
}

void *htable_fetch(htable ht, const char *key)
{
	hbucket bucket;
	unsigned long hash;

	hash = hash_func(key, ht->size);

	for (bucket = ht->table[hash]; bucket != NULL; bucket = bucket->next)
		if (!strcmp(bucket->pair.key, key))
			return bucket->pair.val;

	return NULL;
}

int htable_remove(htable ht, const char *key)
{
	hbucket bucket, last = NULL;
	unsigned long hash;

	hash = hash_func(key, ht->size);

	for (bucket = ht->table[hash]; bucket != NULL; bucket = bucket->next) {
		if (!strcmp(bucket->pair.key, key)) {
			if (bucket == ht->table[hash])
				ht->table[hash] = bucket->next;
			else
				last->next = bucket->next;
			free_bucket(bucket);
			return 0;
		}
		last = bucket;
	}

	return -1;
}

htable htable_foreach(htable ht, hiterator f, ...)
{
	hbucket bucket;
	unsigned i;
        va_list ap;

        va_start(ap, f);

	for (i = 0; i < ht->size; ++i)
		if ((bucket = ht->table[i]) != NULL)
			for (; bucket != NULL; bucket = bucket->next)
				f(&bucket->pair, ap);

        va_end(ap);
	return ht;
}

static void add_to_list(hpair *pair, va_list ap)
{
        alist_append(va_arg(ap, alist), pair);
}

alist htable_list(htable ht)
{
	alist al = alist_new();
        htable_foreach(ht, add_to_list, al);
	return al;
}

#ifdef TEST

#include <stdio.h>

int main(void)
{
	htable ht = htable_new(127);

	assert(htable_store(ht, "hello", NULL) == 0);
	assert(htable_store(ht, "world", NULL) == 0);
	assert(htable_store(ht, "bar", NULL) == 0);
	assert(htable_store(ht, "hello", NULL) != 0);
	assert(htable_fetch(ht, "hello") != NULL);
	assert(htable_fetch(ht, "world") != NULL);
	assert(htable_remove(ht, "world") == 0);
	assert(htable_fetch(ht, "world") == NULL);
	assert(htable_store(ht, "world", NULL) == 0);
	assert(htable_store(ht, "hello", "hello value") == 0);
	assert(htable_store(ht, "baz", "baz value") != 0);
	assert(htable_fetch(ht, "baz") == NULL);
	assert(htable_store(ht, "foo", "foo value") == 0);
	assert(htable_store(ht, "var1", NULL) == 0);
	assert(htable_store(ht, "var2", NULL) == 0);
	assert(htable_fetch(ht, "foo") != NULL &&
	       !strcmp(htable_fetch(ht, "foo"), "foo value"));
	assert(htable_fetch(ht, "hello") != NULL &&
	       !strcmp(htable_fetch(ht, "hello"), "hello value"));
	htable_delete(ht);
	printf("htable test successful.\n");

	return 0;
}

#endif /* TEST */
