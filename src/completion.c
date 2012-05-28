/* Completion facility functions

   Copyright (c) 1997-2004, 2008-2012 Free Software Foundation, Inc.

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

#include <sys/stat.h>
#include <assert.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "dirname.h"
#include "gl_linked_list.h"

#include "main.h"
#include "extern.h"


/*
 * Structure
 */
struct Completion
{
#define FIELD(ty, name) ty name;
#define FIELD_STR(name) char *name;
#include "completion.h"
#undef FIELD
#undef FIELD_STR
};

#define FIELD(ty, field)                        \
  GETTER (Completion, completion, ty, field)    \
  SETTER (Completion, completion, ty, field)

#define FIELD_STR(field)                                  \
  GETTER (Completion, completion, char *, field)          \
  STR_SETTER (Completion, completion, field)

#include "completion.h"
#undef FIELD
#undef FIELD_STR

/*
 * List methods for completions and matches
 */
int
completion_strcmp (const void *p1, const void *p2)
{
  return strcmp ((const char *) p1, (const char *) p2);
}

static bool
completion_STREQ (const void *p1, const void *p2)
{
  return STREQ ((const char *) p1, (const char *) p2);
}

/*
 * Allocate a new completion structure.
 */
Completion *
completion_new (int fileflag)
{
  Completion *cp = (Completion *) XZALLOC (Completion);

  cp->completions = gl_list_create_empty (GL_LINKED_LIST,
                                          completion_STREQ, NULL,
                                          NULL, false);
  cp->matches = gl_list_create_empty (GL_LINKED_LIST,
                                      completion_STREQ, NULL,
                                      NULL, false);

  if (fileflag)
    {
      cp->path = astr_new ();
      cp->flags |= CFLAG_FILENAME;
    }

  return cp;
}

/*
 * Scroll completions up.
 */
void
completion_scroll_up (void)
{
  Window *old_wp = cur_wp;
  Window *wp = find_window ("*Completions*");
  assert (wp != NULL);
  set_current_window (wp);
  if (FUNCALL (scroll_up) == leNIL)
    {
      FUNCALL (beginning_of_buffer);
      window_resync (cur_wp);
    }
  set_current_window (old_wp);

  term_redisplay ();
}

/*
 * Scroll completions down.
 */
void
completion_scroll_down (void)
{
  Window *old_wp = cur_wp;
  Window *wp = find_window ("*Completions*");
  assert (wp != NULL);
  set_current_window (wp);
  if (FUNCALL (scroll_down) == leNIL)
    {
      FUNCALL (end_of_buffer);
      window_resync (cur_wp);
    }
  set_current_window (old_wp);

  term_redisplay ();
}

/*
 * Calculate the maximum length of the completions.
 */
static size_t
calculate_max_length (gl_list_t l)
{
  size_t maxlen = 0;
  for (size_t i = 0; i < gl_list_size (l); i++)
    maxlen = MAX (strlen ((const char *) gl_list_get_at (l, i)), maxlen);
  return maxlen;
}

/*
 * Print the list of completions in a set of columns.
 */
static void
write_completion (va_list ap)
{
  gl_list_t l = va_arg (ap, gl_list_t);
  size_t max = calculate_max_length (l) + 5;
  size_t numcols = (get_window_ewidth (cur_wp) - 1) / max;

  bprintf ("Possible completions are:\n");
  for (size_t i = 0, col = 0; i < gl_list_size (l); i++)
    {
      bprintf ("%-*s", max, (const char *) gl_list_get_at (l, i));

      col = (col + 1) % numcols;
      if (col == 0)
        insert_newline ();
    }
}

/*
 * Popup the completion window.
 */
static void
popup_completion (Completion * cp, int allflag)
{
  cp->flags |= CFLAG_POPPEDUP;
  if (get_window_next (head_wp) == NULL)
    cp->flags |= CFLAG_CLOSE;

  write_temp_buffer ("*Completions*", true, write_completion, allflag ? cp->completions : cp->matches);

  if (!(cp->flags & CFLAG_CLOSE))
    cp->old_bp = cur_bp;

  term_redisplay ();
}

