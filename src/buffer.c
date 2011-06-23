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

#include "main.h"
#include "extern.h"


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
  astr text;
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

size_t
get_buffer_size (Buffer * bp)
{
  return astr_len (bp->text);
}

void set_region_start (Region *rp, Point pt)
{
  rp->start = point_to_offset (pt);
}

void set_region_end (Region *rp, Point pt)
{
  rp->end = point_to_offset (pt);
}

Point get_region_start (const Region r)
{
  return offset_to_point (cur_bp, r.start);
}

Point get_region_end (const Region r)
{
  return offset_to_point (cur_bp, r.end);
}

size_t get_region_size (const Region r)
{
  return r.end - r.start;
}

/*
 * Line structure
 */
struct Line {
  Buffer *bp;
  size_t o;
};

const Line *
get_line_prev (const Line *lp)
{
  if (lp->o == 0)
    return NULL;
  const char *prev = memrmem (astr_cstr (lp->bp->text), lp->o - strlen (lp->bp->eol),
                              lp->bp->eol, strlen (lp->bp->eol));
  Line *n = XZALLOC (Line);
  *n = (Line) {.bp = lp->bp, .o = prev ? prev - astr_cstr (lp->bp->text) + strlen (lp->bp->eol) : 0};
  return n;
}

const Line *
get_line_next (const Line *lp)
{
  const char *next = memmem (astr_cstr (lp->bp->text) + lp->o, astr_len (lp->bp->text) - lp->o,
                             lp->bp->eol, strlen (lp->bp->eol));
  if (next == NULL)
    return NULL;
  Line *n = XZALLOC (Line);
  *n = (Line) {.bp = lp->bp, .o = next - astr_cstr (lp->bp->text) + strlen (lp->bp->eol)};
  return n;
}

castr
get_line_text (const Line *lp)
{
  const Line *next_lp = get_line_next (lp);
  size_t next = next_lp ? next_lp->o : astr_len (lp->bp->text) + strlen (lp->bp->eol);
  return castr_new_nstr (astr_cstr (lp->bp->text) + lp->o, next - lp->o - strlen (lp->bp->eol));
}

/*
 * Determine EOL type from buffer contents.
 */
/* Maximum number of EOLs to check before deciding type. */
#define MAX_EOL_CHECK_COUNT 3
void
buffer_set_eol_type (Buffer *bp)
{
  bool first_eol = true;
  size_t total_eols = 0;
  for (size_t i = 0; i < astr_len (bp->text) && total_eols < MAX_EOL_CHECK_COUNT; i++)
    {
      char c = astr_get (bp->text, i);
      if (c == '\n' || c == '\r')
        {
          const char *this_eol_type;
          total_eols++;
          if (c == '\n')
            this_eol_type = coding_eol_lf;
          else if (i == astr_len (bp->text) - 1 || astr_get (bp->text, i + 1) != '\n')
            this_eol_type = coding_eol_cr;
          else
            {
              this_eol_type = coding_eol_crlf;
              i++;
            }

          if (first_eol)
            {
              /* This is the first end-of-line. */
              set_buffer_eol (cur_bp, this_eol_type);
              first_eol = false;
            }
          else if (get_buffer_eol (cur_bp) != this_eol_type)
            {
              /* This EOL is different from the last; arbitrarily choose
                 LF. */
              set_buffer_eol (cur_bp, coding_eol_lf);
              break;
            }
        }
    }
}

/*
 * Adjust markers (including point) when text is edited.
 *   o is offset at which edit was made
 *   delta gives the number of characters inserted (>0) or
 *     deleted (<0)
 */
static void
adjust_markers (size_t o, ptrdiff_t delta)
{
  Marker *m_pt = point_marker ();

  for (Marker *m = get_buffer_markers (cur_bp); m != NULL; m = get_marker_next (m))
    {
      size_t pt_o = get_marker_o (m);
      if (pt_o > o)
        set_marker_o (m, MAX (o, pt_o + delta));
    }

  /* This marker has been updated to new position. */
  goto_point (get_marker_pt (m_pt));
  unchain_marker (m_pt);
}

/*
 * Check the case of a string.
 * Returns 2 if it is all upper case, 1 if just the first letter is,
 * and 0 otherwise.
 */
static int
check_case (const char *s, size_t len)
{
  size_t i;

  for (i = 0; i < len && isupper ((int) s[i]); i++)
    ;
  if (i == len)
    return 2;
  else if (i == 1)
    for (; i < len && !isupper ((int) s[i]); i++)
      ;
  return i == len;
}

/* Insert the character at the current position and move the text at its right
 * whatever the insert/overwrite mode is.
 * This function doesn't change the current position of the pointer.
 */
