/* Minibuffer handling

   Copyright (c) 1997-2005, 2008-2009, 2011 Free Software Foundation, Inc.

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
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"

void
term_minibuf_write (const char *s)
{
  term_move (term_height () - 1, 0);
  term_clrtoeol ();
  term_addstr (s);
}

static void
draw_minibuf_read (const char *prompt, const char *value,
                   size_t prompt_len, const char *match, size_t pointo)
{
  int margin = 1, n = 0;

  term_minibuf_write (prompt);

  if (prompt_len + pointo + 1 >= term_width ())
    {
      margin++;
      term_addstr ("$");
      n = pointo - pointo % (term_width () - prompt_len - 2);
    }

  term_addstr (value + n);
  term_addstr (match);

  if (strlen (value + n) >= term_width () - prompt_len - margin)
    {
      term_move (term_height () - 1, term_width () - 1);
      term_addstr ("$");
    }

  term_move (term_height () - 1,
             prompt_len + margin - 1 + pointo % (term_width () - prompt_len -
                                                 margin));

  term_refresh ();
}

static void
maybe_close_popup (Completion *cp)
{
  Window *wp, *old_wp = cur_wp;
  if (cp != NULL && (get_completion_flags (cp) & CFLAG_POPPEDUP)
      && (wp = find_window ("*Completions*")) != NULL)
    {
      set_current_window (wp);
      if (get_completion_flags (cp) & CFLAG_CLOSE)
        FUNCALL (delete_window);
      else if (get_completion_old_bp (cp))
        switch_to_buffer (get_completion_old_bp (cp));
      set_current_window (old_wp);
      term_redisplay ();
    }
}

castr
term_minibuf_read (const char *prompt, const char *value, size_t pos,
               Completion * cp, History * hp)
{
  if (hp)
    prepare_history (hp);

  size_t c;
  int thistab, lasttab = -1;
  astr as = astr_new_cstr (value), saved = NULL;

  size_t prompt_len = strlen (prompt);
  if (pos == SIZE_MAX)
    pos = astr_len (as);

  do
    {
      const char *s;
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

      switch (c = getkeystroke (GETKEY_DEFAULT))
        {
        case KBD_NOKEY:
        case KBD_RET:
          break;
        case KBD_CTRL | 'z':
          FUNCALL (suspend_emacs);
          break;
        case KBD_CANCEL:
          as = NULL;
          break;
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
          /* FIXME: do kill-register save. */
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
        case KBD_META | 'v':
        case KBD_PGUP:
          if (cp == NULL)
            {
              ding ();
              break;
            }

          if (get_completion_flags (cp) & CFLAG_POPPEDUP)
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

          if (get_completion_flags (cp) & CFLAG_POPPEDUP)
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
              && get_completion_flags (cp) & CFLAG_POPPEDUP)
            {
              completion_scroll_up ();
              thistab = lasttab;
            }
          else
            {
              astr bs = astr_new ();
              astr_cpy (bs, as);
              thistab = completion_try (cp, bs, true);
              switch (thistab)
                {
                case COMPLETION_MATCHED:
                  maybe_close_popup (cp);
                  set_completion_flags (cp, get_completion_flags (cp) & ~CFLAG_POPPEDUP);
                  /* FALLTHROUGH */
                case COMPLETION_MATCHEDNONUNIQUE:
                case COMPLETION_NONUNIQUE:
                  {
                    bs = astr_new ();
                    if (get_completion_flags (cp) & CFLAG_FILENAME)
                      astr_cat (bs, get_completion_path (cp));
                    astr_cat_nstr (bs, get_completion_match (cp), get_completion_matchsize (cp));
                    if (strncmp (astr_cstr (as), astr_cstr (bs),
                                 astr_len (bs)) != 0)
                      thistab = -1;
                    as = bs;
                    pos = astr_len (as);
                    break;
                  }
                case COMPLETION_NOTMATCHED:
                  ding ();
                  break;
                default:
                  break;
                }
            }
          break;
        case ' ':
          if (cp != NULL)
            goto got_tab;
          /* FALLTHROUGH */
        default:
          if (c > 255 || !isprint (c))
            ding ();
          else
            {
              astr_cpy (as, astr_fmt ("%s%c%s", astr_cstr (astr_substr (as, 0, pos)), c, astr_cstr (astr_substr (as, pos, astr_len (as) - pos))));
              pos++;
            }
        }

      lasttab = thistab;
    } while (c != KBD_RET && c != KBD_CANCEL);

  minibuf_clear ();
  maybe_close_popup (cp);
  return as;
}
