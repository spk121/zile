/* Disk file handling
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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id: file.c,v 1.19 2004/03/09 16:17:59 rrt Exp $	*/

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
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <utime.h>
#include <ctype.h>

#include "zile.h"
#include "agetcwd.h"
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

/*
 * This functions does some corrections and expansions to
 * the passed path:
 * - Splits the path into the directory and the filename;
 * - Expands the `~/' and `~name/' expressions;
 * - replaces the `//' with `/' (restarting from the root directory);
 * - removes the `..' and `.' entries.
 */
int expand_path(const char *path, const char *cwdir, astr dir, astr fname)
{
	struct passwd *pw;
	const char *sp = path;

	if (*sp != '/') {
		const char *p = cwdir;
		while (*p != '\0')
			astr_append_char(dir, *p++);
		if (astr_last_char(dir) != '/')
			astr_append_char(dir, '/');
	}

	while (*sp != '\0') {
		if (*sp == '/') {
			if (*++sp == '/') {
				/*
				 * Got `//'.  Restart from this point.
				 */
				while (*sp == '/')
					sp++;
                                astr_clear(dir);
			}
                        astr_append_char(dir, '/');
		} else if (*sp == '~') {
			if (*(sp + 1) == '/') {
				/*
				 * Got `~/'.  Restart from this point
				 * and insert the user home directory.
				 */
				astr_clear(dir);
				if ((pw = getpwuid(getuid())) == NULL)
					return FALSE;
				if (strcmp(pw->pw_dir, "/") != 0) {
					const char *p = pw->pw_dir;
					while (*p != '\0')
						astr_append_char(dir, *p++);
				}
				++sp;
			} else {
				/*
				 * Got `~something'.  Restart from this point
				 * and insert that user home directory.
				 */
                                const char *p;
                                astr as = astr_new();
				astr_clear(dir);
				++sp;
				while (*sp != '\0' && *sp != '/')
					astr_append_char(as, *sp++);
				pw = getpwnam(astr_cstr(as));
                                astr_delete(as);
				if (pw == NULL)
					return FALSE;
				p = pw->pw_dir;
				while (*p != '\0')
					astr_append_char(dir, *p++);
			}
		} else if (*sp == '.') {
			if (*(sp + 1) == '/' || *(sp + 1) == '\0') {
				++sp;
				if (*sp == '/' && *(sp + 1) != '/')
					++sp;
			} else if (*(sp + 1) == '.' &&
				   (*(sp + 2) == '/' || *(sp + 2) == '\0')) {
			    if (astr_size(dir) >= 1 && astr_last_char(dir) == '/')
					astr_truncate(dir, astr_size(dir) - 1);
				while (astr_last_char(dir) != '/' &&
				       astr_size(dir) >= 1)
					astr_truncate(dir, astr_size(dir) - 1);
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
				astr_append_cstr(fname, sp);
				break;
			} else {
				/* Non-final directory. */
				while (*sp != '/')
					astr_append_char(dir, *sp++);
			}
		}
	}

	if (astr_isempty(dir))
		astr_append_char(dir, '/');

	return TRUE;
}

/*
 * Return a `~/foo' like path if the user is under his home directory,
 * else the unmodified path.
 */
astr compact_path(astr buf, const char *path)
{
	struct passwd *pw;
	int i;

	if ((pw = getpwuid(getuid())) == NULL) {
		/* User not found in password file. */
		astr_assign_cstr(buf, path);
		return buf;
	}

	/*
	 * Replace `/userhome/' (if existent) with `~/'.
	 */
	i = strlen(pw->pw_dir);
	if (!strncmp(pw->pw_dir, path, i)) {
		astr_assign_cstr(buf, "~/");
		if (!strcmp(pw->pw_dir, "/"))
			astr_append_cstr(buf, path + 1);
		else
			astr_append_cstr(buf, path + i + 1);
	} else
		astr_assign_cstr(buf, path);

	return buf;
}

/*
 * Return the current directory.
 */
