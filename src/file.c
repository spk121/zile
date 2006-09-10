/* Disk file handling
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2006 Reuben Thomas.
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

/*      $Id: file.c,v 1.80 2006/09/10 23:53:09 rrt Exp $        */

#include "config.h"

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <assert.h>
#include <errno.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <utime.h>

#include "zile.h"
#include "extern.h"

int exist_file(const char *filename)
{
  struct stat st;

  if (stat(filename, &st) == -1)
    if (errno == ENOENT)
      return FALSE;

  return TRUE;
}

int is_regular_file(const char *filename)
{
  struct stat st;

  if (stat(filename, &st) == -1) {
    if (errno == ENOENT)
      return TRUE;
    return FALSE;
  }
  if (S_ISREG(st.st_mode))
    return TRUE;

  return FALSE;
}

/* Safe getcwd */
astr agetcwd(void)
{
  size_t len = PATH_MAX;
  char *buf = (char *)zmalloc(len);
  char *res;
  astr as;
  while ((res = getcwd(buf, len)) == NULL && errno == ERANGE) {
    len *= 2;
    buf = zrealloc(buf, len);
  }
  /* If there was an error, return the empty string */
  if (res == NULL)
    *buf = '\0';
  as = astr_cpy_cstr(astr_new(), buf);
  free(buf);
  return as;
}

/*
 * This functions does some corrections and expansions to
 * the passed path:
 * - splits the path into directory (with trailing slash) and filename;
 * - expands `~/' and `~name/' expressions;
 * - replaces `//' with `/' (restarting from the root directory);
 * - removes `..' and `.' entries.
 */
int expand_path(const char *path, const char *cwdir, astr dir, astr fname)
{
  struct passwd *pw;
  const char *sp = path;

  if (*sp != '/') {
    astr_cat_cstr(dir, cwdir);
    if (astr_len(dir) == 0 || *astr_char(dir, -1) != '/')
      astr_cat_cstr(dir, "/");
  }

  while (*sp != '\0') {
    if (*sp == '/') {
      if (*++sp == '/') {
        /* Got `//'.  Restart from this point. */
        while (*sp == '/')
          sp++;
        astr_truncate(dir, 0);
      }
      astr_cat_char(dir, '/');
    } else if (*sp == '~') {
      if (*(sp + 1) == '/') {
        /* Got `~/'. Restart from this point and insert the user's
           home directory. */
        astr_truncate(dir, 0);
        if ((pw = getpwuid(getuid())) == NULL)
          return FALSE;
        if (strcmp(pw->pw_dir, "/") != 0)
          astr_cat_cstr(dir, pw->pw_dir);
        ++sp;
      } else {
        /* Got `~something'.  Restart from this point and insert that
           user's home directory. */
        astr as = astr_new();
        astr_truncate(dir, 0);
        ++sp;
        while (*sp != '\0' && *sp != '/')
          astr_cat_char(as, *sp++);
        pw = getpwnam(astr_cstr(as));
        astr_delete(as);
        if (pw == NULL)
          return FALSE;
        astr_cat_cstr(dir, pw->pw_dir);
      }
    } else if (*sp == '.') {
      if (*(sp + 1) == '/' || *(sp + 1) == '\0') {
        ++sp;
        if (*sp == '/' && *(sp + 1) != '/')
          ++sp;
      } else if (*(sp + 1) == '.' &&
                 (*(sp + 2) == '/' || *(sp + 2) == '\0')) {
        if (astr_len(dir) >= 1 && *astr_char(dir, -1) == '/')
          astr_truncate(dir, -1);
        while (*astr_char(dir, -1) != '/' &&
               astr_len(dir) >= 1)
          astr_truncate(dir, -1);
        sp += 2;
        if (*sp == '/' && *(sp + 1) != '/')
          ++sp;
      } else
        goto gotfname;
    } else {
      const char *p;
    gotfname:
      p = sp;
      while (*p != '\0' && *p != '/')
        p++;
      if (*p == '\0') {
        /* Final filename. */
        astr_cat_cstr(fname, sp);
        break;
      } else {
        /* Non-final directory. */
        while (*sp != '/')
          astr_cat_char(dir, *sp++);
      }
    }
  }

  if (astr_len(dir) == 0)
    astr_cat_cstr(dir, "/");

  return TRUE;
}

/*
 * Return a `~/foo' like path if the user is under his home directory,
 * and restart from / if // found,
 * else the unmodified path.
 */
