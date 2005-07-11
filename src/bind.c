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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

/*	$Id: bind.c,v 1.70 2005/07/11 06:12:29 rrt Exp $	*/

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
static Function _last_command;

/*--------------------------------------------------------------------------
 * Key binding.
 *--------------------------------------------------------------------------*/

typedef struct leaf *leafp;

struct leaf {
  /* The key and the function associated with the leaf. */
  size_t key;
  Function func;

  /* Leaf vector, number of items, max number of items. */
  leafp *vec;
  size_t vecnum, vecmax;
};

static leafp leaf_tree;

static leafp leaf_new(int vecmax)
{
  leafp p;

  p = (leafp)zmalloc(sizeof(*p));
  memset(p, 0, sizeof(*p));

  p->vecmax = vecmax;
  p->vec = (leafp *)zmalloc(sizeof(*p) * vecmax);

  return p;
}

static leafp search_leaf(leafp tree, size_t key)
{
  size_t i;

  for (i = 0; i < tree->vecnum; ++i)
    if (tree->vec[i]->key == key)
      return tree->vec[i];

  return NULL;
}

static void add_leaf(leafp tree, leafp p)
{
  size_t i;

  /* Reallocate vector if there is not enough space. */
  if (tree->vecnum + 1 >= tree->vecmax) {
    tree->vecmax += 5;
    tree->vec = (leafp *)zrealloc(tree->vec, sizeof(*p) * tree->vecmax);
  }

  /* Insert the leaf at the sorted position. */
  for (i = 0; i < tree->vecnum; i++)
    if (tree->vec[i]->key > p->key) {
      memmove(&tree->vec[i+1], &tree->vec[i], sizeof(p) * (tree->vecnum - i));
      tree->vec[i] = p;
      break;
    }
  if (i == tree->vecnum)
    tree->vec[tree->vecnum] = p;
  ++tree->vecnum;
}

static void bind_key_vec(leafp tree, size_t *keys, size_t n, Function func)
{
  leafp p, s;

  if ((s = search_leaf(tree, keys[0])) == NULL) {
    p = leaf_new(n == 1 ? 1 : 5);
    p->key = keys[0];
    add_leaf(tree, p);
    if (n == 1)
      p->func = func;
    else
      bind_key_vec(p, &keys[1], n - 1, func);
  } else if (n > 1)
    bind_key_vec(s, &keys[1], n - 1, func);
  else
    s->func = func;
}

static void bind_key_string(char *key, Function func)
{
  size_t numkeys, *keys;

  if ((numkeys = keystrtovec(key, &keys)) > 0) {
    bind_key_vec(leaf_tree, keys, numkeys, func);
    free(keys);
  }
}

static leafp search_key(leafp tree, size_t *keys, size_t n)
{
  leafp p;

  if ((p = search_leaf(tree, keys[0])) != NULL) {
    if (n == 1)
      return p;
    else
      return search_key(p, &keys[1], n - 1);
  }

  return NULL;
}

size_t do_completion(astr as)
{
  size_t key;

  minibuf_write("%s", astr_cstr(as));
  key = getkey();
  minibuf_clear();

  return key;
}

static astr make_completion(size_t *keys, size_t numkeys)
{
  astr as = astr_new(), key;
  size_t i, len = 0;

  for (i = 0; i < numkeys; i++) {
    if (i > 0) {
      astr_cat_cstr(as, " ");
      len++;
    }
    key = chordtostr(keys[i]);
    astr_cat(as, key);
    astr_delete(key);
  }

  return astr_cat_cstr(as, "-");
}

static leafp completion_scan(size_t key, size_t **keys, size_t *numkeys)
{
  leafp p;
  vector *v = vec_new(sizeof(key));

  vec_item(v, 0, size_t) = key;
  *numkeys = 1;

  do {
    if ((p = search_key(leaf_tree, vec_array(v), *numkeys)) == NULL)
      break;
    if (p->func == NULL) {
      astr as = make_completion(vec_array(v), *numkeys);
      vec_item(v, (*numkeys)++, size_t) = do_completion(as);
      astr_delete(as);
    }
  } while (p->func == NULL);

  *keys = vec_toarray(v);
  return p;
}

void process_key(size_t key)
{
  int uni;
  size_t *keys = NULL, numkeys;
  leafp p;

  if (key == KBD_NOKEY)
    return;

  if (key & KBD_META && isdigit(key & 255))
    /* Got an ESC x sequence where `x' is a digit. */
    universal_argument(KBD_META, (int)((key & 0xff) - '0'));
  else {
    if ((p = completion_scan(key, &keys, &numkeys)) == NULL) {
      /* There are no bindings for the pressed key. */
      undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
      for (uni = 0;
           uni < last_uniarg && self_insert_command((ptrdiff_t)key);
           ++uni);
      undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
    } else {
      p->func((lastflag & FLAG_SET_UNIARG) != 0, last_uniarg);
      _last_command = p->func;
    }

    free(keys);
  }

  /* Only add keystrokes if we're already in macro defining mode
     before the function call, to cope with start-kbd-macro. */
  if (lastflag & FLAG_DEFINING_MACRO && thisflag & FLAG_DEFINING_MACRO)
    add_cmd_to_macro();
}

