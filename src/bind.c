/* Key bindings and extended commands
   Copyright (c) 1997-2004 Sandro Sigala.
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

/*	$Id: bind.c,v 1.11 2004/02/17 23:20:33 rrt Exp $	*/

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

static History functions_history;

/*--------------------------------------------------------------------------
 * Key binding.
 *--------------------------------------------------------------------------*/

typedef struct leaf *leafp;

struct leaf {
	/* The key and the function associated with the leaf. */
	int key;
	Function func;

	/* Leaf vector, number of items, max number of items. */
	leafp *vec;
	int vecnum, vecmax;
};

static leafp leaf_tree;

static leafp new_leaf(int vecmax)
{
	leafp p;

	p = (leafp)zmalloc(sizeof *p);
	memset(p, 0, sizeof *p);

	p->vecmax = vecmax;
	p->vec = (leafp *)zmalloc(sizeof *p * vecmax);

	return p;
}

#if 0
static int search_compar(const void *p1, const void *p2)
{
	return ((leafp)p2)->key - ((leafp)p1)->key;
}
#endif

static leafp search_leaf(leafp tree, int key)
{
#if 0
	struct leaf skey = { key };
	return bsearch(&skey, tree->vec, tree->vecnum, sizeof tree->vec[0], search_compar);
#else
	int i;

	for (i = 0; i < tree->vecnum; ++i)
		if (tree->vec[i]->key == key)
			return tree->vec[i];

	return NULL;
#endif
}

static void add_leaf(leafp tree, leafp p)
{
	int i;

	/*
	 * Reallocate vector if there is no enough space.
	 */
	if (tree->vecnum + 1 >= tree->vecmax) {
		tree->vecmax += 5;
		tree->vec = (leafp *)zrealloc(tree->vec, sizeof *p * tree->vecmax);
	}

	/*
	 * Insert the leaf at the sorted position.
	 */
	for (i = 0; i < tree->vecnum; i++)
		if (tree->vec[i]->key > p->key) {
			memmove(&tree->vec[i+1], &tree->vec[i], sizeof p * tree->vecnum - i);
			tree->vec[i] = p;
			break;
		}
	if (i == tree->vecnum)
		tree->vec[tree->vecnum] = p;
	++tree->vecnum;
}

static int bind_key0(leafp tree, int *keys, int n, Function func)
{
	leafp p, s;

	if ((s = search_leaf(tree, keys[0])) == NULL) {
		p = new_leaf(n == 1 ? 1 : 5);
		p->key = keys[0];
		add_leaf(tree, p);
		if (n == 1) {
			p->func = func;
			return TRUE;
		} else
			return bind_key0(p, &keys[1], n - 1, func);
	} else if (n > 1)
		return bind_key0(s, &keys[1], n - 1, func);
	else
		return FALSE;
}

void bind_key(char *key, Function func)
{
	int keys[64];
	int i;

	if ((i = keytovec(key, keys)) > 0) {
		if (leaf_tree == NULL)
			leaf_tree = new_leaf(10);
		if (!bind_key0(leaf_tree, keys, i, func)) {
			minibuf_error("Error: key %s already bound", key);
			waitkey(2 * 1000);
		}
	}
}

static leafp search_key0(leafp tree, int *keys, int n)
{
	leafp p;

	if ((p = search_leaf(tree, keys[0])) != NULL) {
		if (n == 1)
			return p;
		else
			return search_key0(p, &keys[1], n - 1);
	}

	return NULL;
}

#if 0
static leafp search_key(char *key)
{
	int keys[64], i;

	if ((i = keytovec(key, keys)) > 0)
		return search_key0(leaf_tree, keys, i);
	else
		return NULL;
}
#endif

#if DEBUG
static void prspaces(int i)
{
	int x;
	for (x = 0; x < i; x++)
		fputc(' ', stderr);
}