astr compact_path(const char *path)
{
  astr buf = astr_new();
  struct passwd *pw;
  size_t i;

  if ((pw = getpwuid(getuid())) == NULL) {
    /* User not found in password file. */
    astr_cpy_cstr(buf, path);
    return buf;
  }

  /* Replace `/userhome/' (if existent) with `~/'. */
  i = strlen(pw->pw_dir);
  if (!strncmp(pw->pw_dir, path, i)) {
    astr_cpy_cstr(buf, "~/");
    if (!strcmp(pw->pw_dir, "/"))
      astr_cat_cstr(buf, path + 1);
    else
      astr_cat_cstr(buf, path + i + 1);
  } else
    astr_cpy_cstr(buf, path);

  return buf;
}

/*
 * Return the current directory.
 */
astr get_current_dir(int interactive)
{
  astr buf;

  if (interactive && cur_bp->filename != NULL) {
    /* If the current buffer has a filename, get the current directory
       name from it. */
    int p;

    buf = astr_cpy_cstr(astr_new(), cur_bp->filename);
    p = astr_rfind_cstr(buf, "/");
    if (p != -1)
      astr_truncate(buf, (ptrdiff_t)(p ? p : 1));
    if (*astr_char(buf, -1) != '/')
      astr_cat_cstr(buf, "/");
  } else {
    /* Get the current directory name from the system. */
    buf = agetcwd();
    astr_cat_cstr(buf, "/");
  }

  return buf;
}

/*
 * Get HOME directory.
 */
astr get_home_dir(void)
{
  char *s = getenv("HOME");
  astr as;
  if (s != NULL && strlen(s) < PATH_MAX)
    as = astr_cat_cstr(astr_new(), s);
  else
    as = agetcwd();
  return as;
}

void open_file(char *path, size_t lineno)
{
  astr buf, dir, fname;

  dir = astr_new();
  fname = astr_new();
  buf = get_current_dir(FALSE);

  if (!expand_path(path, astr_cstr(buf), dir, fname)) {
    fprintf(stderr, "zile: %s: invalid filename or path\n", path);
    zile_exit(1);
  }

  astr_cpy_cstr(buf, astr_cstr(dir));
  astr_cat_cstr(buf, astr_cstr(fname));
  astr_delete(dir);
  astr_delete(fname);

  find_file(astr_cstr(buf));
  astr_delete(buf);
  if (lineno > 0)
    ngotodown(lineno);
  resync_redisplay();
}

/*
 * Read the file contents into a buffer.
 * Return quietly if the file doesn't exist.
 */
void read_from_disk(const char *filename)
{
  Line *lp;
  FILE *fp;
  int i, size;
  size_t eol_len = 0;
  char buf[BUFSIZ + 1];

  buf[BUFSIZ] = '\0';     /* Sentinel for end of line checks. */

  if ((fp = fopen(filename, "r")) == NULL) {
    if (errno != ENOENT) {
      minibuf_write("%s: %s foo", filename, strerror(errno));
      cur_bp->flags |= BFLAG_READONLY;
    }
    return;
  }

  lp = cur_bp->pt.p;

  while ((size = fread(buf, 1, BUFSIZ, fp)) > 0)
    for (i = 0; i < size; i++)
      if ((eol_len > 0 && (strncmp(cur_bp->eol, buf + i, eol_len) != 0)) ||
          (eol_len == 0 && (buf[i] != '\n' && buf[i] != '\r')))
        astr_cat_char(lp->item, buf[i]);
      else {
        lp = list_prepend(lp, astr_new());
        ++cur_bp->num_lines;

        if (eol_len == 0) {
          if (i < size - 1 &&
              buf[i + 1] != buf[i] && (buf[i + 1] == '\n' ||
                                       buf[i + 1] == '\r')) {
            cur_bp->eol[0] = buf[i];
            cur_bp->eol[1] = buf[i + 1];
            eol_len = 2;
          } else {
            cur_bp->eol[0] = buf[i];
            cur_bp->eol[1] = '\0';
            eol_len = 1;
          }
        }

        i += eol_len - 1;
      }

  list_next(lp) = cur_bp->lines;
  list_prev(cur_bp->lines) = lp;
  cur_bp->pt.p = list_next(cur_bp->lines);

  fclose(fp);
}

