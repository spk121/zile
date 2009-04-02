/* Buffer-oriented functions

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009 Free Software Foundation, Inc.

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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"


/*
 * Structure
 */

struct Buffer
{
#define FIELD(ty, name) ty name;
#define FIELD_STR(name) const char *name;
#include "buffer.h"
#undef FIELD
#undef FIELD_STR
};

#define FIELD(ty, field)                         \
  GETTER (Buffer, buffer, ty, field)             \
  SETTER (Buffer, buffer, ty, field)

#define FIELD_STR(field)                         \
  GETTER (Buffer, buffer, const char *, field)   \
  STR_SETTER (Buffer, buffer, field)

#include "buffer.h"
#undef FIELD
#undef FIELD_STR

struct Region
{
#define FIELD(ty, name) ty name;
#include "region.h"
#undef FIELD
};

#define FIELD(ty, field)                         \
  GETTER (Region, region, ty, field)             \
  SETTER (Region, region, ty, field)

#include "region.h"
#undef FIELD

/*
 * Allocate a new buffer structure, set the default local
 * variable values, and insert it into the buffer list.
 * The allocation of the first empty line is done here to simplify
 * the code.
 */
Buffer *
buffer_new (void)
{
  Buffer *bp = (Buffer *) XZALLOC (Buffer);

  /* Allocate a line. */
  bp->pt.p = line_new ();
  set_line_text (bp->pt.p, astr_new ());

  /* Allocate the limit marker. */
  bp->lines = line_new ();

  set_line_prev (bp->lines, bp->pt.p);
  set_line_next (bp->lines, bp->pt.p);
  set_line_prev (bp->pt.p, bp->lines);
  set_line_next (bp->pt.p, bp->lines);

  /* Set default EOL string. */
  bp->eol = coding_eol_lf;

  /* Insert into buffer list. */
  bp->next = head_bp;
  head_bp = bp;

  init_buffer (bp);

  return bp;
}

/*
 * Free the buffer's allocated memory.
 */
void
free_buffer (Buffer * bp)
{
  line_delete (bp->lines);
  free_undo (bp->last_undop);

  while (bp->markers)
    free_marker (bp->markers);

  free ((char *) bp->name);
  free ((char *) bp->filename);

  if (bp->vars != 0)
    free_variable_list (bp->vars);

  free (bp);
}

/*
 * Free all the allocated buffers (used at Zile exit).
 */
void
free_buffers (void)
{
  Buffer *bp, *next;

  for (bp = head_bp; bp != NULL; bp = next)
    {
      next = bp->next;
      free_buffer (bp);
    }
}

/*
 * Initialise a buffer
 */
void
init_buffer (Buffer * bp)
{
  if (get_variable_bool ("auto-fill-mode"))
    set_buffer_autofill (bp, true);
}

/*
 * Get filename, or buffer name if NULL.
 */
const char *
get_buffer_filename_or_name (Buffer * bp)
{
  const char *fname = get_buffer_filename (bp);
  return fname ? fname : get_buffer_name (bp);
}

/*
 * Create a buffer name using the file name.
 */
static char *
make_buffer_name (const char *filename)
{
  const char *p = strrchr (filename, '/');

  if (p == NULL)
    p = filename;
  else
    ++p;

  if (find_buffer (p) == NULL)
    return xstrdup (p);
  else
    {
      char *name;
      size_t i;

      /* Note: there can't be more than SIZE_MAX buffers. */
      for (i = 2; true; i++)
        {
          xasprintf (&name, "%s<%ld>", p, i);
          if (find_buffer (name) == NULL)
            return name;
          free (name);
        }
    }
}

/*
 * Set a new filename, and from it a name, for the buffer.
 */
void
set_buffer_names (Buffer * bp, const char *filename)
{
  if (filename[0] != '/')
    {
      astr as = agetcwd ();
      astr_cat_char (as, '/');
      astr_cat_cstr (as, filename);
      set_buffer_filename (bp, astr_cstr (as));
      filename = xstrdup (astr_cstr (as));
      astr_delete (as);
    }
  else
    set_buffer_filename (bp, filename);

  free ((char *) bp->name);
  bp->name = make_buffer_name (filename);
}

/*
 * Search for a buffer named `name'.
 */
Buffer *
find_buffer (const char *name)
{
  Buffer *bp;

  for (bp = head_bp; bp != NULL; bp = bp->next)
    {
      const char *bname = get_buffer_name (bp);
      if (bname && !strcmp (bname, name))
        return bp;
    }

  return NULL;
}

/* Move the selected buffer to head.  */

static void
move_buffer_to_head (Buffer * bp)
{
  Buffer *it, *prev = NULL;

  for (it = head_bp; it; prev = it, it = it->next)
    {
      if (bp == it)
        {
          if (prev)
            {
              prev->next = bp->next;
              bp->next = head_bp;
              head_bp = bp;
            }
          break;
        }
    }
}

