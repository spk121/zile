/* Redisplay engine

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "config.h"
#include "extern.h"

static size_t width = 0, height = 0;
static size_t cur_tab_width;
static size_t cur_topline;
static size_t point_screen_column;

size_t
term_width (void)
{
  return width;
}

size_t
term_height (void)
{
  return height;
}

void
term_set_size (size_t cols, size_t rows)
{
  width = cols;
  height = rows;
}

static int
make_char_printable (char **buf, int c)
{
  if (c == '\0')
    return xasprintf (buf, "^@");
  else if (c > 0 && c <= '\32')
    return xasprintf (buf, "^%c", 'A' + c - 1);
  else
    return xasprintf (buf, "\\%o", c & 0xff);
}

static void
outch (int c, size_t font, size_t * x)
{
  int j, w;
  char *buf;

  if (*x >= term_width ())
    return;

  term_attrset (font);

  if (c == '\t')
    for (w = cur_tab_width - *x % cur_tab_width; w > 0 && *x < term_width ();
         w--)
      term_addch (' '), ++(*x);
  else if (isprint (c))
    term_addch (c), ++(*x);
  else
    {
      j = make_char_printable (&buf, c);
      for (w = 0; w < j && *x < term_width (); ++w)
        term_addch (buf[w]), ++(*x);
      free (buf);
    }

  term_attrset (FONT_NORMAL);
}

static void
draw_end_of_line (size_t line, Window * wp, size_t lineno, Region * rp,
                  int highlight, size_t x, size_t i)
{
  if (x >= term_width ())
    {
      term_move (line, term_width () - 1);
      term_addch ('$');
    }
  else if (highlight)
    {
      for (; x < get_window_ewidth (wp); ++i)
        {
          if (in_region (lineno, i, rp))
            outch (' ', FONT_REVERSE, &x);
          else
            x++;
        }
    }
}

static void
draw_line (size_t line, size_t startcol, Window * wp, Line * lp,
           size_t lineno, Region * rp, int highlight)
{
  size_t x, i;

  term_move (line, 0);
  for (x = 0, i = startcol; i < astr_len (get_line_text (lp)) && x < get_window_ewidth (wp); i++)
    {
      if (highlight && in_region (lineno, i, rp))
        outch (astr_get (get_line_text (lp), i), FONT_REVERSE, &x);
      else
        outch (astr_get (get_line_text (lp), i), FONT_NORMAL, &x);
    }

  draw_end_of_line (line, wp, lineno, rp, highlight, x, i);
}

static void
calculate_highlight_region (Window * wp, Region * rp, int *highlight)
{
  if ((wp != cur_wp
       && !get_variable_bool ("highlight-nonselected-windows"))
      || (get_buffer_mark (get_window_bp (wp)) == NULL)
      || (!transient_mark_mode ())
      || (transient_mark_mode () && !get_buffer_mark_active (get_window_bp (wp))))
    {
      *highlight = false;
      return;
    }

  *highlight = true;
  set_region_start (rp, window_pt (wp));
  set_region_end (rp, get_marker_pt (get_buffer_mark (get_window_bp (wp))));
  if (cmp_point (get_region_end (rp), get_region_start (rp)) < 0)
    {
      Point pt1 = get_region_start (rp);
      Point pt2 = get_region_end (rp);
      set_region_start (rp, pt2);
      set_region_end (rp, pt1);
    }
}

static void
draw_window (size_t topline, Window * wp)
{
  size_t i, startcol, lineno;
  Line * lp;
  Region * rp = region_new ();
  int highlight;
  Point pt = window_pt (wp);

  calculate_highlight_region (wp, rp, &highlight);

  /* Find the first line to display on the first screen line. */
  for (lp = pt.p, lineno = pt.n, i = get_window_topdelta (wp);
       i > 0 && get_line_prev (lp) != get_buffer_lines (get_window_bp (wp));
       lp = get_line_prev (lp), --i, --lineno)
    ;

  cur_tab_width = tab_width (get_window_bp (wp));

  /* Draw the window lines. */
  for (i = topline; i < get_window_eheight (wp) + topline; ++i, ++lineno)
    {
      /* Clear the line. */
      term_move (i, 0);
      term_clrtoeol ();

      /* If at the end of the buffer, don't write any text. */
      if (lp == get_buffer_lines (get_window_bp (wp)))
        continue;

      startcol = get_window_start_column (wp);

      draw_line (i, startcol, wp, lp, lineno, rp, highlight);

      if (get_window_start_column (wp) > 0)
        {
          term_move (i, 0);
          term_addch ('$');
        }

      lp = get_line_next (lp);
    }

  free (rp);
}