static void show_tree0(leafp tree)
{
	char buf[128];
	static int s;
	int i, l;

	prspaces(s);
	fprintf(stderr, "==> key %s\r\n", keytostr(buf, tree->key, &l));
	s += 4;
	for (i = 0; i < tree->vecnum; ++i) {
		if (tree->vec[i]->func != NULL) {
			prspaces(s);
			fprintf(stderr, "key %s\r\n",
				keytostr(buf, tree->vec[i]->key, &l));
		} else
			show_tree0(tree->vec[i]);
	}
	s -= 4;
	prspaces(s);
	fprintf(stderr, "<== key %s\r\n", keytostr(buf, tree->key, &l));
}

void show_tree(void)
{
	show_tree0(leaf_tree);
}
#endif /* DEBUG */

int do_completion(char *s, int *compl)
{
	int c;
	char buf[32];

	strcpy(buf, s);
	if (!*compl) {
		c = cur_tp->xgetkey(GETKEY_DELAYED, 500);
		if (c == KBD_NOKEY) {
			minibuf_write("%s", buf);
			c = cur_tp->getkey();
			*compl = 1;
		}
	} else {
		minibuf_write("%s", buf);
		c = cur_tp->getkey();
	}
	minibuf_clear();

	return c;
}

static char *make_completion(char *buf, int *keys, int numkeys)
{
	int i, l, len = 0;

	for (i = 0; i < numkeys; i++) {
		if (i > 0)
			buf[len] = ' ', len++;
		keytostr_nobs(buf + len, keys[i], &l);
		len += l - 1;
	}

	buf[len] = '-';
	buf[len+1] = '\0';

	return buf;
}

static leafp completion_scan(int c, int keys[], int *numkeys)
{
	int compl = 0;
	leafp p;
	keys[0] = c;
	*numkeys = 1;

	for (;;) {
		if ((p = search_key0(leaf_tree, keys, *numkeys)) == NULL)
			return NULL;
		if (p->func == NULL) {
			char buf[64];
			make_completion(buf, keys, *numkeys);
			keys[(*numkeys)++] = do_completion(buf, &compl);
			continue;
		} else
			return p;
	}
}

void process_key(int c)
{
	int uni, keys[64], numkeys;
	leafp p;

	if (c == KBD_NOKEY)
		return;

	if (c & KBD_META && isdigit(c & 255)) {
		/*
		 * Got a ESC x sequence where `x' is a digit.
		 */
		universal_argument(KBD_META, (c & 255) - '0');
	} else if ((p = completion_scan(c, keys, &numkeys)) == NULL) {
		/*
		 * There are no bindings for the pressed key.
		 */
		undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
		for (uni = 0; uni < last_uniarg; ++uni) {
			if (!self_insert_command(c)) {
				char buf[64];
				make_completion(buf, keys, numkeys);
				buf[strlen(buf)-1] = '\0';
				minibuf_error("%s not defined.", buf);
				undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
				return;
			}
			if (thisflag & FLAG_DEFINING_MACRO)
				add_kbd_macro(self_insert_command, FALSE, c);
		}
		undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
	} else {
		int oldflag = thisflag;
		p->func(last_uniarg);
		if ((oldflag & FLAG_DEFINING_MACRO)
		    && (thisflag & FLAG_DEFINING_MACRO)
		    && p->func != F_universal_argument)
			add_kbd_macro(p->func,
				      lastflag & FLAG_SET_UNIARG,
				      last_uniarg);
	}
}

/*--------------------------------------------------------------------------
 * Default functions binding.
 *--------------------------------------------------------------------------*/

struct fentry {
	/* The function name. */
	char *name;

	/* The function pointer. */
	Function func;

	/* The assigned keys. */
	char *key1;
	char *key2;
	char *key3;
};

typedef struct fentry *fentryp;