astr get_current_dir(astr buf, int interactive)
{
	if (interactive && cur_bp->filename != NULL) {
		/*
		 * If the current buffer has a filename, get the current
		 * directory name from it.
		 */
		int p;

		astr_assign_cstr(buf, cur_bp->filename);
                p = astr_rfind_char(buf, '/');
                if (p != -1)
                        astr_truncate(buf, p ? p : 1);
		if (astr_last_char(buf) != '/')
			astr_append_char(buf, '/');
	} else {
		/*
		 * Get the current directory name from the system.
		 */
		agetcwd(buf);
		astr_append_char(buf, '/');
	}

	return buf;
}

void open_file(char *path, int lineno)
{
	astr buf, dir, fname;

	buf = astr_new();
	dir = astr_new();
	fname = astr_new();
	get_current_dir(buf, FALSE);
	ZTRACE(("original filename: %s, cwd: %s\n", path, astr_cstr(buf)));
	if (!expand_path(path, astr_cstr(buf), dir, fname)) {
		fprintf(stderr, "zile: %s: invalid filename or path\n", path);
		zile_exit(1);
	}
	ZTRACE(("new filename: %s, dir: %s\n", astr_cstr(fname), astr_cstr(dir)));
	astr_assign_cstr(buf, astr_cstr(dir));
	astr_append_cstr(buf, astr_cstr(fname));
	astr_delete(dir);
	astr_delete(fname);

	find_file(astr_cstr(buf));
	astr_delete(buf);
	if (lineno > 0)
		ngotodown(lineno);
	resync_redisplay();
}

/*
 * Add the character `c' to the end of the line `lp'.
 * Reallocate the line if there is no more space left for the addition.
 */
