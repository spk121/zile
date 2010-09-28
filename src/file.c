/* Disk file handling

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

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include "dirname.h"

#include "main.h"
#include "extern.h"

int
exist_file (const char *filename)
{
  struct stat st;

  if (stat (filename, &st) == -1)
    if (errno == ENOENT)
      return false;

  return true;
}

static int
is_regular_file (const char *filename)
{
  struct stat st;
  return stat (filename, &st) == 0 && S_ISREG (st.st_mode);
}

/*
 * Return the current directory, if available.
 */
astr
agetcwd (void)
{
  char *s = getcwd (NULL, 0);
  astr as;
  if (s == NULL)
    s = "";
  as = astr_new_cstr (s);
  free (s);
  return as;
}

/*
 * This functions does some corrections and expansions to
 * the passed path:
 *
 * - expands `~/' and `~name/' expressions;
 * - replaces `//' with `/' (restarting from the root directory);
 * - removes `..' and `.' entries.
 *
 * The return value indicates success or failure.
 */
bool
expand_path (astr path)
{
  int ok = true;
  const char *sp = astr_cstr (path), *p;
  astr epath = astr_new ();

  if (*sp != '/' && *sp != '~')
    {
      astr cwd = agetcwd ();
      astr_cat (epath, cwd);
      astr_delete (cwd);
      if (astr_len (epath) == 0 ||
          astr_get (epath, astr_len (epath) - 1) != '/')
        astr_cat_char (epath, '/');
    }

  for (p = sp; *p != '\0';)
    {
      if (*p == '/')
        {
          if (*++p == '/')
            { /* Got `//'.  Restart from this point. */
              while (*p == '/')
                p++;
              astr_truncate (epath, 0);
            }
          if (astr_len (epath) == 0 ||
              astr_get (epath, astr_len (epath) - 1) != '/')
            astr_cat_char (epath, '/');
        }
      else if (*p == '~' && (p == sp || p[-1] == '/'))
        { /* Got `/~' or leading `~'.  Restart from this point. */
          struct passwd *pw;

          astr_truncate (epath, 0);
          ++p;

          if (*p == '/')
            { /* Got `~/'.  Insert the user's home directory. */
              pw = getpwuid (getuid ());
              if (pw == NULL)
                {
                  ok = false;
                  break;
                }
              if (strcmp (pw->pw_dir, "/") != 0)
                astr_cat_cstr (epath, pw->pw_dir);
            }
          else
            { /* Got `~something'.  Insert that user's home directory. */
              astr as = astr_new ();
              while (*p != '\0' && *p != '/')
                astr_cat_char (as, *p++);
              pw = getpwnam (astr_cstr (as));
              astr_delete (as);
              if (pw == NULL)
                {
                  ok = false;
                  break;
                }
              astr_cat_cstr (epath, pw->pw_dir);
            }
        }
      else if (*p == '.' && (p[1] == '/' || p[1] == '\0'))
        { /* Got `.'. */
          ++p;
        }
      else if (*p == '.' && p[1] == '.' && (p[2] == '/' || p[2] == '\0'))
        { /* Got `..'. */
          if (astr_len (epath) >= 1 && astr_get (epath, astr_len (epath) - 1) == '/')
            astr_truncate (epath, astr_len (epath) - 1);
          while (astr_get (epath, astr_len (epath) - 1) != '/' && astr_len (epath) >= 1)
            astr_truncate (epath, astr_len (epath) - 1);
          p += 2;
        }

      if (*p != '~')
        while (*p != '\0' && *p != '/')
          astr_cat_char (epath, *p++);
    }

  astr_cpy (path, epath);
  astr_delete (epath);

  return ok;
}

/*
 * Return a `~/foo' like path if the user is under his home directory,
 * else the unmodified path.
 */
