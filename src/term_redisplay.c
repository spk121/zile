/* Redisplay engine

   Copyright (c) 1997-2011 Free Software Foundation, Inc.

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

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include <config.h>
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

static char *
make_char_printable (int c)
{
  if (c == '\0')
    return xasprintf ("^@");
  else if (c > 0 && c <= '\32')
    return xasprintf ("^%c", 'A' + c - 1);
  else
    return xasprintf ("\\%o", c & 0xff);
}

static size_t
outch (int c, size_t font, size_t x)
{
  int w;

  if (x >= term_width ())
    return x;

  term_attrset (font);

  if (c == '\t')
    for (w = cur_tab_width - x % cur_tab_width; w > 0 && x < term_width ();
         w--)
      term_addch (' '), ++x;
  else if (isprint (c))
    term_addch (c), ++x;
  else
    {
      char *buf = make_char_printable (c);
      int j = strlen (buf);
      for (w = 0; w < j && x < term_width (); ++w)
        term_addch (buf[w]), ++x;
    }

  term_attrset (FONT_NORMAL);

  return x;
}

static void
draw_end_of_line (size_t line, Window * wp, size_t lineno, Region r,
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
          if (in_region (lineno, i, r))
            x = outch (' ', FONT_REVERSE, x);
          else
            x++;
        }
    }
}

static void
draw_line (size_t line, size_t startcol, Window * wp, size_t o,
           size_t lineno, Region r, int highlight)
{
  term_move (line, 0);

  size_t x, i;
  for (x = 0, i = startcol; i < estr_end_of_line (get_buffer_text (get_window_bp (wp)), o) - o && x < get_window_ewidth (wp); i++)
    x = outch (astr_get (get_buffer_text (get_window_bp (wp)).as, o + i),
               highlight && in_region (lineno, i, r) ? FONT_REVERSE : FONT_NORMAL,
               x);

  draw_end_of_line (line, wp, lineno, r, highlight, x, i);
}

static int
calculate_highlight_region (Window * wp, Region * rp)
{
  if ((wp != cur_wp
       && !get_variable_bool ("highlight-nonselected-windows"))
      || (get_buffer_mark (get_window_bp (wp)) == NULL)
      || !get_buffer_mark_active (get_window_bp (wp)))
    return false;

  rp->start = point_to_offset (get_window_bp (wp), window_pt (wp));
  rp->end = get_marker_o (get_buffer_mark (get_window_bp (wp)));
  if (rp->end < rp->start)
    {
      size_t o = rp->start;
      rp->start = rp->end;
      rp->start = o;
    }
  return true;
}

static void
draw_window (size_t topline, Window * wp)
{
  size_t i, lineno, o;
  Point pt = window_pt (wp);
  Region r;
  int highlight = calculate_highlight_region (wp, &r);

  /* Find the first line to display on the first screen line. */
  for (o = get_buffer_line_o (get_window_bp (wp)), lineno = pt.n, i = get_window_topdelta (wp);
       i > 0 && lineno > 0;
       assert ((o = estr_prev_line (get_buffer_text (get_window_bp (wp)), o)) != SIZE_MAX), --i, --lineno)
    ;

  cur_tab_width = tab_width (get_window_bp (wp));

  /* Draw the window lines. */
  for (i = topline; i < get_window_eheight (wp) + topline; ++i, ++lineno)
    {
      /* Clear the line. */
      term_move (i, 0);
      term_clrtoeol ();

      /* If at the end of the buffer, don't write any text. */
      if (o == SIZE_MAX)
        continue;

      draw_line (i, get_window_start_column (wp), wp, o, lineno, r, highlight);

      if (get_window_start_column (wp) > 0)
        {
          term_move (i, 0);
          term_addch ('$');
        }

      o = estr_next_line (get_buffer_text (get_window_bp (wp)), o);
    }

  set_window_all_displayed (wp, o >= get_buffer_size (get_window_bp (wp)));
}

