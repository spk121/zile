/*	$Id: file.c,v 1.1 2001/01/19 22:02:08 ssigala Exp $	*/

/*
 * Copyright (c) 1997-2001 Sandro Sigala.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

#include "config.h"
#include "zile.h"
#include "extern.h"

int exist_file(char *filename)
{
	struct stat st;

	if (stat(filename, &st) == -1)
		if (errno == ENOENT)
			return FALSE;

	return TRUE;
}

int is_regular_file(char *filename)
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

/*
 * This functions does some corrections and expansions to
 * the passed path:
 * - Splits the path into the directory and the filename;
 * - Expands the `~/' and `~name/' expressions;
 * - replaces the `//' with `/' (restarting from the root directory);
 * - removes the `..' and `.' entries.
 */
int expand_path(char *path, char *cwdir, char *dir, char *fname)
{
	char buf[1024];
	struct passwd *pw;
	char *sp, *dp, *fp, *p;

	sp = path, dp = dir, fp = fname;

	if (*sp != '/') {
		p = cwdir;
		while (*p != '\0')
			*dp++ = *p++;
		if (*(dp - 1) != '/')
			*dp++ = '/';
	}

	while (sp != '\0')
		if (*sp == '/') {
			if (*(sp + 1) == '/') {
				/*
				 * Got `//'.  Restart from this point.
				 */
				while (*++sp == '/')
					;
				dp = dir;
				*dp++ = '/';
			} else {
				++sp;
				*dp++ = '/';
			}
		} else if (*sp == '~') {
			if (*(sp + 1) == '/') {
				/*
				 * Got `~/'.  Restart from this point
				 * and insert the user home directory.
				 */
				dp = dir;
				if ((pw = getpwuid(getuid())) == NULL)
					return FALSE;
				if (strcmp(pw->pw_dir, "/") != 0) {
					p = pw->pw_dir;
					while (*p != '\0')
						*dp++ = *p++;
				}
				++sp;
			} else {
				/*
				 * Got `~something'.  Restart from this point
				 * and insert that user home directory.
				 */
				dp = dir;
				p = buf;
				++sp;
				while (*sp != '\0' && *sp != '/')
					*p++ = *sp++;
				*p = '\0';
				if ((pw = getpwnam(buf)) == NULL)
					return FALSE;
				p = pw->pw_dir;
				while (*p != '\0')
					*dp++ = *p++;
			}
		} else if (*sp == '.') {
			if (*(sp + 1) == '/' || *(sp + 1) == '\0') {
				++sp;
				if (*sp == '/' && *(sp + 1) != '/')
					++sp;
			} else if (*(sp + 1) == '.' &&
				   (*(sp + 2) == '/' || *(sp + 2) == '\0')) {
				if (dp > dir && *(dp - 1) == '/')
					--dp;
				while (*(dp - 1) != '/' && dp > dir)
					--dp;
				sp += 2;
				if (*sp == '/' && *(sp + 1) != '/')
					++sp;
			} else
				goto gotfname;
		} else {
		gotfname:
			p = sp;
			while (*p != '\0' && *p != '/')
				p++;
			if (*p == '\0') {
				/* Final filename. */
				while ((*fp++ = *sp++) != '\0')
					;
				break;
			} else {
				/* Non-final directory. */
				while (*sp != '/')
					*dp++ = *sp++;
			}
		}

	if (dp == dir)
		*dp++ = '/';

	*dp = '\0';
	*fp = '\0';

	return TRUE;
}

/*
 * Return a `~/foo' like path if the user is under his home directory,
 * else the unmodified path.
 */
char *compact_path(char *buf, char *path)
{
	struct passwd *pw;
	int i;

	if ((pw = getpwuid(getuid())) == NULL) {
		/* User not found in password file. */
		strcpy(buf, path);
		return buf;
	}

	/*
	 * Replace `/userhome/' (if existent) with `~/'.
	 */
	i = strlen(pw->pw_dir);
	if (!strncmp(pw->pw_dir, path, i)) {
		strcpy(buf, "~/");
		if (!strcmp(pw->pw_dir, "/"))
			strcat(buf, path + 1);
		else
			strcat(buf, path + i + 1);
	} else
		strcpy(buf, path);

	return buf;
}

/*
 * Return the current directory.
 */
