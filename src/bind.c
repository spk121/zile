/* Key bindings and extended commands
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2008 Reuben Thomas.
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
    else if (n > 0)
      bind_key_vec(p, &keys[1], n - 1, func);
  } else if (n > 1)
    bind_key_vec(s, &keys[1], n - 1, func);
  else
    s->func = func;
}

static int bind_key_string(char *key, Function func)
{
  int numkeys;
  size_t *keys;

  if ((numkeys = keystrtovec(key, &keys)) > 0) {
    bind_key_vec(leaf_tree, keys, numkeys, func);
    free(keys);
    return TRUE;
  } else
    return FALSE;
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
      astr_cat_char(as, ' ');
      len++;
    }
    key = chordtostr(keys[i]);
    astr_cat(as, key);
    astr_delete(key);
  }

  return astr_cat_char(as, '-');
}

static leafp completion_scan(size_t key, size_t **keys, int *numkeys)
{
  leafp p;
  vector *v = vec_new(sizeof(key));

  vec_item(v, (size_t)0, size_t) = key;
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

DEFUN("self-insert-command", self_insert_command)
/*+
Insert the character you type.
Whichever character you type to run this command is inserted.
+*/
{
  int uni, ret = TRUE;

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  for (uni = 0; uni < uniarg; ++uni) {
    /* Mask out ~KBD_CTRL to allow control sequences to be themselves. */
    int key = lastkey() & ~KBD_CTRL;
    deactivate_mark();
    if (key <= 0xff) {
      if (isspace(key) && cur_bp->flags & BFLAG_AUTOFILL &&
          get_goalc() > (size_t)get_variable_number("fill-column"))
        fill_break_line();
      insert_char(key);
    } else {
      ding(); 
      ret = FALSE;
      break;
    }
  }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return ret;
}
END_DEFUN

void process_key(size_t key)
{
  int numkeys;
  size_t *keys = NULL;
  leafp p;

  if (key == KBD_NOKEY)
    return;

  if (key & KBD_META && isdigit((int)(key & 0xff)))
    /* Got an ESC x sequence where `x' is a digit. */
    universal_argument(KBD_META, (int)((key & 0xff) - '0'));
  else if ((p = completion_scan(key, &keys, &numkeys)) != NULL) {
    p->func(last_uniarg, NULL);
    _last_command = p->func;
  } else {
    astr as = keyvectostr(keys, numkeys);
    minibuf_error("%s is undefined", astr_cstr(as));
    astr_delete(as);
  }
  free(keys);

  /* Only add keystrokes if we were already in macro defining mode
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
  size_t i, j, *keys;

  keys = zmalloc(sizeof(*keys));
  leaf_tree = leaf_new(10);

  /* Bind all printing keys to self_insert_command */
  for (i = 0; i <= 0xff; i++)
    if (isprint(*keys = i))
      bind_key_vec(leaf_tree, keys, 1, F_self_insert_command);
  free(keys);
  
  /* Bind all the default functions. */
  for (i = 0; i < fentry_table_size; i++)
    for (j = 0; j < 3; ++j)
      if (fentry_table[i].key[j] != NULL)
        if (!bind_key_string(fentry_table[i].key[j], fentry_table[i].func)) {
          fprintf(stderr, PACKAGE ": Key sequence %s is invalid in default bindings\n\n", fentry_table[i].key[j]);
          exit(1);
        }
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

int execute_function(char *name, int uniarg)
{
  Function func;
  Macro *mp;

  if ((func = get_function(name)))
    return func(uniarg, NULL);
  else if ((mp = get_macro(name))) {
    call_macro(mp);
    return TRUE;
  } else
    return FALSE;
}

DEFUN("execute-extended-command", execute_extended_command)
/*+
Read function name, then read its arguments and call it.
+*/
{
  int res;
  char *name;
  astr msg = astr_new();

  if (lastflag & FLAG_SET_UNIARG)
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

DEFUN("global-set-key", global_set_key)
/*+
Bind a command to a key sequence.
Read key sequence and function name, and bind the function to the key
sequence.
+*/
{
  int ok = FALSE, numkeys;
  size_t key, *keys = NULL;
  Function func;
  char *name;

  if (arglist) {
    numkeys = keystrtovec(arglist->list_next->data, &keys);
    if (numkeys == -1) {
      minibuf_error("Key sequence %s is invalid", arglist->list_next->data);
      return FALSE;
    }
    name = arglist->list_next->list_next->data;
  } else {
    astr as;

    minibuf_write("Set key globally: ");
    key = getkey();
    completion_scan(key, &keys, &numkeys);

    as = keyvectostr(keys, numkeys);
    name = minibuf_read_function_name("Set key %s to command: ", astr_cstr(as));
    astr_delete(as);
  }

  if (name == NULL)
    return FALSE;

  func = get_function(name);
  if (func) {
    ok = TRUE;
    bind_key_vec(leaf_tree, keys, numkeys, func);
  } else
    minibuf_error("No such function `%s'", name);

  if (!arglist)
    free(name);
  free(keys);

  return ok;
}
END_DEFUN

static void walk_bindings_tree(leafp tree, list keys, void (*process)(astr key, leafp p, void *st), void *st)
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
      astr_cat_delete(key, as);
      process(key, p, st);
      astr_delete(key);
    } else
      walk_bindings_tree(p, keys, process, st);
  }

  astr_delete(list_betail(keys));
}

