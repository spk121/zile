/* Buffer-oriented functions

   Copyright (c) 1997-2006, 2008-2011 Free Software Foundation, Inc.

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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "dirname.h"

#include "main.h"
#include "extern.h"
#include "memrmem.h"


/*
 * Buffer structure
 */
struct Buffer
{
#define FIELD(ty, name) ty name;
#define FIELD_STR(name) char *name;
#include "buffer.h"
#undef FIELD
#undef FIELD_STR
  estr text;         /* The text. */
  size_t gap;        /* Size of gap after point. */
};

#define FIELD(ty, field)                         \
  GETTER (Buffer, buffer, ty, field)             \
  SETTER (Buffer, buffer, ty, field)

#define FIELD_STR(field)                         \
  GETTER (Buffer, buffer, char *, field)         \
  STR_SETTER (Buffer, buffer, field)

#include "buffer.h"
#undef FIELD
#undef FIELD_STR

/* Buffer methods that know about the gap. */

size_t
get_buffer_size (Buffer * bp)
{
  return astr_len (bp->text.as);
}

size_t
buffer_line_len (Buffer *bp, size_t o)
{
  return estr_line_len (bp->text, o);
}

size_t
get_region_size (const Region r)
{
  return r.end - r.start;
}

castr
get_buffer_pre_point (Buffer *bp)
{
  return astr_substr (bp->text.as, 0, get_buffer_o (bp));
}

castr
get_buffer_post_point (Buffer *bp)
{
  return astr_substr (bp->text.as, get_buffer_o (bp), get_buffer_size (bp) - get_buffer_o (bp));
}

/*
 * Adjust markers (including point) at offset `o' by offset `delta'.
 */
static size_t
adjust_markers (size_t o, ptrdiff_t delta)
{
  Marker *m_pt = point_marker ();

  for (Marker *m = get_buffer_markers (cur_bp); m != NULL; m = get_marker_next (m))
    if (get_marker_o (m) > o)
      set_marker_o (m, MAX ((ptrdiff_t) o, (ptrdiff_t) get_marker_o (m) + delta));

  /* This marker has been updated to new position. */
  size_t m_o = get_marker_o (m_pt);
  unchain_marker (m_pt);
  return m_o;
}

/*
 * Replace text at offset `offset' with `newtext'. If `replace_case'
 * and `case-replace' are true then the new characters will be the
 * same case as the old.
 */
void
buffer_replace (Buffer *bp, size_t offset, size_t oldlen, const char *newtext, size_t newlen)
{
  undo_save_block (offset, oldlen, newlen);
  set_buffer_modified (bp, true);
  astr_nreplace_cstr (bp->text.as, offset, oldlen, newtext, newlen);
  bp->o = adjust_markers (offset, (ptrdiff_t) (newlen - oldlen));
  thisflag |= FLAG_NEED_RESYNC;
}

void
set_buffer_text (Buffer *bp, estr es)
{
  bp->text = es;
}

char
get_buffer_char (Buffer *bp, size_t o)
{
  return astr_get (bp->text.as, o);
}


/* Buffer methods that don't know about the gap. */

const char *
get_buffer_eol (Buffer *bp)
{
  return bp->text.eol;
}

size_t
buffer_prev_line (Buffer *bp, size_t o)
{
  return estr_prev_line (bp->text, o);
}

size_t
buffer_next_line (Buffer *bp, size_t o)
{
  return estr_next_line (bp->text, o);
}

size_t
buffer_start_of_line (Buffer *bp, size_t o)
{
  return estr_start_of_line (bp->text, o);
}

size_t
buffer_end_of_line (Buffer *bp, size_t o)
{
  return estr_end_of_line (bp->text, o);
}

Point
get_buffer_pt (Buffer *bp)
{
  return offset_to_point (bp, bp->o);
}

size_t
get_buffer_line_o (Buffer *bp)
{
  return estr_start_of_line (bp->text, bp->o);
}

/*
 * Copy a region of text into an allocated buffer.
 */
estr
get_buffer_region (Buffer *bp, Region r)
{
  astr as = astr_new ();
  if (r.start < get_buffer_o (bp))
    astr_cat (as, astr_substr (get_buffer_pre_point (bp), r.start, MIN (r.end, get_buffer_o (bp)) - r.start));
  if (r.end > get_buffer_o (bp))
    {
      size_t from = MAX (r.start, get_buffer_o (bp)) - get_buffer_o (bp);
      astr_cat (as, astr_substr (get_buffer_post_point (bp), from, r.end - get_buffer_o (bp)));
    }
  return (estr) {.as = as, .eol = get_buffer_eol (bp)};
}

/*
 * Insert the character `c' at the current point position
 * into the current buffer.
 * In overwrite mode, overwrite if current character isn't the end of
 * line or a \t, or current char is a \t and we are on the tab limit.
 */
