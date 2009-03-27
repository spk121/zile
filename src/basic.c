/* Basic movement functions

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

#include "config.h"

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

/* Goal-column to arrive when `prev/next-line' functions are used.  */
static int cur_goalc;

DEFUN ("beginning-of-line", beginning_of_line)
/*+
Move point to beginning of current line.
+*/
{
  cur_bp->pt = line_beginning_position (uniarg);

  /* Change the `goalc' to the beginning of line for next
     `prev/next-line' calls.  */
  thisflag |= FLAG_DONE_CPCN;
  cur_goalc = 0;
}
END_DEFUN

DEFUN ("end-of-line", end_of_line)
/*+
Move point to end of current line.
+*/
{
  cur_bp->pt = line_end_position (uniarg);

  /* Change the `goalc' to the end of line for next
     `prev/next-line' calls.  */
  thisflag |= FLAG_DONE_CPCN;
  cur_goalc = INT_MAX;
}
END_DEFUN

/*
 * Get the goal column.  Take care of expanding tabulations.
 */
size_t
get_goalc_bp (Buffer * bp, Point pt)
{
  size_t col = 0, t = tab_width (bp), i;
  const char *sp = astr_cstr (pt.p->text);

  for (i = 0; i < pt.o; i++)
    {
      if (sp[i] == '\t')
        col |= t - 1;
      ++col;
    }

  return col;
}

size_t
get_goalc (void)
{
  return get_goalc_bp (cur_bp, cur_bp->pt);
}

/*
 * Go to the column `goalc'.  Take care of expanding
 * tabulations.
 */
static void
goto_goalc (int goalc)
{
  int col = 0, t = tab_width (cur_bp), w;
  size_t i;
  const char *sp = astr_cstr (cur_bp->pt.p->text);

  for (i = 0; i < astr_len (cur_bp->pt.p->text); i++)
    {
      if (col == goalc)
        break;
      else if (sp[i] == '\t')
        {
          for (w = t - col % t; w > 0; w--)
            if (++col == goalc)
              break;
        }
      else
        ++col;
    }

  cur_bp->pt.o = i;
}

int
previous_line (void)
{
  if (cur_bp->pt.p->prev != cur_bp->lines)
    {
      thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;

      if (!(lastflag & FLAG_DONE_CPCN))
        cur_goalc = get_goalc ();

      cur_bp->pt.p = cur_bp->pt.p->prev;
      cur_bp->pt.n--;

      goto_goalc (cur_goalc);

      return true;
    }

  return false;
}

DEFUN ("previous-line", previous_line)
/*+
Move cursor vertically up one line.
If there is no character in the target line exactly over the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
+*/
{
  if (uniarg < 0)
    return FUNCALL_ARG (next_line, -uniarg);

  if (!bobp ())
    {
      int i;

      for (i = 0; i < uniarg; i++)
        if (!previous_line ())
          {
            thisflag |= FLAG_DONE_CPCN;
            FUNCALL (beginning_of_line);
            break;
          }
    }
  else if (lastflag & FLAG_DONE_CPCN)
    thisflag |= FLAG_DONE_CPCN;
}
END_DEFUN

int
next_line (void)
{
  if (cur_bp->pt.p->next != cur_bp->lines)
    {
      thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;

      if (!(lastflag & FLAG_DONE_CPCN))
        cur_goalc = get_goalc ();

      cur_bp->pt.p = cur_bp->pt.p->next;
      cur_bp->pt.n++;

      goto_goalc (cur_goalc);

      return true;
    }

  return false;
}

DEFUN ("next-line", next_line)
/*+
Move cursor vertically down one line.
If there is no character in the target line exactly under the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
+*/
{
  if (uniarg < 0)
    return FUNCALL_ARG (previous_line, -uniarg);

  if (!eobp ())
    {
      int i;

      for (i = 0; i < uniarg; i++)
        if (!next_line ())
          {
            int old = cur_goalc;
            thisflag |= FLAG_DONE_CPCN;
            FUNCALL (end_of_line);
            cur_goalc = old;
            break;
          }
    }
  else if (lastflag & FLAG_DONE_CPCN)
    thisflag |= FLAG_DONE_CPCN;
}
END_DEFUN

DEFUN_ARGS ("goto-char", goto_char,
            INT_ARG (n))
/*+
Read a number N and move the cursor to character number N.
Position 1 is the beginning of the buffer.
+*/
{
  INT_INIT (n)
  else
    do
      {
        char *ms = minibuf_read ("Goto char: ", "");
        if (ms == NULL)
          {
            ok = FUNCALL (keyboard_quit);
            break;
          }
        n = strtoul (ms, NULL, 10);
        if (n == LONG_MAX)
          ding ();
        free (ms);
      }
    while (n == LONG_MAX);

  if (ok == leT && n != LONG_MAX)
    {
      long count;

      gotobob ();
      for (count = 1; count < n; count++)
        if (!forward_char ())
          break;
    }

  INT_FREE (n);
}
END_DEFUN

