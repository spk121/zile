/* Registers facility functions

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.

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

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "zile.h"
#include "extern.h"

#define NUM_REGISTERS	256

static struct
{
  char *text;
  size_t size;
} regs[NUM_REGISTERS];

DEFUN ("copy-to-register", copy_to_register)
/*+
Copy region into the user specified register.
+*/
{
  Region r;
  char *p;
  int reg;

  minibuf_write ("Copy to register: ");
  term_refresh ();
  reg = getkey ();
  if (reg == KBD_CANCEL)
    return FUNCALL (keyboard_quit);
  minibuf_clear ();
  reg %= NUM_REGISTERS;

  if (warn_if_no_mark ())
    return leNIL;

  calculate_the_region (&r);

  p = copy_text_block (r.start.n, r.start.o, r.size);
  free (regs[reg].text);
  regs[reg].text = p;
  regs[reg].size = r.size;

  return leT;
}
END_DEFUN

static int reg;

static int
insert_register (void)
{
  undo_save (UNDO_REMOVE_BLOCK, cur_bp->pt, regs[reg].size, 0);
  undo_nosave = true;
  insert_nstring (regs[reg].text, regs[reg].size);
  undo_nosave = false;
  return true;
}

DEFUN ("insert-register", insert_register)
/*+
Insert contents of the user specified register.
Puts point before and mark after the inserted text.
+*/
{
  if (warn_if_readonly_buffer ())
    return leNIL;

  minibuf_write ("Insert register: ");
  term_refresh ();
  reg = getkey ();
  if (reg == KBD_CANCEL)
    return FUNCALL (keyboard_quit);
  minibuf_clear ();
  reg %= NUM_REGISTERS;

  if (regs[reg].text == NULL)
    {
      minibuf_error ("Register does not contain text");
      return leNIL;
    }

  set_mark_interactive ();
  execute_with_uniarg (true, uniarg, insert_register, NULL);
  exchange_point_and_mark ();
  deactivate_mark ();

  return leT;
}
END_DEFUN

static void
write_registers_list (va_list ap GCC_UNUSED)
{
  size_t i, count;

  bprintf ("%-8s %8s\n", "Register", "Size");
  bprintf ("%-8s %8s\n", "--------", "----");
  for (i = count = 0; i < NUM_REGISTERS; ++i)
    if (regs[i].text != NULL)
      {
	astr as = astr_new ();
	++count;
	if (isprint (i))
	  astr_afmt (as, "`%c'", i);
	else
	  astr_afmt (as, "`\\%o'", i);
	bprintf ("%-8s %8d\n", astr_cstr (as), regs[i].size);
	astr_delete (as);
      }
  if (!count)
    bprintf ("No registers defined\n");
}

DEFUN ("list-registers", list_registers)
/*+
List defined registers.
+*/
{
  write_temp_buffer ("*Registers List*", write_registers_list);
  return leT;
}
END_DEFUN

void
free_registers (void)
{
  int i;
  for (i = 0; i < NUM_REGISTERS; ++i)
    free (regs[i].text);
}