astr
compact_path (astr path)
{
  struct passwd *pw = getpwuid (getuid ());

  if (pw != NULL)
    {
      /* Replace `/userhome/' (if found) with `~/'. */
      size_t homelen = strlen (pw->pw_dir);
      if (astr_len (path) >= homelen &&
          !strncmp (pw->pw_dir, astr_cstr (path), homelen))
        {
          astr buf = astr_new_cstr ("~/");
          if (!strcmp (pw->pw_dir, "/"))
            astr_cat_cstr (buf, astr_cstr (path) + 1);
          else
            astr_cat_cstr (buf, astr_cstr (path) + homelen + 1);
          astr_cpy (path, buf);
          astr_delete (buf);
        }
    }

  return path;
}

/*
 * Return the current directory for the buffer, always ending in `/'.
 */
static astr
get_buffer_dir (void)
{
  astr ret;

  if (get_buffer_filename (cur_bp))
    {
      const char *name = dir_name (get_buffer_filename (cur_bp));
      ret = astr_new_cstr (name);
      free ((char *) name);
    }
  else
    ret = agetcwd ();

  if (astr_len (ret) != 0 && astr_get (ret, astr_len (ret) - 1) != '/')
    astr_cat_char (ret, '/');

  return ret;
}

/*
 * Get HOME directory.
 */
astr
get_home_dir (void)
{
  char *s = getenv ("HOME");
  if (s != NULL)
    return astr_new_cstr (s);
  return NULL;
}

/* Return nonzero if file exists and can be written. */
static int
check_writable (const char *filename)
{
  return euidaccess (filename, W_OK) >= 0;
}

/* Formats of end-of-line. */
const char *coding_eol_lf = "\n";
const char *coding_eol_crlf = "\r\n";
const char *coding_eol_cr = "\r";

/* Maximum number of EOLs to check before deciding type. */
#define MAX_EOL_CHECK_COUNT 3

/*
 * Read the file contents into current buffer.
 * Return quietly if the file doesn't exist, or other error.
 */
