/* History facility functions

   Copyright (c) 2004, 2008, 2010-2011 Free Software Foundation, Inc.

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

#include <stdlib.h>
#include "gl_linked_list.h"

#include "main.h"
#include "extern.h"

struct History
{
  gl_list_t elements;		/* Elements (strings). */
  ptrdiff_t sel;		/* Selected element. */
};

History *history_new (void)
{
  return XZALLOC (History);
}

void
add_history_element (History * hp, const char *string)
{
  const char *last = NULL;

  if (!hp->elements)
    hp->elements = gl_list_create_empty (GL_LINKED_LIST,
                                         NULL, NULL, NULL, true);
  else
    last = (const char *) gl_list_get_at (hp->elements, gl_list_size (hp->elements) - 1);
  if (!last || !STREQ (last, string))
    gl_list_add_last (hp->elements, xstrdup (string));
}

void
prepare_history (History * hp)
{
  hp->sel = -1;
}

const char *
previous_history_element (History * hp)
{
  const char *s = NULL;

  if (hp->elements)
    {
      /* First time that we use `previous-history-element'. */
      if (hp->sel == -1)
        {
          /* Select last element. */
          if (gl_list_size (hp->elements) > 0)
            {
              hp->sel = gl_list_size (hp->elements) - 1;
              s = (const char *) gl_list_get_at (hp->elements, hp->sel);
            }
        }
      /* Is there another element? */
      else if (hp->sel > 0)
        {
          /* Select it. */
          hp->sel--;
          s = (const char *) gl_list_get_at (hp->elements, hp->sel);
        }
    }

  return s;
}

const char *
next_history_element (History * hp)
{
  const char *s = NULL;

  if (hp->elements && hp->sel != -1)
    {
      /* Next element. */
      if (hp->sel < (ptrdiff_t) (gl_list_size (hp->elements) - 1))
        {
          hp->sel++;
          s = (const char *) gl_list_get_at (hp->elements, hp->sel);
        }
      /* No more elements (back to original status). */
      else
        hp->sel = -1;
    }

  return s;
}