/*
 * Reread directory for completions.
 */
static int
completion_readdir (Completion * cp, astr path)
{
  cp->completions = gl_list_create_empty (GL_LINKED_LIST,
                                          completion_STREQ, NULL,
                                          NULL, false);

  if (!expand_path (path))
    return false;

  /* Split up path with dirname and basename, unless it ends in `/',
     in which case it's considered to be entirely dirname. */
  astr pdir;
  if (astr_get (path, astr_len (path) - 1) != '/')
    {
      pdir = astr_new_cstr (dir_name (astr_cstr (path)));
      if (!STREQ (astr_cstr (pdir), "/"))
        astr_cat_char (pdir, '/');
      astr_cpy_cstr (path, base_name (astr_cstr (path)));
    }
  else
    {
      pdir = astr_cpy (astr_new (), path);
      astr_truncate (path, 0);
    }

  DIR *dir = opendir (astr_cstr (pdir));
  if (dir != NULL)
    {
      for (struct dirent *d = readdir (dir); d != NULL; d = readdir (dir))
        {
          astr as = astr_new_cstr (d->d_name);
          struct stat st;
          if (stat (xasprintf ("%s%s", astr_cstr (pdir), d->d_name), &st) == 0 &&
              S_ISDIR (st.st_mode))
            astr_cat_char (as, '/');
          gl_sortedlist_add (cp->completions, completion_strcmp,
                             xstrdup (astr_cstr (as)));
        }
      closedir (dir);

      cp->path = compact_path (pdir);
    }

  return dir != NULL;
}

/*
 * Match completions.
 */
int
completion_try (Completion * cp, astr search, int popup_when_complete)
{
  cp->matches = gl_list_create_empty (GL_LINKED_LIST, completion_STREQ, NULL, NULL, false);

  if (cp->flags & CFLAG_FILENAME)
    if (!completion_readdir (cp, search))
      return COMPLETION_NOTMATCHED;

  size_t ssize = astr_len (search);
  if (ssize == 0)
    {
      cp->match = xstrdup ((const char *) gl_list_get_at (cp->completions, 0));
      if (gl_list_size (cp->completions) > 1)
        {
          cp->matchsize = 0;
          popup_completion (cp, true);
          return COMPLETION_NONUNIQUE;
        }
      else
        {
          cp->matchsize = strlen (cp->match);
          return COMPLETION_MATCHED;
        }
    }

  size_t fullmatches = 0;
  for (size_t i = 0; i < gl_list_size (cp->completions); i++)
    {
      const char *s = (const char *) gl_list_get_at (cp->completions, i);
      if (!strncmp (s, astr_cstr (search), ssize))
        {
          gl_sortedlist_add (cp->matches, completion_strcmp, s);
          if (STREQ (s, astr_cstr (search)))
            ++fullmatches;
        }
    }

  if (gl_list_size (cp->matches) == 0)
    return COMPLETION_NOTMATCHED;
  else if (gl_list_size (cp->matches) == 1)
    {
      cp->match = xstrdup ((const char *) gl_list_get_at (cp->matches, 0));
      cp->matchsize = strlen (cp->match);
      return COMPLETION_MATCHED;
    }
  else if (gl_list_size (cp->matches) > 1 && fullmatches == 1)
    {
      cp->match = xstrdup ((const char *) gl_list_get_at (cp->matches, 0));
      cp->matchsize = strlen (cp->match);
      if (popup_when_complete)
        popup_completion (cp, false);
      return COMPLETION_MATCHEDNONUNIQUE;
    }

  for (size_t j = ssize;; ++j)
    {
      char c = ((const char *) gl_list_get_at (cp->matches, 0))[j];
      for (size_t i = 1; i < gl_list_size (cp->matches); ++i)
        {
          if (((const char *)(gl_list_get_at (cp->matches, i)))[j] != c)
            {
              cp->match = xstrdup ((const char *) gl_list_get_at (cp->matches, 0));
              cp->matchsize = j;
              popup_completion (cp, false);
              return COMPLETION_NONUNIQUE;
            }
        }
    }

  abort ();
}