static Line *fadd_char(Line *lp, int c)
{
	Line *lp1;

	if (lp->size + 1 >= lp->maxsize) {
		lp->maxsize += 10;
		lp1 = (Line *)zrealloc(lp, sizeof *lp + lp->maxsize - sizeof lp->text);
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
static Line *fadd_newline(Line *lp)
{
	Line *lp1;

#if 0
	lp->maxsize = lp->size ? lp->size : 1;
	lp1 = (linep)zrealloc(lp, sizeof *lp + lp->maxsize - sizeof lp->text);
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
void read_from_disk(const char *filename)
{
	Line *lp;
	int fd, i, size;
	char buf[BUFSIZ];

	if ((fd = open(filename, O_RDONLY)) < 0) {
		if (errno != ENOENT) {
			minibuf_write("%s: %s", filename, strerror(errno));
			cur_bp->flags |= BFLAG_READONLY;
		}
		return;
	}

	lp = cur_bp->pt.p;

	while ((size = read(fd, buf, BUFSIZ)) > 0)
		for (i = 0; i < size; i++)
			if (buf[i] != '\n')
				lp = fadd_char(lp, buf[i]);
			else
				lp = fadd_newline(lp);

	lp->next = cur_bp->limitp;
	cur_bp->limitp->prev = lp;
	cur_bp->pt.p = cur_bp->limitp->next;

	close(fd);
}

static int have_extension(const char *filename, const char *exts[])
{
	int i, len, extlen;

	len = strlen(filename);
	for (i = 0; exts[i] != NULL; ++i) {
		extlen = strlen(exts[i]);
		if (len > extlen && !strcmp(filename + len - extlen, exts[i]))
			return TRUE;
	}

	return FALSE;
}

static int exists_corr_file(char *dest, char *src, const char *ext)
{
	char *p;
	strcpy(dest, src);
	if ((p = strrchr(dest, '.')) != NULL)
		*p = '\0';
	strcat(dest, ext);
	if (exist_file(dest))
		return TRUE;
	strcpy(dest, src);
	strcat(dest, ext);
	if (exist_file(dest))
		return TRUE;
	return FALSE;
}

DEFUN("switch-to-correlated-buffer", switch_to_correlated_buffer)
/*+
Find and open a file correlated with the current buffer.
Some examples of correlated files are the following:
    anyfile.c  --> anyfile.h
    anyfile.h  --> anyfile.c
    anyfile.in --> anyfile
    anyfile    --> anyfile.in
+*/
{
	const char *c_src[] = { ".c", ".C", ".cc", ".cpp", ".cxx", ".m", NULL };
	const char *c_hdr[] = { ".h", ".H", ".hpp", ".hh", NULL };
	const char *in_ext[] = { ".in", ".IN", NULL };
	int i;
	char *nfile;
	char *fname;

	fname = cur_bp->filename;
	if (fname == NULL)
		fname = cur_bp->name;
	nfile = (char *)zmalloc(strlen(fname) + 10);

	if (have_extension(fname, c_src))
		for (i = 0; c_hdr[i] != NULL; ++i)
			if (exists_corr_file(nfile, fname, c_hdr[i])) {
				int retvalue = find_file(nfile);
				free(nfile);
				return retvalue;
			}
	if (have_extension(fname, c_hdr))
		for (i = 0; c_src[i] != NULL; ++i)
			if (exists_corr_file(nfile, fname, c_src[i])) {
				int retvalue = find_file(nfile);
				free(nfile);
				return retvalue;
			}
	if (have_extension(fname, in_ext))
		if (exists_corr_file(nfile, fname, "")) {
			int retvalue = find_file(nfile);
			free(nfile);
			return retvalue;
		}
	if (exists_corr_file(nfile, fname, ".in")) {
		int retvalue = find_file(nfile);
		free(nfile);
		return retvalue;
	}

	minibuf_error("No correlated file was found for this buffer");
	return FALSE;
}

#if ENABLE_SHELL_MODE
/*
 * Try to identify a shell script file.
 *
 * A shell file is identified if the first line is in the format:
 *     <any whitespace> <the # char> <any whitespace> <the ! char>
 */
static int is_shell_file(const char *filename)
{
	FILE *f;
	char buf[1024], *p;
	if ((f = fopen(filename, "r")) == NULL)
		return FALSE;
	p = fgets(buf, 1024, f);
	fclose(f);
	if (p == NULL)
		return FALSE;
	while (isspace(*p))
		++p;
	if (*p != '#')
		return FALSE;
	++p;
	while (isspace(*p))
		++p;
	if (*p != '!')
		return FALSE;
	return TRUE;
}
#endif

static void find_file_hooks(const char *filename)
{
#if ENABLE_C_MODE
	const char *c_file[] = { ".c", ".h", ".m", NULL };
#endif
#if ENABLE_CPP_MODE
	const char *cpp_file[] = { ".C", ".H", ".cc", ".cpp",
				   ".cxx", ".hpp", ".hh", NULL };
#endif
#if ENABLE_CSHARP_MODE
	const char *csharp_file[] = { ".cs", ".CS", NULL };
#endif
#if ENABLE_JAVA_MODE
	const char *java_file[] = { ".java", ".JAVA", NULL };
#endif
#if ENABLE_SHELL_MODE
	const char *shell_file[] = { ".sh", ".csh", NULL };
#endif

        (void)filename; /* Avoid compiler warning if no non-text modes
                           configured. */
	if (0) {} /* Hack */
#if ENABLE_C_MODE
	else if (have_extension(filename, c_file))
		FUNCALL(c_mode);
#endif
#if ENABLE_CPP_MODE
	else if (have_extension(filename, cpp_file))
		FUNCALL(cpp_mode);
#endif
#if ENABLE_CSHARP_MODE
	else if (have_extension(filename, csharp_file))
		FUNCALL(csharp_mode);
#endif
#if ENABLE_JAVA_MODE
	else if (have_extension(filename, java_file))
		FUNCALL(java_mode);
#endif
#if ENABLE_SHELL_MODE
	else if (have_extension(filename, shell_file) ||
		 is_shell_file(filename))
		FUNCALL(shell_script_mode);
#endif
	else
		FUNCALL(text_mode);
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
		waitkey(1 * 1000);
		return FALSE;
	}

	bp = create_buffer(s);
	free(s);
	bp->filename = zstrdup(filename);
	bp->flags = 0;

	switch_to_buffer(bp);
	read_from_disk(filename);
	find_file_hooks(filename);

	thisflag |= FLAG_NEED_RESYNC;

	return TRUE;
}

Completion *make_buffer_completion(void)
{
	Buffer *bp;
	Completion *cp;

	cp = new_completion(FALSE);
	for (bp = head_bp; bp != NULL; bp = bp->next)
		alist_append(cp->completions, zstrdup(bp->name));

	return cp;
}

DEFUN("find-file", find_file)
/*+
Edit a file specified by the user.  Switch to a buffer visiting the file,
creating one if none already exists.
+*/
{
	char *ms;
	astr buf;

	buf = astr_new();
	get_current_dir(buf, TRUE);
	if ((ms = minibuf_read_dir("Find file: ", astr_cstr(buf)))
	    == NULL) {
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

DEFUN("find-alternate-file", find_alternate_file)
/*+
Find the file specified by the user, select its buffer, kill previous buffer.
If the current buffer now contains an empty file that you just visited
(presumably by mistake), use this command to visit the file you really want.
+*/
{
	char *ms;
	astr buf;

	buf = astr_new();
	get_current_dir(buf, TRUE);
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

DEFUN("switch-to-buffer", switch_to_buffer)
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
		Buffer *bp;
		Window *wp;

		assert(kill_bp != next_bp);

		/*
		 * Search for windows displaying the buffer to kill.
		 */
		for (wp = head_wp; wp != NULL; wp = wp->next)
			if (wp->bp == kill_bp) {
				wp->bp = next_bp;
				/* The marker will be free.  */
				wp->saved_pt = NULL;
			}

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

		/* Free the buffer.  */
		free_buffer(kill_bp);

		thisflag |= FLAG_NEED_RESYNC;
	}
}

DEFUN("kill-buffer", kill_buffer)
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

void insert_buffer(Buffer *bp)
{
	Line *lp;
	char *p;
	int size = calculate_buffer_size(bp);

	undo_save(UNDO_REMOVE_BLOCK, cur_bp->pt, size, 0);
	undo_nosave = TRUE;
	for (lp = bp->limitp->next; lp != bp->limitp; lp = lp->next) {
		for (p = lp->text; p < lp->text + lp->size; ++p)
			insert_char(*p);
		if (lp->next != bp->limitp)
			insert_newline();
	}
	undo_nosave = FALSE;
}

DEFUN("insert-buffer", insert_buffer)
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

static int insert_file(char *filename)
{
	int fd, i, size;
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

DEFUN("insert-file", insert_file)
/*+
Insert contents of the user specified file into buffer after point.
Set mark after the inserted text.
+*/
{
	char *ms;
	astr buf;

	if (warn_if_readonly_buffer())
		return FALSE;

	buf = astr_new();
	get_current_dir(buf, TRUE);
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

static int ask_delete_old_revisions(const char *filename)
{
	int c;
	minibuf_write("Delete excess backup versions of %s? (y or n) ", filename);
	c = cur_tp->getkey();
	while (c != 'y') {
		if (c == 'n' || c == 'q' || c == KBD_CANCEL)
			return FALSE;
		minibuf_write("Please answer y or n.  Delete excess backup versions of %s? (y or n) ", filename);
		c = cur_tp->getkey();
	}
	return TRUE;
}

static int get_new_revision(const char *filename)
{
	int i, fd, first, latest, maxrev;
        int count = 0;
	char *buf = (char *)zmalloc(strlen(filename) + 10);

	/* Find the latest existing revision. */
	for (i = 999; i >= 1; --i) { /* XXX need to fix this. */
		sprintf(buf, "%s.~%d~", filename, i);
		if ((fd = open(buf, O_RDONLY, 0)) != -1) {
			close(fd);
			break;
		}
	}
	latest = i + 1;
	first = 0;
	/* Count the existing revisions. */
	for (i = 1; i <= latest; ++i) {
		sprintf(buf, "%s.~%d~", filename, i);
		if ((fd = open(buf, O_RDONLY, 0)) != -1) {
			close(fd);
			if (first == 0)
				first = i;
			++count;
		}
	}
	maxrev = atoi(get_variable("revisions-kept"));
	maxrev = max(maxrev, 2);
	if (count >= maxrev) {
		char *action = get_variable("revisions-delete");
		int no = count - maxrev + 1;
		int confirm = FALSE;
		if (!strcmp(action, "ask"))
			confirm = ask_delete_old_revisions(filename);
		else if (!strcmp(action, "noask"))
			confirm = TRUE;
		if (confirm) {
			for (i = 1; i < latest && no > 0; ++i) {
				sprintf(buf, "%s.~%d~", filename, i);
				if ((fd = open(buf, O_RDONLY, 0)) != -1) {
					close(fd);
					remove(buf);
					--no;
				}
			}
		}
	}

	free(buf);

	return latest;
}

/*
 * Create a backup filename according to user specified variables.
 */
static char *create_backup_filename(const char *filename, int withrevs,
				    int withdirectory)
{
	astr buf;
	char *s;

	buf = NULL;  /* to know if it has been used */
	/* Add the backup directory path to the filename */
	if (withdirectory) {
		astr dir, fname;
		char *backupdir;

		backupdir = get_variable("backup-directory");
		buf = astr_new();

		while (*backupdir != '\0')
			astr_append_char(buf, *backupdir++);
		if (astr_last_char(buf) != '/')
			astr_append_char(buf, '/');
		while (*filename != '\0') {
			if (*filename == '/')
				astr_append_char(buf, '!');
			else
				astr_append_char(buf, *filename);
			++filename;
		}

		dir = astr_new();
		fname = astr_new();
		if (!expand_path(astr_cstr(buf), "", dir, fname)) {
			fprintf(stderr, "zile: %s: invalid backup directory\n", astr_cstr(dir));
			zile_exit(1);
		}
		astr_assign_cstr(buf, astr_cstr(dir));
		astr_append_cstr(buf, astr_cstr(fname));
		astr_delete(dir);
		astr_delete(fname);
		filename = astr_cstr(buf);
	}

	s = (char *)zmalloc(strlen(filename) + 10);

	if (withrevs) {
		int n = get_new_revision(filename);
		sprintf(s, "%s.~%d~", filename, n);
	} else { /* simple backup */
		strcpy(s, filename);
		strcat(s, "~");
	}

	if (buf != NULL)
		astr_delete(buf);

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
		minibuf_error("%s: unable to backup", source);
		return FALSE;
	}

	ofd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (ifd < 0) {
		close(ifd);
		minibuf_error("%s: unable to create backup", dest);
		return FALSE;
	}

	while ((len = read(ifd, buf, sizeof buf)) > 0)
		if(write(ofd, buf, len) < 0) {
			minibuf_error("unable to write to backup %s", dest);
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

static int raw_write_to_disk(Buffer *bp, const char *filename)
{
	int fd;
	Line *lp;

	if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
		return FALSE;

	/* Save all the lines. */
	for (lp = bp->limitp->next; lp != bp->limitp; lp = lp->next) {
		write(fd, lp->text, lp->size);
		if (lp->next != bp->limitp)
			write(fd, "\n", 1);
	}

	close(fd);

	return TRUE;
}

/*
 * Write the buffer contents to a file.  Create a backup file if specified
 * by the user variables.
 */
static int write_to_disk(Buffer *bp, char *filename)
{
	int fd, backupsimple, backuprevs, backupwithdir;
	Line *lp;

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
		minibuf_error("%s: %s", filename, strerror(errno));
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
			minibuf_write("Wrote %s", ms);
			bp->flags &= ~BFLAG_MODIFIED;
		}
		bp->flags &= ~BFLAG_TEMPORARY;

		if (ms_is_from_minibuffer)
			free(ms);
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

					minibuf_error("Please answer y, n, !, . or q.");
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
				astr_assign_cstr(buf, bp->filename);
			else
				astr_assign_cstr(buf, bp->name);
			astr_append_cstr(buf, ".ZILESAVE");
			fprintf(stderr, "Saving %s...\r\n",
				astr_cstr(buf));
			raw_write_to_disk(bp, astr_cstr(buf));
			astr_delete(buf);
		}
	exit(exitcode);
}

DEFUN("cd", cd)
/*+
Make the user specified directory become the current buffer's default
directory.
+*/
{
	char *ms;
	astr buf;
	struct stat st;

	buf = astr_new();
	get_current_dir(buf, TRUE);
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
