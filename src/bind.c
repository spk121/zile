/* Key bindings and extended commands

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Reuben Thomas.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gl_array_list.h"
#include "gl_linked_list.h"

#include "zile.h"
#include "extern.h"

static Function _last_command;

/*--------------------------------------------------------------------------
 * Key binding.
 *--------------------------------------------------------------------------*/

typedef struct leaf *leafp;

struct leaf
{
  /* The key and the function associated with the leaf. */
  size_t key;
  Function func;

  /* Leaf vector, number of items, max number of items. */
  leafp *vec;
  size_t vecnum, vecmax;
};

static leafp leaf_tree;

static leafp
leaf_new (int vecmax)
{
  leafp p;

  p = (leafp) XZALLOC (struct leaf);

  p->vecmax = vecmax;
  p->vec = (leafp *) XCALLOC (vecmax, struct leaf);

  return p;
}

static leafp
search_leaf (leafp tree, size_t key)
{
  size_t i;

  for (i = 0; i < tree->vecnum; ++i)
    if (tree->vec[i]->key == key)
      return tree->vec[i];

  return NULL;
}

static void
add_leaf (leafp tree, leafp p)
{
  size_t i;

  /* Reallocate vector if there is not enough space. */
  if (tree->vecnum + 1 >= tree->vecmax)
    {
      tree->vecmax += 5;
      tree->vec = (leafp *) xrealloc (tree->vec, sizeof (*p) * tree->vecmax);
    }

  /* Insert the leaf at the sorted position. */
  for (i = 0; i < tree->vecnum; i++)
    if (tree->vec[i]->key > p->key)
      {
        memmove (&tree->vec[i + 1], &tree->vec[i],
                 sizeof (p) * (tree->vecnum - i));
        tree->vec[i] = p;
        break;
      }
  if (i == tree->vecnum)
    tree->vec[tree->vecnum] = p;
  ++tree->vecnum;
}

static void
bind_key_vec (leafp tree, gl_list_t keys, size_t from, Function func)
{
  leafp p, s = search_leaf (tree, (size_t) gl_list_get_at (keys, from));
  size_t n = gl_list_size (keys) - from;

  if (s == NULL)
    {
      p = leaf_new (n == 1 ? 1 : 5);
      p->key = (size_t) gl_list_get_at (keys, from);
      add_leaf (tree, p);
      if (n == 1)
        p->func = func;
      else if (n > 0)
        bind_key_vec (p, keys, from + 1, func);
    }
  else if (n > 1)
    bind_key_vec (s, keys, from + 1, func);
  else
    s->func = func;
}

static void
bind_key_string (char *key, Function func)
{
  gl_list_t keys = keystrtovec (key);

  if (keys && gl_list_size (keys) > 0)
    bind_key_vec (leaf_tree, keys, 0, func);
  else
    {
      fprintf (stderr,
               "%s: Key sequence %s is invalid in default bindings\n\n",
               prog_name, key);
      exit (1);
    }
  gl_list_free (keys);
}

static leafp
search_key (leafp tree, gl_list_t keys, size_t from)
{
  leafp p = search_leaf (tree, (size_t) gl_list_get_at (keys, from));

  if (p != NULL)
    {
      if (gl_list_size (keys) - from == 1)
        return p;
      else
        return search_key (p, keys, from + 1);
    }

  return NULL;
}

size_t
do_completion (astr as)
{
  size_t key;
  astr bs = astr_new ();

  if (lastflag & FLAG_SET_UNIARG)
    {
      int arg = last_uniarg;

      if (arg < 0)
        {
          astr_cat_cstr (bs, "- ");
          arg = -arg;
        }

      do {
        astr_insert_char (bs, 0, ' ');
        astr_insert_char (bs, 0, arg % 10 + '0');
        arg /= 10;
      } while (arg != 0);
    }

  minibuf_write ("%s%s%s",
                 lastflag & (FLAG_SET_UNIARG | FLAG_UNIARG_EMPTY) ? "C-u " : "",
                 astr_cstr (bs),
                 astr_cstr (as));
  astr_delete (bs);
  key = getkey ();
  minibuf_clear ();

  return key;
}

static astr
make_completion (gl_list_t keys)
{
  astr as = astr_new (), key;
  size_t i, len = 0;

  for (i = 0; i < gl_list_size (keys); i++)
    {
      if (i > 0)
        {
          astr_cat_char (as, ' ');
          len++;
        }
      key = chordtostr ((size_t) gl_list_get_at (keys, i));
      astr_cat (as, key);
      astr_delete (key);
    }

  return astr_cat_char (as, '-');
}

