/* Key bindings and extended commands

   Copyright (c) 1997-2011 Free Software Foundation, Inc.
   Copyright (c) 2011, 2012 Michael Gran

   This file is part of Michael Gran's unofficial fork of GNU Zile.

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

#include <config.h>

#include <assert.h>
#include <ctype.h>
#include <libguile.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "gl_array_list.h"
#include "gl_linked_list.h"

#include "main.h"
#include "extern.h"

/*--------------------------------------------------------------------------
 * Key binding.
 *--------------------------------------------------------------------------*/

struct Binding
{
  size_t key; /* The key code (for every level except the root). */
  SCM proc; /* The Guile procedure for this key (if a leaf node), or #f*/

  /* Branch vector, number of items, max number of items. */
  Binding *vec;
  size_t vecnum, vecmax;
};

static Binding root_bindings;

static Binding
node_new (int vecmax)
{
  Binding p = (Binding) XZALLOC (struct Binding);

  p->vecmax = vecmax;
  p->vec = (Binding *) XCALLOC (vecmax, struct Binding);
  p->proc = SCM_BOOL_F;

  return p;
}

static Binding
search_node (Binding tree, size_t key)
{
  for (size_t i = 0; i < tree->vecnum; ++i)
    if (tree->vec[i]->key == key)
      return tree->vec[i];

  return NULL;
}

static void
add_node (Binding tree, Binding p)
{
  /* Erase any previous binding the current key might have had in case
     it was non-prefix and is now being made prefix, as we don't want
     to accidentally create a default for the prefix map. */
  if (tree->vecnum == 0)
    tree->proc = SCM_BOOL_F;

  /* Reallocate vector if there is not enough space. */
  if (tree->vecnum + 1 >= tree->vecmax)
    {
      tree->vecmax += 5;
      tree->vec = xrealloc (tree->vec, sizeof (*p) * tree->vecmax);
    }

  /* Insert the node. */
  tree->vec[tree->vecnum++] = p;
}

static void
bind_key_vec (Binding tree, gl_list_t keys, size_t from, SCM proc)
{
  Binding p, s = search_node (tree, (size_t) gl_list_get_at (keys, from));
  size_t n = gl_list_size (keys) - from;

  if (s == NULL)
    {
      p = node_new (n == 1 ? 1 : 5);
      p->key = (size_t) gl_list_get_at (keys, from);
      add_node (tree, p);
      if (n == 1)
        p->proc = proc;
      else if (n > 0)
        bind_key_vec (p, keys, from + 1, proc);
    }
  else if (n > 1)
    bind_key_vec (s, keys, from + 1, proc);
  else
    s->proc = proc;
}