/*
 * Switch to the specified buffer.
 */
void
switch_to_buffer (Buffer * bp)
{
  assert (get_window_bp (cur_wp) == cur_bp);

  /* The buffer is the current buffer; return safely.  */
  if (cur_bp == bp)
    return;

  /* Set current buffer.  */
  cur_bp = bp;
  set_window_bp (cur_wp, cur_bp);

  /* Move the buffer to head.  */
  move_buffer_to_head (bp);

  thisflag |= FLAG_NEED_RESYNC;
}

/*
 * Print an error message into the echo area and return true
 * if the current buffer is readonly; otherwise return false.
 */
int
warn_if_readonly_buffer (void)
{
  if (get_buffer_readonly (cur_bp))
    {
      minibuf_error ("Buffer is readonly: %s", get_buffer_name (cur_bp));
      return true;
    }

  return false;
}

static int
warn_if_no_mark (void)
{
  if (!cur_bp->mark)
    {
      minibuf_error ("The mark is not set now");
      return true;
    }
  else if (!get_buffer_mark_active (cur_bp) && transient_mark_mode ())
    {
      minibuf_error ("The mark is not active now");
      return true;
    }
  else
    return false;
}

Region *
region_new (void)
{
  return (Region *) XZALLOC (Region);
}

/*
 * Calculate the region size between point and mark and set the region
 * structure.
 */
int
calculate_the_region (Region * rp)
{
  if (warn_if_no_mark ())
    return false;

  if (cmp_point (cur_bp->pt, get_marker_pt (cur_bp->mark)) < 0)
    {
      /* Point is before mark. */
      set_region_start (rp, cur_bp->pt);
      set_region_end (rp, get_marker_pt (cur_bp->mark));
    }
  else
    {
      /* Mark is before point. */
      set_region_start (rp, get_marker_pt (cur_bp->mark));
      set_region_end (rp, cur_bp->pt);
    }

  set_region_size (rp, point_dist (get_region_start (rp),
                                   get_region_end (rp)));
  return true;
}

bool
delete_region (const Region * rp)
{
  size_t size = get_region_size (rp);

  if (warn_if_readonly_buffer ())
    return false;

  if (get_buffer_pt (cur_bp).p != get_region_start (rp).p ||
      get_region_start (rp).o != get_buffer_pt (cur_bp).o)
    FUNCALL (exchange_point_and_mark);

  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), size, 0);
  undo_nosave = true;
  while (size--)
    FUNCALL (delete_char);
  undo_nosave = false;

  return true;
}

bool
in_region (size_t lineno, size_t x, Region * rp)
{
  if (lineno >= rp->start.n && lineno <= rp->end.n)
    {
      if (rp->start.n == rp->end.n)
        {
          if (x >= rp->start.o && x < rp->end.o)
            return true;
        }
      else if (lineno == rp->start.n)
        {
          if (x >= rp->start.o)
            return true;
        }
      else if (lineno == rp->end.n)
        {
          if (x < rp->end.o)
            return true;
        }
      else
        return true;
    }

  return false;
}

/*
 * Set the specified buffer temporary flag and move the buffer
 * to the end of the buffer list.
 */
void
set_temporary_buffer (Buffer * bp)
{
  Buffer *bp0;

  set_buffer_temporary (bp, true);

  if (bp == head_bp)
    {
      if (head_bp->next == NULL)
        return;
      head_bp = head_bp->next;
    }
  else if (bp->next == NULL)
    return;

  for (bp0 = head_bp; bp0 != NULL; bp0 = bp0->next)
    if (bp0->next == bp)
      {
        bp0->next = bp0->next->next;
        break;
      }

  for (bp0 = head_bp; bp0->next != NULL; bp0 = bp0->next)
    ;

  bp0->next = bp;
  bp->next = NULL;
}

size_t
calculate_buffer_size (Buffer * bp)
{
  Line *lp = get_line_next (bp->lines);
  size_t size = 0;

  if (lp == bp->lines)
    return 0;

  for (;;)
    {
      size += astr_len (get_line_text (lp));
      lp = get_line_next (lp);
      if (lp == bp->lines)
        break;
      ++size;
    }

  return size;
}

int
transient_mark_mode (void)
{
  return get_variable_bool ("transient-mark-mode");
}

void
activate_mark (void)
{
  set_buffer_mark_active (cur_bp, true);
}

void
deactivate_mark (void)
{
  set_buffer_mark_active (cur_bp, false);
}

/*
 * Return a safe tab width for the given buffer.
 */
size_t
tab_width (Buffer * bp)
{
  size_t t = get_variable_number_bp (bp, "tab-width");

  return t ? t : 1;
}

/*
 * Copy a region of text into an allocated buffer.
 */
