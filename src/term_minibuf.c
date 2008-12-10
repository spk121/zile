/* Minibuffer handling

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2003, 2004, 2005 Reuben Thomas.

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

#include "zile.h"
#include "extern.h"

void
term_minibuf_write (const char *s)
{
  size_t x;

  term_move (term_height () - 1, 0);
  term_clrtoeol ();

  for (x = 0; *s != '\0' && x < term_width (); s++)
    {
      term_addch (*(unsigned char *) s);
      ++x;
    }
}

static void
draw_minibuf_read (const char *prompt, const char *value,
                   size_t prompt_len, char *match, size_t pointo)
{
  int margin = 1, n = 0;

  term_minibuf_write (prompt);

  if (prompt_len + pointo + 1 >= term_width ())
    {
      margin++;
      term_addch ('$');
      n = pointo - pointo % (term_width () - prompt_len - 2);
    }

  term_addnstr (value + n,
                MIN (term_width () - prompt_len - margin,
                     strlen (value) - n));
  term_addnstr (match, strlen (match));

  if (strlen (value + n) >= term_width () - prompt_len - margin)
    {
      term_move (term_height () - 1, term_width () - 1);
      term_addch ('$');
    }

  term_move (term_height () - 1,
             prompt_len + margin - 1 + pointo % (term_width () - prompt_len -
                                                 margin));

  term_refresh ();
}

static astr
do_minibuf_read (const char *prompt, const char *value, size_t pos,
               Completion * cp, History * hp)
{
  static int overwrite_mode = 0;
  int c, thistab, lasttab = -1;
  size_t prompt_len;
  char *s;
  astr as = astr_new_cstr (value), saved = NULL;

  prompt_len = strlen (prompt);
  if (pos == SIZE_MAX)
    pos = astr_len (as);

  for (;;)
    {
      switch (lasttab)
        {
        case COMPLETION_MATCHEDNONUNIQUE:
          s = " [Complete, but not unique]";
          break;
        case COMPLETION_NOTMATCHED:
          s = " [No match]";
          break;
        case COMPLETION_MATCHED:
          s = " [Sole completion]";
          break;
        default:
          s = "";
        }
      draw_minibuf_read (prompt, astr_cstr (as), prompt_len, s, pos);

      thistab = -1;

      switch (c = getkey ())
        {
        case KBD_NOKEY:
          break;
        case KBD_CTRL | 'z':
          FUNCALL (suspend_zile);
          break;
        case KBD_RET:
          term_move (term_height () - 1, 0);
          term_clrtoeol ();
          if (saved)
            astr_delete (saved);
          return as;
        case KBD_CANCEL:
          term_move (term_height () - 1, 0);
          term_clrtoeol ();
          if (saved)
            astr_delete (saved);
          return NULL;
        case KBD_CTRL | 'a':
        case KBD_HOME:
          pos = 0;
          break;
        case KBD_CTRL | 'e':
        case KBD_END:
          pos = astr_len (as);
          break;
        case KBD_CTRL | 'b':
        case KBD_LEFT:
          if (pos > 0)
            --pos;
          else
            ding ();
          break;
        case KBD_CTRL | 'f':
        case KBD_RIGHT:
          if (pos < astr_len (as))
            ++pos;
          else
            ding ();
          break;
        case KBD_CTRL | 'k':
          /* FIXME: no kill-register save is done yet. */
          if (pos < astr_len (as))
            astr_truncate (as, pos);
          else
            ding ();
          break;
        case KBD_BS:
          if (pos > 0)
            astr_remove (as, --pos, 1);
          else
            ding ();
          break;
        case KBD_CTRL | 'd':
        case KBD_DEL:
          if (pos < astr_len (as))
            astr_remove (as, pos, 1);
          else
            ding ();
          break;
        case KBD_INS:
          overwrite_mode = overwrite_mode ? 0 : 1;
          break;
        case KBD_META | 'v':
        case KBD_PGUP:
          if (cp == NULL)
            {
              ding ();
              break;
            }

          if (cp->flags & CFLAG_POPPEDUP)
            {
              completion_scroll_down ();
              thistab = lasttab;
            }
          break;
        case KBD_CTRL | 'v':
        case KBD_PGDN:
          if (cp == NULL)
            {
              ding ();
              break;
            }

          if (cp->flags & CFLAG_POPPEDUP)
            {
              completion_scroll_up ();
              thistab = lasttab;
            }
          break;
        case KBD_UP:
        case KBD_META | 'p':
          if (hp)
            {
              const char *elem = previous_history_element (hp);
              if (elem)
                {
                  if (!saved)
                    saved = astr_cpy (astr_new (), as);

                  astr_cpy_cstr (as, elem);
                }
            }
          break;
        case KBD_DOWN:
        case KBD_META | 'n':
          if (hp)
            {
              const char *elem = next_history_element (hp);
              if (elem)
                astr_cpy_cstr (as, elem);
              else if (saved)
                {
                  astr_cpy (as, saved);
                  astr_delete (saved);
                  saved = NULL;
                }
            }
          break;
        case KBD_TAB:
        got_tab:
          if (cp == NULL)
            {
              ding ();
              break;
            }

          if (lasttab != -1 && lasttab != COMPLETION_NOTMATCHED
              && cp->flags & CFLAG_POPPEDUP)
            {
              completion_scroll_up ();
              thistab = lasttab;
            }
          else
            {
              astr bs = astr_new ();
              astr_cpy (bs, as);
              thistab = completion_try (cp, bs, true);
              astr_delete (bs);
              switch (thistab)
                {
                case COMPLETION_MATCHED:
                case COMPLETION_MATCHEDNONUNIQUE:
                case COMPLETION_NONUNIQUE:
                  {
                    bs = astr_new ();
                    if (cp->flags & CFLAG_FILENAME)
                      astr_cat (bs, cp->path);
                    astr_ncat_cstr (bs, cp->match, cp->matchsize);
                    if (strncmp (astr_cstr (as), astr_cstr (bs),
                                 astr_len (bs)) != 0)
                      thistab = -1;
                    astr_delete (as);
                    as = bs;
                    pos = astr_len (as);
                    break;
                  }
                case COMPLETION_NOTMATCHED:
                  ding ();
                }
            }
          break;
        case ' ':
          if (cp != NULL)
            goto got_tab;
          /* FALLTHROUGH */
        default:
          if (c > 255 || !isprint (c))
            {
              ding ();
              break;
            }
          if (!overwrite_mode || pos == astr_len (as))
            astr_insert_char (as, pos++, c);
          else
            astr_replace_char (as, pos, c);
        }

      lasttab = thistab;
    }
}

char *
term_minibuf_read (const char *prompt, const char *value, size_t pos,
                   Completion * cp, History * hp)
{
  Window *wp, *old_wp = cur_wp;
  char *s = NULL;
  astr as;

  if (hp)
    prepare_history (hp);

  as = do_minibuf_read (prompt, value, pos, cp, hp);
  if (as)
    {
      s = xstrdup (astr_cstr (as));
      astr_delete (as);
    }

  if (cp != NULL && (cp->flags & CFLAG_POPPEDUP)
      && (wp = find_window ("*Completions*")) != NULL)
    {
      set_current_window (wp);
      if (cp->flags & CFLAG_CLOSE)
        FUNCALL (delete_window);
      else if (cp->old_bp)
        switch_to_buffer (cp->old_bp);
      set_current_window (old_wp);
    }

  return s;
}