int
type_char (int c, bool overwrite)
{
  char ch = (char) c;
  size_t t = tab_width (cur_bp);

  return replace_estr (overwrite &&
                       ((!eolp () && following_char () != '\t')
                        || ((following_char () == '\t') && ((get_goalc () % t) == t - 1))) ? 1 : 0,
                       (estr) {.as = castr_new_nstr (&ch, 1), .eol = coding_eol_lf});
}

bool
delete_char (void)
{
  deactivate_mark ();

  if (eobp ())
    {
      minibuf_error ("End of buffer");
      return false;
    }

  if (warn_if_readonly_buffer ())
    return false;

  if (eolp ())
    {
      buffer_replace (cur_bp, cur_bp->o, strlen (get_buffer_eol (cur_bp)), NULL, 0);
      thisflag |= FLAG_NEED_RESYNC;
    }
  else
    buffer_replace (cur_bp, cur_bp->o, 1, NULL, 0);

  set_buffer_modified (cur_bp, true);

  return true;
}

void
insert_buffer (Buffer * bp)
{
  /* Copy text to avoid problems when bp == cur_bp. */
  insert_estr ((estr) {.as = astr_cpy (astr_new (), get_buffer_pre_point (bp)), .eol = get_buffer_eol (bp)});
  insert_estr ((estr) {.as = astr_cpy (astr_new (), get_buffer_post_point (bp)), .eol = get_buffer_eol (bp)});
}

/*
 * Allocate a new buffer structure, set the default local
 * variable values, and insert it into the buffer list.
 */
Buffer *
buffer_new (void)
{
  Buffer *bp = (Buffer *) XZALLOC (Buffer);
  bp->text.as = astr_new ();
  bp->text.eol = coding_eol_lf;
  bp->dir = agetcwd ();

  /* Insert into buffer list. */
  bp->next = head_bp;
  head_bp = bp;

  init_buffer (bp);

  return bp;
}

/*
 * Unchain the buffer's markers.
 */