static const char *
make_mode_line_flags (Window * wp)
{
  if (get_buffer_modified (get_window_bp (wp)) && get_buffer_readonly (get_window_bp (wp)))
    return "%*";
  else if (get_buffer_modified (get_window_bp (wp)))
    return "**";
  else if (get_buffer_readonly (get_window_bp (wp)))
    return "%%";
  return "--";
}

/*
 * This function calculates the best start column to draw if the line
 * at point has to be truncated.
 */
static void
calculate_start_column (Window * wp)
{
  Buffer *bp = get_window_bp (wp);
  size_t col = 0, lastcol = 0, t = tab_width (bp);
  int rpfact, lpfact;
  Point pt = window_pt (wp);
  size_t rp, lp, p, o = point_to_offset (bp, pt) - pt.o;

  rp = pt.o;
  rpfact = pt.o / (get_window_ewidth (wp) / 3);

  for (lp = rp; lp != SIZE_MAX; --lp)
    {
      for (col = 0, p = lp; p < rp; ++p)
        {
          char c = astr_get (get_buffer_text (bp).as, o + p);
          if (c == '\t')
            {
              col |= t - 1;
              ++col;
            }
          else if (isprint ((int) c))
            ++col;
          else
            {
              char *buf = make_char_printable (c);
              col += strlen (buf);
            }
        }

      lpfact = lp / (get_window_ewidth (wp) / 3);

      if (col >= get_window_ewidth (wp) - 1 || lpfact < rpfact - 2)
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
  bool tv = window_top_visible (wp);
  bool bv = window_bottom_visible (wp);

  if (tv && bv)
    *buf = xasprintf ("All");
  else if (tv)
    *buf = xasprintf ("Top");
  else if (bv)
    *buf = xasprintf ("Bot");
  else
    *buf = xasprintf ("%2d%%",
                      (int) ((float) (window_pt (wp).n - get_window_topdelta (wp)) / offset_to_point (get_window_bp (wp), get_buffer_size (get_window_bp (wp))).n * 100));

  return *buf;
}

static void
draw_status_line (size_t line, Window * wp)
{
  size_t i;
  char *buf;
  const char *eol_type;
  Point pt = window_pt (wp);
  astr as, bs;

  term_attrset (FONT_REVERSE);

  term_move (line, 0);
  for (i = 0; i < get_window_ewidth (wp); ++i)
    term_addch ('-');

  if (get_buffer_text (cur_bp).eol == coding_eol_cr)
    eol_type = "(Mac)";
  else if (get_buffer_text (cur_bp).eol == coding_eol_crlf)
    eol_type = "(DOS)";
  else
    eol_type = ":";

  term_move (line, 0);
  bs = astr_fmt ("(%d,%d)", pt.n + 1,
                 get_goalc_bp (get_window_bp (wp), window_pt (wp)));
  as = astr_fmt ("--%s%2s  %-15s   %s %-9s (Fundamental",
                 eol_type, make_mode_line_flags (wp), get_buffer_name (get_window_bp (wp)),
                 make_screen_pos (wp, &buf), astr_cstr (bs));

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
  for (size_t i = 0; i < term_height () - 2; ++i)
    {
      term_move (i, 0);
      term_clrtoeol ();
    }

  term_move (0, 0);
  const char *p = splash;
  for (size_t i = 0; *p != '\0' && i < term_height () - 2; ++p)
    if (*p == '\n')
      term_move (++i, 0);
    else
      term_addch (*p);
}

/*
 * Tidy and close the terminal ready to leave Zile.
 */
void
term_finish (void)
{
  term_move (term_height () - 1, 0);
  term_clrtoeol ();
  term_attrset (FONT_NORMAL);
  term_refresh ();
  term_close ();
}

/*
 * Add a string to the terminal
 */
void
term_addnstr (const char *s, size_t len)
{
  for (size_t i = 0; i < len; i++)
    term_addch (*s++);
}
