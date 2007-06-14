/* Registers facility functions
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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#include "config.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "zile.h"
#include "extern.h"

#define NUM_REGISTERS	256

static struct {
  char	*text;
  size_t	size;
} regs[NUM_REGISTERS];

DEFUN("copy-to-register", copy_to_register)
/*+
Copy region into the user specified register.
+*/
{
  Region r;
  char *p;
  int reg;

  minibuf_write("Copy to register: ");
  term_refresh();
  if ((reg = getkey()) == KBD_CANCEL)
    return cancel();
  minibuf_clear();
  reg %= NUM_REGISTERS;

  if (warn_if_no_mark())
    return FALSE;

  calculate_the_region(&r);

  p = copy_text_block(r.start.n, r.start.o, r.size);
  if (regs[reg].text != NULL)
    free(regs[reg].text);
  regs[reg].text = p;
  regs[reg].size = r.size;

  return TRUE;
}
END_DEFUN

static void insert_register(int reg)
{
  undo_save(UNDO_REMOVE_BLOCK, cur_bp->pt, regs[reg].size, 0);
  undo_nosave = TRUE;
  insert_nstring(regs[reg].text, regs[reg].size);
  undo_nosave = FALSE;
}

DEFUN("insert-register", insert_register)
/*+
Insert contents of the user specified register.
Puts point before and mark after the inserted text.
+*/
{
  int reg, uni;

  if (warn_if_readonly_buffer())
    return FALSE;

  minibuf_write("Insert register: ");
  term_refresh();
  if ((reg = getkey()) == KBD_CANCEL)
    return cancel();
  minibuf_clear();
  reg %= NUM_REGISTERS;

  if (regs[reg].text == NULL) {
    minibuf_error("Register does not contain text");
    return FALSE;
  }

  set_mark_command();

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  for (uni = 0; uni < last_uniarg; ++uni)
    insert_register(reg);
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  exchange_point_and_mark();
  deactivate_mark();

  return TRUE;
}
END_DEFUN

static void write_registers_list(va_list ap)
{
  size_t i, count;

  (void)ap;
  bprintf("%-8s %8s\n", "Register", "Size");
  bprintf("%-8s %8s\n", "--------", "----");
  for (i = count = 0; i < NUM_REGISTERS; ++i)
    if (regs[i].text != NULL) {
      astr as = astr_new();
      ++count;
      if (isprint(i))
        astr_afmt(as, "`%c'", i);
      else
        astr_afmt(as, "`\\%o'", i);
      bprintf("%-8s %8d\n", astr_cstr(as), regs[i].size);
      astr_delete(as);
    }
  if (!count)
    bprintf("No registers defined\n");
}

DEFUN("list-registers", list_registers)
/*+
List defined registers.
+*/
{
  write_temp_buffer("*Registers List*", write_registers_list);
  return TRUE;
}
END_DEFUN

void free_registers(void)
{
  int i;
  for (i = 0; i < NUM_REGISTERS; ++i)
    if (regs[i].text != NULL)
      free(regs[i].text);
}
