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

/*	$Id: macro.c,v 1.9 2005/01/09 23:56:05 rrt Exp $	*/

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
  Function func;
  int ndata;
  int *data;
  Macro *next;
};

static Macro *head_mp, *last_mp;
static Macro *running_mp, *wait_mp = NULL;
static int macro_key;

static Macro *macro_new(void)
{
  Macro *mp;

  mp = (Macro *)zmalloc(sizeof(Macro));
  mp->func = NULL;
  mp->ndata = 0;
  mp->data = NULL;
  mp->next = NULL;

  return mp;
}

static void macro_free(Macro *mp)
{
  if (mp->data)
    free(mp->data);
  free(mp);
}

static void add_macro_data(Macro *mp, int data)
{
  int n = mp->ndata++;
  mp->data = zrealloc(mp->data, sizeof(int)*mp->ndata);
  mp->data[n] = data;
}

void cancel_kbd_macro(void)
{
  Macro *mp, *next_mp;

  for (mp = head_mp; mp != NULL; mp = next_mp) {
    next_mp = mp->next;
    macro_free(mp);
  }

  head_mp = last_mp = NULL;

  thisflag &= ~FLAG_DEFINING_MACRO;
}

void add_kbd_macro(Function func, int set_uniarg, int uniarg)
{
  Macro *mp = macro_new();

  mp->func = func;
  add_macro_data(mp, set_uniarg);
  add_macro_data(mp, uniarg);

  if (wait_mp) {
    int c;
    for (c = 0; c < wait_mp->ndata; ++c)
      add_macro_data(mp, wait_mp->data[c]);
    macro_free(wait_mp);
    wait_mp = NULL;
  }

  if (head_mp == NULL)
    head_mp = last_mp = mp;
  else {
    last_mp->next = mp;
    last_mp = mp;
  }
}

void add_macro_key_data(int key)
{
  if (!wait_mp)
    wait_mp = macro_new();
  add_macro_data(wait_mp, key);
}

int get_macro_key_data(void)
{
  if (macro_key < running_mp->ndata)
    return running_mp->data[macro_key++];
  return 0;
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

  if (head_mp != NULL)
    cancel_kbd_macro();

  minibuf_write("Defining keyboard macro...");

  thisflag |= FLAG_DEFINING_MACRO;

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
  Macro *mp;

  if (head_mp == NULL) {
    minibuf_error("Keyboard macro not defined");
    return FALSE;
  }

  for (mp = head_mp; mp != NULL; mp = mp->next) {
    thisflag = FLAG_EXECUTING_MACRO;

    running_mp = mp;
    macro_key = 0;

    if (get_macro_key_data())
      lastflag |= FLAG_SET_UNIARG;
    else
      lastflag &= ~FLAG_SET_UNIARG;

    (*mp->func)(get_macro_key_data());

    thisflag &= ~FLAG_EXECUTING_MACRO;

    if (lastflag & FLAG_GOT_ERROR) {
      ret = FALSE;
      break;
    }

    lastflag = thisflag;
  }

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
  Macro *mp, *next;

  for (mp = head_mp; mp != NULL; mp = next) {
    next = mp->next;
    macro_free(mp);
  }
}
