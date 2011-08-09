/* Registers facility functions

   Copyright (c) 2001, 2003-2006, 2008-2011 Free Software Foundation, Inc.

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

#include <config.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"

#define NUM_REGISTERS	256

static estr regs[NUM_REGISTERS];

DEFUN_ARGS ("copy-to-register", copy_to_register,
            INT_ARG (reg))
/*+
Copy region into register @i{register}.
+*/
{
  INT_INIT (reg)
  else
    {
      minibuf_write ("Copy to register: ");
      reg = getkey (GETKEY_DEFAULT);
    }

  if (reg == KBD_CANCEL)
    ok = FUNCALL (keyboard_quit);
  else
    {
      minibuf_clear ();
      if (reg < 0)
        reg = 0;
      reg %= NUM_REGISTERS; /* Nice numbering relies on NUM_REGISTERS
                               being a power of 2. */

      if (warn_if_no_mark ())
        ok = leNIL;
      else
        regs[reg] = get_buffer_region (cur_bp, calculate_the_region ());
    }
}
END_DEFUN

static int regnum;

static bool
insert_register (void)
{
  insert_estr (regs[regnum]);
  return true;
}

DEFUN_ARGS ("insert-register", insert_register,
            INT_ARG (reg))
/*+
Insert contents of the user specified register.
Puts point before and mark after the inserted text.
+*/
{
  if (warn_if_readonly_buffer ())
    return leNIL;

  INT_INIT (reg)
  else
    {
      minibuf_write ("Insert register: ");
      reg = getkey (GETKEY_DEFAULT);
    }

  if (reg == KBD_CANCEL)
    ok = FUNCALL (keyboard_quit);
  else
    {
      minibuf_clear ();
      reg %= NUM_REGISTERS;

      if (regs[reg].as == NULL)
        {
          minibuf_error ("Register does not contain text");
          ok = leNIL;
        }
      else
        {
          FUNCALL (set_mark_command);
          regnum = reg;
          execute_with_uniarg (true, uniarg, insert_register, NULL);
          FUNCALL (exchange_point_and_mark);
          deactivate_mark ();
        }
    }
}
END_DEFUN

static void
write_registers_list (va_list ap _GL_UNUSED_PARAMETER)
{
  for (size_t i = 0; i < NUM_REGISTERS; ++i)
    if (regs[i].as != NULL)
      {
        const char *s = astr_cstr (regs[i].as);
        while (*s == ' ' || *s == '\t' || *s == '\n')
          s++;
        size_t len = MIN (20, MAX (0, ((int) get_window_ewidth (cur_wp)) - 6)) + 1;

        bprintf ("Register %s contains ", astr_cstr (astr_fmt (isprint (i) ? "%c" : "\\%o", i)));
        if (strlen (s) > 0)
          bprintf ("text starting with\n    %.*s\n", len, s);
        else if (s != astr_cstr (regs[i].as))
          bprintf ("whitespace\n");
        else
          bprintf ("the empty string\n");
      }
}

DEFUN ("list-registers", list_registers)
/*+
List defined registers.
+*/
{
  write_temp_buffer ("*Registers List*", true, write_registers_list);
}
END_DEFUN