static void
read_file (const char *filename)
{
  Line *lp;
  int i, size;
  bool first_eol = true;
  const char *this_eol_type;
  size_t eol_len = 0, total_eols = 0;
  char buf[BUFSIZ];
  FILE *fp = fopen (filename, "r");
  Point pt;

  if (fp == NULL)
    {
      if (errno != ENOENT)
        {
          minibuf_write ("%s: %s", filename, strerror (errno));
          set_buffer_readonly (cur_bp, true);
        }
      return;
    }

  if (!check_writable (filename))
    set_buffer_readonly (cur_bp, true);

  pt = get_buffer_pt (cur_bp);
  lp = pt.p;

  /* Read first chunk and determine EOL type. */
  /* FIXME: Don't assume first EOL occurs in first chunk. */
  size = fread (buf, 1, BUFSIZ, fp);
  if (size > 0)
    {
      for (i = 0; i < size && total_eols < MAX_EOL_CHECK_COUNT; i++)
        {
          if (buf[i] == '\n' || buf[i] == '\r')
            {
              total_eols++;
              if (buf[i] == '\n')
                this_eol_type = coding_eol_lf;
              else if (i >= size || buf[i + 1] != '\n')
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

      /* Process this and subsequent chunks into lines. */
      eol_len = strlen (get_buffer_eol (cur_bp));
      do
        {
          for (i = 0; i < size; i++)
            {
              if (strncmp (get_buffer_eol (cur_bp), buf + i, eol_len) != 0)
                astr_cat_char (get_line_text (lp), buf[i]);
              else
                {
                  lp = line_insert (lp, astr_new ());
                  set_buffer_last_line (cur_bp,
                                        get_buffer_last_line (cur_bp) + 1);
                  i += eol_len - 1;
                }
            }
        }
      while ((size = fread (buf, 1, BUFSIZ, fp)) > 0);
    }

  set_line_next (lp, get_buffer_lines (cur_bp));
  set_line_prev (get_buffer_lines (cur_bp), lp);
  pt = get_buffer_pt (cur_bp);
  pt.p = get_line_next (get_buffer_lines (cur_bp));
  set_buffer_pt (cur_bp, pt);

  fclose (fp);
}

bool
find_file (const char *filename)
{
  Buffer *bp;

  for (bp = head_bp; bp != NULL; bp = get_buffer_next (bp))
    {
      if (get_buffer_filename (bp) != NULL &&
          !strcmp (get_buffer_filename (bp), filename))
        {
          switch_to_buffer (bp);
          return true;
        }
    }

  if (exist_file (filename) && !is_regular_file (filename))
    {
      minibuf_error ("File exists but could not be read");
      return false;
    }

  bp = buffer_new ();
  set_buffer_names (bp, filename);

  switch_to_buffer (bp);
  read_file (filename);

  thisflag |= FLAG_NEED_RESYNC;

  return true;
}

DEFUN ("find-file", find_file)
/*+
Edit the specified file.
Switch to a buffer visiting the file,
creating one if none already exists.
+*/
{
  astr buf = get_buffer_dir ();
  char *ms = minibuf_read_filename ("Find file: ", astr_cstr (buf), NULL);

  ok = leNIL;

  astr_delete (buf);
  if (ms == NULL)
    ok = FUNCALL (keyboard_quit);
  else if (ms[0] != '\0')
    ok = bool_to_lisp (find_file (ms));

  free (ms);
}
END_DEFUN

DEFUN ("find-file-read-only", find_file_read_only)
/*+
Edit the specified file but don't allow changes.
Like `find-file' but marks buffer as read-only.
Use @kbd{M-x toggle-read-only} to permit editing.
+*/
{
  ok = FUNCALL (find_file);
  if (ok == leT)
    set_buffer_readonly (cur_bp, true);
}
END_DEFUN

DEFUN ("find-alternate-file", find_alternate_file)
/*+
Find the file specified by the user, select its buffer, kill previous buffer.
If the current buffer now contains an empty file that you just visited
(presumably by mistake), use this command to visit the file you really want.
+*/
{
  const char *buf = get_buffer_filename (cur_bp);
  char *base = NULL;
  char *ms;
  astr as = NULL;

  if (buf == NULL)
    {
      as = get_buffer_dir ();
      buf = astr_cstr (as);
    }
  else
    base = base_name (buf);
  ms = minibuf_read_filename ("Find alternate: ", buf, base);
  free (base);

  ok = leNIL;
  if (ms == NULL)
    ok = FUNCALL (keyboard_quit);
  else if (ms[0] != '\0' && check_modified_buffer (cur_bp))
    {
      kill_buffer (cur_bp);
      ok = bool_to_lisp (find_file (ms));
    }

  free (ms);
  if (as)
    astr_delete (as);
}
END_DEFUN

DEFUN_ARGS ("switch-to-buffer", switch_to_buffer,
            STR_ARG (buffer))
/*+
Select buffer @i{buffer} in the current window.
+*/
{
  Buffer *bp = ((get_buffer_next (cur_bp) != NULL) ? get_buffer_next (cur_bp) : head_bp);

  STR_INIT (buffer)
  else
    {
      Completion *cp = make_buffer_completion ();
      buffer = minibuf_read_completion ("Switch to buffer (default %s): ",
                                        "", cp, NULL, get_buffer_name (bp));
      free_completion (cp);

      if (buffer == NULL)
        ok = FUNCALL (keyboard_quit);
    }

  if (ok == leT)
    {
      if (buffer && buffer[0] != '\0')
        {
          bp = find_buffer (buffer);
          if (bp == NULL)
            {
              bp = buffer_new ();
              set_buffer_name (bp, buffer);
              set_buffer_needname (bp, true);
              set_buffer_nosave (bp, true);
            }
        }

      switch_to_buffer (bp);
    }

  STR_FREE (buffer);
}
END_DEFUN

static size_t
insert_lines (size_t n, size_t end, size_t last, Line *from_lp)
{
  for (; n < end; n++, from_lp = get_line_next (from_lp))
    {
      insert_astr (get_line_text (from_lp));
      if (n < last)
        insert_newline ();
    }
  return n;
}

static void
insert_buffer (Buffer * bp)
{
  Line *old_next = get_line_next (get_buffer_pt (bp).p);
  astr old_cur_line = astr_cpy (astr_new (), get_line_text (get_buffer_pt (bp).p));
  size_t old_cur_n = get_buffer_pt (bp).n, old_lines = get_buffer_last_line (bp);
  size_t size = calculate_buffer_size (bp);

  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), 0, size);
  undo_nosave = true;
  insert_lines (0, old_cur_n, old_lines, get_line_next (get_buffer_lines (bp)));
  insert_astr (old_cur_line);
  if (old_cur_n < old_lines)
    insert_newline ();
  insert_lines (old_cur_n + 1, old_lines, old_lines, old_next);
  astr_delete (old_cur_line);
  undo_nosave = false;
}

