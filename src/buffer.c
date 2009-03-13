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
 * Allocate a new buffer structure and set the default local
 * variable values.
 * The allocation of the first empty line is done here to simplify
 * a lot the code.
 */
static Buffer *
buffer_new (void)
{
  Buffer *bp;

  bp = (Buffer *) XZALLOC (Buffer);

  /* Allocate a line. */
  bp->pt.p = line_new ();
  bp->pt.p->text = astr_new ();

  /* Allocate the limit marker. */
  bp->lines = line_new ();

  bp->lines->prev = bp->lines->next = bp->pt.p;
  bp->pt.p->prev = bp->pt.p->next = bp->lines;

  /* Set default EOL string. */
  bp->eol = coding_eol_lf;

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

  free (bp->name);
  free (bp->filename);

  if (bp->vars != NULL)
    hash_free (bp->vars);

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
    bp->flags ^= BFLAG_AUTOFILL;
}

/*
 * Allocate a new buffer and insert it into the buffer list.
 */
Buffer *
create_buffer (const char *name)
{
  Buffer *bp;

  bp = buffer_new ();
  set_buffer_name (bp, name);

  bp->next = head_bp;
  head_bp = bp;

  init_buffer (bp);

  return bp;
}

/*
 * Set a new name for the buffer.
 */
void
set_buffer_name (Buffer * bp, const char *name)
{
  free (bp->name);
  bp->name = xstrdup (name);
}

/*
 * Set a new filename (and a name) for the buffer.
 */
void
set_buffer_filename (Buffer * bp, const char *filename)
{
  free (bp->filename);
  bp->filename = xstrdup (filename);

  free (bp->name);
  bp->name = make_buffer_name (filename);
}

/*
 * Search for a buffer named `name'.  If buffer is not found and
 * the `cflag' variable is set to `true', create a new buffer.
 */
Buffer *
find_buffer (const char *name, int cflag)
{
  Buffer *bp;

  for (bp = head_bp; bp != NULL; bp = bp->next)
    if (!strcmp (bp->name, name))
      return bp;

  /*
   * Create flag not specified, return NULL.
   */
  if (!cflag)
    return NULL;

  /*
   * No buffer found with the specified name, then
   * create one.
   */
  bp = create_buffer (name);

  return bp;
}

/*
 * Create a buffer name using the file name.
 */
char *
make_buffer_name (const char *filename)
{
  const char *p = strrchr (filename, '/');
  char *name;
  int i;

  if (p == NULL)
    p = filename;
  else
    ++p;

  if (find_buffer (p, false) == NULL)
    return xstrdup (p);

  /*
   * The limit of 999 buffers with the same name should
   * be enough; if it is too much restrictive for you,
   * just change 999 to your preferred value :-)
   */
  name = (char *) xzalloc (strlen (p) + 6);	/* name + "<nnn>\0" */
  for (i = 2; i <= 999; i++)
    {
      sprintf (name, "%s<%d>", p, i);
      if (find_buffer (name, false) == NULL)
        return name;
    }

   /* This should never happen. */
  abort ();
}

/* Move the selected buffer to head.  */

static void
move_buffer_to_head (Buffer * bp)
{
  Buffer *it, *prev = NULL;

  for (it = head_bp; it; it = it->next)
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
      prev = it;
    }
}

/*
 * Switch to the specified buffer.
 */
void
switch_to_buffer (Buffer * bp)
{
  assert (cur_wp->bp == cur_bp);

  /* The buffer is the current buffer; return safely.  */
  if (cur_bp == bp)
    return;

  /* Set current buffer.  */
  cur_wp->bp = cur_bp = bp;

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
  if (cur_bp->flags & BFLAG_READONLY)
    {
      minibuf_error ("Buffer is readonly: %s", cur_bp->name);
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
  else if (!(cur_bp->flags & BFLAG_MARK) && transient_mark_mode ())
    {
      minibuf_error ("The mark is not active now");
      return true;
    }
  else
    return false;
}

/*
 * Calculate the region size between point and mark and set the region
   structure.
 */
int
calculate_the_region (Region * rp)
{
  if (warn_if_no_mark ())
    return false;

  if (cmp_point (cur_bp->pt, cur_bp->mark->pt) < 0)
    {
      /* Point is before mark. */
      rp->start = cur_bp->pt;
      rp->end = cur_bp->mark->pt;
    }
  else
    {
      /* Mark is before point. */
      rp->start = cur_bp->mark->pt;
      rp->end = cur_bp->pt;
    }

  rp->size = point_dist (rp->start, rp->end);
  return true;
}

/*
 * Set the specified buffer temporary flag and move the buffer
 * to the end of the buffer list.
 */
void
set_temporary_buffer (Buffer * bp)
{
  Buffer *bp0;

  bp->flags |= BFLAG_TEMPORARY;

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
  Line *lp = bp->lines->next;
  size_t size = 0;

  if (lp == bp->lines)
    return 0;

  for (;;)
    {
      size += astr_len (lp->text);
      lp = lp->next;
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
  cur_bp->flags |= BFLAG_MARK;
}

void
deactivate_mark (void)
{
  cur_bp->flags &= ~BFLAG_MARK;
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
      lp = lp->prev;
    while (--n > startn);
  else if (n < startn)
    do
      lp = lp->next;
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
      if (i < astr_len (lp->text))
        *dp++ = astr_get (lp->text, i++);
      else
        {
          *dp++ = '\n';
          lp = lp->next;
          i = 0;
        }
    }

  return buf;
}