int find_file(const char *filename)
{
  Buffer *bp;
  char *s;

  for (bp = head_bp; bp != NULL; bp = bp->next)
    if (bp->filename != NULL && !strcmp(bp->filename, filename)) {
      switch_to_buffer(bp);
      return TRUE;
    }

  s = make_buffer_name(filename);
  if (strlen(s) < 1) {
    free(s);
    return FALSE;
  }

  if (!is_regular_file(filename)) {
    minibuf_error("%s is not a regular file", filename);
    waitkey(WAITKEY_DEFAULT);
    return FALSE;
  }

  bp = create_buffer(s);
  free(s);
  bp->filename = zstrdup(filename);

  switch_to_buffer(bp);
  read_from_disk(filename);

  thisflag |= FLAG_NEED_RESYNC;

  return TRUE;
}

Completion *make_buffer_completion(void)
{
  Buffer *bp;
  Completion *cp;

  cp = completion_new(FALSE);
  for (bp = head_bp; bp != NULL; bp = bp->next)
    list_append(cp->completions, zstrdup(bp->name));

  return cp;
}

DEFUN_INT("find-file", find_file)
/*+
Edit a file specified by the user.  Switch to a buffer visiting the file,
creating one if none already exists.
+*/
{
  char *ms;
  astr buf;

  buf = get_current_dir(TRUE);
  if ((ms = minibuf_read_dir("Find file: ", astr_cstr(buf))) == NULL) {
    astr_delete(buf);
    return cancel();
  }
  astr_delete(buf);

  if (ms[0] != '\0') {
    int ret_value = find_file(ms);
    free(ms);
    return ret_value;
  }

  free(ms);
  return FALSE;
}
END_DEFUN

DEFUN_INT("find-alternate-file", find_alternate_file)
/*+
Find the file specified by the user, select its buffer, kill previous buffer.
If the current buffer now contains an empty file that you just visited
(presumably by mistake), use this command to visit the file you really want.
+*/
{
  char *ms;
  astr buf;

  buf = get_current_dir(TRUE);
  if ((ms = minibuf_read_dir("Find alternate: ", astr_cstr(buf)))
      == NULL) {
    astr_delete(buf);
    return cancel();
  }
  astr_delete(buf);

  if (ms[0] != '\0' && check_modified_buffer(cur_bp)) {
    int ret_value;
    kill_buffer(cur_bp);
    ret_value = find_file(ms);
    free(ms);
    return ret_value;
  }

  free(ms);
  return FALSE;
}
END_DEFUN

DEFUN_INT("switch-to-buffer", switch_to_buffer)
/*+
Select to the user specified buffer in the current window.
+*/
{
  char *ms;
  Buffer *swbuf;
  Completion *cp;

  swbuf = ((cur_bp->next != NULL) ? cur_bp->next : head_bp);

  cp = make_buffer_completion();
  ms = minibuf_read_completion("Switch to buffer (default %s): ",
                               "", cp, NULL, swbuf->name);
  free_completion(cp);
  if (ms == NULL)
    return cancel();

  if (ms[0] != '\0') {
    if ((swbuf = find_buffer(ms, FALSE)) == NULL) {
      swbuf = find_buffer(ms, TRUE);
      swbuf->flags = BFLAG_NEEDNAME | BFLAG_NOSAVE;
    }
  }

  switch_to_buffer(swbuf);

  return TRUE;
}
END_DEFUN

/*
 * Check if the buffer has been modified.  If so, asks the user if
 * he/she wants to save the changes.  If the response is positive, return
 * TRUE, else FALSE.
 */
int check_modified_buffer(Buffer *bp)
{
  int ans;

  if (bp->flags & BFLAG_MODIFIED && !(bp->flags & BFLAG_NOSAVE))
    for (;;) {
      if ((ans = minibuf_read_yesno("Buffer %s modified; kill anyway? (yes or no) ", bp->name)) == -1)
        return cancel();
      else if (!ans)
        return FALSE;
      break;
    }

  return TRUE;
}

/*
 * Remove the specified buffer from the buffer list and deallocate
 * its space.  Avoid killing the sole buffers and creates the scratch
 * buffer when required.
 */