static leafp
completion_scan (size_t key, gl_list_t * keys)
{
  leafp p;
  *keys = gl_list_create_empty (GL_ARRAY_LIST,
                                NULL, NULL, NULL, true);

  gl_list_add_last (*keys, (void *) key);

  do
    {
      p = search_key (leaf_tree, *keys, 0);
      if (p == NULL)
        break;
      if (p->func == NULL)
        {
          astr as = make_completion (*keys);
          gl_list_add_last (*keys, (void *) do_completion (as));
          astr_delete (as);
        }
    }
  while (p->func == NULL);

  return p;
}

static int
self_insert_command (void)
{
  int ret = true;
  /* Mask out ~KBD_CTRL to allow control sequences to be themselves. */
  int key = lastkey () & ~KBD_CTRL;
  deactivate_mark ();
  if (key <= 0xff)
    {
      if (isspace (key) && cur_bp->flags & BFLAG_AUTOFILL &&
          get_goalc () > (size_t) get_variable_number ("fill-column"))
        fill_break_line ();
      insert_char (key);
    }
  else
    {
      ding ();
      ret = false;
    }

  return ret;
}

DEFUN ("self-insert-command", self_insert_command)
/*+
Insert the character you type.
Whichever character you type to run this command is inserted.
+*/
{
  return execute_with_uniarg (true, uniarg, self_insert_command, NULL);
}
END_DEFUN

void
process_key (size_t key)
{
  leafp p;

  if (key == KBD_NOKEY)
    return;

  if (key & KBD_META && isdigit ((int) (key & 0xff)))
    /* Got an ESC x sequence where `x' is a digit. */
    universal_argument (KBD_META, (int) ((key & 0xff) - '0'));
  else
    {
      gl_list_t keys;
      p = completion_scan (key, &keys);
      if (p != NULL)
        {
          p->func (last_uniarg, NULL);
          _last_command = p->func;
        }
      else
        {
          astr as = keyvectostr (keys);
          minibuf_error ("%s is undefined", astr_cstr (as));
          astr_delete (as);
        }
      gl_list_free (keys);
    }

  /* Only add keystrokes if we were already in macro defining mode
     before the function call, to cope with start-kbd-macro. */
  if (lastflag & FLAG_DEFINING_MACRO && thisflag & FLAG_DEFINING_MACRO)
    add_cmd_to_macro ();
}

Function
last_command (void)
{
  return _last_command;
}

