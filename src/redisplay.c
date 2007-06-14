/* Terminal independent redisplay routines
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

#include <stdarg.h>

#include "config.h"
#include "zile.h"
#include "extern.h"

void resync_redisplay(void)
{
  int delta = cur_bp->pt.n - cur_wp->lastpointn;

  if (delta) {
    if ((delta > 0 && cur_wp->topdelta + delta < cur_wp->eheight) ||
        (delta < 0 && cur_wp->topdelta >= (size_t)(-delta)))
      cur_wp->topdelta += delta;
    else if (cur_bp->pt.n > cur_wp->eheight / 2)
      cur_wp->topdelta = cur_wp->eheight / 2;
    else
      cur_wp->topdelta = cur_bp->pt.n;
  }
  cur_wp->lastpointn = cur_bp->pt.n;
}

void resize_windows(void)
{
  Window *wp;
  int hdelta;

  /* Resize windows horizontally. */
  for (wp = head_wp; wp != NULL; wp = wp->next)
    wp->fwidth = wp->ewidth = term_width();

  /* Work out difference in window height; windows may be taller than
     terminal if the terminal was very short. */
  for (hdelta = term_height() - 1, wp = head_wp;
       wp != NULL;
       hdelta -= wp->fheight, wp = wp->next);

  /* Resize windows vertically. */
  if (hdelta > 0) { /* Increase windows height. */
    for (wp = head_wp; hdelta > 0; wp = wp->next) {
      if (wp == NULL)
        wp = head_wp;
      ++wp->fheight;
      ++wp->eheight;
      --hdelta;
    }
  } else { /* Decrease windows' height, and close windows if necessary. */
    int decreased = TRUE;
    while (decreased) {
      decreased = FALSE;
      for (wp = head_wp; wp != NULL && hdelta < 0; wp = wp->next) {
        if (wp->fheight > 2) {
          --wp->fheight;
          --wp->eheight;
          ++hdelta;
          decreased = TRUE;
        } else if (cur_wp != head_wp || cur_wp->next != NULL) {
          Window *new_wp = wp->next;
          delete_window(wp);
          wp = new_wp;
          decreased = TRUE;
        }
      }
    }
  }

  FUNCALL(recenter);
}

void recenter(Window *wp)
{
  Point pt = window_pt(wp);

  if (pt.n > wp->eheight / 2)
    wp->topdelta = wp->eheight / 2;
  else
    wp->topdelta = pt.n;
}

DEFUN("recenter", recenter)
/*+
Center point in window and redisplay screen.
The desired position of point is always relative to the current window.
+*/
{
  recenter(cur_wp);
  term_full_redisplay();
  return TRUE;
}
END_DEFUN