void kill_buffer(Buffer *kill_bp)
{
  Buffer *next_bp;

  if (kill_bp->next != NULL)
    next_bp = kill_bp->next;
  else
    next_bp = head_bp;

  if (next_bp == kill_bp) {
    Window *wp;
    Buffer *new_bp = create_buffer(cur_bp->name);
    /* If this is the sole buffer available, then remove the contents
       and set the name to `*scratch*' if it is not already set. */
    assert(cur_bp == kill_bp);

    free_buffer(cur_bp);

    /* Scan all the windows that display this buffer. */
    for (wp = head_wp; wp != NULL; wp = wp->next)
      if (wp->bp == cur_bp) {
        wp->bp = new_bp;
        wp->topdelta = 0;
        wp->saved_pt = NULL;    /* It was freed. */
      }

    head_bp = cur_bp = new_bp;
    head_bp->next = NULL;
    cur_bp->flags = BFLAG_NOSAVE | BFLAG_NEEDNAME | BFLAG_TEMPORARY;
    if (strcmp(cur_bp->name, "*scratch*") != 0) {
      set_buffer_name(cur_bp, "*scratch*");
      if (cur_bp->filename != NULL) {
        free(cur_bp->filename);
        cur_bp->filename = NULL;
      }
    }
  } else {
    Buffer *bp;
    Window *wp;

    /* Search for windows displaying the buffer to kill. */
    for (wp = head_wp; wp != NULL; wp = wp->next)
      if (wp->bp == kill_bp) {
        wp->bp = next_bp;
        wp->topdelta = 0;
        wp->saved_pt = NULL;    /* The marker will be freed. */
      }

    /* Remove the buffer from the buffer list. */
    cur_bp = next_bp;
    if (head_bp == kill_bp)
      head_bp = head_bp->next;
    for (bp = head_bp; bp->next != NULL; bp = bp->next)
      if (bp->next == kill_bp) {
        bp->next = bp->next->next;
        break;
      }

    free_buffer(kill_bp);

    thisflag |= FLAG_NEED_RESYNC;
  }
}

DEFUN_INT("kill-buffer", kill_buffer)
/*+
Kill the current buffer or the user specified one.
+*/
{
  Buffer *bp;
  char *ms;
  Completion *cp;

  cp = make_buffer_completion();
  if ((ms = minibuf_read_completion("Kill buffer (default %s): ",
                                    "", cp, NULL, cur_bp->name)) == NULL)
    return cancel();
  free_completion(cp);
  if (ms[0] != '\0') {
    if ((bp = find_buffer(ms, FALSE)) == NULL) {
      minibuf_error("Buffer `%s' not found", ms);
      return FALSE;
    }
  } else
    bp = cur_bp;

  if (check_modified_buffer(bp)) {
    kill_buffer(bp);
    return TRUE;
  }

  return FALSE;
}
END_DEFUN

static void insert_buffer(Buffer *bp)
{
  Line *lp;
  size_t size = calculate_buffer_size(bp), i;

  undo_save(UNDO_REMOVE_BLOCK, cur_bp->pt, size, 0);
  undo_nosave = TRUE;
  for (lp = list_next(bp->lines); lp != bp->lines; lp = list_next(lp)) {
    for (i = 0; i < astr_len(lp->item); i++)
      insert_char(*astr_char(lp->item, (ptrdiff_t)i));
    if (list_next(lp) != bp->lines)
      insert_newline();
  }
  undo_nosave = FALSE;
}

DEFUN_INT("insert-buffer", insert_buffer)
/*+
Insert after point the contents of the user specified buffer.
Puts mark after the inserted text.
+*/
{
  Buffer *bp, *swbuf;
  char *ms;
  Completion *cp;

  if (warn_if_readonly_buffer())
    return FALSE;

  swbuf = ((cur_bp->next != NULL) ? cur_bp->next : head_bp);
  cp = make_buffer_completion();
  if ((ms = minibuf_read_completion("Insert buffer (default %s): ",
                                    "", cp, NULL, swbuf->name)) == NULL)
    return cancel();
  free_completion(cp);
  if (ms[0] != '\0') {
    if ((bp = find_buffer(ms, FALSE)) == NULL) {
      minibuf_error("Buffer `%s' not found", ms);
      return FALSE;
    }
  } else
    bp = swbuf;

  if (bp == cur_bp) {
    minibuf_error("Cannot insert the current buffer");
    return FALSE;
  }

  insert_buffer(bp);
  set_mark_command();

  return TRUE;
}
END_DEFUN