static Binding
search_key (Binding tree, gl_list_t keys, size_t from)
{
  Binding p = search_node (tree, (size_t) gl_list_get_at (keys, from));

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
do_binding_completion (astr as)
{
  astr bs = astr_new ();

  if (lastflag & FLAG_SET_UNIARG)
    {
      unsigned arg = abs (last_uniarg);
      do
        {
          bs = astr_fmt ("%c %s", arg % 10 + '0', astr_cstr (bs));
          arg /= 10;
        }
      while (arg != 0);

      if (last_uniarg < 0)
        bs = astr_fmt ("- %s", astr_cstr (bs));
    }

  minibuf_write ("%s%s%s-",
                 lastflag & (FLAG_SET_UNIARG | FLAG_UNIARG_EMPTY) ? "C-u " : "",
                 astr_cstr (bs),
                 astr_cstr (as));
  size_t key = getkey (GETKEY_DEFAULT);
  minibuf_clear ();

  return key;
}

/* Get a key sequence from the keyboard; the sequence returned
   has at most the last stroke unbound. */
gl_list_t
get_key_sequence (void)
{
  gl_list_t keys = gl_list_create_empty (GL_ARRAY_LIST,
                                         NULL, NULL, NULL, true);
  size_t key;

  do
    key = getkey (GETKEY_DEFAULT);
  while (key == KBD_NOKEY);
  gl_list_add_last (keys, (void *) key);
  for (;;)
    {
      astr as;
      Binding p = search_key (root_bindings, keys, 0);
      if (p == NULL || scm_is_true (scm_procedure_p (p->proc)))
        break;
      as = keyvectodesc (keys);
      gl_list_add_last (keys, (void *) do_binding_completion (as));
    }

  return keys;
}

SCM
get_function_by_keys (gl_list_t keys)
{
  Binding p;

  /* Detect Meta-digit */
  if (gl_list_size (keys) == 1)
    {
      size_t key = (size_t) gl_list_get_at (keys, 0);
      if (key & KBD_META &&
          (isdigit ((int) (key & 0xff)) || (int) (key & 0xff) == '-'))
        return F_universal_argument ();
    }

  /* See if we've got a valid key sequence */
  p = search_key (root_bindings, keys, 0);

  return p ? p->proc : SCM_BOOL_F;
}

static bool
self_insert_command (void)
{
  int ret = true;
  /* Mask out ~KBD_CTRL to allow control sequences to be themselves. */
  int key = (int) (lastkey () & ~KBD_CTRL);
  deactivate_mark ();
  if (key <= 0xff)
    {
      if (isspace (key) && get_buffer_autofill (cur_bp) &&
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

SCM_DEFINE (G_self_insert_command, "self-insert-command", 0, 1, 0, (SCM gn), "\
Insert the character you type.\n\
Whichever character you type to run this command is inserted.")
{
  long n = guile_to_long_or_error ("self-insert-command", SCM_ARG1, gn);
  if (interactive)
    return execute_with_uniarg (true, n, self_insert_command, NULL);
  return SCM_BOOL_F;
}

static SCM _last_command;
static SCM _this_command;

SCM
last_command (void)
{
  return _last_command;
}

void
set_this_command (SCM cmd)
{
  _this_command = cmd;
}

SCM
call_command (SCM proc, int uniarg, bool uniflag)
{
  SCM ok;
  thisflag = lastflag & FLAG_DEFINING_MACRO;

  /* Reset last_uniarg before function call, so recursion (e.g. in
     macros) works. */
  if (!(thisflag & FLAG_SET_UNIARG))
    last_uniarg = 1;

  /* Execute the command. */
  _this_command = proc;
  if (guile_procedure_takes_uniarg_integer (proc))
    {
      if (uniflag)
	ok = guile_call_procedure_with_long (proc, uniarg);
      else
	ok = guile_call_procedure (proc);
    }
  else if (guile_procedure_takes_uniarg_boolean (proc))
    {
      if (uniflag)
	ok = guile_call_procedure_with_boolean (proc, uniarg);
      else
	ok = guile_call_procedure (proc);
    }
  else
    ok = guile_call_procedure (proc);
  _last_command = _this_command;

  /* Only add keystrokes if we were already in macro defining mode
     before the function call, to cope with start-kbd-macro. */
  if (lastflag & FLAG_DEFINING_MACRO && thisflag & FLAG_DEFINING_MACRO)
    add_cmd_to_macro ();

  if (cur_bp && scm_is_eq (last_command (), F_undo ()))
    set_buffer_next_undop (cur_bp, get_buffer_last_undop (cur_bp));

  lastflag = thisflag;

  return ok;
}

void
get_and_run_command (void)
{
  gl_list_t keys = get_key_sequence ();
  SCM proc = get_function_by_keys (keys);

  minibuf_clear ();

  if (scm_is_true (proc))
    call_command (proc, last_uniarg, (lastflag & FLAG_SET_UNIARG) != 0);
  else
    minibuf_error ("%s is undefined", astr_cstr (keyvectodesc (keys)));
}

static Binding
init_bindings (void)
{
  return node_new (10);
}

#undef DEBUG_SET_KEY
static void
set_key (const char *keystr, const char *funcname)
{
  SCM funcsym;
  gl_list_t keys;
#ifdef DEBUG_SET_KEY
  static FILE *fp = NULL;
  if (fp == NULL)
    fp = fopen ("set_key.debug", "wt");
  fprintf(fp, "entering set key: %s %s\n", keystr, funcname);
  fflush (fp);
#endif

  funcsym = scm_from_locale_symbol (funcname);
  if (guile_symbol_is_name_of_defined_function (funcsym))
    {
      keys = keystrtovec (keystr);
#ifdef DEBUG_SET_KEY
      fprintf(fp, "keystr %s keys %p proc %p\n", keystr, keys,
	      guile_variable_ref_safe (guile_lookup_safe (funcsym)));
      fflush(fp);
#endif
      if (keys != NULL)
	bind_key_vec (root_bindings, keys, 0,
		      guile_variable_ref_safe (guile_lookup_safe (funcsym)));
    }
  else
    {
      char *msg;
      asprintf (&msg, "cannot set key '%s': no such procedure '%s'",
		keystr, funcname);
      minibuf_error (msg);
    }
}

void
init_default_bindings (void)
{
  root_bindings = init_bindings ();

  /* Bind all printing keys to self_insert_command */
  gl_list_t keys = gl_list_create_empty (GL_ARRAY_LIST,
                                         NULL, NULL, NULL, true);
  gl_list_add_last (keys, NULL);
  for (size_t i = 0; i <= 0xff; i++)
    {
      if (isprint (i))
        {
          gl_list_set_at (keys, 0, (void *) i);
          bind_key_vec (root_bindings, keys, 0, F_self_insert_command ());
        }
    }

  set_key ("\\M-m", "back-to-indentation");
  set_key ("\\LEFT", "backward-char");
  //set_key ("\\C-b", "backward-char");
  set_key ("\\BACKSPACE", "backward-delete-char");
  set_key ("\\M-\\BACKSPACE", "backward-kill-word");
  set_key ("\\M-{", "backward-paragraph");
  set_key ("\\C-\\M-b", "backward-sexp");
  set_key ("\\M-b", "backward-word");
  set_key ("\\M-<", "beginning-of-buffer");
  set_key ("\\HOME", "beginning-of-line");
  set_key ("\\C-a", "beginning-of-line");
  set_key ("\\C-xe", "call-last-kbd-macro");
  //set_key ("\\M-c", "capitalize-word");
  set_key ("\\M-w", "copy-region-as-kill");
  set_key ("\\C-xrs", "copy-to-register");
  set_key ("\\C-xrx", "copy-to-register");
  set_key ("\\C-x\\C-o", "delete-blank-lines");
  set_key ("\\DELETE", "delete-char");
  set_key ("\\C-d", "delete-char");
  set_key ("\\M-\\\\", "delete-horizontal-space");
  set_key ("\\C-x1", "delete-other-windows");
  set_key ("\\C-x0", "delete-window");
  //set_key ("\\C-hb", "describe-bindings");
  set_key ("\\C-b", "describe-bindings");
  set_key ("\\F1b", "describe-bindings");
  set_key ("\\C-hf", "describe-function");
  set_key ("\\F1f", "describe-function");
  set_key ("\\C-hk", "describe-key");
  set_key ("\\F1k", "describe-key");
  set_key ("\\C-hv", "describe-variable");
  set_key ("\\F1v", "describe-variable");
  set_key ("\\C-x\\C-l", "downcase-region");
  set_key ("\\M-l", "downcase-word");
  set_key ("\\C-x|", "end-kbd-macro");
  set_key ("\\M->", "end-of-buffer");
  set_key ("\\END", "end-of-line");
  set_key ("\\C-e", "end-of-line");
  set_key ("\\C-x^", "enlarge-window");
  set_key ("\\C-x\\C-x", "exchange-point-and-mark");
  set_key ("\\M-x", "execute-extended-command");
  set_key ("\\M-q", "fill-paragraph");
  set_key ("\\C-x\\C-v", "find-alternate-file");
  set_key ("\\C-x\\C-f", "find-file");
  set_key ("\\C-x\\C-r", "find-file-read-only");
  set_key ("\\RIGHT", "forward-char");
  set_key ("\\C-f", "forward-char");
  set_key ("\\M-}", "forward-paragraph");
  set_key ("\\C-\\M-f", "forward-sexp");
  set_key ("\\M-f", "forward-word");
  set_key ("\\M-gg", "goto-line");
  set_key ("\\M-g\\M-g", "goto-line");
  set_key ("\\TAB", "indent-for-tab-command");
  set_key ("\\C-xi", "insert-file");
  set_key ("\\C-xri", "insert-register");
  set_key ("\\C-xrg", "insert-register");
  set_key ("\\C-r", "isearch-backward");
  set_key ("\\C-\\M-r", "isearch-backward-regexp");
  set_key ("\\C-s", "isearch-forward");
  set_key ("\\C-\\M-s", "isearch-forward-regexp");
  set_key ("\\M-\\SPC", "just-one-space");
  set_key ("\\C-g", "keyboard-quit");
  set_key ("\\C-xk", "kill-buffer");
  set_key ("\\C-k", "kill-line");
  set_key ("\\C-w", "kill-region");
  set_key ("\\C-\\M-k", "kill-sexp");
  set_key ("\\M-d", "kill-word");
  set_key ("\\C-x\\C-b", "list-buffers");
  set_key ("\\M-h", "mark-paragraph");
  set_key ("\\C-\\M-@", "mark-sexp");
  set_key ("\\C-xh", "mark-whole-buffer");
  set_key ("\\M-@", "mark-word");
  set_key ("\\RET", "znewline");
  set_key ("\\C-j", "newline-and-indent");
  set_key ("\\DOWN", "next-line");
  set_key ("\\C-n", "next-line");
  set_key ("\\C-o", "open-line");
  set_key ("\\C-xo", "other-window");
  set_key ("\\UP", "previous-line");
  set_key ("\\C-p", "previous-line");
  set_key ("\\M-%", "query-replace");
  set_key ("\\C-q", "quoted-insert");
  set_key ("\\C-l", "recenter");
  set_key ("\\C-x\\C-s", "save-buffer");
  set_key ("\\C-x\\C-c", "save-buffers-kill-emacs");
  set_key ("\\C-xs", "save-some-buffers");
  set_key ("\\PRIOR", "scroll-down");
  set_key ("\\M-v", "scroll-down");
  set_key ("\\NEXT", "scroll-up");
  set_key ("\\C-v", "scroll-up");
  set_key ("\\C-xf", "set-fill-column");
  set_key ("\\C-@", "set-mark-command");
  set_key ("\\M-!", "shell-command");
  set_key ("\\M-|", "shell-command-on-region");
  set_key ("\\C-x2", "split-window");
  set_key ("\\C-x(", "start-kbd-macro");
  set_key ("\\C-x\\C-z", "suspend-emacs");
  set_key ("\\C-z", "suspend-emacs");
  set_key ("\\C-xb", "switch-to-buffer");
  set_key ("\\M-i", "tab-to-tab-stop");
  set_key ("\\C-x\\C-q", "toggle-read-only");
  set_key ("\\C-t", "transpose-chars");
  set_key ("\\C-x\\C-t", "transpose-lines");
  set_key ("\\C-\\M-t", "transpose-sexps");
  set_key ("\\M-t", "transpose-words");
  set_key ("\\C-xu", "undo");
  set_key ("\\C-_", "undo");
  set_key ("\\C-u", "universal-argument");
  set_key ("\\C-x\\C-u", "upcase-region");
  set_key ("\\M-u", "upcase-word");
  set_key ("\\C-hw", "where-is");
  set_key ("\\F1w", "where-is");
  set_key ("\\C-x\\C-w", "write-file");
  set_key ("\\C-y", "yank");
  set_key ("\\M-c", "console");

}

SCM_DEFINE (G_set_key, "set-key", 0, 2, 0,
	    (SCM gkeystr, SCM gname), "\
Bind a command to a key sequence.\n\
Read key sequence and function name (a symbol), and bind the function to the key\n\
sequence.")
{
  const char *keystr;
  astr keyname;
  SCM ok = SCM_BOOL_T;
  gl_list_t keys;

  if (interactive && SCM_UNBNDP (gkeystr))
    {
      minibuf_write ("Set key: ");
      keys = get_key_sequence ();
      keyname = keyvectodesc (keys);
      keystr = astr_cstr (keyname);
    }
  else if (!interactive && SCM_UNBNDP (gkeystr))
    {
      guile_wrong_number_of_arguments_error ("set-key");
      return SCM_UNSPECIFIED;
    }
  else if (scm_is_string (gkeystr))
    {
      keystr = scm_to_locale_string (gkeystr);
      keys = keystrtovec (keystr);
      if (keys == NULL)
	guile_error ("set-key", "invalid escape character syntax");
    }
  else
    guile_wrong_type_argument_error ("set-key", SCM_ARG1, gkeystr, "string");

  if (interactive && SCM_UNBNDP (gname))
    {
      const astr name = minibuf_read_function_name ("Set key %s to command: ",
						    keystr);
      if (name == NULL)
	return SCM_BOOL_F;
      gname = scm_from_locale_symbol (astr_cstr (name));
    }
  else if (!interactive && SCM_UNBNDP (gname))
    guile_wrong_number_of_arguments_error ("set-key");
  else if (!scm_is_symbol (gname))
    guile_wrong_type_argument_error ("set-key", SCM_ARG2, gname, "symbol");
  /* else
     gname is ok */

  if (!scm_is_true (guile_lookup_safe (gname)))
    guile_error ("set-key", "no such function");

  bind_key_vec (root_bindings, keys, 0,
		guile_variable_ref_safe (guile_lookup_safe (gname)));

  return ok;
}

static void
walk_bindings_tree (Binding tree, gl_list_t keys,
                    void (*process) (astr key, Binding p, void *st), void *st)
{
  for (size_t i = 0; i < tree->vecnum; ++i)
    {
      Binding p = tree->vec[i];
      if (scm_is_true (scm_procedure_p (p->proc)))
        {
          astr key = astr_new ();
          for (size_t j = 0; j < gl_list_size (keys); j++)
            {
              astr_cat (key, (castr) gl_list_get_at (keys, j));
              astr_cat_char (key, ' ');
            }
          astr_cat (key, chordtodesc (p->key));
          process (key, p, st);
        }
      else
        {
          gl_list_add_last (keys, chordtodesc (p->key));
          walk_bindings_tree (p, keys, process, st);
          assert (gl_list_remove_at (keys, gl_list_size (keys) - 1));
        }
    }
}

static void
walk_bindings (Binding tree, void (*process) (astr key, Binding p, void *st),
               void *st)
{
  walk_bindings_tree (tree, gl_list_create_empty (GL_LINKED_LIST,
                                                  NULL, NULL, NULL, true), process, st);
}

typedef struct
{
  SCM f;
  astr bindings;
} gather_bindings_state;

static void
gather_bindings (astr key, Binding p, void *st)
{
  gather_bindings_state *g = (gather_bindings_state *) st;

  if (scm_is_eq (p->proc, g->f))
    {
      if (astr_len (g->bindings) > 0)
        astr_cat_cstr (g->bindings, ", ");
      astr_cat (g->bindings, key);
    }
}

SCM_DEFINE (G_where_is, "where-is", 0, 1, 0, (SCM sym), "\
Print message listing key sequences that invoke the command DEFINITION.\n\
Argument is a command name.")
{
  castr name;
  gather_bindings_state g;


  if (!interactive && SCM_UNBNDP (sym))
    guile_wrong_number_of_arguments_error ("where-is");
  else if (!interactive && !scm_is_symbol (sym))
    guile_wrong_type_argument_error ("where-is", SCM_ARG1, sym, "symbol");
  else if (!interactive && scm_is_symbol (sym))
    {
      char *csym = scm_to_locale_string (scm_symbol_to_string (sym));
      name = astr_new_cstr (csym);
      free (csym);
    }
  else
    name = minibuf_read_function_name ("Where is command: ");

  if (name && astr_cstr (name))
    {
      g.f = scm_from_locale_symbol (astr_cstr (name));
      if (g.f)
        {
          g.bindings = astr_new ();
          walk_bindings (root_bindings, gather_bindings, &g);

          if (astr_len (g.bindings) == 0)
	    if (interactive)
	      minibuf_write ("%s is not on any key", astr_cstr (name));
	    else
	      return scm_from_locale_string ("");
          else
	    if (interactive)
	      minibuf_write ("%s is on %s", astr_cstr (name), astr_cstr (g.bindings));
	    else
	      return scm_from_locale_string (astr_cstr (g.bindings));
        }
    }
  return SCM_BOOL_F;
}

static void
print_binding (astr key, Binding p, void *st _GL_UNUSED_PARAMETER)
{
  SCM name = guile_procedure_name_safe (p->proc);
  bprintf ("%-15s %s\n", astr_cstr (key),
	   scm_to_locale_string (scm_symbol_to_string (name)));
}

static void
write_bindings_list (va_list ap _GL_UNUSED_PARAMETER)
{
  bprintf ("Key translations:\n");
  bprintf ("%-15s %s\n", "key", "binding");
  bprintf ("%-15s %s\n", "---", "-------");

  walk_bindings (root_bindings, print_binding, NULL);
}

SCM_DEFINE (G_describe_bindings, "describe-bindings", 0, 0, 0, (void), "\
Show a list of all defined keys, and their definitions.")
{
  if (interactive)
    write_temp_buffer ("*Help*", true, write_bindings_list);
  else
    guile_error ("describe-bindings",
		 "this function cannot be called from the REPL");
  return SCM_BOOL_T;
}

static void
display_binding (astr key, Binding p, void *st _GL_UNUSED_PARAMETER)
{
  char *buf, *str = NULL;
  SCM name;

  name = guile_procedure_name_safe (p->proc);
  if (scm_is_true (name))
    str = guile_to_locale_string_safe (scm_symbol_to_string (name));
  if (str)
    asprintf (&buf, "%-15s %s\n", astr_cstr (key), str);
  else
    asprintf (&buf, "%-15s <unknown>\n", astr_cstr (key));

  scm_simple_format (scm_current_output_port (),
		     scm_from_locale_string ("~A"),
		     scm_list_1 (scm_from_locale_string (buf)));;
  free (buf);
}

static void
display_bindings_list (void)
{
  char *buf;
  asprintf (&buf, "Key translations:\n");
  scm_simple_format (scm_current_output_port(), scm_from_locale_string (buf),
		     SCM_EOL);
  free (buf);
  asprintf (&buf, "%-15s %s\n", "key", "binding");
  scm_simple_format (scm_current_output_port(), scm_from_locale_string (buf),
		     SCM_EOL);
  free (buf);
  asprintf (&buf, "%-15s %s\n", "---", "-------");
  scm_simple_format (scm_current_output_port(), scm_from_locale_string (buf),
		     SCM_EOL);
  free (buf);

  walk_bindings (root_bindings, display_binding, NULL);
}

SCM_DEFINE (G_key_map, "key-map", 0, 0, 0, (void), "\
A REPL-only func[tion to print the key map.")
{
  if (!interactive)
    display_bindings_list ();
  else
    G_describe_bindings ();
  return SCM_BOOL_T;
}

void
init_guile_bind_procedures ()
{
#include "bind.x"
  scm_c_export ("self-insert-command",
		"where-is",
		"set-key",
		"describe-bindings",
		"key-map",
		NULL);
}