static char *
make_mode_line_flags (Window * wp)
{
  static char buf[3];

  if (get_buffer_modified (get_window_bp (wp)) && get_buffer_readonly (get_window_bp (wp)))
    buf[0] = '%', buf[1] = '*';
  else if (get_buffer_modified (get_window_bp (wp)))
    buf[0] = buf[1] = '*';
  else if (get_buffer_readonly (get_window_bp (wp)))
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
static void
calculate_start_column (Window * wp)
{
  size_t col = 0, lastcol = 0, t = tab_width (get_window_bp (wp));
  int rpfact, lpfact;
  char *buf;
  size_t rp, lp, p;
  Point pt = window_pt (wp);

  rp = pt.o;
  rpfact = pt.o / (get_window_ewidth (wp) / 3);

  for (lp = rp; lp != SIZE_MAX; --lp)
    {
      for (col = 0, p = lp; p < rp; ++p)
        if (astr_get (get_line_text (pt.p), p) == '\t')
          {
            col |= t - 1;
            ++col;
          }
        else if (isprint ((int) astr_get (get_line_text (pt.p), p)))
          ++col;
        else
          {
            col += make_char_printable (&buf, astr_get (get_line_text (pt.p), p));
            free (buf);
          }

      lpfact = lp / (get_window_ewidth (wp) / 3);

      if (col >= get_window_ewidth (wp) - 1 || lpfact < (rpfact - 2))
        {
          set_window_start_column (wp, lp + 1);
          point_screen_column = lastcol;
          return;
        }

      lastcol = col;
    }

  set_window_start_column (wp, 0);
  point_screen_column = col;
}

static char *
make_screen_pos (Window * wp, char **buf)
{
  Point pt = window_pt (wp);

  if (get_buffer_last_line (get_window_bp (wp)) <= get_window_eheight (wp) && get_window_topdelta (wp) == pt.n)
    xasprintf (buf, "All");
  else if (pt.n == get_window_topdelta (wp))
    xasprintf (buf, "Top");
  else if (pt.n + (get_window_eheight (wp) - get_window_topdelta (wp)) > get_buffer_last_line (get_window_bp (wp)))
    xasprintf (buf, "Bot");
  else
    xasprintf (buf, "%2d%%",
               (int) ((float) pt.n / get_buffer_last_line (get_window_bp (wp)) * 100));

  return *buf;
}

static void
draw_status_line (size_t line, Window * wp)
{
  size_t i;
  char *buf, *eol_type;
  Point pt = window_pt (wp);
  astr as, bs;

  term_attrset (FONT_REVERSE);

  term_move (line, 0);
  for (i = 0; i < get_window_ewidth (wp); ++i)
    term_addch ('-');

  if (get_buffer_eol (cur_bp) == coding_eol_cr)
    eol_type = "(Mac)";
  else if (get_buffer_eol (cur_bp) == coding_eol_crlf)
    eol_type = "(DOS)";
  else
    eol_type = ":";

  term_move (line, 0);
  bs = astr_afmt (astr_new (), "(%d,%d)", pt.n + 1,
                  get_goalc_bp (get_window_bp (wp), window_pt (wp)));
  as = astr_afmt (astr_new (), "--%s%2s  %-15s   %s %-9s (Fundamental",
                  eol_type, make_mode_line_flags (wp), get_buffer_name (get_window_bp (wp)),
                  make_screen_pos (wp, &buf), astr_cstr (bs));
  free (buf);
  astr_delete (bs);

  if (get_buffer_autofill (get_window_bp (wp)))
    astr_cat_cstr (as, " Fill");
  if (get_buffer_overwrite (get_window_bp (wp)))
    astr_cat_cstr (as, " Ovwrt");
  if (thisflag & FLAG_DEFINING_MACRO)
    astr_cat_cstr (as, " Def");
  if (get_buffer_isearch (get_window_bp (wp)))
    astr_cat_cstr (as, " Isearch");

  astr_cat_char (as, ')');
  term_addnstr (astr_cstr (as), MIN (term_width (), astr_len (as)));
  astr_delete (as);

  term_attrset (FONT_NORMAL);
}

void
term_redisplay (void)
{
  size_t topline;
  Window *wp;

  cur_topline = topline = 0;

  calculate_start_column (cur_wp);

  for (wp = head_wp; wp != NULL; wp = get_window_next (wp))
    {
      if (wp == cur_wp)
        cur_topline = topline;

      draw_window (topline, wp);

      /* Draw the status line only if there is available space after the
         buffer text space. */
      if (get_window_fheight (wp) - get_window_eheight (wp) > 0)
        draw_status_line (topline + get_window_eheight (wp), wp);

      topline += get_window_fheight (wp);
    }

  /* Redraw cursor. */
  term_move (cur_topline + get_window_topdelta (cur_wp), point_screen_column);
}

void
term_full_redisplay (void)
{
  term_clear ();
  term_redisplay ();
}

void
show_splash_screen (const char *splash)
{
  size_t i;
  const char *p;

  for (i = 0; i < term_height () - 2; ++i)
    {
      term_move (i, 0);
      term_clrtoeol ();
    }

  term_move (0, 0);
  for (i = 0, p = splash; *p != '\0' && i < term_height () - 2; ++p)
    if (*p == '\n')
      term_move (++i, 0);
    else
      term_addch (*p);
}

/*
 * Tidy up the term ready to leave Zile (temporarily or permanently!).
 */
void
term_tidy (void)
{
  term_move (term_height () - 1, 0);
  term_clrtoeol ();
  term_attrset (FONT_NORMAL);
  term_refresh ();
}

/*
 * Tidy and close the terminal ready to leave Zile.
 */
void
term_finish (void)
{
  term_tidy ();
  term_close ();
}

/*
 * Add a string to the terminal
 */
void
term_addnstr (const char *s, size_t len)
{
  size_t i;
  for (i = 0; i < len; i++)
    term_addch (*s++);
}
