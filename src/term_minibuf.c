/* Minibuffer handling
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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

/* Write minibuf prompt, assuming cursor starts at column 0. */
static void xminibuf_write(const char *s)
{
  size_t x;

  for (x = 0; *s != '\0' && x < term_width(); s++) {
    term_addch(*(unsigned char *)s);
    ++x;
  }
}

void term_minibuf_write(const char *s)
{
  term_move(term_height() - 1, 0);
  xminibuf_write(s);
  term_clrtoeol();
}

static void draw_minibuf_read(const char *prompt, const char *value,
                              size_t prompt_len, char *match, size_t pointo)
{
  int margin = 1, n = 0;

  term_move(term_height() - 1, 0);
  term_clrtoeol();
  xminibuf_write(prompt);

  if (prompt_len + pointo + 1 >= term_width()) {
    margin++;
    term_addch('$');
    n = pointo - pointo % (term_width() - prompt_len - 2);
  }

  term_addnstr(value + n, min(term_width() - prompt_len - margin, strlen(value) - n));
  term_addnstr(match, strlen(match));

  if (strlen(value + n) >= term_width() - prompt_len - margin) {
    term_move(term_height() - 1, term_width() - 1);
    term_addch('$');
  }

  term_move(term_height() - 1, prompt_len + margin - 1 + pointo % (term_width() - prompt_len - margin));

  term_refresh();
}

