/*	$Id: hash.h,v 1.1 2001/01/19 22:02:21 ssigala Exp $	*/

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

#ifndef _HASH_H_
#define _HASH_H_

#define HASH_TABLE_DEFAULT_SIZE		127

typedef unsigned long (*hashing_function_t)(const char *data, unsigned long table_size);

typedef struct hash_table_bucket *bucketp;

struct hash_table_bucket {
	char	*key;
	char	*data;
	bucketp	next;
};

typedef struct hash_table *htablep;

struct hash_table {
	unsigned long size;
	bucketp *table;
	hashing_function_t hashing_function;
};

extern htablep hash_table_build(unsigned long size);
extern htablep hash_table_build_default(void);
extern void hash_table_set_hashing_function(htablep table, hashing_function_t function);
extern int hash_table_free(htablep table);
extern int hash_table_store_key(htablep table, const char *key);
extern int hash_table_exists(htablep table, const char *key);
extern int hash_table_delete(htablep table, const char *key);
extern int hash_table_store_dataptr(htablep table, const char *key, const char *data);
extern int hash_table_store_data(htablep table, const char *key, const char *data);
extern int hash_table_store(htablep table, const char *key, const char *data);
extern char *hash_table_fetch(htablep table, const char *key);
extern void hash_table_dump(htablep table, FILE *fout);

#endif /* !_HASH_H_ */