void
init_bindings (void)
{
  size_t i;
  gl_list_t keys = gl_list_create_empty (GL_ARRAY_LIST,
                                         NULL, NULL, NULL, true);

  leaf_tree = leaf_new (10);

  /* Bind all printing keys to self_insert_command */
  gl_list_add_last (keys, NULL);
  for (i = 0; i <= 0xff; i++)
    {
      if (isprint (i))
        {
          gl_list_set_at (keys, 0, (void *) i);
          bind_key_vec (leaf_tree, keys, 0, F_self_insert_command);
        }
    }
  gl_list_free (keys);

#define X1(c_name, key1)                 \
  bind_key_string (key1, F_ ## c_name);
#define X2(c_name, key1, key2)           \
  bind_key_string (key1, F_ ## c_name);  \
  bind_key_string (key2, F_ ## c_name);
#define X3(c_name, key1, key2, key3)     \
  bind_key_string (key1, F_ ## c_name);  \
  bind_key_string (key2, F_ ## c_name);  \
  bind_key_string (key3, F_ ## c_name);
#include "tbl_bind.h"
#undef X1
#undef X2
#undef X3
}

static void
recursive_free_bindings (leafp p)
{
  size_t i;
  for (i = 0; i < p->vecnum; ++i)
    recursive_free_bindings (p->vec[i]);
  free (p->vec);
  free (p);
}

void
free_bindings (void)
{
  recursive_free_bindings (leaf_tree);
}

DEFUN ("global-set-key", global_set_key)
/*+
Bind a command to a key sequence.
Read key sequence and function name, and bind the function to the key
sequence.
+*/
{
  int ret = false;
  size_t key;
  gl_list_t keys;
  Function func;
  const char *name;
  size_t argc = countNodes (arglist);

  if (arglist && argc >= 3)
    {
      keys = keystrtovec (arglist->next->data);
      if (keys == NULL)
        {
          minibuf_error ("Key sequence %s is invalid",
                         arglist->next->data);
          return false;
        }
      name = arglist->next->next->data;
    }
  else
    {
      astr as;

      minibuf_write ("Set key globally: ");
      key = getkey ();
      completion_scan (key, &keys);

      as = keyvectostr (keys);
      name = minibuf_read_function_name ("Set key %s to command: ",
                                         astr_cstr (as));
      astr_delete (as);
    }

  if (name == NULL)
    return false;

  func = get_function (name);
  if (func)
    {
      ret = true;
      bind_key_vec (leaf_tree, keys, 0, func);
    }
  else
    minibuf_error ("No such function `%s'", name);

  if (!arglist)
    free ((char *) name);
  gl_list_free (keys);

  return bool_to_lisp (ret);
}
END_DEFUN

static void
walk_bindings_tree (leafp tree, gl_list_t keys,
                    void (*process) (astr key, leafp p, void *st), void *st)
{
  size_t i, j;
  astr as = chordtostr (tree->key);

  gl_list_add_last (keys, as);

  for (i = 0; i < tree->vecnum; ++i)
    {
      leafp p = tree->vec[i];
      if (p->func != NULL)
        {
          astr key = astr_new ();
          astr as = chordtostr (p->key);
          for (j = 1; j < gl_list_size (keys); j++)
            {
              astr_cat (key, (astr) gl_list_get_at (keys, j));
              astr_cat_char (key, ' ');
            }
          astr_cat_delete (key, as);
          process (key, p, st);
          astr_delete (key);
        }
      else
        walk_bindings_tree (p, keys, process, st);
    }

  astr_delete ((astr) gl_list_get_at (keys, gl_list_size (keys) - 1));
  assert (gl_list_remove_at (keys, gl_list_size (keys) - 1));
}

static void
walk_bindings (leafp tree, void (*process) (astr key, leafp p, void *st),
               void *st)
{
  gl_list_t l = gl_list_create_empty (GL_LINKED_LIST,
                                      NULL, NULL, NULL, true);
  walk_bindings_tree (tree, l, process, st);
  gl_list_free (l);
}

typedef struct
{
  Function f;
  astr bindings;
} gather_bindings_state;

static void
gather_bindings (astr key, leafp p, void *st)
{
  gather_bindings_state *g = (gather_bindings_state *) st;

  if (p->func == g->f)
    {
      if (astr_len (g->bindings) > 0)
        astr_cat_cstr (g->bindings, ", ");
      astr_cat (g->bindings, key);
    }
}

DEFUN ("where-is", where_is)
/*+
Print message listing key sequences that invoke the command DEFINITION.
Argument is a command name.  If the prefix arg is non-nil, insert the
message in the buffer.
+*/
{
  const char *name = minibuf_read_function_name ("Where is command: ");
  int ret = false;
  gather_bindings_state g;

  if (name)
    {
      g.f = get_function (name);
      if (g.f)
        {
          g.bindings = astr_new ();
          walk_bindings (leaf_tree, gather_bindings, &g);

          if (astr_len (g.bindings) == 0)
            minibuf_write ("%s is not on any key", name);
          else
            {
              astr as = astr_new ();
              astr_afmt (as, "%s is on %s", name, astr_cstr (g.bindings));
              if (lastflag & FLAG_SET_UNIARG)
                bprintf ("%s", astr_cstr (as));
              else
                minibuf_write ("%s", astr_cstr (as));
              astr_delete (as);
            }
          astr_delete (g.bindings);
          ret = true;
        }
    }

  free ((char *) name);
  return bool_to_lisp (ret);
}
END_DEFUN

const char *
get_function_by_key_sequence (gl_list_t * keys)
{
  leafp p;
  size_t c = getkey ();

  if (c & KBD_META && isdigit ((int) (c & 0xff)))
    {
      *keys = gl_list_create_empty (GL_ARRAY_LIST,
                                    NULL, NULL, NULL, true);
      gl_list_add_last (*keys, (void *) c);
      return "universal-argument";
    }

  p = completion_scan (c, keys);
  if (p == NULL)
    return NULL;
  else
    return get_function_name (p->func);
}

static void
print_binding (astr key, leafp p, void *st GCC_UNUSED)
{
  bprintf ("%-15s %s\n", astr_cstr (key), get_function_name (p->func));
}

static void
write_bindings_list (va_list ap GCC_UNUSED)
{
  bprintf ("Key translations:\n");
  bprintf ("%-15s %s\n", "key", "binding");
  bprintf ("%-15s %s\n", "---", "-------");

  walk_bindings (leaf_tree, print_binding, NULL);
}

DEFUN ("describe-bindings", describe_bindings)
/*+
Show a list of all defined keys, and their definitions.
+*/
{
  write_temp_buffer ("*Help*", true, write_bindings_list);
  return leT;
}
END_DEFUN