DEFUN_ARGS ("insert-buffer", insert_buffer,
            STR_ARG (buffer))
/*+
Insert after point the contents of BUFFER.
Puts mark after the inserted text.
+*/
{
  Buffer *def_bp = ((get_buffer_next (cur_bp) != NULL) ? get_buffer_next (cur_bp) : head_bp);

  if (warn_if_readonly_buffer ())
    return leNIL;

  STR_INIT (buffer)
  else
    {
      Completion *cp = make_buffer_completion ();
      buffer = minibuf_read_completion ("Insert buffer (default %s): ",
                                        "", cp, NULL, get_buffer_name (def_bp));
      if (buffer == NULL)
        ok = FUNCALL (keyboard_quit);
      free_completion (cp);
    }

  if (ok == leT)
    {
      Buffer *bp;

      if (buffer && buffer[0] != '\0')
        {
          bp = find_buffer (buffer);
          if (bp == NULL)
            {
              minibuf_error ("Buffer `%s' not found", buffer);
              ok = leNIL;
            }
        }
      else
        bp = def_bp;

      insert_buffer (bp);
      set_mark_interactive ();
    }

  STR_FREE (buffer);
}
END_DEFUN

static int
insert_file (const char *filename)
{
  int fd;
  size_t size;
  char buf[BUFSIZ];

  if (!exist_file (filename))
    {
      minibuf_error ("Unable to read file `%s'", filename);
      return false;
    }

  fd = open (filename, O_RDONLY);
  if (fd < 0)
    {
      minibuf_write ("%s: %s", filename, strerror (errno));
      return false;
    }

  size = lseek (fd, 0, SEEK_END);
  if (size < 1)
    {
      close (fd);
      return true;
    }

  lseek (fd, 0, SEEK_SET);
  undo_save (UNDO_REPLACE_BLOCK, get_buffer_pt (cur_bp), 0, size);
  undo_nosave = true;
  while ((size = read (fd, buf, BUFSIZ)) > 0)
    insert_nstring (buf, size);
  undo_nosave = false;
  close (fd);

  return true;
}

DEFUN_ARGS ("insert-file", insert_file,
       STR_ARG (file))
/*+
Insert contents of file FILENAME into buffer after point.
Set mark after the inserted text.
+*/
{
  if (warn_if_readonly_buffer ())
    return leNIL;

  STR_INIT (file)
  else
    {
      astr buf = get_buffer_dir ();
      file = minibuf_read_filename ("Insert file: ", astr_cstr (buf), NULL);
      if (file == NULL)
        ok = FUNCALL (keyboard_quit);
    }

  if (file == NULL || file[0] == '\0' || !insert_file (file))
    ok = leNIL;
  else
    set_mark_interactive ();

  STR_FREE (file);
}
END_DEFUN

/*
 * Copy a file.
 */