static void walk_bindings(leafp tree, void (*process)(astr key, leafp p, void *st), void *st)
{
  list l = list_new();
  walk_bindings_tree(tree, l, process, st);
  list_delete(l);
}

typedef struct {
  Function f;
  astr bindings;
} gather_bindings_state;

static void gather_bindings(astr key, leafp p, void *st)
{
  gather_bindings_state *g = (gather_bindings_state *)st;

  if (p->func == g->f) {
    if (astr_len(g->bindings) > 0)
      astr_cat_cstr(g->bindings, ", ");
    astr_cat(g->bindings, key);
  }
}

DEFUN("where-is", where_is)
/*+
Print message listing key sequences that invoke the command DEFINITION.
Argument is a command name.  If the prefix arg is non-nil, insert the
message in the buffer.
+*/
{
  char *name = minibuf_read_function_name("Where is command: ");
  int ret = FALSE;
  gather_bindings_state g;

  if (name && (g.f = get_function(name))) {
    g.bindings = astr_new();
    walk_bindings(leaf_tree, gather_bindings, &g);

    if (astr_len(g.bindings) == 0)
      minibuf_write("%s is not on any key", name);
    else {
      astr as = astr_new();
      astr_afmt(as, "%s is on %s", name, astr_cstr(g.bindings));
      if (lastflag & FLAG_SET_UNIARG)
        bprintf("%s", astr_cstr(as));
      else
        minibuf_write("%s", astr_cstr(as));
      astr_delete(as);
    }
    astr_delete(g.bindings);
    ret = TRUE;
  }

  free(name);
  return ret;
}
END_DEFUN

char *get_function_by_key_sequence(size_t **keys, int *numkeys)
{
  leafp p;
  size_t c = getkey();

  if (c & KBD_META && isdigit((int)(c & 0xff))) {
    *numkeys = 1;
    *keys = malloc(sizeof(**keys));
    *keys[0] = c;
    return "universal-argument";
  }

  p = completion_scan(c, keys, numkeys);
  if (p == NULL)
    return NULL;
  else
    return get_function_name(p->func);
}

static void print_binding(astr key, leafp p, void *st GCC_UNUSED)
{
  bprintf("%-15s %s\n", astr_cstr(key), get_function_name(p->func));
}

static void write_bindings_list(va_list ap)
{
  (void)ap;

  bprintf("Global Bindings:");
  bprintf("%-15s %s\n", "key", "binding");
  bprintf("%-15s %s\n", "---", "-------");

  walk_bindings(leaf_tree, print_binding, NULL);
}

DEFUN("describe-bindings", describe_bindings)
/*+
Show a list of all defined keys, and their definitions.
+*/
{
  write_temp_buffer("*Help*", write_bindings_list);
  return TRUE;
}
END_DEFUN