Function last_command(void)
{
  return _last_command;
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
  char *key[3];
};

typedef struct fentry *fentryp;

static struct fentry fentry_table[] = {
#define X0(zile_name, c_name) \
	{zile_name, F_ ## c_name, {NULL, NULL, NULL}},
#define X1(zile_name, c_name, key1) \
	{zile_name, F_ ## c_name, {key1, NULL, NULL}},
#define X2(zile_name, c_name, key1, key2) \
	{zile_name, F_ ## c_name, {key1, key2, NULL}},
#define X3(zile_name, c_name, key1, key2, key3) \
	{zile_name, F_ ## c_name, {key1, key2, key3}},
#include "tbl_funcs.h"
#undef X0
#undef X1
#undef X2
#undef X3
};

#define fentry_table_size (sizeof(fentry_table) / sizeof(fentry_table[0]))

static int bind_compar(const void *p1, const void *p2)
{
  return strcmp(((fentryp)p1)->name, ((fentryp)p2)->name);
}

void init_bindings(void)
{
  size_t i, j;

  leaf_tree = leaf_new(10);

  if (lookup_bool_variable("alternative-bindings"))
    for (i = 0; i < fentry_table_size; ++i)
      for (j = 0; j < 3; ++j)
        if (fentry_table[i].key[j] != NULL) {
          if (strcmp(fentry_table[i].key[j], "\\M-h") == 0)
            fentry_table[i].key[j] = "\\M-h\\M-h";
          else if (strncmp(fentry_table[i].key[j], "\\C-h", 4) == 0) {
            fentry_table[i].key[j] = zstrdup(fentry_table[i].key[j]);
            fentry_table[i].key[j][1] = 'M';
          }
        }

  /* Sort the array for better searching later. */
  qsort(fentry_table, fentry_table_size, sizeof(fentry_table[0]), bind_compar);

  /* Bind all the default functions. */
  for (i = 0; i < fentry_table_size; i++)
    for (j = 0; j < 3; ++j)
      if (fentry_table[i].key[j] != NULL)
        bind_key_string(fentry_table[i].key[j], fentry_table[i].func);
}

static void recursive_free_bindings(leafp p)
{
  size_t i;
  for (i = 0; i < p->vecnum; ++i)
    recursive_free_bindings(p->vec[i]);
  free(p->vec);
  free(p);
}

void free_bindings(void)
{
  recursive_free_bindings(leaf_tree);
  free_history_elements(&functions_history);
}

static char *bsearch_function(char *name)
{
  struct fentry key, *entryp;
  key.name = name;
  entryp = bsearch(&key, fentry_table, fentry_table_size, sizeof(fentry_table[0]), bind_compar);
  return entryp ? entryp->name : NULL;
}

static fentryp get_fentry(char *name)
{
  size_t i;
  assert(name);
  for (i = 0; i < fentry_table_size; ++i)
    if (!strcmp(name, fentry_table[i].name))
      return &fentry_table[i];
  return NULL;
}

Function get_function(char *name)
{
  fentryp f = get_fentry(name);
  return f ? f->func : NULL;
}

char *get_function_name(Function p)
{
  size_t i;
  for (i = 0; i < fentry_table_size; ++i)
    if (fentry_table[i].func == p)
      return fentry_table[i].name;
  return NULL;
}

static astr bindings_string(fentryp f)
{
  size_t i;
  astr as = astr_new();

  for (i = 0; i < 3; ++i) {
    astr key = simplify_key(f->key[i]);
    if (astr_len(key) > 0) {
        astr_cat_cstr(as, (i == 0) ? "" : ", ");
        astr_cat(as, key);
    }
    astr_delete(key);
  }

  return as;
}

/*
 * Read a function name from the minibuffer.
 * The returned buffer must be freed by the caller.
 */
char *minibuf_read_function_name(const char *fmt, ...)
{
  va_list ap;
  size_t i;
  char *buf, *ms;
  list p;
  Completion *cp;

  va_start(ap, fmt);
  buf = minibuf_format(fmt, ap);
  va_end(ap);

  cp = completion_new(FALSE);
  for (i = 0; i < fentry_table_size; ++i)
    list_append(cp->completions, zstrdup(fentry_table[i].name));

  for (;;) {
    ms = minibuf_read_completion(buf, "", cp, &functions_history);

    if (ms == NULL) {
      cancel();
      ms = NULL;
      break;
    } else if (ms[0] == '\0') {
      minibuf_error("No function name given");
      ms = NULL;
      break;
    } else {
      astr as = astr_new();
      astr_cpy_cstr(as, ms);
      /* Complete partial words if possible. */
      if (completion_try(cp, as, FALSE) == COMPLETION_MATCHED)
        ms = cp->match;
      astr_delete(as);
      for (p = list_first(cp->completions); p != cp->completions;
           p = list_next(p))
        if (!strcmp(ms, p->item)) {
          ms = p->item;
          break;
        }
      if (bsearch_function(ms) || get_macro(ms)) {
        add_history_element(&functions_history, ms);
        minibuf_clear();        /* Remove any error message. */
        ms = zstrdup(ms);       /* Might be about to be freed. */
        break;                  /* We're finished. */
      } else {
        minibuf_error("Undefined function name `%s'", ms);
        waitkey(WAITKEY_DEFAULT);
      }
    }
  }

  free(buf);
  free_completion(cp);

  return ms;
}

static int execute_function(char *name, int uniarg)
{
  Function func;
  Macro *mp;

  if ((func = get_function(name)))
    return func(uniarg ? 1 : 0, uniarg);
  else if ((mp = get_macro(name)))
    return call_macro(mp);
  else
    return FALSE;
}

DEFUN_INT("execute-extended-command", execute_extended_command)
/*+
Read function name, then read its arguments and call it.
+*/
{
  int res;
  char *name;
  astr msg = astr_new();

  if (uniused && uniarg != 0)
    astr_afmt(msg, "%d M-x ", uniarg);
  else
    astr_cat_cstr(msg, "M-x ");

  name = minibuf_read_function_name(astr_cstr(msg));
  astr_delete(msg);
  if (name == NULL)
    return FALSE;

  res = execute_function(name, uniarg);
  free(name);

  return res;
}
END_DEFUN

DEFUN_INT("global-set-key", global_set_key)
/*+
Bind a command to a key sequence.
Read key sequence and function name, and bind the function to the key
sequence.
+*/
{
  int ok = FALSE;
  size_t key, *keys = NULL, numkeys;
  leafp p;
  Function func;
  char *name;
  astr as;

  minibuf_write("Set key globally: ");
  key = getkey();
  p = completion_scan(key, &keys, &numkeys);

  as = keyvectostr(keys, numkeys);
  name = minibuf_read_function_name("Set key %s to command: ", astr_cstr(as));
  astr_delete(as);

  if (name == NULL)
    return FALSE;

  func = get_function(name);
  if (func) {
    ok = TRUE;
    bind_key_vec(leaf_tree, keys, numkeys, func);
  } else
    minibuf_error("No such function `%s'", name);

  free(name);
  if (uniused == 0)
    free(keys);

  return ok;
}
END_DEFUN

DEFUN_INT("where-is", where_is)
/*+
Print message listing key sequences that invoke the command DEFINITION.
Argument is a command definition, usually a symbol with a function definition.
If INSERT (the prefix arg) is non-nil, insert the message in the buffer.
+*/
{
  char *name;
  fentryp f;

  name = minibuf_read_function_name("Where is command: ");

  if (name == NULL || (f = get_fentry(name)) == NULL)
    return FALSE;

  if (f->key[0]) {
    astr bindings = bindings_string(f);
    astr as = astr_new();

    astr_afmt(as, "%s is on %s", name, astr_cstr(bindings));
    if (uniused)
      bprintf("%s", astr_cstr(as));
    else
      minibuf_write("%s", astr_cstr(as));

    astr_delete(as);
    astr_delete(bindings);
  } else
    minibuf_write("%s is not on any key", name);

  return TRUE;
}
END_DEFUN

char *get_function_by_key_sequence(void)
{
  leafp p;
  size_t c = getkey();
  size_t *keys, numkeys;

  if (c & KBD_META && isdigit(c & 255))
    return "universal-argument";

  p = completion_scan(c, &keys, &numkeys);
  free(keys);
  if (p == NULL) {
    if (c == KBD_RET || c == KBD_TAB || c <= 255)
      return "self-insert-command";
    else
      return NULL;
  } else
    return get_function_name(p->func);
}

static void write_bindings_tree(leafp tree, list keys)
{
  size_t i;
  list l;
  astr as = chordtostr(tree->key);

  list_append(keys, as);

  for (i = 0; i < tree->vecnum; ++i) {
    leafp p = tree->vec[i];
    if (p->func != NULL) {
      astr key = astr_new();
      astr as = chordtostr(p->key);
      for (l = list_next(list_first(keys));
           l != keys;
           l = list_next(l)) {
        astr_cat(key, l->item);
        astr_cat_char(key, ' ');
      }
      astr_cat(key, as);
      astr_delete(as);
      bprintf("%-15s %s\n", astr_cstr(key), get_function_name(p->func));
      astr_delete(key);
    } else
      write_bindings_tree(p, keys);
  }

  astr_delete(list_betail(keys));
}

static void write_bindings_list(va_list ap)
{
  list l = list_new();

  (void)ap;

  bprintf("%-15s %s\n", "Binding", "Function");
  bprintf("%-15s %s\n", "-------", "--------");

  write_bindings_tree(leaf_tree, l);
  list_delete(l);
}

DEFUN_INT("list-bindings", list_bindings)
/*+
List defined bindings.
+*/
{
  write_temp_buffer("*Bindings List*", write_bindings_list);
  return TRUE;
}
END_DEFUN
