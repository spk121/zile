/*	$Id: htable.c,v 1.1 2003/04/24 15:11:59 rrt Exp $	*/

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

#ifdef TEST
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "htable.h"

#define HTABLE_DEFAULT_SIZE		127

typedef struct hbucket_s *hbucket;

struct hbucket_s {
	hpair	pair;
	hbucket	next;
};

struct htable_s {
	unsigned long size;
	hbucket *table;
	hfunc_t hfunc;
};

/*
 * Default hashing function.
 */
static unsigned long htable_hash(const char *p, unsigned long size)
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
	hbucket ptr;

	ptr = (hbucket)xmalloc(sizeof(*ptr));
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

htable htable_new(void)
{
	return htable_new_custom(HTABLE_DEFAULT_SIZE);
}

htable htable_new_custom(unsigned long size)
{
	htable ptr;

	ptr = (htable)xmalloc(sizeof(*ptr));
	ptr->size = size;
	ptr->table = (hbucket *)xmalloc(ptr->size * sizeof(hbucket));
	memset(ptr->table, 0, ptr->size * sizeof(hbucket));
	ptr->hfunc = htable_hash;

	return ptr;
}

void htable_delete(htable ht)
{
	hbucket bucket, next;
	unsigned int i;

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

void htable_set_hash_func(htable ht, hfunc_t hfunc)
{
	ht->hfunc = hfunc;
}

int htable_store_key(htable ht, const char *key)
{
	unsigned long hash;

	if (htable_exists(ht, key))
		return -1;

	hash = ht->hfunc(key, ht->size);

	if (ht->table[hash] == NULL)
		ht->table[hash] = build_bucket(key);
	else {
		hbucket bucket = build_bucket(key);
		bucket->next = ht->table[hash];
		ht->table[hash] = bucket;
	}

	return 0;
}

int htable_store_data(htable ht, const char *key, void *data)
{
	hbucket bucket;
	unsigned long hash;

	hash = ht->hfunc(key, ht->size);

	for (bucket = ht->table[hash]; bucket != NULL; bucket = bucket->next)
		if (!strcmp(bucket->pair.key, key)) {
			bucket->pair.data = data;
			return 0;
		}

	return -1;
}

int htable_store(htable ht, const char *key, void *data)
{
	if (htable_exists(ht, key))
		htable_remove(ht, key);

	htable_store_key(ht, key);
	htable_store_data(ht, key, data);

	return 0;
}

int htable_exists(htable ht, const char *key)
{
	hbucket bucket;
	unsigned long hash;

	hash = ht->hfunc(key, ht->size);

	for (bucket = ht->table[hash]; bucket != NULL; bucket = bucket->next)
		if (!strcmp(bucket->pair.key, key))
			return 1;

	return 0;
}

void *htable_fetch(htable ht, const char *key)
{
	hbucket bucket;
	unsigned long hash;

	hash = ht->hfunc(key, ht->size);

	for (bucket = ht->table[hash]; bucket != NULL; bucket = bucket->next)
		if (!strcmp(bucket->pair.key, key))
			return bucket->pair.data;

	return NULL;
}

int htable_remove(htable ht, const char *key)
{
	hbucket bucket, last = NULL;
	unsigned long hash;

	hash = ht->hfunc(key, ht->size);

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

void htable_dump(htable ht, FILE *fout)
{
	hbucket bucket;
	unsigned int i;
	int nelements = 0, dups = 0;

	fprintf(fout, "Hash table dump:\n");
	for (i = 0; i < ht->size; ++i) {
		bucket = ht->table[i];
		if (bucket != NULL) {
			fprintf(fout, "table[%d] = {\n", i);
			for (; bucket != NULL; bucket = bucket->next) {
				++nelements;
				if (bucket != ht->table[i])
					++dups;
				fprintf(fout, "\ttable[\"%s\"] = %p\n",
					bucket->pair.key, bucket->pair.data);
			}
			fprintf(fout, "}\n");
		}
	}
	fprintf(fout, "Table size: %ld\n", ht->size);
	fprintf(fout, "Number of elements: %d\n", nelements);
	fprintf(fout, "Duplicated hash values: %d\n", dups);
	fprintf(fout, "Performance: %.2f%%\n", (1-(float)dups/nelements)*100);
}

alist htable_list(htable ht)
{
	alist al;
	hbucket bucket;
	unsigned int i;

	al = alist_new();
	for (i = 0; i < ht->size; ++i)
		if ((bucket = ht->table[i]) != NULL)
			for (; bucket != NULL; bucket = bucket->next)
				alist_append(al, &bucket->pair);
	return al;
}

#ifdef TEST

int main(void)
{
	htable ht = htable_new();

	assert(htable_store_key(ht, "hello") == 0);
	assert(htable_store_key(ht, "world") == 0);
	assert(htable_store_key(ht, "bar") == 0);
	assert(htable_store_key(ht, "hello") != 0);
	assert(htable_exists(ht, "hello"));
	assert(htable_exists(ht, "world"));
	assert(htable_remove(ht, "world") == 0);
	assert(!htable_exists(ht, "world"));
	assert(htable_store_key(ht, "world") == 0);
	assert(htable_store_data(ht, "hello", "hello data") == 0);
	assert(htable_store_data(ht, "baz", "baz data") != 0);
	assert(!htable_exists(ht, "baz"));
	assert(htable_store(ht, "foo", "foo data") == 0);
	assert(htable_store_key(ht, "var1") == 0);
	assert(htable_store_key(ht, "var2") == 0);
	assert(htable_fetch(ht, "foo") != NULL && 
	       !strcmp(htable_fetch(ht, "foo"), "foo data"));
	assert(htable_fetch(ht, "hello") != NULL && 
	       !strcmp(htable_fetch(ht, "hello"), "hello data"));
	htable_dump(ht, stdout);
	htable_delete(ht);
	printf("htable test successful.\n");

	return 0;
}

#endif /* TEST */