static int
copy_file (const char *source, const char *dest)
{
  char buf[BUFSIZ];
  int ifd, ofd, stat_valid, serrno;
  size_t len;
  struct stat st;
  char *tname;

  ifd = open (source, O_RDONLY, 0);
  if (ifd < 0)
    {
      minibuf_error ("%s: unable to backup", source);
      return false;
    }

  tname = xasprintf ("%s_XXXXXXXXXX", dest);
  ofd = mkstemp (tname);
  if (ofd == -1)
    {
      serrno = errno;
      close (ifd);
      minibuf_error ("%s: unable to create backup", dest);
      free (tname);
      errno = serrno;
      return false;
    }

  while ((len = read (ifd, buf, sizeof buf)) > 0)
    if (write (ofd, buf, len) < 0)
      {
        minibuf_error ("Unable to write to backup file `%s'", dest);
        close (ifd);
        close (ofd);
        return false;
      }

  serrno = errno;
  stat_valid = fstat (ifd, &st) != -1;

  /* Recover file permissions and ownership. */
  if (stat_valid)
    {
#ifdef HAVE_FCHMOD
      fchmod (ofd, st.st_mode);
#endif
      fchown (ofd, st.st_uid, st.st_gid);
    }

  close (ifd);
  close (ofd);

  if (stat_valid)
    {
      if (rename (tname, dest) == -1)
        {
          minibuf_error ("Cannot rename temporary file `%s'",
                         strerror (errno));
          unlink (tname);
          stat_valid = false;
        }
    }
  else if (unlink (tname) == -1)
    minibuf_error ("Cannot remove temporary file `%s'", strerror (errno));

  free (tname);
  errno = serrno;

  /* Recover file modification time. */
  if (stat_valid)
    {
      struct utimbuf t;
      t.actime = st.st_atime;
      t.modtime = st.st_mtime;
      utime (dest, &t);
    }

  return stat_valid;
}

/*
 * Write buffer to given file name with given mode.
 */
static int
raw_write_to_disk (Buffer * bp, const char *filename, mode_t mode)
{
  ssize_t eol_len = (ssize_t) strlen (get_buffer_eol (bp)), written;
  Line *lp;
  int ret = 0;
  int fd = creat (filename, mode);

  if (fd < 0)
    return -1;

  /* Save the lines. */
  for (lp = get_line_next (get_buffer_lines (bp));
       lp != get_buffer_lines (bp);
       lp = get_line_next (lp))
    {
      ssize_t len = (ssize_t) astr_len (get_line_text (lp));

      written = write (fd, astr_cstr (get_line_text (lp)), len);
      if (written != len)
        {
          ret = written;
          break;
        }
      if (get_line_next (lp) != get_buffer_lines (bp))
        {
          written = write (fd, get_buffer_eol (bp), eol_len);
          if (written != eol_len)
            {
              ret = written;
              break;
            }
        }
    }

  if (close (fd) < 0 && ret == 0)
    ret = -1;

  return ret;
}

/*
 * Create a backup filename according to user specified variables.
 */
static astr
create_backup_filename (const char *filename, const char *backupdir)
{
  astr res;

  /* Prepend the backup directory path to the filename */
  if (backupdir)
    {
      astr buf = astr_new ();

      astr_cpy_cstr (buf, backupdir);
      if (astr_get (buf, astr_len (buf) - 1) != '/')
        astr_cat_char (buf, '/');
      while (*filename)
        {
          if (*filename == '/')
            astr_cat_char (buf, '!');
          else
            astr_cat_char (buf, *filename);
          ++filename;
        }

      if (!expand_path (buf))
        {
          astr_delete (buf);
          buf = NULL;
        }
      res = buf;
    }
  else
    res = astr_new_cstr (filename);

  return astr_cat_char (res, '~');
}

/*
 * Write the buffer contents to a file.
 * Create a backup file if specified by the user variables.
 */
