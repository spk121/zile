/* Redisplay engine
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
   All rights reserved.

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

/*	$Id: term_redisplay.c,v 1.28 2005/01/16 13:06:53 rrt Exp $	*/

#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "config.h"
#include "extern.h"

static int cur_tab_width;
static int point_start_column;
static int point_screen_column;

static int make_char_printable(char **buf, unsigned int c)
{
  if (c == '\0')
    return asprintf(buf, "^@");
  else if (c <= '\32')
    return asprintf(buf, "^%c", 'A' + c - 1);
  else
    return asprintf(buf, "\\%o", c & 255);
}

static void outch(int c, Font font, int *x)
{
  int j, w;
  char *buf;

  if (*x >= ZILE_COLS)
    return;

  term_attrset(1, font);
                
  if (c == '\t')
    for (w = cur_tab_width - *x % cur_tab_width; w > 0 && *x < ZILE_COLS; w--)
      term_addch(' '), ++(*x);
  else if (isprint(c))
    term_addch(c), ++(*x);
  else {
    j = make_char_printable(&buf, c);
    for (w = 0; w < j && *x < ZILE_COLS; ++w)
      term_addch(buf[w]), ++(*x);
    free(buf);
  }

  term_attrset(1, ZILE_NORMAL);
}

static int in_region(int lineno, int x, Region *r)
{
  if (lineno >= r->start.n && lineno <= r->end.n) {
    if (r->start.n == r->end.n) {
      if (x >= r->start.o && x < r->end.o)
        return TRUE;
    } else if (lineno == r->start.n) {
      if (x >= r->start.o)
        return TRUE;
    } else if (lineno == r->end.n) {
      if (x < r->end.o)
        return TRUE;
    } else {
      return TRUE;
    }
  }

  return FALSE;
}

static void draw_end_of_line(int line, Window *wp, int lineno, Region *r,
                             int highlight, int x, int i)
{
  if (x >= ZILE_COLS) {
    term_move(line, ZILE_COLS - 1);
    term_addch('$');
  } else if (highlight) {
    for (; x < wp->ewidth; ++i) {
      if (in_region(lineno, i, r))
        outch(' ', ZILE_REVERSE, &x);
      else
        x++;
    }
  }
}

static void draw_line(int line, int startcol, Window *wp, Line *lp,
		      int lineno, Region *r, int highlight)
{
  int x, j;

  term_move(line, 0);
  for (x = 0, j = startcol; j < astr_len(lp->item) && x < wp->ewidth; ++j) {
    if (highlight && in_region(lineno, j, r))
      outch(*astr_char(lp->item, j), ZILE_REVERSE, &x);
    else
      outch(*astr_char(lp->item, j), ZILE_NORMAL, &x);
  }

  draw_end_of_line(line, wp, lineno, r, highlight, x, j);
}

static void calculate_highlight_region(Window *wp, Region *r, int *highlight)
{
  if ((wp != cur_wp
       && !lookup_bool_variable("highlight-nonselected-windows"))
      || (!wp->bp->mark)
      || (!transient_mark_mode())
      || (transient_mark_mode() && !(wp->bp->mark_active))) {
    *highlight = FALSE;
    return;
  }

  *highlight = TRUE;
  r->start = window_pt(wp);
  r->end = wp->bp->mark->pt;
  if (cmp_point(r->end, r->start) < 0)
    swap_point(&r->end, &r->start);
}

static void draw_window(int topline, Window *wp)
{
  int i, startcol, lineno;
  Line *lp;
  Region r;
  int highlight;
  Point pt = window_pt(wp);

  calculate_highlight_region(wp, &r, &highlight);

  /* Find the first line to display on the first screen line. */
  for (lp = pt.p, lineno = pt.n, i = wp->topdelta;
       i > 0 && list_prev(lp) != wp->bp->lines; lp = list_prev(lp), --i, --lineno)
    ;

  cur_tab_width = wp->bp->tab_width;

  /* Draw the window lines. */
  for (i = topline; i < wp->eheight + topline; ++i, ++lineno) {
    /* Clear the line. */
    term_move(i, 0);
    term_clrtoeol();

    /* If at the end of the buffer, don't write any text. */
    if (lp == wp->bp->lines)
      continue;

    startcol = point_start_column;

    draw_line(i, startcol, wp, lp, lineno, &r, highlight);

    if (point_start_column > 0) {
      term_move(i, 0);
      term_addch('$');
    }

    lp = list_next(lp);
  }
}