DEFUN_ARGS ("goto-line", goto_line,
            INT_ARG (n))
/*+
Goto line arg, counting from line 1 at beginning of buffer.
+*/
{
  INT_INIT (n)
  else
    {
      n = minibuf_read_number ("Goto line: ");
      if (n == LONG_MAX - 1)
        minibuf_error ("End of file during parsing");
    }

  if (n >= LONG_MAX - 1)
    ok = leNIL;
  else
    {
      if (n <= 0)
        n = 1;
      n--; /* Re-base to counting from zero */
      if (cur_bp->pt.n > (size_t) n)
        ngotoup (cur_bp->pt.n - (size_t) n);
      else if (cur_bp->pt.n < (size_t) n)
        ngotodown ((size_t) n - cur_bp->pt.n);
      cur_bp->pt.o = 0;
    }

  INT_FREE (n);
}
END_DEFUN

/*
 * Move point to the beginning of the buffer; do not touch the mark.
 */
void
gotobob (void)
{
  cur_bp->pt = point_min ();
  thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;
}

DEFUN ("beginning-of-buffer", beginning_of_buffer)
/*+
Move point to the beginning of the buffer; leave mark at previous position.
+*/
{
  set_mark_interactive ();
  gotobob ();
}
END_DEFUN

/*
 * Move point to the end of the buffer; do not touch the mark.
 */
void
gotoeob (void)
{
  cur_bp->pt = point_max ();
  thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;
}

DEFUN ("end-of-buffer", end_of_buffer)
/*+
Move point to the end of the buffer; leave mark at previous position.
+*/
{
  set_mark_interactive ();
  gotoeob ();
}
END_DEFUN

int
backward_char (void)
{
  if (!bolp ())
    {
      cur_bp->pt.o--;
      return true;
    }
  else if (!bobp ())
    {
      thisflag |= FLAG_NEED_RESYNC;
      cur_bp->pt.p = cur_bp->pt.p->prev;
      cur_bp->pt.n--;
      FUNCALL (end_of_line);
      return true;
    }

  return false;
}

int
forward_char (void)
{
  if (!eolp ())
    {
      cur_bp->pt.o++;
      return true;
    }
  else if (!eobp ())
    {
      thisflag |= FLAG_NEED_RESYNC;
      cur_bp->pt.p = cur_bp->pt.p->next;
      cur_bp->pt.n++;
      FUNCALL (beginning_of_line);
      return true;
    }

  return false;
}

DEFUN ("backward-char", backward_char)
/*+
Move point left N characters (right if N is negative).
On attempt to pass beginning or end of buffer, stop and signal error.
+*/
{
  ok = execute_with_uniarg (false, uniarg, backward_char, forward_char);
  if (ok == leNIL)
    minibuf_error ("Beginning of buffer");
}
END_DEFUN

DEFUN ("forward-char", forward_char)
/*+
Move point right N characters (left if N is negative).
On reaching end of buffer, stop and signal error.
+*/
{
  ok = execute_with_uniarg (false, uniarg, forward_char, backward_char);
  if (ok == leNIL)
    minibuf_error ("End of buffer");
}
END_DEFUN

int
ngotoup (size_t n)
{
  for (; n > 0; n--)
    if (cur_bp->pt.p->prev != cur_bp->lines)
      FUNCALL (previous_line);
    else
      return false;

  return true;
}

int
ngotodown (size_t n)
{
  for (; n > 0; n--)
    if (cur_bp->pt.p->next != cur_bp->lines)
      FUNCALL (next_line);
    else
      return false;

  return true;
}

static int
scroll_down (void)
{
  if (cur_bp->pt.n > 0)
    return ngotoup (cur_wp->eheight) ? true : false;

  minibuf_error ("Beginning of buffer");
  return false;
}

static int
scroll_up (void)
{
  if (cur_bp->pt.n < cur_bp->last_line)
    return ngotodown (cur_wp->eheight) ? true : false;

  minibuf_error ("End of buffer");
  return false;
}

DEFUN ("scroll-down", scroll_down)
/*+
Scroll text of current window downward near full screen.
+*/
{
  ok = execute_with_uniarg (false, uniarg, scroll_down, scroll_up);
}
END_DEFUN

DEFUN ("scroll-up", scroll_up)
/*+
Scroll text of current window upward near full screen.
+*/
{
  ok = execute_with_uniarg (false, uniarg, scroll_up, scroll_down);
}
END_DEFUN
