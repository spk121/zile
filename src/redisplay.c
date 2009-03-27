/* Terminal independent redisplay routines

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2008, 2009 Free Software Foundation, Inc.

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

#include <stdarg.h>

#include "config.h"
#include "main.h"
#include "extern.h"

void
resync_redisplay (void)
{
  int delta = get_buffer_pt (cur_bp).n - get_window_lastpointn (cur_wp);

  if (delta)
    {
      if ((delta > 0 && get_window_topdelta (cur_wp) + delta < get_window_eheight (cur_wp)) ||
          (delta < 0 && get_window_topdelta (cur_wp) >= (size_t) (-delta)))
        set_window_topdelta (cur_wp, get_window_topdelta (cur_wp) + delta);
      else if (get_buffer_pt (cur_bp).n > get_window_eheight (cur_wp) / 2)
        set_window_topdelta (cur_wp, get_window_eheight (cur_wp) / 2);
      else
        set_window_topdelta (cur_wp, get_buffer_pt (cur_bp).n);
    }
  set_window_lastpointn (cur_wp, get_buffer_pt (cur_bp).n);
}

void
resize_windows (void)
{
  Window *wp;
  int hdelta;

  /* Resize windows horizontally. */
  for (wp = head_wp; wp != NULL; wp = get_window_next (wp))
    {
      set_window_fwidth (wp, term_width ());
      set_window_ewidth (wp, get_window_fwidth (wp));
    }

  /* Work out difference in window height; windows may be taller than
     terminal if the terminal was very short. */
  for (hdelta = term_height () - 1, wp = head_wp;
       wp != NULL; hdelta -= get_window_fheight (wp), wp = get_window_next (wp));

  /* Resize windows vertically. */
  if (hdelta > 0)
    { /* Increase windows height. */
      for (wp = head_wp; hdelta > 0; wp = get_window_next (wp))
        {
          if (wp == NULL)
            wp = head_wp;
          set_window_fheight (wp, get_window_fheight (wp) + 1);
          set_window_eheight (wp, get_window_eheight (wp) + 1);
          --hdelta;
        }
    }
  else
    { /* Decrease windows' height, and close windows if necessary. */
      int decreased = true;
      while (decreased)
        {
          decreased = false;
          for (wp = head_wp; wp != NULL && hdelta < 0; wp = get_window_next (wp))
            {
              if (get_window_fheight (wp) > 2)
                {
                  set_window_fheight (wp, get_window_fheight (wp) - 1);
                  set_window_eheight (wp, get_window_eheight (wp) - 1);
                  ++hdelta;
                  decreased = true;
                }
              else if (cur_wp != head_wp || get_window_next (cur_wp) != NULL)
                {
                  Window *new_wp = get_window_next (wp);
                  delete_window (wp);
                  wp = new_wp;
                  decreased = true;
                }
            }
        }
    }

  FUNCALL (recenter);
}

void
recenter (Window * wp)
{
  Point pt = window_pt (wp);

  if (pt.n > get_window_eheight (wp) / 2)
    set_window_topdelta (wp, get_window_eheight (wp) / 2);
  else
    set_window_topdelta (wp, pt.n);
}

DEFUN ("recenter", recenter)
/*+
Center point in window and redisplay screen.
The desired position of point is always relative to the current window.
+*/
{
  recenter (cur_wp);
  term_full_redisplay ();
}
END_DEFUN