static char *make_mode_line_flags(Window *wp)
{
  static char buf[3];

  if ((wp->bp->flags & (BFLAG_MODIFIED | BFLAG_READONLY)) == (BFLAG_MODIFIED | BFLAG_READONLY))
    buf[0] = '%', buf[1] = '*';
  else if (wp->bp->flags & BFLAG_MODIFIED)
    buf[0] = buf[1] = '*';
  else if (wp->bp->flags & BFLAG_READONLY)
    buf[0] = buf[1] = '%';
  else
    buf[0] = buf[1] = '-';

  return buf;
}

/*
 * This function calculates the best start column to draw if the line
 * needs to get truncated.
 * Called only for the line where is the point.
 */
static void calculate_start_column(Window *wp)
{
  int col = 0, lastcol = 0, t = wp->bp->tab_width;
  int rpfact, lpfact;
  char *buf, *rp, *lp, *p;
  Point pt = window_pt(wp);

  rp = astr_char(pt.p->item, pt.o);
  rpfact = pt.o / (wp->ewidth / 3);

  for (lp = rp; lp >= astr_cstr(pt.p->item); --lp) {
    for (col = 0, p = lp; p < rp; ++p)
      if (*p == '\t') {
        col |= t - 1;
        ++col;
      } else if (isprint(*p))
        ++col;
      else {
        col += make_char_printable(&buf, *p);
        free(buf);
      }

    lpfact = (lp - astr_cstr(pt.p->item)) / (wp->ewidth / 3);

    if (col >= wp->ewidth - 1 || lpfact < (rpfact - 2)) {
      point_start_column = lp + 1 - astr_cstr(pt.p->item);
      point_screen_column = lastcol;
      return;
    }

    lastcol = col;
  }

  point_start_column = 0;
  point_screen_column = col;
}

static char *make_screen_pos(Window *wp, char **buf)
{
  Point pt = window_pt(wp);

  if (wp->bp->num_lines <= wp->eheight && wp->topdelta == pt.n)
    asprintf(buf, "All");
  else if (pt.n == wp->topdelta)
    asprintf(buf, "Top");
  else if (pt.n + (wp->eheight - wp->topdelta) > wp->bp->num_lines)
    asprintf(buf, "Bot");
  else
    asprintf(buf, "%2d%%", (int)((float)pt.n / wp->bp->num_lines * 100));

  return *buf;
}

static void draw_status_line(int line, Window *wp)
{
  int i, someflag = 0;
  char *buf;
  Point pt = window_pt(wp);

  term_attrset(1, ZILE_REVERSE);

  term_move(line, 0);
  for (i = 0; i < wp->ewidth; ++i)
    term_addch('-');

  term_move(line, 0);
  term_printw("--%2s- %-18s (", make_mode_line_flags(wp), wp->bp->name);

  if (wp->bp->flags & BFLAG_AUTOFILL) {
    term_printw("Fill");
    someflag = 1;
  }
  if (wp->bp->flags & BFLAG_OVERWRITE) {
    term_printw("%sOvwrt", someflag ? " " : "");
    someflag = 1;
  }
  if (thisflag & FLAG_DEFINING_MACRO) {
    term_printw("%sDef", someflag ? " " : "");
    someflag = 1;
  }
  if (wp->bp->flags & BFLAG_ISEARCH)
    term_printw("%sIsearch", someflag ? " " : "");

  term_printw(")--L%d--C%d--%s",
              pt.n+1, get_goalc_wp(wp),
              make_screen_pos(wp, &buf));
  free(buf);

  term_attrset(1, ZILE_NORMAL);
}

void term_redisplay(void)
{
  int topline, cur_topline = 0;
  Window *wp;

  topline = 0;

  calculate_start_column(cur_wp);

  for (wp = head_wp; wp != NULL; wp = wp->next) {
    if (wp == cur_wp)
      cur_topline = topline;

    draw_window(topline, wp);

    /* Draw the status line only if there is available space after the
       buffer text space. */
    if (wp->fheight - wp->eheight > 0)
      draw_status_line(topline + wp->eheight, wp);

    topline += wp->fheight;
  }

  term_move(cur_topline + cur_wp->topdelta, point_screen_column);
}

void term_full_redisplay(void)
{
  term_clear();
  term_redisplay();
}

void show_splash_screen(const char *splash)
{
  int i;
  const char *p;

  for (i = 0; i < ZILE_LINES - 2; ++i) {
    term_move(i, 0);
    term_clrtoeol();
  }

  term_move(0, 0);
  for (i = 0, p = splash; *p != '\0'; ++p)
    if (*p == '\n')
      term_move(++i, 0);
    else
      term_addch(*p);

  term_refresh();
  waitkey(20 * 10);
}

/*
 * Tidy up the term ready to leave Zile (temporarily or permanently!).
 */
void term_tidy(void)
{
  term_move(ZILE_LINES - 1, 0);
  term_clrtoeol();
  term_attrset(1, ZILE_NORMAL);
  term_refresh();
}