static int insert_file(char *filename)
{
  int fd;
  size_t i, size;
  char buf[BUFSIZ];

  if (!exist_file(filename)) {
    minibuf_error("Unable to read file `%s'", filename);
    return FALSE;
  }

  if ((fd = open(filename, O_RDONLY)) < 0) {
    minibuf_write("%s: %s", filename, strerror(errno));
    return FALSE;
  }

  size = lseek(fd, 0, SEEK_END);
  if (size < 1) {
    close(fd);
    return TRUE;
  }

  lseek(fd, 0, SEEK_SET);
  undo_save(UNDO_REMOVE_BLOCK, cur_bp->pt, size, 0);
  undo_nosave = TRUE;
  while ((size = read(fd, buf, BUFSIZ)) > 0)
    for (i = 0; i < size; i++)
      if (buf[i] != '\n')
        insert_char(buf[i]);
      else
        insert_newline();
  undo_nosave = FALSE;
  close(fd);

  return TRUE;
}

DEFUN_INT("insert-file", insert_file)
/*+
Insert contents of the user specified file into buffer after point.
Set mark after the inserted text.
+*/
{
  char *ms;
  astr buf;

  if (warn_if_readonly_buffer())
    return FALSE;

  buf = get_current_dir(TRUE);
  if ((ms = minibuf_read_dir("Insert file: ", astr_cstr(buf)))
      == NULL) {
    astr_delete(buf);
    return cancel();
  }
  astr_delete(buf);

  if (ms[0] == '\0') {
    free(ms);
    return FALSE;
  }

  if (!insert_file(ms)) {
    free(ms);
    return FALSE;
  }

  set_mark_command();

  free(ms);
  return TRUE;
}
END_DEFUN

/*
 * Copy a file.
 */
static int copy_file(const char *source, const char *dest)
{
  char buf[BUFSIZ];
  int ifd, ofd, stat_valid, serrno;
  size_t len;
  struct stat st;
  char *tname;

  ifd = open(source, O_RDONLY, 0);
  if (ifd < 0) {
    minibuf_error("%s: unable to backup", source);
    return FALSE;
  }

  if (zasprintf(&tname, "%s_XXXXXXXXXX", dest) == -1) {
    minibuf_error("Cannot allocate temporary file name `%s'",
                  strerror(errno));
    return FALSE;
  }

  ofd = mkstemp(tname);
  if (ofd == -1) {
    serrno = errno;
    close(ifd);
    minibuf_error("%s: unable to create backup", dest);
    free(tname);
    errno = serrno;
    return FALSE;
  }

  while ((len = read(ifd, buf, sizeof buf)) > 0)
    if (write(ofd, buf, len) < 0) {
      minibuf_error("Unable to write to backup file `%s'", dest);
      close(ifd);
      close(ofd);
      return FALSE;
    }

  serrno = errno;
  stat_valid = fstat(ifd, &st) != -1;

#if defined(HAVE_FCHMOD) || defined(HAVE_FCHOWN)
  /* Recover file permissions and ownership. */
  if (stat_valid) {
#ifdef HAVE_FCHMOD
    fchmod(ofd, st.st_mode);
#endif
#ifdef HAVE_FCHOWN
    fchown(ofd, st.st_uid, st.st_gid);
#endif
  }
#endif

  close(ifd);
  close(ofd);

  if (stat_valid) {
    if (rename(tname, dest) == -1) {
      minibuf_error("Cannot rename temporary file `%s'", strerror(errno));
      (void)unlink(tname);
      stat_valid = FALSE;
    }
  } else if (unlink(tname) == -1)
    minibuf_error("Cannot remove temporary file `%s'", strerror(errno));

  free(tname);
  errno = serrno;

  /* Recover file modification time. */
  if (stat_valid) {
    struct utimbuf t;
    t.actime = st.st_atime;
    t.modtime = st.st_mtime;
    utime(dest, &t);
  }

  return stat_valid;
}

/*
 * Write buffer to given file name with given umask.
 */
static int raw_write_to_disk(Buffer *bp, const char *filename, int umask)
{
  int fd;
  size_t eol_len = strlen(bp->eol);
  Line *lp;

  if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, umask)) < 0)
    return FALSE;

  /* Save all the lines. */
  for (lp = list_next(bp->lines); lp != bp->lines; lp = list_next(lp)) {
    write(fd, astr_cstr(lp->item), astr_len(lp->item));
    if (list_next(lp) != bp->lines)
      write(fd, bp->eol, eol_len);
  }

  if (close(fd) < 0)
    return FALSE;

  return TRUE;
}

/*
 * Create a backup filename according to user specified variables.
 */
