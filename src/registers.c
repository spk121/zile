/* Registers facility functions

   Copyright (c) 2008, 2009 Free Software Foundation, Inc.

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

#include "main.h"
#include "extern.h"

#define NUM_REGISTERS	256

static astr regs[NUM_REGISTERS];

static void
register_free (size_t n)
{
  if (regs[n] != NULL)
    astr_delete (regs[n]);
}

DEFUN_ARGS ("copy-to-register", copy_to_register,
            STR_ARG (regchar))
/*+
Copy region into the user specified register.
+*/
{
  int reg = 0;

  STR_INIT (regchar)
  else
    {
      minibuf_write ("Copy to register: ");
      reg = getkey ();
    }
  if (regchar != NULL)
    reg = *regchar;

  if (reg == KBD_CANCEL)
    ok = FUNCALL (keyboard_quit);
  else
    {
      Region * rp = region_new ();

      minibuf_clear ();
      if (reg < 0)
        reg = 0;
      reg %= NUM_REGISTERS;

      if (!calculate_the_region (rp))
        ok = leNIL;
      else
        {
          char *p = copy_text_block (get_region_start (rp).n, get_region_start (rp).o, get_region_size (rp));
          register_free ((size_t) reg);
          regs[reg] = astr_new_cstr (p);
          free (p);
        }

      free (rp);
    }

  STR_FREE (regchar);
}
END_DEFUN

static int reg;

static bool
insert_register (void)
{
  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), 0, astr_len (regs[reg]));
  undo_nosave = true;
  insert_astr (regs[reg]);
  undo_nosave = false;
  return true;
}

DEFUN_ARGS ("insert-register", insert_register,
            STR_ARG (regchar))
/*+
Insert contents of the user specified register.
Puts point before and mark after the inserted text.
+*/
{
  if (warn_if_readonly_buffer ())
    return leNIL;

  STR_INIT (regchar)
  else
    {
      minibuf_write ("Insert register: ");
      reg = getkey ();
    }
  if (regchar != NULL)
    reg = *regchar;

  if (reg == KBD_CANCEL)
    ok = FUNCALL (keyboard_quit);
  else
    {
      minibuf_clear ();
      reg %= NUM_REGISTERS;

      if (regs[reg] == NULL)
        {
          minibuf_error ("Register does not contain text");
          ok = leNIL;
        }
      else
        {
          set_mark_interactive ();
          execute_with_uniarg (true, uniarg, insert_register, NULL);
          FUNCALL (exchange_point_and_mark);
          deactivate_mark ();
        }
    }

  STR_FREE (regchar);
}
END_DEFUN

static void
write_registers_list (va_list ap GCC_UNUSED)
{
  size_t i, count;

  bprintf ("%-8s %8s\n", "Register", "Size");
  bprintf ("%-8s %8s\n", "--------", "----");
  for (i = count = 0; i < NUM_REGISTERS; ++i)
    if (regs[i] != NULL)
      {
        astr as = astr_new ();
        ++count;
        if (isprint (i))
          astr_afmt (as, "`%c'", i);
        else
          astr_afmt (as, "`\\%o'", i);
        bprintf ("%-8s %8d\n", astr_cstr (as), astr_len (regs[i]));
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
  write_temp_buffer ("*Registers List*", true, write_registers_list);
}
END_DEFUN

void
free_registers (void)
{
  size_t i;
  for (i = 0; i < NUM_REGISTERS; ++i)
    register_free (i);
}
