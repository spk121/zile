/* Macro facility functions

   Copyright (c) 1997-2005, 2008-2011 Free Software Foundation, Inc.
   Copyright (c) 2012 Michael L. Gran

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
#include <libguile.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "gl_array_list.h"

#include "main.h"
#include "extern.h"


struct Macro
{
  gl_list_t keys;	/* List of keystrokes. */
  char *name;		/* Name of the macro. */
  Macro *next;		/* Next macro in the list. */
};

static Macro *cur_mp = NULL, *cmd_mp = NULL;

static Macro *
macro_new (void)
{
  Macro * mp = XZALLOC (Macro);
  mp->keys = gl_list_create_empty (GL_ARRAY_LIST,
                                   NULL, NULL, NULL, true);
  return mp;
}

static void
add_macro_key (Macro * mp, size_t key)
{
  gl_list_add_last (mp->keys, (void *) key);
}

void
add_cmd_to_macro (void)
{
  assert (cmd_mp);
  for (size_t i = 0; i < gl_list_size (cmd_mp->keys); i++)
    add_macro_key (cur_mp, (size_t) gl_list_get_at (cmd_mp->keys, i));
  cmd_mp = NULL;
}

void
add_key_to_cmd (size_t key)
{
  if (cmd_mp == NULL)
    cmd_mp = macro_new ();

  add_macro_key (cmd_mp, key);
}

void
remove_key_from_cmd (void)
{
  assert (cmd_mp);
  gl_list_remove_at (cmd_mp->keys, gl_list_size (cmd_mp->keys) - 1);
}

void
cancel_kbd_macro (void)
{
  cmd_mp = cur_mp = NULL;
  thisflag &= ~FLAG_DEFINING_MACRO;
}

SCM_DEFINE (G_start_kbd_macro, "start-kbd-macro", 0, 0, 0, (void), "\
Record subsequent keyboard input, defining a keyboard macro.\n\
The commands are recorded even as they are executed.\n\
Use @kbd{C-x )} to finish recording and make the macro available.")
{
  if (thisflag & FLAG_DEFINING_MACRO)
    {
      minibuf_error ("Already defining a keyboard macro");
      return SCM_BOOL_F;
    }

  if (cur_mp)
    cancel_kbd_macro ();

  minibuf_write ("Defining keyboard macro...");

  thisflag |= FLAG_DEFINING_MACRO;
  cur_mp = macro_new ();
  return SCM_BOOL_T;
}

SCM_DEFINE (G_end_kbd_macro, "end-kbd-macro", 0, 0, 0, (void), "\
Finish defining a keyboard macro.\n\
The definition was started by 'C-x ('.\n\
The macro is now available for use via 'C-x e'.")
{
  if (!(thisflag & FLAG_DEFINING_MACRO))
    {
      minibuf_error ("Not defining a keyboard macro");
      return SCM_BOOL_F;
    }

  thisflag &= ~FLAG_DEFINING_MACRO;
  return SCM_BOOL_T;
}

static void
process_keys (gl_list_t keys)
{
  size_t len = gl_list_size (keys);
  size_t cur = term_buf_len ();
  for (size_t i = 0; i < len; i++)
    pushkey ((size_t) gl_list_get_at (keys, len - i - 1));

  undo_start_sequence ();
  while (term_buf_len () > cur)
    get_and_run_command ();
  undo_end_sequence ();
}

static gl_list_t macro_keys;

static bool
call_macro (void)
{
  process_keys (macro_keys);
  return true;
}

SCM_DEFINE (G_call_last_kbd_macro, "call-last-kbd-macro", 0, 1, 0, (SCM n), "\
Call the last keyboard macro that you defined with 'C-x ('.\n\
A prefix argument serves as a repeat count.")
{
  long uniarg = guile_to_long_or_error (s_G_call_last_kbd_macro, SCM_ARG1, n);
  if (cur_mp == NULL)
    {
      minibuf_error ("No kbd macro has been defined");
      return SCM_BOOL_F;
    }

  /* FIXME: Call execute-kbd-macro (needs a way to reverse keystrtovec) */
  /* F_execute_kbd_macro (uniarg, true, leAddDataElement (leNew (NULL), astr_cstr (keyvectostr (cur_mp->keys)), false)); */
  macro_keys = cur_mp->keys;
  execute_with_uniarg (true, uniarg, call_macro, NULL);
  return SCM_BOOL_T;
}

SCM_DEFINE (G_execute_kbd_macro, "execute-kbd-macro", 0, 1, 0,
	    (SCM gkeystr), "\
Execute macro as string of editor command characters.")
{
  SCM ok = SCM_BOOL_T;
  char *str = NULL;
  astr keystr = NULL;

  str = guile_to_locale_string_safe (gkeystr);
  if (str != NULL)
    keystr = astr_new_cstr (str);

  gl_list_t keys = keystrtovec (astr_cstr (keystr));
  if (keys)
    {
      macro_keys = keys;
      execute_with_uniarg (true, 1, call_macro, NULL);
    }
  else
    ok = SCM_BOOL_F;
  return ok;
}


void
init_guile_macro_procedures (void)
{
#include "macro.x"
  scm_c_export ("start-kbd-macro",
		"call-last-kbd-macro",
		"end-kbd-macro",
		"execute-kbd-macro",
		0);
}