char *
copy_text_block (size_t startn, size_t starto, size_t size)
{
  char *buf, *dp;
  size_t max_size, n, i;
  Line *lp;

  max_size = 10;
  dp = buf = (char *) xzalloc (max_size);

  lp = cur_bp->pt.p;
  n = cur_bp->pt.n;
  if (n > startn)
    do
      lp = get_line_prev (lp);
    while (--n > startn);
  else if (n < startn)
    do
      lp = get_line_next (lp);
    while (++n < startn);

  for (i = starto; dp - buf < (int) size;)
    {
      if (dp >= buf + max_size)
        {
          int save_off = dp - buf;
          max_size += 10;
          buf = (char *) xrealloc (buf, max_size);
          dp = buf + save_off;
        }
      if (i < astr_len (get_line_text (lp)))
        *dp++ = astr_get (get_line_text (lp), i++);
      else
        {
          *dp++ = '\n';
          lp = get_line_next (lp);
          i = 0;
        }
    }

  return buf;
}

Buffer *
create_scratch_buffer (void)
{
  Buffer *bp = buffer_new ();
  set_buffer_name (bp, "*scratch*");
  set_buffer_needname (bp, true);
  set_buffer_temporary (bp, true);
  set_buffer_nosave (bp, true);
  return bp;
}

/*
 * Remove the specified buffer from the buffer list and deallocate
 * its space.  Avoid killing the sole buffers and creates the scratch
 * buffer when required.
 */
void
kill_buffer (Buffer * kill_bp)
{
  Buffer *next_bp;

  if (get_buffer_next (kill_bp) != NULL)
    next_bp = get_buffer_next (kill_bp);
  else
    next_bp = head_bp;

  if (next_bp == kill_bp)
    {
      Window *wp;

      create_scratch_buffer ();
      assert (cur_bp == kill_bp);
      free_buffer (cur_bp);

      /* Close all the windows that display this buffer. */
      for (wp = head_wp; wp != NULL; wp = get_window_next (wp))
        if (get_window_bp (wp) == cur_bp)
          delete_window (wp);
    }
  else
    {
      Buffer *bp;
      Window *wp;

      /* Search for windows displaying the buffer to kill. */
      for (wp = head_wp; wp != NULL; wp = get_window_next (wp))
        if (get_window_bp (wp) == kill_bp)
          {
            set_window_bp (wp, next_bp);
            set_window_topdelta (wp, 0);
            set_window_saved_pt (wp, NULL);	/* The marker will be freed. */
          }

      /* Remove the buffer from the buffer list. */
      cur_bp = next_bp;
      if (head_bp == kill_bp)
        head_bp = get_buffer_next (head_bp);
      for (bp = head_bp; get_buffer_next (bp) != NULL; bp = get_buffer_next (bp))
        if (get_buffer_next (bp) == kill_bp)
          {
            set_buffer_next (bp, get_buffer_next (get_buffer_next (bp)));
            break;
          }

      free_buffer (kill_bp);

      thisflag |= FLAG_NEED_RESYNC;
    }
}

DEFUN_ARGS ("kill-buffer", kill_buffer,
            STR_ARG (buffer))
/*+
Kill buffer BUFFER.
With a nil argument, kill the current buffer.
+*/
{
  Buffer *bp;

  STR_INIT (buffer)
  else
    {
      Completion *cp = make_buffer_completion ();
      buffer = minibuf_read_completion ("Kill buffer (default %s): ",
                                        "", cp, NULL, get_buffer_name (cur_bp));
      free_completion (cp);
      if (buffer == NULL)
        ok = FUNCALL (keyboard_quit);
    }

  if (buffer && buffer[0] != '\0')
    {
      bp = find_buffer (buffer);
      if (bp == NULL)
        {
          minibuf_error ("Buffer `%s' not found", buffer);
          free ((char *) buffer);
          ok = leNIL;
        }
    }
  else
    bp = cur_bp;

  if (ok == leT)
    {
      if (!check_modified_buffer (bp))
        ok = leNIL;
      else
        kill_buffer (bp);
    }

  STR_FREE (buffer);
}
END_DEFUN

Completion *
make_buffer_completion (void)
{
  Buffer *bp;
  Completion *cp;

  cp = completion_new (false);
  for (bp = head_bp; bp != NULL; bp = get_buffer_next (bp))
    gl_sortedlist_add (get_completion_completions (cp), completion_strcmp,
                       xstrdup (get_buffer_name (bp)));

  return cp;
}

/*
 * Check if the buffer has been modified.  If so, asks the user if
 * he/she wants to save the changes.  If the response is positive, return
 * true, else false.
 */
bool
check_modified_buffer (Buffer * bp)
{
  if (get_buffer_modified (bp) && !get_buffer_nosave (bp))
    for (;;)
      {
        int ans = minibuf_read_yesno
          ("Buffer %s modified; kill anyway? (yes or no) ", get_buffer_name (bp));
        if (ans == -1)
          {
            FUNCALL (keyboard_quit);
            return false;
          }
        else if (!ans)
          return false;
        break;
      }

  return true;
}