static astr create_backup_filename(const char *filename,
                                   const char *backupdir)
{
  astr res;

  /* Prepend the backup directory path to the filename */
  if (backupdir) {
    astr dir = astr_new(), fname = astr_new(), buf = astr_new();

    astr_cpy_cstr(buf, backupdir);
    if (*astr_char(buf, -1) != '/')
      astr_cat_char(buf, '/');
    while (*filename) {
      if (*filename == '/')
        astr_cat_char(buf, '!');
      else
        astr_cat_char(buf, *filename);
      ++filename;
    }

    if (!expand_path(astr_cstr(buf), "", dir, fname)) {
      astr_delete(buf);
      astr_delete(dir);
      astr_delete(fname);
      return NULL;
    }
    astr_cat_delete(dir, fname);
    res = dir;
  } else {
    res = astr_new();
    astr_cat_cstr(res, filename);
  }

  astr_cat_char(res, '~');

  return res;
}

/*
 * Write the buffer contents to a file.  Create a backup file if specified
 * by the user variables.
 */
static int write_to_disk(Buffer *bp, char *filename)
{
  int fd, backup = lookup_bool_variable("make-backup-files");
  char *backupdir = lookup_bool_variable("backup-directory") ?
    get_variable("backup-directory") : NULL;

  /*
   * Make backup of original file.
   */
  if (!(bp->flags & BFLAG_BACKUP) && backup
      && (fd = open(filename, O_RDWR, 0)) != -1) {
    astr bfilename;
    close(fd);
    bfilename = create_backup_filename(filename, backupdir);
    if (bfilename && copy_file(filename, astr_cstr(bfilename)))
      bp->flags |= BFLAG_BACKUP;
    else {
      minibuf_error("Cannot make backup file: %s", strerror(errno));
      waitkey(WAITKEY_DEFAULT);
    }
    astr_delete(bfilename);
  }

  if (raw_write_to_disk(bp, filename, 0644) == FALSE) {
    minibuf_error("%s: %s", filename, strerror(errno));
    return FALSE;
  }

  return TRUE;
}

static int save_buffer(Buffer *bp)
{
  char *ms, *fname = bp->filename != NULL ? bp->filename : bp->name;
  int ms_is_from_minibuffer = 0;

  if (!(bp->flags & BFLAG_MODIFIED))
    minibuf_write("(No changes need to be saved)");
  else {
    if (bp->flags & BFLAG_NEEDNAME) {
      if ((ms = minibuf_read_dir("File to save in: ", fname)) == NULL)
        return cancel();
      ms_is_from_minibuffer = 1;
      if (ms[0] == '\0') {
        free(ms);
        return FALSE;
      }

      set_buffer_filename(bp, ms);

      bp->flags &= ~BFLAG_NEEDNAME;
    } else
      ms = bp->filename;

    if (write_to_disk(bp, ms)) {
      Undo *up;

      minibuf_write("Wrote %s", ms);
      bp->flags &= ~BFLAG_MODIFIED;

      /* Set unchanged flags to FALSE except for the
         last undo action, which is set to TRUE. */
      up = bp->last_undop;
      if (up)
        up->unchanged = TRUE;
      for (up = up->next; up; up = up->next)
        up->unchanged = FALSE;
    }

    bp->flags &= ~BFLAG_TEMPORARY;

    if (ms_is_from_minibuffer)
      free(ms);
  }

  return TRUE;
}

DEFUN_INT("save-buffer", save_buffer)
/*+
Save current buffer in visited file if modified. By default, makes the
previous version into a backup file if this is the first save.
+*/
{
  return save_buffer(cur_bp);
}
END_DEFUN

DEFUN_INT("write-file", write_file)
/*+
Write current buffer into the user specified file.
Makes buffer visit that file, and marks it not modified.
+*/
{
  char *fname = cur_bp->filename != NULL ? cur_bp->filename : cur_bp->name;
  char *ms;

  if ((ms = minibuf_read_dir("Write file: ", fname)) == NULL)
    return cancel();
  if (ms[0] == '\0') {
    free(ms);
    return FALSE;
  }

  set_buffer_filename(cur_bp, ms);

  cur_bp->flags &= ~(BFLAG_NEEDNAME | BFLAG_TEMPORARY);

  if (write_to_disk(cur_bp, ms)) {
    minibuf_write("Wrote %s", ms);
    cur_bp->flags &= ~BFLAG_MODIFIED;
  }

  free(ms);
  return TRUE;
}
END_DEFUN