static int
write_to_disk (Buffer * bp, const char *filename)
{
  int fd, backup = get_variable_bool ("make-backup-files"), ret;
  const char *backupdir = get_variable_bool ("backup-directory") ?
    get_variable ("backup-directory") : NULL;

  /* Make backup of original file. */
  if (!get_buffer_backup (bp) && backup
      && (fd = open (filename, O_RDWR, 0)) != -1)
    {
      astr bfilename;
      close (fd);
      bfilename = create_backup_filename (filename, backupdir);
      if (bfilename && copy_file (filename, astr_cstr (bfilename)))
        set_buffer_backup (bp, true);
      else
        {
          minibuf_error ("Cannot make backup file: %s", strerror (errno));
          waitkey (WAITKEY_DEFAULT);
        }
      astr_delete (bfilename);
    }

  ret = raw_write_to_disk (bp, filename, S_IRUSR | S_IWUSR |
                           S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (ret != 0)
    {
      if (ret == -1)
        minibuf_error ("Error writing `%s': %s", filename, strerror (errno));
      else
        minibuf_error ("Error writing `%s'", filename);
      return false;
    }

  return true;
}

static le *
write_buffer (Buffer *bp, bool needname, bool confirm,
              const char *name, const char *prompt)
{
  bool ans = true;
  bool name_from_minibuffer = false;
  le * ok = leT;

  if (needname)
    {
      name = minibuf_read_filename (prompt, "", NULL);
      name_from_minibuffer = true;
      if (name == NULL)
        return FUNCALL (keyboard_quit);
      if (name[0] == '\0')
        {
          free ((char *) name);
          return leNIL;
        }
      confirm = true;
    }

  if (confirm && exist_file (name))
    {
      ans = minibuf_read_yn ("File `%s' exists; overwrite? (y or n) ", name);
      if (ans == -1)
        FUNCALL (keyboard_quit);
      else if (ans == false)
        minibuf_error ("Canceled");
      if (ans != true)
        ok = leNIL;
    }

  if (ans == true)
    {
      if (name != get_buffer_filename (bp))
        set_buffer_names (bp, name);
      set_buffer_needname (bp, false);
      set_buffer_temporary (bp, false);
      set_buffer_nosave (bp, false);
      if (write_to_disk (bp, name))
        {
          minibuf_write ("Wrote %s", name);
          set_buffer_modified (bp, false);
          undo_set_unchanged (get_buffer_last_undop (bp));
        }
      else
        ok = leNIL;
    }

  if (name_from_minibuffer)
    free ((char *) name);

  return ok;
}

static le *
save_buffer (Buffer * bp)
{
  if (!get_buffer_modified (bp))
    {
      minibuf_write ("(No changes need to be saved)");
      return leT;
    }
  else
    return write_buffer (bp, get_buffer_needname (bp), false, get_buffer_filename (bp),
                         "File to save in: ");
}

DEFUN ("save-buffer", save_buffer)
/*+
Save current buffer in visited file if modified. By default, makes the
previous version into a backup file if this is the first save.
+*/
{
  ok = save_buffer (cur_bp);
}
END_DEFUN

DEFUN ("write-file", write_file)
/*+
Write current buffer into file @i{filename}.
This makes the buffer visit that file, and marks it as not modified.

Interactively, confirmation is required unless you supply a prefix argument.
+*/
{
  ok = write_buffer (cur_bp, true,
                     arglist && !(lastflag & FLAG_SET_UNIARG),
                     NULL, "Write file: ");
}
END_DEFUN

static int
save_some_buffers (void)
{
  Buffer *bp;
  bool none_to_save = true;
  bool noask = false;

  for (bp = head_bp; bp != NULL; bp = get_buffer_next (bp))
    {
      if (get_buffer_modified (bp) && !get_buffer_nosave (bp))
        {
          const char *fname = get_buffer_filename_or_name (bp);

          none_to_save = false;

          if (noask)
            save_buffer (bp);
          else
            for (;;)
              {
                int c;

                minibuf_write ("Save file %s? (y, n, !, ., q) ", fname);
                c = getkey ();
                minibuf_clear ();

                switch (c)
                  {
                  case KBD_CANCEL:	/* C-g */
                    FUNCALL (keyboard_quit);
                    return false;
                  case 'q':
                    goto endoffunc;
                  case '.':
                    save_buffer (bp);
                    return true;
                  case '!':
                    noask = true;
                    /* FALLTHROUGH */
                  case ' ':
                  case 'y':
                    save_buffer (bp);
                    /* FALLTHROUGH */
                  case 'n':
                  case KBD_RET:
                  case KBD_DEL:
                    goto exitloop;
                  default:
                    minibuf_error ("Please answer y, n, !, . or q.");
                    waitkey (WAITKEY_DEFAULT);
                    break;
                  }
              }
        }
    exitloop:
      (void)0; /* Label not allowed at end of compound statement. */
    }

endoffunc:
  if (none_to_save)
    minibuf_write ("(No files need saving)");

  return true;
}


DEFUN ("save-some-buffers", save_some_buffers)
/*+
Save some modified file-visiting buffers.  Asks user about each one.
+*/
{
  ok = bool_to_lisp (save_some_buffers ());
}
END_DEFUN

DEFUN ("save-buffers-kill-emacs", save_buffers_kill_emacs)
/*+
Offer to save each buffer, then kill this Zile process.
+*/
{
  Buffer *bp;

  if (!save_some_buffers ())
    return leNIL;

  for (bp = head_bp; bp != NULL; bp = get_buffer_next (bp))
    if (get_buffer_modified (bp) && !get_buffer_needname (bp))
      {
        for (;;)
          {
            int ans = minibuf_read_yesno
              ("Modified buffers exist; exit anyway? (yes or no) ");
            if (ans == -1)
              return FUNCALL (keyboard_quit);
            else if (!ans)
              return leNIL;
            break;
          }
        break; /* We have found a modified buffer, so stop. */
      }

  thisflag |= FLAG_QUIT;
}
END_DEFUN

/*
 * Function called on unexpected error or Zile crash (SIGSEGV).
 * Attempts to save modified buffers.
 * If doabort is true, aborts to allow core dump generation;
 * otherwise, exit.
 */
void
zile_exit (int doabort)
{
  Buffer *bp;

  fprintf (stderr, "Trying to save modified buffers (if any)...\r\n");
  for (bp = head_bp; bp != NULL; bp = get_buffer_next (bp))
    if (get_buffer_modified (bp) && !get_buffer_nosave (bp))
      {
        astr buf = astr_new (), as;
        const char *fname = get_buffer_filename_or_name (bp);
        astr_cpy_cstr (buf, fname);
        as = astr_new_cstr (PACKAGE);
        astr_recase (as, case_upper);
        astr_afmt (buf, ".%sSAVE", astr_cstr (as));
        astr_delete (as);
        fprintf (stderr, "Saving %s...\r\n", astr_cstr (buf));
        raw_write_to_disk (bp, astr_cstr (buf), S_IRUSR | S_IWUSR);
        astr_delete (buf);
      }

  if (doabort)
    abort ();
  else
    exit (2);
}

DEFUN ("cd", cd)
/*+
Make the user specified directory become the current buffer's default
directory.
+*/
{
  struct stat st;
  astr buf = get_buffer_dir ();
  char *ms = minibuf_read_filename ("Change default directory: ",
                                    astr_cstr (buf), NULL);

  if (ms == NULL)
    {
      astr_delete (buf);
      return FUNCALL (keyboard_quit);
    }
  astr_delete (buf);

  if (ms[0] != '\0')
    {
      if (stat (ms, &st) != 0 || !S_ISDIR (st.st_mode))
        {
          minibuf_error ("`%s' is not a directory", ms);
          free (ms);
          return leNIL;
        }
      if (chdir (ms) == -1)
        {
          minibuf_write ("%s: %s", ms, strerror (errno));
          free (ms);
          return leNIL;
        }
      free (ms);
      return leT;
    }

  free (ms);
  return leNIL;
}
END_DEFUN