static char *rot_vminibuf_read(const char *prompt, const char *value,
			       Completion *cp, History *hp, char **p, size_t *max)
{
  static int overwrite_mode = 0;
  int c, thistab, lasttab = -1;
  size_t i, len, prompt_len;
  char *s, *saved = NULL;

  prompt_len = strlen(prompt);

  len = i = strlen(value);
  if (*max < i + 10) {
    *max = i + 10;
    if (*p != NULL)
      *p = (char *)zrealloc(*p, *max);
    else
      *p = (char *)zmalloc(*max);
  }
  strcpy(*p, value);

  for (;;) {
    switch (lasttab) {
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
    draw_minibuf_read(prompt, *p, prompt_len, s, i);

    thistab = -1;

    switch (c = getkey()) {
    case KBD_NOKEY:
      break;
    case KBD_CTRL | 'z':
      FUNCALL(suspend_zile);
      break;
    case KBD_RET:
      term_move(term_height() - 1, 0);
      term_clrtoeol();
      if (saved)
        free(saved);
      return *p;
    case KBD_CANCEL:
      term_move(term_height() - 1, 0);
      term_clrtoeol();
      if (saved)
        free(saved);
      return NULL;
    case KBD_CTRL | 'a':
    case KBD_HOME:
      i = 0;
      break;
    case KBD_CTRL | 'e':
    case KBD_END:
      i = len;
      break;
    case KBD_CTRL | 'b':
    case KBD_LEFT:
      if (i > 0)
        --i;
      else
        ding();
      break;
    case KBD_CTRL | 'f':
    case KBD_RIGHT:
      if (i < len)
        ++i;
      else
        ding();
      break;
    case KBD_CTRL | 'k':
      /* XXX no kill-register save is done yet. */
      if (i < len) {
        len -= len - i;
        (*p)[i] = '\0';
      } else
        ding();
      break;
    case KBD_BS:
      if (i > 0) {
        --i, --len;
        strcpy(*p + i, *p + i + 1);
      } else
        ding();
      break;
    case KBD_DEL:
      if (i < len) {
        --len;
        strcpy(*p + i, *p + i + 1);
      } else
        ding();
      break;
    case KBD_INS:
      overwrite_mode = overwrite_mode ? 0 : 1;
      break;
    case KBD_META | 'v':
    case KBD_PGUP:
      if (cp == NULL) {
        ding();
        break;
      }

      if (cp->fl_poppedup) {
        completion_scroll_down();
        thistab = lasttab;
      }
      break;
    case KBD_CTRL | 'v':
    case KBD_PGDN:
      if (cp == NULL) {
        ding();
        break;
      }

      if (cp->fl_poppedup) {
        completion_scroll_up();
        thistab = lasttab;
      }
      break;
    case KBD_UP:
    case KBD_META | 'p':
      if (hp) {
        const char *elem = previous_history_element(hp);
        if (elem) {
          if (!saved)
            saved = zstrdup(*p);

          len = i = strlen(elem);
          *max = i + 10;
          *p = zrealloc(*p, *max);
          strcpy(*p, elem);
        }
      }
      break;
    case KBD_DOWN:
    case KBD_META | 'n':
      if (hp) {
        const char *elem = next_history_element(hp);
        if (elem) {
          len = i = strlen(elem);
          *max = i + 10;
          *p = zrealloc(*p, *max);
          strcpy(*p, elem);
        }
        else if (saved) {
          len = i = strlen(saved);
          *max = i + 10;
          *p = zrealloc(*p, *max);
          strcpy(*p, saved);

          free(saved);
          saved = NULL;
        }
      }
      break;
    case KBD_TAB:
    got_tab:
      if (cp == NULL) {
        ding();
        break;
      }

      if (lasttab != -1 && lasttab != COMPLETION_NOTMATCHED
          && cp->fl_poppedup) {
        completion_scroll_up();
        thistab = lasttab;
      } else {
        astr as = astr_new();
        astr_cpy_cstr(as, *p);
        thistab = completion_try(cp, as, TRUE);
        astr_delete(as);
        switch (thistab) {
        case COMPLETION_NONUNIQUE:
        case COMPLETION_MATCHED:
        case COMPLETION_MATCHEDNONUNIQUE:
          if (cp->fl_dir)
            len = i = cp->matchsize + strlen(astr_cstr(cp->path));
          else
            len = i = cp->matchsize;
          *max = len + 1;
          s = (char *)zmalloc(*max);
          if (cp->fl_dir) {
            strcpy(s, astr_cstr(cp->path));
            strncat(s, cp->match, cp->matchsize);
          } else
            strncpy(s, cp->match, cp->matchsize);
          s[len] = '\0';
          if (strncmp(s, *p, len) != 0)
            thistab = -1;
          free(*p);
          *p = s;
          break;
        case COMPLETION_NOTMATCHED:
          ding();
        }
      }
      break;
    case ' ':
      if (cp != NULL && !cp->fl_space)
        goto got_tab;
      /* FALLTHROUGH */
    default:
      if (c > 255 || !isprint(c)) {
        ding();
        break;
      }
      if (!overwrite_mode || i == len) {
        char *s;
        if (len >= *max - 1) {
          *max *= 2;
          *p = (char *)zrealloc(*p, *max);
        }
        s = *p;
        for (; *s != '\0'; s++)
          ;
        for (++s; s > *p + i; --s)
          *s = *(s - 1);
        *(*p + i) = c;
        ++i, ++len;
      } else
        (*p)[i++] = c;
    }

    lasttab = thistab;
  }
}

/*
 * Rotation of text buffer prevents trashing of the returned string
 * when doing less or equal `MAX_ROTATIONS' sequential calls.
 * This means that the string returned by `minibuf_read()' need
 * not be deallocated by the user.
 */
#define MAX_ROTATIONS	5

static char *rotation_buffers[MAX_ROTATIONS];

char *term_minibuf_read(const char *prompt, const char *value,
                        Completion *cp, History *hp)
{
  static size_t max[MAX_ROTATIONS], rot;
  Window *wp, *old_wp = cur_wp;
  char **p = rotation_buffers, *s;

  /* Prepare the history. */
  if (hp)
    prepare_history(hp);

  /* Rotate text buffer. */
  if (++rot >= MAX_ROTATIONS)
    rot = 0;

  s = rot_vminibuf_read(prompt, value, cp, hp, &p[rot], &max[rot]);

  if (cp != NULL && cp->fl_poppedup && (wp = find_window("*Completions*")) != NULL) {
    set_current_window(wp);
    if (cp->fl_close)
      FUNCALL(delete_window);
    else if (cp->old_bp)
      switch_to_buffer(cp->old_bp);
    set_current_window(old_wp);
  }

  return s;
}

void free_rotation_buffers(void)
{
  int i;
  for (i = 0; i < MAX_ROTATIONS; ++i)
    if (rotation_buffers[i] != NULL)
      free(rotation_buffers[i]);
}