static int
intercalate_char (int c)
{
  if (warn_if_readonly_buffer ())
    return false;

  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), 0, 1);
  astr_insert_char (cur_bp->text, get_buffer_pt (cur_bp).p->o + get_buffer_pt (cur_bp).o, c);
  set_buffer_modified (cur_bp, true);

  return true;
}

/*
 * Insert the character `c' at the current point position
 * into the current buffer.
 */
int
insert_char (int c)
{
  size_t t = tab_width (cur_bp);

  if (warn_if_readonly_buffer ())
    return false;

  if (get_buffer_overwrite (cur_bp))
    {
      Point pt = get_buffer_pt (cur_bp);
      /* Current character isn't the end of line or a \t
         || current char is a \t && we are on the tab limit.  */
      if ((pt.o < astr_len (get_line_text (pt.p))) && ((astr_get (get_line_text (pt.p), pt.o) != '\t')
                                                       || ((astr_get (get_line_text (pt.p), pt.o) == '\t') && ((get_goalc () % t) == t))))
        {
          /* Replace the character.  */
          char ch = (char) c;
          undo_save (UNDO_REPLACE_BLOCK, pt, 1, 1);
          astr_nreplace_cstr (cur_bp->text, get_buffer_pt (cur_bp).p->o + get_buffer_pt (cur_bp).o, 1, &ch, 1);
          ++pt.o;
          goto_point (pt);
          set_buffer_modified (cur_bp, true);

          return true;
        }
      /*
       * Fall through the "insertion" mode of a character at the end
       * of the line, since it is the same as "overwrite" mode.
       */
    }

  intercalate_char (c);
  adjust_markers (cur_bp->pt.p->o + cur_bp->pt.o, 1);
  forward_char ();

  return true;
}

/*
 * Insert a newline at the current position without moving the cursor.
 * Update markers after point in the split line.
 */
bool
intercalate_newline (void)
{
  const char *eol = cur_bp->eol;
  while (*eol)
    if (!intercalate_char (*eol++))
      return false;

  adjust_markers (cur_bp->pt.p->o + cur_bp->pt.o, 1);
  set_buffer_last_line (cur_bp, get_buffer_last_line (cur_bp) + 1);
  set_buffer_modified (cur_bp, true);
  thisflag |= FLAG_NEED_RESYNC;

  return true;
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

  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), 1, 0);
  if (eolp ())
    {
      size_t eol_len = strlen (cur_bp->eol);
      adjust_markers (cur_bp->pt.p->o + cur_bp->pt.o, -eol_len);
      astr_remove (cur_bp->text, get_buffer_pt (cur_bp).p->o + get_buffer_pt (cur_bp).o, eol_len);
      set_buffer_last_line (cur_bp, get_buffer_last_line (cur_bp) - 1);
      thisflag |= FLAG_NEED_RESYNC;
    }
  else
    {
      adjust_markers (cur_bp->pt.p->o + cur_bp->pt.o, -1);
      astr_remove (cur_bp->text, get_buffer_pt (cur_bp).p->o + get_buffer_pt (cur_bp).o, 1);
    }

  set_buffer_modified (cur_bp, true);

  return true;
}

/*
 * Replace text in the line `lp' with `newtext'. If `replace_case' is
 * true then the new characters will be the same case as the old if
 * `case-replace' is true.
 */
void
line_replace_text (const Line * lp, size_t offset, size_t oldlen,
                   astr newtext, int replace_case)
{
  if (replace_case && get_variable_bool ("case-replace"))
    {
      int case_type = check_case (astr_cstr (get_line_text (lp)) + offset, oldlen);

      if (case_type != 0)
        astr_recase (newtext, case_type == 1 ? case_capitalized : case_upper);
    }

  set_buffer_modified (cur_bp, true);
  adjust_markers (lp->o + offset, (ptrdiff_t) (astr_len (newtext) - oldlen));
  /* FIXME: Don't rely on editing at point. */
  astr_replace (lp->bp->text, get_buffer_pt (lp->bp).p->o + offset, oldlen, newtext);
}

void
insert_buffer (Buffer * bp)
{
  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
  insert_astr (astr_cpy (astr_new (), bp->text));
  undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
}


/*
 * Allocate a new buffer structure, set the default local
 * variable values, and insert it into the buffer list.
 */
Buffer *
buffer_new (void)
{
  Buffer *bp = (Buffer *) XZALLOC (Buffer);
  Line *l = XZALLOC (Line);
  l->bp = bp;
  bp->text = astr_new ();
  bp->lines = bp->pt.p = l;

  bp->eol = coding_eol_lf;
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
          name = xasprintf ("%s<%ld>", p, (unsigned long) i);
          if (find_buffer (name) == NULL)
            return name;
        }
    }
}

/*
 * Set a new filename, and from it a name, for the buffer.
 */
