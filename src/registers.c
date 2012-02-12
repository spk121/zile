/* Registers facility functions

   Copyright (c) 2001, 2003-2006, 2008-2011 Free Software Foundation, Inc.
   Copyright (c) 2012 Michael L. Gran

   This file is part of Michael Gran's unofficial port of GNU Zile.

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
#include <libguile.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"

#define NUM_REGISTERS	256

static estr regs[NUM_REGISTERS];

SCM_DEFINE (G_copy_to_register, "copy-to-register", 0, 1, 0,
	    (SCM greg), "\
Copy region into register.")
{
  long reg;
  SCM ok = SCM_BOOL_T;

  if (SCM_UNBNDP (greg)) 
    {
      minibuf_write ("Copy to register: ");
      reg = getkey (GETKEY_DEFAULT);
    }
  else 
    reg = guile_to_long_or_error ("copy-to-register", SCM_ARG1, greg);

  if (reg == KBD_CANCEL)
    ok = G_keyboard_quit ();
  else
    {
      minibuf_clear ();
      if (reg < 0)
        reg = 0;
      reg %= NUM_REGISTERS; /* Nice numbering relies on NUM_REGISTERS
                               being a power of 2. */

      if (warn_if_no_mark ())
        ok = SCM_BOOL_F;
      else
        regs[reg] = get_buffer_region (cur_bp, calculate_the_region ());
    }
  return ok;
}

static int regnum;

static bool
insert_register (void)
{
  insert_estr (regs[regnum]);
  return true;
}

SCM_DEFINE (G_insert_register, "insert-register", 0, 1, 0, 
	    (SCM greg), "\
Insert contents of the user specified register.\n\
Puts point before and mark after the inserted text.")
{
  SCM ok = SCM_BOOL_T;
  if (warn_if_readonly_buffer ())
    return SCM_BOOL_F;

  long reg = guile_to_long_or_error (s_G_insert_register, SCM_ARG1, greg);
  if (SCM_UNBNDP (greg))
    {
      minibuf_write ("Insert register: ");
      reg = getkey (GETKEY_DEFAULT);
    }

  if (reg == KBD_CANCEL)
    ok = G_keyboard_quit ();
  else
    {
      minibuf_clear ();
      reg %= NUM_REGISTERS;

      if (regs[reg].as == NULL)
        {
          minibuf_error ("Register does not contain text");
          ok = SCM_BOOL_F;
        }
      else
        {
          G_set_mark_command ();
          regnum = reg;
	  insert_register ();
          G_exchange_point_and_mark ();
          deactivate_mark ();
        }
    }
  return ok;
}

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

SCM_DEFINE (G_list_registers, "list-registers", 0, 0, 0, (void), "\
List defined registers.")
{
  write_temp_buffer ("*Registers List*", true, write_registers_list);
  return SCM_BOOL_T;
}

void
init_guile_registers_procedures (void)
{
#include "registers.x"
  scm_c_export ("copy-to-register",
		"insert-register",
		"list-registers",
		0);
}