void
free_buffer (Buffer * bp)
{
  while (bp->markers)
    unchain_marker (bp->markers);
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
 * Set a new filename, and from it a name, for the buffer.
 */
void
set_buffer_names (Buffer * bp, const char *filename)
{
  if (filename[0] != '/')
    filename = astr_cstr (astr_fmt ("%s/%s", astr_cstr (agetcwd ()), filename));
  set_buffer_filename (bp, filename);

  char *s = base_name (filename);
  char *name = xstrdup (s);
  /* Note: there can't be more than SIZE_MAX buffers. */
  for (size_t i = 2; find_buffer (name) != NULL; i++)
    name = xasprintf ("%s<%zd>", s, i);
  set_buffer_name (bp, name);
}

/*
 * Search for a buffer named `name'.
 */
Buffer *
find_buffer (const char *name)
{
  for (Buffer *bp = head_bp; bp != NULL; bp = bp->next)
    {
      const char *bname = get_buffer_name (bp);
      if (bname && STREQ (bname, name))
        return bp;
    }

  return NULL;
}

/*
 * Move the given buffer to head.
 */
static void
move_buffer_to_head (Buffer * bp)
{
  Buffer *prev = NULL;
  for (Buffer *it = head_bp; it != bp; prev = it, it = it->next)
    ;
  if (prev)
    {
      prev->next = bp->next;
      bp->next = head_bp;
      head_bp = bp;
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

  /* Change to buffer's default directory.  */
  if (chdir (astr_cstr (bp->dir))) {
    /* Avoid compiler warning for ignoring return value. */
  }

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

int
warn_if_no_mark (void)
{
  if (!cur_bp->mark)
    {
      minibuf_error ("The mark is not set now");
      return true;
    }
  else if (!get_buffer_mark_active (cur_bp))
    {
      minibuf_error ("The mark is not active now");
      return true;
    }
  return false;
}

/*
 * Make a region from two offsets.
 */
Region
region_new (size_t o1, size_t o2)
{
  return (Region) {.start = MIN (o1, o2), .end = MAX (o1, o2)};
}

/*
 * Return the region between point and mark.
 */
Region
calculate_the_region (void)
{
  return region_new (cur_bp->o, get_marker_o (cur_bp->mark));
}

bool
delete_region (const Region r)
{
  if (warn_if_readonly_buffer ())
    return false;

  buffer_replace (cur_bp, r.start, get_region_size (r), NULL, 0);
  return true;
}

bool
in_region (size_t o, size_t x, Region r)
{
  return o + x >= r.start && o + x < r.end;
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
  return MAX (get_variable_number_bp (bp, "tab-width"), 1);
}

Buffer *
create_auto_buffer (const char *name)
{
  Buffer *bp = buffer_new ();
  set_buffer_name (bp, name);
  set_buffer_needname (bp, true);
  set_buffer_temporary (bp, true);
  set_buffer_nosave (bp, true);
  return bp;
}

Buffer *
create_scratch_buffer (void)
{
  return create_auto_buffer ("*scratch*");
}

/*
 * Remove the specified buffer from the buffer list and deallocate
 * its space.  Recreate the scratch buffer when required.
 */
void
kill_buffer (Buffer * kill_bp)
{
  Buffer *next_bp;

  if (get_buffer_next (kill_bp) != NULL)
    next_bp = get_buffer_next (kill_bp);
  else
    next_bp = (head_bp == kill_bp) ? NULL : head_bp;

  /* Search for windows displaying the buffer to kill. */
  for (Window *wp = head_wp; wp != NULL; wp = get_window_next (wp))
    if (get_window_bp (wp) == kill_bp)
      {
        set_window_bp (wp, next_bp);
        set_window_topdelta (wp, 0);
        set_window_saved_pt (wp, NULL); /* The old marker will be freed. */
      }

  /* Remove the buffer from the buffer list. */
  if (cur_bp == kill_bp)
    cur_bp = next_bp;
  if (head_bp == kill_bp)
    head_bp = get_buffer_next (head_bp);
  for (Buffer *bp = head_bp; bp != NULL && get_buffer_next (bp) != NULL; bp = get_buffer_next (bp))
    if (get_buffer_next (bp) == kill_bp)
      {
        set_buffer_next (bp, get_buffer_next (get_buffer_next (bp)));
        break;
      }

  free_buffer (kill_bp);

  /* If no buffers left, recreate scratch buffer and point windows at
     it. */
  if (next_bp == NULL)
    {
      cur_bp = head_bp = next_bp = create_scratch_buffer ();
      for (Window *wp = head_wp; wp != NULL; wp = get_window_next (wp))
        set_window_bp (wp, head_bp);
    }

  /* Resync windows that need it. */
  for (Window *wp = head_wp; wp != NULL; wp = get_window_next (wp))
    if (get_window_bp (wp) == next_bp)
      resync_redisplay (wp);
}

DEFUN_ARGS ("kill-buffer", kill_buffer,
            STR_ARG (buf))
/*+
Kill buffer BUFFER.
With a nil argument, kill the current buffer.
+*/
{
  Buffer *bp;

  STR_INIT (buf)
  else
    {
      Completion *cp = make_buffer_completion ();
      buf = minibuf_read_completion ("Kill buffer (default %s): ",
                                     "", cp, NULL, get_buffer_name (cur_bp));
      if (buf == NULL)
        ok = FUNCALL (keyboard_quit);
    }

  if (buf && astr_len (buf) > 0)
    {
      bp = find_buffer (astr_cstr (buf));
      if (bp == NULL)
        {
          minibuf_error ("Buffer `%s' not found", buf);
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
}
END_DEFUN

Completion *
make_buffer_completion (void)
{
  Completion *cp = completion_new (false);
  for (Buffer *bp = head_bp; bp != NULL; bp = get_buffer_next (bp))
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


/* Basic movement routines */

bool
move_char (int offset)
{
  int dir = offset >= 0 ? 1 : -1;
  for (size_t i = 0; i < (size_t) (abs (offset)); i++)
    {
      if (dir > 0 ? !eolp () : !bolp ())
        cur_bp->o += dir;
      else if (dir > 0 ? !eobp () : !bobp ())
        {
          thisflag |= FLAG_NEED_RESYNC;
          cur_bp->o += dir * strlen (get_buffer_eol (cur_bp));
          if (dir > 0)
            FUNCALL (beginning_of_line);
          else
            FUNCALL (end_of_line);
        }
      else
        return false;
    }

  return true;
}

/*
 * Go to the goal column.  Take care of expanding tabulations.
 */
static void
goto_goalc (void)
{
  size_t i, col = 0, t = tab_width (cur_bp);

  for (i = get_buffer_line_o (cur_bp);
       i < get_buffer_line_o (cur_bp) + buffer_line_len (cur_bp, get_buffer_o (cur_bp));
       i++)
    if (col == get_buffer_goalc (cur_bp))
      break;
    else if (get_buffer_char (cur_bp, i) == '\t')
      for (size_t w = t - col % t; w > 0 && ++col < get_buffer_goalc (cur_bp); w--)
        ;
    else
      ++col;

  cur_bp->o = i;
}

bool
move_line (int n)
{
  if (n == 0)
    return false;

  bool ok = true;
  size_t (*func) (Buffer *bp, size_t o) = buffer_next_line;
  if (n < 0)
    {
      n = -n;
      func = buffer_prev_line;
    }

  if (last_command () != F_next_line && last_command () != F_previous_line)
    set_buffer_goalc (cur_bp, get_goalc ());

  for (; n > 0; n--)
    {
      size_t o = func (cur_bp, cur_bp->o);
      if (o == SIZE_MAX)
        {
          ok = false;
          break;
        }
      cur_bp->o = o;
    }

  goto_goalc ();
  thisflag |= FLAG_NEED_RESYNC;

  return ok;
}