void
set_buffer_names (Buffer * bp, const char *filename)
{
  astr as = NULL;
  char *oldname;

  if (filename[0] != '/')
    {
      as = agetcwd ();
      astr_cat_char (as, '/');
      astr_cat_cstr (as, filename);
      set_buffer_filename (bp, astr_cstr (as));
      filename = astr_cstr (as);
    }
  else
    set_buffer_filename (bp, filename);

  oldname = bp->name;
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
      if (bname && STREQ (bname, name))
        return bp;
    }

  return NULL;
}

/*
 * Move the selected buffer to head.
 */
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
  else if (!get_buffer_mark_active (cur_bp) &&
           get_variable_bool ("transient-mark-mode"))
    {
      minibuf_error ("The mark is not active now");
      return true;
    }
  else
    return false;
}

/*
 * Calculate the region size between point and mark and set the region
 * structure.
 */
Region
calculate_the_region (void)
{
  Region r;
  if (cmp_point (cur_bp->pt, get_marker_pt (cur_bp->mark)) < 0)
    {
      /* Point is before mark. */
      set_region_start (&r, cur_bp->pt);
      set_region_end (&r, get_marker_pt (cur_bp->mark));
    }
  else
    {
      /* Mark is before point. */
      set_region_start (&r, get_marker_pt (cur_bp->mark));
      set_region_end (&r, cur_bp->pt);
    }
  return r;
}

bool
delete_region (const Region r)
{
  size_t size = get_region_size (r);
  Marker *m = point_marker ();

  if (warn_if_readonly_buffer ())
    return false;

  goto_point (get_region_start (r));
  undo_save (UNDO_REPLACE_BLOCK, get_region_start (r), size, 0);
  undo_nosave = true;
  while (size--)
    delete_char ();
  undo_nosave = false;
  goto_point (get_marker_pt (m));
  unchain_marker (m);

  return true;
}

bool
in_region (size_t lineno, size_t x, Region r)
{
  size_t o = point_to_offset ((Point) {.n = lineno, .o = x});
  return o >= r.start && o <= r.end;
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

/*
 * Copy a region of text into an allocated buffer.
 */
astr
copy_text_block (Point pt, size_t size)
{
  const Line * lp = pt.p;
  astr as = astr_substr (get_line_text (lp), pt.o, astr_len (get_line_text (lp)) - pt.o);

  astr_cat_cstr (as, cur_bp->eol);
  for (lp = get_line_next (lp); astr_len (as) < size; lp = get_line_next (lp))
    {
      astr_cat (as, get_line_text (lp));
      astr_cat_cstr (as, cur_bp->eol);
    }
  return astr_truncate (as, size);
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
move_char (int dir)
{
  if (dir > 0 ? !eolp () : !bolp ())
    {
      cur_bp->pt.o += dir;
      return true;
    }
  else if (dir > 0 ? !eobp () : !bobp ())
    {
      thisflag |= FLAG_NEED_RESYNC;
      cur_bp->pt.p = (dir > 0 ? get_line_next : get_line_prev) (cur_bp->pt.p);
      assert (cur_bp->pt.p);
      cur_bp->pt.n += dir;
      if (dir > 0)
        FUNCALL (beginning_of_line);
      else
        FUNCALL (end_of_line);
      return true;
    }

  return false;
}

/*
 * Go to the column `goalc'.  Take care of expanding
 * tabulations.
 */
static void
goto_goalc (void)
{
  size_t i, col = 0, t = tab_width (cur_bp);

  for (i = 0; i < astr_len (get_line_text (cur_bp->pt.p)); i++)
    {
      if (col == get_buffer_goalc (cur_bp))
        break;
      else if (astr_get (get_line_text (cur_bp->pt.p), i) == '\t')
        {
          size_t w;
          for (w = t - col % t; w > 0; w--)
            if (++col == get_buffer_goalc (cur_bp))
              break;
        }
      else
        ++col;
    }

  cur_bp->pt.o = i;
}

bool
move_line (int n)
{
  bool ok = true;
  int dir;

  if (n == 0)
    return false;
  else if (n > 0)
    {
      dir = 1;
      if ((size_t) n > get_buffer_last_line (cur_bp) - cur_bp->pt.n)
        {
          ok = false;
          n = get_buffer_last_line (cur_bp) - cur_bp->pt.n;
        }
    }
  else
    {
      dir = -1;
      n = -n;
      if ((size_t) n > cur_bp->pt.n)
        {
          ok = false;
          n = cur_bp->pt.n;
        }
    }

  for (; n > 0; n--)
    {
      cur_bp->pt.p = (dir > 0 ? get_line_next : get_line_prev) (cur_bp->pt.p);
      assert (cur_bp->pt.p);
      cur_bp->pt.n += dir;
    }

  if (last_command () != F_next_line && last_command () != F_previous_line)
    set_buffer_goalc (cur_bp, get_goalc ());
  goto_goalc ();

  thisflag |= FLAG_NEED_RESYNC;

  return ok;
}