char *get_current_dir(char *buf)
{
	if (cur_bp->filename != NULL) {
		/*
		 * If the current buffer has a filename, get the current
		 * directory name from it.
		 */
		char *p;

		strcpy(buf, cur_bp->filename);
		if ((p = strrchr(buf, '/')) != NULL) {
			if (p != buf)
				p[0] = '\0';
			else
				p[1] = '\0';
		}
		if (buf[strlen(buf) - 1] != '/')
			strcat(buf, "/");
	} else {
		/*
		 * Get the current directory name from the system.
		 */
		getcwd(buf, PATH_MAX);
		strcat(buf, "/");
	}

	return buf;
}

void open_file(char *path, int lineno)
{
	char buf[PATH_MAX], dir[PATH_MAX], fname[PATH_MAX];

	getcwd(buf, PATH_MAX);
	if (!expand_path(path, buf, dir, fname)) {
		fprintf(stderr, "zile: %s: invalid filename or path\n", path);
		exit(1);
	}
	strcpy(buf, dir);
	strcat(buf, fname);
	find_file(buf);
	if (lineno > 0)
		ngotodown(lineno);
	resync_redisplay();
}

/*
 * Add the character `c' to the end of the line `lp'.
 * Reallocate the line if there is no more space left for the addition.
 */
static linep fadd_char(linep lp, int c)
{
	linep lp1;

	if (lp->size + 1 >= lp->maxsize) {
		lp->maxsize += 10;
		lp1 = (linep)xrealloc(lp, sizeof *lp + lp->maxsize - sizeof lp->text);
		if (lp != lp1) {
			if (cur_bp->limitp->next == lp)
				cur_bp->limitp->next = lp1;
			lp1->prev->next = lp1;
			lp1->next->prev = lp1;
			lp = lp1;
		}
	}
	lp->text[lp->size++] = c;

	return lp;
}

/*
 * Add a newline at the end of the line `lp'.
 */
static linep fadd_newline(linep lp)
{
	linep lp1;

#if 0
	lp->maxsize = lp->size ? lp->size : 1;
	lp1 = (linep)xrealloc(lp, sizeof *lp + lp->maxsize - sizeof lp->text);
	if (lp != lp1) {
		if (cur_bp->limitp->next == lp)
			cur_bp->limitp->next = lp1;
		lp1->prev->next = lp1;
		lp1->next->prev = lp1;
		lp = lp1;
	}
#endif
	lp1 = new_line(1);
	lp1->next = lp->next;
	lp1->next->prev = lp1;
	lp->next = lp1;
	lp1->prev = lp;
	lp = lp1;
	++cur_bp->num_lines;

	return lp;
}

/*
 * Read the file contents into a buffer.
 * Return quietly if the file doesn't exist.
 */
void read_from_disk(char *filename)
{
	linep lp;
	int fd, i, size;
	char buf[BUFSIZ];

	if ((fd = open(filename, O_RDONLY)) < 0) {
		if (errno != ENOENT) {
			minibuf_write("%FY%s%E: %s", filename, strerror(errno));
			cur_bp->flags |= BFLAG_READONLY;
		}
		return;
	}

	lp = cur_wp->pointp;

	while ((size = read(fd, buf, BUFSIZ)) > 0)
		for (i = 0; i < size; i++)
			if (buf[i] != '\n')
				lp = fadd_char(lp, buf[i]);
			else
				lp = fadd_newline(lp);

	lp->next = cur_bp->limitp;
	cur_bp->limitp->prev = lp;
	cur_wp->pointp = cur_bp->limitp->next;

	close(fd);
}

static int have_extension(char *filename, ...)
{
	va_list ap;
	int i, len, extlen;
	char *s;

	len = strlen(filename);

	va_start(ap, filename);
	for (i = 0; (s = va_arg(ap, char *)) != NULL; ++i) {
		extlen = strlen(s);
		if (len > extlen && !strcmp(filename + len - extlen, s))
			return TRUE;
	}
	va_end(ap);

	return FALSE;
}

static void find_file_hooks(char *filename)
{
	if (have_extension(filename, ".c", ".h", NULL)) {
		FUNCALL(c_mode);
		if (lookup_bool_variable("auto-font-lock"))
			FUNCALL(font_lock_mode);
	} else if (have_extension(filename, ".C", ".H", ".cc", ".cpp",
				  ".cxx", ".hpp", NULL)) {
		FUNCALL(cpp_mode);
		if (lookup_bool_variable("auto-font-lock"))
			FUNCALL(font_lock_mode);
	}
}