static int save_some_buffers(void)
{
  Buffer *bp;
  int i = 0, noask = FALSE, c;

  for (bp = head_bp; bp != NULL; bp = bp->next)
    if (bp->flags & BFLAG_MODIFIED
        && !(bp->flags & BFLAG_NOSAVE)) {
      char *fname = bp->filename != NULL ? bp->filename : bp->name;

      ++i;

      if (noask)
        save_buffer(bp);
      else {
        for (;;) {
          minibuf_write("Save file %s? (y, n, !, ., q) ", fname);

          c = getkey();
          switch (c) {
          case KBD_CANCEL:
          case KBD_RET:
          case ' ':
          case 'y':
          case 'n':
          case 'q':
          case '.':
          case '!':
            goto exitloop;
          }

          minibuf_error("Please answer y, n, !, . or q.");
          waitkey(WAITKEY_DEFAULT);
        }

      exitloop:
        minibuf_clear();

        switch (c) {
        case KBD_CANCEL: /* C-g */
          return cancel();
        case 'q':
          goto endoffunc;
        case '.':
          save_buffer(bp);
          ++i;
          return TRUE;
        case '!':
          noask = TRUE;
          /* FALLTHROUGH */
        case ' ':
        case 'y':
          save_buffer(bp);
          ++i;
          break;
        case 'n':
        case KBD_RET:
        case KBD_DEL:
          break;
        }
      }
    }

 endoffunc:
  if (i == 0)
    minibuf_write("(No files need saving)");

  return TRUE;
}


DEFUN_INT("save-some-buffers", save_some_buffers)
/*+
Save some modified file-visiting buffers.  Asks user about each one.
+*/
{
  return save_some_buffers();
}
END_DEFUN

DEFUN_INT("save-buffers-kill-zile", save_buffers_kill_zile)
/*+
Offer to save each buffer, then kill this Zile process.
+*/
{
  Buffer *bp;
  int ans, i = 0;

  if (!save_some_buffers())
    return FALSE;

  for (bp = head_bp; bp != NULL; bp = bp->next)
    if (bp->flags & BFLAG_MODIFIED && !(bp->flags & BFLAG_NEEDNAME))
      ++i;

  if (i > 0)
    for (;;) {
      if ((ans = minibuf_read_yesno("Modified buffers exist; exit anyway? (yes or no) ", "")) == -1)
        return cancel();
      else if (!ans)
        return FALSE;
      break;
    }

  thisflag |= FLAG_QUIT_ZILE;

  return TRUE;
}
END_DEFUN

/*
 * Function called on unexpected error or Zile crash (SIGSEGV).
 * Attempts to save modified buffers.
 */
void zile_exit(int exitcode)
{
  Buffer *bp;

  fprintf(stderr, "Trying to save modified buffers (if any)...\r\n");
  for (bp = head_bp; bp != NULL; bp = bp->next)
    if (bp->flags & BFLAG_MODIFIED &&
        !(bp->flags & BFLAG_NOSAVE)) {
      astr buf;
      buf = astr_new();
      if (bp->filename != NULL)
        astr_cpy_cstr(buf, bp->filename);
      else
        astr_cpy_cstr(buf, bp->name);
      astr_cat_cstr(buf, ".ZILESAVE");
      fprintf(stderr, "Saving %s...\r\n",
              astr_cstr(buf));
      raw_write_to_disk(bp, astr_cstr(buf), 0600);
      astr_delete(buf);
    }
  exit(exitcode);
}

DEFUN_INT("cd", cd)
/*+
Make the user specified directory become the current buffer's default
directory.
+*/
{
  char *ms;
  astr buf;
  struct stat st;

  buf = get_current_dir(TRUE);
  if ((ms = minibuf_read_dir("Change default directory: ",
                             astr_cstr(buf))) == NULL) {
    astr_delete(buf);
    return cancel();
  }
  astr_delete(buf);

  if (ms[0] != '\0') {
    if (stat(ms, &st) != 0 || !S_ISDIR(st.st_mode)) {
      minibuf_error("`%s' is not a directory", ms);
      free(ms);
      return FALSE;
    }
    if (chdir(ms) == -1) {
      minibuf_write("%s: %s", ms, strerror(errno));
      free(ms);
      return FALSE;
    }
    free(ms);
    return TRUE;
  }

  free(ms);
  return FALSE;
}
END_DEFUN