static struct fentry fentry_table[] = {
#define X0(zile_name, c_name) \
	{ zile_name, F_ ## c_name, NULL, NULL, NULL },
#define X1(zile_name, c_name, key1) \
	{ zile_name, F_ ## c_name, key1, NULL, NULL },
#define X2(zile_name, c_name, key1, key2) \
	{ zile_name, F_ ## c_name, key1, key2, NULL },
#define X3(zile_name, c_name, key1, key2, key3) \
	{ zile_name, F_ ## c_name, key1, key2, key3 },
#include "tbl_funcs.h"
#undef X0
#undef X1
#undef X2
#undef X3
};

#define fentry_table_size (sizeof(fentry_table) / sizeof fentry_table[0])

static int bind_compar(const void *p1, const void *p2)
{
	return strcmp(((fentryp)p1)->name, ((fentryp)p2)->name);
}

static int alternative_bindings = 0;

void init_bindings(void)
{
	unsigned int i;

	if (lookup_bool_variable("alternative-bindings")) {
		alternative_bindings = 1;
		for (i = 0; i < fentry_table_size; ++i) {
			if (fentry_table[i].key1 != NULL) {
				fentry_table[i].key1 = zstrdup(fentry_table[i].key1);
				replace_string(fentry_table[i].key1, "C-h", "M-h");
			}
			if (fentry_table[i].key2 != NULL) {
				fentry_table[i].key2 = zstrdup(fentry_table[i].key2);
				replace_string(fentry_table[i].key2, "C-h", "M-h");
			}
			if (fentry_table[i].key3 != NULL) {
				fentry_table[i].key3 = zstrdup(fentry_table[i].key3);
				replace_string(fentry_table[i].key3, "C-h", "M-h");
			}
		}
	}

	/*
	 * Sort the array for better searching later.
	 */
	qsort(fentry_table, fentry_table_size, sizeof fentry_table[0], bind_compar);

	/*
	 * Bind all the default functions.
	 */
	for (i = 0; i < fentry_table_size; i++) {
		if (fentry_table[i].key1 != NULL)
			bind_key(fentry_table[i].key1, fentry_table[i].func);
		if (fentry_table[i].key2 != NULL)
			bind_key(fentry_table[i].key2, fentry_table[i].func);
		if (fentry_table[i].key3 != NULL)
			bind_key(fentry_table[i].key3, fentry_table[i].func);
	}
}

static void recursive_free_bindings(leafp p)
{
	int i;
	for (i = 0; i < p->vecnum; ++i)
		recursive_free_bindings(p->vec[i]);
	free(p->vec);
	free(p);
}

void free_bindings(void)
{
	recursive_free_bindings(leaf_tree);

	if (alternative_bindings) {
		unsigned int i;
		for (i = 0; i < fentry_table_size; ++i) {
			if (fentry_table[i].key1 != NULL)
				free(fentry_table[i].key1);
			if (fentry_table[i].key2 != NULL)
				free(fentry_table[i].key2);
			if (fentry_table[i].key3 != NULL)
				free(fentry_table[i].key3);
		}
	}

	free_history_elements(&functions_history);
}

static struct fentry *bsearch_function(char *name)
{
	struct fentry key;
	key.name = name;
	return bsearch(&key, fentry_table, fentry_table_size, sizeof fentry_table[0], bind_compar);
}

char *minibuf_read_function_name(char *msg)
{
	unsigned int i;
	char *p, *ms;
	fentryp entryp;
	Completion *cp;

	cp = new_completion(FALSE);
	for (i = 0; i < fentry_table_size; ++i)
		alist_append(cp->completions, zstrdup(fentry_table[i].name));

	for (;;) {
		ms = minibuf_read_completion(msg, "", cp, &functions_history);

		if (ms == NULL) {
			free_completion(cp);
			cancel();
			return NULL;
		}

		if (ms[0] == '\0') {
			free_completion(cp);
			minibuf_error("No function name given");
			return NULL;
		} else {
                        astr as = astr_copy_cstr(ms);
			/* Complete partial words if possible. */
			if (cp->try(cp, as) == COMPLETION_MATCHED)
				ms = cp->match;
                        astr_delete(as);
			for (p = alist_first(cp->completions); p != NULL;
			     p = alist_next(cp->completions))
				if (!strcmp(ms, p)) {
					ms = p;
					break;
				}
			if ((entryp = bsearch_function(ms)) == NULL) {
				minibuf_error("Undefined function name `%s'", ms);
				waitkey(2 * 1000);
			} else {
				/* Add history element.  */
				add_history_element(&functions_history,
						    entryp->name);

				minibuf_clear();
				break;
			}
		}
	}

	free_completion(cp);

	return entryp->name;
}

int execute_function(char *name, int uniarg)
{
	unsigned int i;

	for (i = 0; i < fentry_table_size; ++i)
		if (!strcmp(name, fentry_table[i].name)) {
			fentry_table[i].func(uniarg);
			return TRUE;
		}

	return FALSE;
}

DEFUN("execute-extended-command", execute_extended_command)
/*+
Read function name, then read its arguments and call it.
+*/
{
	char msg[128], *name;

	if (lastflag & FLAG_SET_UNIARG && last_uniarg != 0)
		sprintf(msg, "%d M-x ", last_uniarg);
	else
		strcpy(msg, "M-x ");

	name = minibuf_read_function_name(msg);
	if (name == NULL)
		return FALSE;

	return execute_function(name, last_uniarg);
}

static char *get_function_name(Function p)
{
	unsigned int i;
	for (i = 0; i < fentry_table_size; ++i)
		if (fentry_table[i].func == p)
			return fentry_table[i].name;
	return NULL;
}

char *get_function_by_key_sequence(void)
{
	leafp p;
	int c = cur_tp->getkey();
	int keys[64], numkeys;

	if (c & KBD_META && isdigit(c & 255))
		return "universal-argument";
	else if ((p = completion_scan(c, keys, &numkeys)) == NULL) {
		if (c == KBD_RET || c == KBD_TAB || c <= 255)
			return "self-insert-command";
		else
			return NULL;
	} else
		return get_function_name(p->func);

	return NULL;
}

static void write_functions_list(va_list ap)
{
	unsigned int i;

	bprintf("%-30s%s\n", "Function", "Bindings");
	bprintf("%-30s%s\n", "--------", "--------");
	for (i = 0; i < fentry_table_size; ++i) {
		char key1[64], key2[64], key3[64];
		simplify_key(key1, fentry_table[i].key1);
		simplify_key(key2, fentry_table[i].key2);
		simplify_key(key3, fentry_table[i].key3);
		bprintf("%-30s", fentry_table[i].name);
		if (key1[0] != '\0')
			bprintf("%s", key1);
		if (key2[0] != '\0')
			bprintf(", %s", key2);
		if (key3[0] != '\0')
			bprintf(", %s", key3);
		bprintf("\n");
	}
}

DEFUN("list-functions", list_functions)
/*+
List defined functions.
+*/
{
	write_temp_buffer("*Functions List*", write_functions_list);
	return TRUE;
}

static void write_bindings_tree(leafp tree, int level)
{
	char key[128], buf[128];
	static char keys[8][128];
	int i, j, l;

	keytostr_nobs(keys[level], tree->key, &l);
	for (i = 0; i < tree->vecnum; ++i) {
		leafp p = tree->vec[i];
		if (p->func != NULL) {
			key[0] = '\0';
			for (j = 1; j <= level; ++j) {
				strcat(key, keys[j]);
				strcat(key, " ");
			}
			keytostr_nobs(buf, p->key, &l);
			strcat(key, buf);
			bprintf("%-15s %s\n", key, get_function_name(p->func));
		} else
			write_bindings_tree(p, level + 1);
	}
}

static void write_bindings_list(va_list ap)
{
	bprintf("%-15s %s\n", "Binding", "Function");
	bprintf("%-15s %s\n", "-------", "--------");

	write_bindings_tree(leaf_tree, 0);
}

DEFUN("list-bindings", list_bindings)
/*+
List defined bindings.
+*/
{
	write_temp_buffer("*Bindings List*", write_bindings_list);
	return TRUE;
}