int find_file(char *filename)
{
	bufferp bp;
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
		minibuf_error("%FY%s%FC is not a regular file%E", filename);
		waitkey(1 * 1000);
		return FALSE;
	}

	bp = create_buffer(s);
	free(s);
	bp->filename = xstrdup(filename);
	bp->flags = 0;

	switch_to_buffer(bp);

	read_from_disk(filename);

	find_file_hooks(filename);

	thisflag |= FLAG_NEED_RESYNC;

	return TRUE;
}

historyp make_buffer_history(void)
{
	bufferp bp;
	historyp hp;
	int i;

	for (i = 0, bp = head_bp; bp != NULL; bp = bp->next)
		++i;
	hp = new_history(i, FALSE);
	for (i = 0, bp = head_bp; bp != NULL; bp = bp->next)
		hp->completions[i++] = xstrdup(bp->name);

	return hp;
}

DEFUN("find-file", find_file)
/*+
Edit a file specified by the user.  Switch to a buffer visiting the file,
creating one if none already exists.
+*/
{
	char buf[PATH_MAX], *ms;

	get_current_dir(buf);
	if ((ms = minibuf_read_dir("Find file: ", buf)) == NULL)
		return cancel();

	if (ms[0] != '\0') {
		int retvalue = find_file(ms);
		if (!(cur_bp->flags & (BFLAG_CMODE | BFLAG_CPPMODE)) &&
		    lookup_bool_variable("text-mode-auto-fill"))
			FUNCALL(auto_fill_mode);
		return retvalue;
	}

	return FALSE;
}

DEFUN("find-alternate-file", find_alternate_file)
/*+
Find the file specified by the user, select its buffer, kill previous buffer.
If the current buffer now contains an empty file that you just visited
(presumably by mistake), use this command to visit the file you really want.
+*/
{
	char buf[PATH_MAX], *ms;

	get_current_dir(buf);
	if ((ms = minibuf_read_dir("Find alternate: ", buf)) == NULL)
		return cancel();

	if (ms[0] != '\0' && check_modified_buffer(cur_bp)) {
		kill_buffer(cur_bp);
		if (!find_file(ms))
			return FALSE;

		if (!(cur_bp->flags & (BFLAG_CMODE | BFLAG_CPPMODE)) &&
		    lookup_bool_variable("text-mode-auto-fill"))
			FUNCALL(auto_fill_mode);

		return TRUE;
	}

	return FALSE;
}

DEFUN("switch-to-buffer", switch_to_buffer)
/*+
Select to the user specified buffer in the current window.
+*/
{
	char *ms;
	bufferp swbuf;
	historyp hp;

	swbuf = prev_bp != NULL ? prev_bp : cur_bp->next != NULL ? cur_bp->next : head_bp;

	hp = make_buffer_history();
	ms = minibuf_read_history("Switch to buffer (default %FY%s%E): ", "", hp, swbuf->name);
	free_history(hp);
	if (ms == NULL)
		return cancel();

	if (ms[0] != '\0') {
		if ((swbuf = find_buffer(ms, FALSE)) == NULL) {
			swbuf = find_buffer(ms, TRUE);
			swbuf->flags = BFLAG_NEEDNAME | BFLAG_NOSAVE;
		}
	}

	prev_bp = cur_bp;
	switch_to_buffer(swbuf);

	return TRUE;
}

/*
 * Check if the buffer has been modified.  If so, asks the user if
 * he/she wants to save the changes.  If the response is positive, return
 * TRUE, else FALSE.
 */
