/* Macro facility functions
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

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

/*	$Id: macro.c,v 1.11 2005/01/30 17:46:16 rrt Exp $	*/

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"


typedef struct Macro Macro;

struct Macro {
  size_t nkeys;
  size_t *keys;
};

static Macro *head_mp;

void add_macro_key(size_t key)
{
  head_mp->keys = zrealloc(head_mp->keys, sizeof(size_t) * ++head_mp->nkeys);
  head_mp->keys[head_mp->nkeys - 1] = key;
}

void cancel_kbd_macro(void)
{
  free_macros();
  head_mp = NULL;
  thisflag &= ~FLAG_DEFINING_MACRO;
}

DEFUN("start-kbd-macro", start_kbd_macro)
  /*+
    Record subsequent keyboard input, defining a keyboard macro.
    The commands are recorded even as they are executed.
    Use C-x ) to finish recording and make the macro available.
    +*/
{
  if (thisflag & FLAG_DEFINING_MACRO) {
    minibuf_error("Already defining a keyboard macro");
    return FALSE;
  }

  if (head_mp)
    cancel_kbd_macro();

  minibuf_write("Defining keyboard macro...");

  thisflag |= FLAG_DEFINING_MACRO;
  head_mp = (Macro *)zmalloc(sizeof(Macro));
  return TRUE;
}

DEFUN("end-kbd-macro", end_kbd_macro)
  /*+
    Finish defining a keyboard macro.
    The definition was started by C-x (.
    The macro is now available for use via C-x e.
    +*/
{
  if (!(thisflag & FLAG_DEFINING_MACRO)) {
    minibuf_error("Not defining a keyboard macro");
    return FALSE;
  }

  thisflag &= ~FLAG_DEFINING_MACRO;
  return TRUE;
}

DEFUN("name-last-kbd-macro", name_last_kbd_macro)
  /*+
    Bind the last keyboard macro to a command with the given name.
    +*/
{
  /* XXX */
  return FALSE;
}

static int call_last_kbd_macro(void)
{
  int old_thisflag = thisflag;
  int old_lastflag = lastflag;
  int ret = TRUE;
  size_t i;

  if (head_mp == NULL) {
    minibuf_error("Keyboard macro not defined");
    return FALSE;
  }

  for (i = head_mp->nkeys - 1; i < head_mp->nkeys ; i--)
    term_ungetkey(head_mp->keys[i]);

  if (lastflag & FLAG_GOT_ERROR)
    ret = FALSE;

  thisflag = old_thisflag;
  lastflag = old_lastflag;

  return ret;
}

DEFUN("call-last-kbd-macro", call_last_kbd_macro)
  /*+
    Call the last keyboard macro that you defined with C-x (.
    A prefix argument serves as a repeat count.  Zero means repeat until error.
    +*/
{
  int uni, ret = TRUE;

  /*	undo_save(UNDO_START_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0); */
  if (uniarg == 0) {
    for (;;)
      if (!call_last_kbd_macro())
        break;
  } else {
    for (uni = 0; uni < uniarg; ++uni)
      if (!call_last_kbd_macro()) {
        ret = FALSE;
        break;
      }
  }
  /*	undo_save(UNDO_END_SEQUENCE, cur_wp->pointn, cur_wp->pointo, 0, 0); */

  return ret;
}

/*
 * Free all the allocated macros (used at Zile exit).
 */
void free_macros(void)
{
  if (head_mp) {
    free(head_mp->keys);
    free(head_mp);
  }
}
