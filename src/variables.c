/*	$Id: variables.c,v 1.1 2001/01/19 22:02:55 ssigala Exp $	*/

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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "zile.h"
#include "extern.h"
#include "hash.h"

/*
 * Default variables values table.
 */
static struct var_entry { char *var; char *fmt; char *val; } def_vars[] = {
#define X(zile_var, fmt, val, doc) { zile_var, fmt, val },
#include "tbl_vars.h"
#undef X
};

static htablep var_table;

void init_variables(void)
{
	struct var_entry *p;

	var_table = hash_table_build_default();

	for (p = &def_vars[0]; p < &def_vars[sizeof(def_vars) / sizeof(def_vars[0])]; p++)
		set_variable(p->var, p->val);
}

void free_variables(void)
{
	hash_table_free(var_table);
}

void set_variable(char *var, char *val)
{
	hash_table_store(var_table, var, xstrdup(val));

	/* Force refresh of cached variables. */
	cur_tp->refresh_cached_variables();
}

void unset_variable(char *var)
{
	hash_table_delete(var_table, var);
}

char *get_variable(char *var)
{
	return hash_table_fetch(var_table, var);
}

int is_variable_equal(char *var, char *val)
{
	char *v = hash_table_fetch(var_table, var);
	return v != NULL && !strcmp(v, val);
}

int lookup_bool_variable(char *var)
{
	char *p;

	if ((p = hash_table_fetch(var_table, var)) != NULL)
		return !strcmp(p, "true");

#if 0
	minibuf_error("%FCWarning: used uninitialized variable %FY`%s'%E", var);
	waitkey(2 * 1000);
#endif

	return FALSE;
}

static historyp make_variable_history(void)
{
	unsigned int i, n;
	bucketp bucket;
	historyp hp;

	for (i = n = 0; i < var_table->size; ++i)
		for (bucket = var_table->table[i]; bucket != NULL; bucket = bucket->next)
			++n;
	hp = new_history(n, FALSE);
	for (i = n = 0; i < var_table->size; ++i)
		for (bucket = var_table->table[i]; bucket != NULL; bucket = bucket->next)
			hp->completions[n++] = xstrdup(bucket->key);

	return hp;
}

char *minibuf_read_variable_name(char *msg)
{
	char *ms;
	historyp hp;

	hp = make_variable_history();

	for (;;) {
		ms = minibuf_read_history(msg, "", hp);

		if (ms == NULL) {
			free_history(hp);
			cancel();
			return NULL;
		}

		if (ms[0] == '\0') {
			free_history(hp);
			minibuf_error("%FCNo variable name given%E");
			return NULL;
		} else if (get_variable(ms) == NULL) {
			minibuf_error("%FCUndefined variable name%E");
			waitkey(2 * 1000);
		} else {
			minibuf_clear();
			break;
		}
	}

	free_history(hp);

	return ms;
}

static char *get_variable_format(char *var)
{
	struct var_entry *p;
	for (p = &def_vars[0]; p < &def_vars[sizeof(def_vars) / sizeof(def_vars[0])]; p++)
		if (!strcmp(p->var, var))
			return p->fmt;

	return NULL;
}

DEFUN("set-variable", set_variable)
/*+
Set a variable value to the user specified value.
+*/
{
	char buf[128], *var, *val, *fmt;

	var = minibuf_read_variable_name("Set variable: ");
	if (var == NULL)
		return FALSE;

	/* `tab-width' and `fill-column' are local to buffers. */
	if (!strcmp(var, "tab-width"))
		sprintf(buf, "%d", cur_bp->tab_width);
	else if (!strcmp(var, "fill-column"))
		sprintf(buf, "%d", cur_bp->fill_column);
	else
		strcpy(buf, get_variable(var));
	fmt = get_variable_format(var);
	if (!strcmp(fmt, "c")) {
		if ((val = minibuf_read_color("Set %s to value: ", var)) == NULL)
			return cancel();
	} else if (!strcmp(fmt, "b")) {
		int i;
		if ((i = minibuf_read_boolean("Set %s to value: ", var)) == -1)
			return cancel();
		val = (i == TRUE) ? "true" : "false";
	} else { /* Non color, boolean or such fixed value varabile. */
		if ((val = minibuf_read("Set %s to value: ", buf, var)) == NULL)
			return cancel();
	}

	/*
	 * The `tab-width' and `fill-column' variables are local
	 * to the buffer (automatically becomes buffer-local when
	 * set in any fashion).
	 */
	if (!strcmp(var, "tab-width")) {
		int i = atoi(val);
		if (i < 1) {
			minibuf_error("%FCInvalid tab-width value %FY`%s'%E", val);
			waitkey(2 * 1000);
		} else
			cur_bp->tab_width = i;

	} else if (!strcmp(var, "fill-column")) {
		int i = atoi(val);
		if (i < 2) {
			minibuf_error("%FCInvalid fill-column value %FY`%s'%E", val);
			waitkey(2 * 1000);
		} else
			cur_bp->fill_column = i;
	} else
		set_variable(var, val);

	return TRUE;
}

static int sort_func(const void *ptr1, const void *ptr2)
{
	return strcmp((*(bucketp *)ptr1)->key, (*(bucketp *)ptr2)->key);
}

static void write_variables_list(void *p1, void *p2, void *p3, void *p4)
{
	windowp old_wp = (windowp)p1;
	bucketp bucket, *sorted_table;
	unsigned int i, j, n;

	/*
	 * Count the hash table buckets.
	 */
	for (i = n = 0; i < var_table->size; ++i)
		for (bucket = var_table->table[i]; bucket != NULL; bucket = bucket->next)
			++n;

	/*
	 * Allocate space for the sorted table.
	 */
	sorted_table = (bucketp *)xmalloc(sizeof(bucketp) * n);

	/*
	 * Add the buckets to the sorted table.
	 */
	for (i = j = 0; i < var_table->size; ++i)
		for (bucket = var_table->table[i]; bucket != NULL; bucket = bucket->next)
			sorted_table[j++] = bucket;

	/*
	 * Sort the table.
	 */
	qsort(sorted_table, n, sizeof(bucketp), sort_func);

	bprintf("Global variables:\n\n");
	bprintf("%-30s %s\n", "Variable", "Value");
	bprintf("%-30s %s\n", "--------", "-----");

	/*
	 * Output the table.
	 */
	for (i = 0; i < n; i++)
		if (sorted_table[i]->data != NULL)
			bprintf("%-30s \"%s\"\n", sorted_table[i]->key,
				sorted_table[i]->data);

	free(sorted_table);

	bprintf("\nLocal buffer variables:\n\n");
	bprintf("%-30s %s\n", "Variable", "Value");
	bprintf("%-30s %s\n", "--------", "-----");
	bprintf("%-30s \"%d\"\n", "fill-column", old_wp->bp->fill_column);
	bprintf("%-30s \"%d\"\n", "tab-width", old_wp->bp->tab_width);
}

DEFUN("list-variables", list_variables)
/*+
List defined variables.
+*/
{
	write_to_temporary_buffer("*Variable List*", write_variables_list,
				  cur_wp, NULL, NULL, NULL);
	return TRUE;
}