int check_modified_buffer(bufferp bp)
{
	int ans;

	if (bp->flags & BFLAG_MODIFIED && !(bp->flags & BFLAG_NOSAVE))
		for (;;) {
			if ((ans = minibuf_read_yesno("Buffer %FY%s%E modified; kill anyway? (yes or no) ", bp->name)) == -1)
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
void kill_buffer(bufferp kill_bp)
{
	bufferp next_bp;

	if (kill_bp == prev_bp)
		prev_bp = NULL;

	if (kill_bp->next != NULL)
		next_bp = kill_bp->next;
	else
		next_bp = head_bp;

	if (next_bp == kill_bp) {
		/*
		 * This is the sole buffer available, then
		 * remove the contents and set the name to `*scratch*'
		 * if it is not already set.
		 */
		assert(cur_bp == kill_bp);
		zap_buffer_content();
		cur_bp->flags = BFLAG_NOSAVE | BFLAG_NEEDNAME | BFLAG_TEMPORARY;
		if (strcmp(cur_bp->name, "*scratch*") != 0) {
			set_buffer_name(cur_bp, "*scratch*");
			if (cur_bp->filename != NULL) {
				free(cur_bp->filename);
				cur_bp->filename = NULL;
			}
		}
	} else {
		bufferp bp;
		windowp wp;
		linep pointp;
		int pointo, pointn;

		assert(kill_bp != next_bp);

		pointp = next_bp->save_pointp;
		pointo = next_bp->save_pointo;
		pointn = next_bp->save_pointn;

		/*
		 * Search for windows displaying the next buffer available.
		 */
		for (wp = head_wp; wp != NULL; wp = wp->next)
			if (wp->bp == next_bp) {
				pointp = wp->pointp;
				pointo = wp->pointo;
				pointn = wp->pointn;
				break;
			}

		/*
		 * Search for windows displaying the buffer to kill.
		 */
		for (wp = head_wp; wp != NULL; wp = wp->next)
			if (wp->bp == kill_bp) {
				--kill_bp->num_windows;
				++next_bp->num_windows;
				wp->bp = next_bp;
				wp->pointp = pointp;
				wp->pointo = pointo;
				wp->pointn = pointn;
			}

		assert(kill_bp->num_windows == 0);

		/*
		 * Remove the buffer from the buffer list.
		 */
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

DEFUN("kill-buffer", kill_buffer)
/*+
Kill the current buffer or the user specified one.
+*/
{
	bufferp bp;
	char *ms;
	historyp hp;

	hp = make_buffer_history();
	if ((ms = minibuf_read_history("Kill buffer (default %FY%s%E): ", "", hp, cur_bp->name)) == NULL)
		return cancel();
	free_history(hp);
	if (ms[0] != '\0') {
		if ((bp = find_buffer(ms, FALSE)) == NULL) {
			minibuf_error("%FCBuffer %FY`%s'%FC not found%E", ms);
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

void insert_buffer(bufferp bp)
{
	linep lp;
	char *p;

	for (lp = bp->limitp->next; lp != bp->limitp; lp = lp->next) {
		for (p = lp->text; p < lp->text + lp->size; ++p)
			insert_char(*p);
		if (lp->next != bp->limitp)
			insert_newline();
	}
}

DEFUN("insert-buffer", insert_buffer)
/*+
Insert after point the contents of the user specified buffer.
Puts mark after the inserted text.
+*/
{
	bufferp bp, swbuf;
	char *ms;
	historyp hp;

	if (warn_if_readonly_buffer())
		return FALSE;

	swbuf = prev_bp != NULL ? prev_bp : cur_bp->next != NULL ? cur_bp->next : head_bp;
	hp = make_buffer_history();
	if ((ms = minibuf_read_history("Insert buffer (default %FY%s%E): ", "", hp, swbuf->name)) == NULL)
		return cancel();
	free_history(hp);
	if (ms[0] != '\0') {
		if ((bp = find_buffer(ms, FALSE)) == NULL) {
			minibuf_error("%FCBuffer %FY`%s'%FC not found%E", ms);
			return FALSE;
		}
	} else
		bp = swbuf;

	if (bp == cur_bp) {
		minibuf_error("%FCCannot insert the current buffer%E");
		return FALSE;
	}

	insert_buffer(bp);
	set_mark_command();

	return TRUE;
}

int insert_file(char *filename)
{
	int fd, i, size;
	char buf[BUFSIZ];

	if (!exist_file(filename)) {
		minibuf_error("%FCUnable to read file %FY`%s'%E", filename);
		return FALSE;
	}

	if ((fd = open(filename, O_RDONLY)) < 0) {
		minibuf_write("%FY%s%E: %s", filename, strerror(errno));
		return FALSE;
	}

	while ((size = read(fd, buf, BUFSIZ)) > 0)
		for (i = 0; i < size; i++)
			if (buf[i] != '\n')
				insert_char(buf[i]);
			else
				insert_newline();
	close(fd);

	return TRUE;
}

DEFUN("insert-file", insert_file)
/*+
Insert contents of the user specified file into buffer after point.
Set mark after the inserted text.
+*/
{
	char buf[PATH_MAX], *ms;

	if (warn_if_readonly_buffer())
		return FALSE;

	get_current_dir(buf);
	if ((ms = minibuf_read_dir("Insert file: ", buf)) == NULL)
		return cancel();

	if (ms[0] == '\0')
		return FALSE;

	if (!insert_file(ms))
		return FALSE;
	
	set_mark_command();

	return TRUE;
}

/*
 * Create a backup filename according to user specified variables.
 */

static char *create_backup_filename(char *filename, int withrevs,
				    int withdirectory)
{
	char buf[PATH_MAX];
	char *s;

	/* Add the backup directory path to the filename */
	if (withdirectory) {
		char dir[PATH_MAX], fname[PATH_MAX];
		char *bp, *backupdir;

		backupdir = get_variable("backup-directory");
		bp = buf;

		while (*backupdir != '\0')
			*bp++ = *backupdir++;
		if (*(bp-1) != '/')
			*bp++ = '/';
		while (*filename != '\0') {
			if (*filename == '/')
				*bp++ = '!';
			else
				*bp++ = *filename;
			++filename;
		}
		*bp = '\0';

		if (!expand_path(buf, "", dir, fname)) {
			fprintf(stderr, "zile: %s: invalid backup directory\n", dir);
			exit(1);
		}
		strcpy(buf, dir);
		strcat(buf, fname);
		filename = buf;
 	}

	s = (char *)xmalloc(strlen(filename) + 10);

	if (withrevs) {
		int n = 1, fd;
		/* Find a non existing filename. */
		for (;;) {
			sprintf(s, "%s~%d~", filename, n);
			if ((fd = open(s, O_RDONLY, 0)) != -1)
				close(fd);
			else
				break;
			++n;
		}
	} else { /* simple backup */
		strcpy(s, filename);
		strcat(s, "~");
	}

	return s;
}

/*
 * Copy a file.
 */
static int copy_file(char *source, char *dest)
{
	char buf[BUFSIZ];	
	int ifd, ofd, len, stat_valid;
	struct stat st;

	ifd = open(source, O_RDONLY, 0);
	if (ifd < 0) {
		minibuf_error("%FY%s:%FC unable to backup%E", source);
		return FALSE;
	}

	ofd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (ifd < 0) {
		close(ifd);
		minibuf_error("%FY%s:%FC unable to create backup%E", dest);
		return FALSE;
	}

	while ((len = read(ifd, buf, sizeof buf)) > 0)
		if(write(ofd, buf, len) < 0) {
			minibuf_error("%FCunable to write to backup %FY%s%E", dest);
			close(ifd);
			close(ofd);
			return FALSE;
		}

	stat_valid = fstat(ifd, &st) != -1;

	/* Recover file permissions and ownership. */
	if (stat_valid) {
		fchmod(ofd, st.st_mode);
		fchown(ofd, st.st_uid, st.st_gid);
	}

	close(ifd);
	close(ofd);

	/* Recover file modification time. */
	if (stat_valid) {
		struct utimbuf t;
		t.actime = st.st_atime; 
		t.modtime = st.st_mtime; 
		utime(dest, &t);
	}

	return TRUE;
}

/*
 * Write the buffer contents to a file.  Create a backup file if specified
 * by the user variables.
 */
static int write_to_disk(bufferp bp, char *filename)
{
	int fd, backupsimple, backuprevs, backupwithdir;
	linep lp;

	backupsimple = is_variable_equal("backup-method", "simple");
	backuprevs = is_variable_equal("backup-method", "revision");
	backupwithdir = lookup_bool_variable("backup-with-directory");

	/*
	 * Make backup of original file.
	 */
	if (!(bp->flags & BFLAG_BACKUP) && (backupsimple || backuprevs)
	    && (fd = open(filename, O_RDWR, 0)) != -1) {
		char *bfilename;
		close(fd);
		bfilename = create_backup_filename(filename, backuprevs,
						   backupwithdir);
		if (!copy_file(filename, bfilename))
			waitkey_discard(3 * 1000);
		free(bfilename);
		bp->flags |= BFLAG_BACKUP;
	}

	if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
		minibuf_error("%FY%s%E: %s", filename, strerror(errno));
		return FALSE;
	}

	/* Save all the lines. */
	for (lp = bp->limitp->next; lp != bp->limitp; lp = lp->next) {
		write(fd, lp->text, lp->size);
		if (lp->next != bp->limitp)
			write(fd, "\n", 1);
	}

	close(fd);

	return TRUE;
}

static int save_buffer(bufferp bp)
{
	char *ms, *fname = bp->filename != NULL ? bp->filename : bp->name;

	if (!(bp->flags & BFLAG_MODIFIED))
		minibuf_write("%FC(No changes need to be saved)%E");
	else {
		if (bp->flags & BFLAG_NEEDNAME) {
			if ((ms = minibuf_read_dir("File to save in: ", fname)) == NULL)
				return cancel();
			if (ms[0] == '\0')
				return FALSE;

			set_buffer_filename(bp, ms);

			bp->flags &= ~BFLAG_NEEDNAME;
		} else
			ms = bp->filename;
		if (write_to_disk(bp, ms)) {
			minibuf_write("Wrote %FY%s%E", ms);
			bp->flags &= ~BFLAG_MODIFIED;
		}
		bp->flags &= ~BFLAG_TEMPORARY;
	}

	return TRUE;
}

DEFUN("save-buffer", save_buffer)
/*+
Save current buffer in visited file if modified. By default, makes the
previous version into a backup file if this is the first save.
+*/
{
	return save_buffer(cur_bp);
}

DEFUN("write-file", write_file)
/*+
Write current buffer into the user specified file.
Makes buffer visit that file, and marks it not modified.
+*/
{
	char *fname = cur_bp->filename != NULL ? cur_bp->filename : cur_bp->name;
	char *ms;

	if ((ms = minibuf_read_dir("Write file: ", fname)) == NULL)
		return cancel();
	if (ms[0] == '\0')
		return FALSE;

	set_buffer_filename(cur_bp, ms);

	cur_bp->flags &= ~(BFLAG_NEEDNAME | BFLAG_TEMPORARY);

	if (write_to_disk(cur_bp, ms)) {
		minibuf_write("Wrote %FY%s%E", ms);
		cur_bp->flags &= ~BFLAG_MODIFIED;
	}

	return TRUE;
}

static int save_some_buffers(void)
{
	bufferp bp;
	int i = 0, noask = 0, c;

	for (bp = head_bp; bp != NULL; bp = bp->next)
		if (bp->flags & BFLAG_MODIFIED
		    && !(bp->flags & BFLAG_NOSAVE)) {
			char *fname = bp->filename != NULL ? bp->filename : bp->name;

			++i;

			if (noask)
				save_buffer(bp);
			else {
				for (;;) {
					minibuf_write("Save file %FY%s%E? (SPC, y, n, !, ., q) ", fname);
					c = cur_tp->getkey();
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

					minibuf_error("Please answer SPC, y, n, !, . or q.");
					waitkey(2 * 1000);
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
					noask = 1;
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
		minibuf_write("%FC(No files need saving)%E");

	return TRUE;
}


DEFUN("save-some-buffers", save_some_buffers)
/*+
Save some modified file-visiting buffers.  Asks user about each one.
+*/
{
	return save_some_buffers();
}

DEFUN("save-buffers-kill-zile", save_buffers_kill_zile)
/*+
Offer to save each buffer, then kill this Zile process.
+*/
{
	bufferp bp;
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

DEFUN("cd", cd)
/*+
Make the user specified directory become the current buffer's default
directory.
+*/
{
	char buf[PATH_MAX], *ms;
	struct stat st;

	get_current_dir(buf);
	if ((ms = minibuf_read_dir("Change default directory: ", buf)) == NULL)
		return cancel();

	if (ms[0] != '\0') {
		if (stat(ms, &st) != 0 || !S_ISDIR(st.st_mode)) {
			minibuf_error("%FY`%s'%E is not a directory", ms);
			return FALSE;
		}
		if (chdir(ms) == -1) {
			minibuf_write("%FY%s%E: %s", ms, strerror(errno));
			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}
