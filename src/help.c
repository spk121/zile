/*	$Id: help.c,v 1.3 2003/04/24 15:36:50 rrt Exp $	*/

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

#include "config.h"

#include <assert.h>
#include <ctype.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"
#include "paths.h"
#include "version.h"
#include "astr.h"
#include "pathbuffer.h"

DEFUN("zile-version", zile_version)
/*+
Show the zile version.
+*/
{
	minibuf_write("Zile " ZILE_VERSION " of " CONFIGURE_DATE " on " CONFIGURE_HOST);

	return TRUE;
}

static int minihelp_page = 1;

/*
 * Replace each occurrence of `C-h' in buffer with `M-h'.
 */
static void fix_alternative_keys(bufferp bp)
{
	linep lp;
	char *p;
	for (lp = bp->limitp->next; lp != bp->limitp; lp = lp->next)
		for (p = lp->text; p - lp->text < lp->size - 2; ++p)
			if (p[0] == 'C' && p[1] == '-' && p[2] == 'h')
				p[0] = 'M', p += 2;
}

/*
 * Switch to the `bp' buffer and replace any contents with the current
 * Mini Help page (read from disk).
 */
static int read_minihelp_page(bufferp bp)
{
	pathbuffer_t *fname;
	int delta;

	switch_to_buffer(bp);
	zap_buffer_content();
	bp->flags = BFLAG_NOUNDO | BFLAG_READONLY | BFLAG_NOSAVE
		| BFLAG_NEEDNAME | BFLAG_TEMPORARY | BFLAG_MODIFIED
		| BFLAG_NOEOB;
	set_temporary_buffer(bp);

	fname = pathbuffer_create(0);
	pathbuffer_fput(fname, "%s%d", PATH_DATA "/MINIHELP", minihelp_page);
	if (!exist_file(pathbuffer_str(fname))) {
		minihelp_page = 1;
		pathbuffer_fput(fname, "%s%d", PATH_DATA "/MINIHELP",
				minihelp_page);
		if (!exist_file(pathbuffer_str(fname))) {
			minibuf_error("Unable to read file `@b%s@@'",
				      pathbuffer_str(fname));
			pathbuffer_free(fname);
			return FALSE;
		}
	}

	read_from_disk(pathbuffer_str(fname));
	if (lookup_bool_variable("alternative-bindings"))
		fix_alternative_keys(bp);
	gotobob();

	while ((delta = cur_wp->eheight - head_wp->bp->num_lines) < 1) {
		FUNCALL(enlarge_window);
		/* Break if cannot enlarge further. */
		if (delta == cur_wp->eheight - head_wp->bp->num_lines)
			break;
	}

	pathbuffer_free(fname);

	return TRUE;
}

DEFUN("toggle-minihelp-window", toggle_minihelp_window)
/*+
Toggle the mini help window.
+*/
{
	const char *bname = "*Mini Help*";
	bufferp bp;
	windowp wp;
	int delta;

	if ((wp = find_window(bname)) != NULL) {
		cur_wp = wp;
		cur_bp = wp->bp;
		FUNCALL(delete_window);
	} else {
		FUNCALL(delete_other_windows);
		FUNCALL(split_window);
		cur_wp = head_wp;
		if ((bp = find_buffer(bname, FALSE)) == NULL)
			bp = find_buffer(bname, TRUE);
		read_minihelp_page(bp);
		cur_wp = head_wp->next;
		cur_bp = head_wp->next->bp;
		while ((delta = head_wp->eheight - head_wp->bp->num_lines) > 1) {
			FUNCALL(enlarge_window);
			/* Break if cannot enlarge further. */
			if (delta == head_wp->eheight - head_wp->bp->num_lines)
				break;
		}
	}

	return TRUE;
}

DEFUN("rotate-minihelp-window", rotate_minihelp_window)
/*+
Show the next mini help entry.
+*/
{
	const char *bname = "*Mini Help*";

	if (find_window(bname) == NULL)
		FUNCALL(toggle_minihelp_window);
	else { /* Easy hack */
		FUNCALL(toggle_minihelp_window);
		++minihelp_page;
		FUNCALL(toggle_minihelp_window);
	}

	return TRUE;
}

static int show_file(char *filename)
{
	if (!exist_file(filename)) {
		minibuf_error("Unable to read file `@b%s@@'", filename);
		return FALSE;
	}

	open_file(filename, 0);
	cur_bp->flags = BFLAG_READONLY | BFLAG_NOSAVE | BFLAG_NEEDNAME
		| BFLAG_NOUNDO;

	return TRUE;
}

DEFUN("help", help)
/*+
Show a help window.
+*/
{
	return show_file(PATH_DATA "/HELP");
}

DEFUN("help-latest-version", help_latest_version)
/*+
Show informations about getting the latest version.
+*/
{
	return show_file(PATH_DATA "/LATEST_VERSION");
}

DEFUN("help-config-sample", help_config_sample)
/*+
Show a configuration file sample.
+*/
{
	return show_file(PATH_DATA "/zilerc.sample");
}

DEFUN("help-faq", help_faq)
/*+
Show the Zile Frequently Asked Questions (FAQ).
+*/
{
	return show_file(PATH_DATA "/FAQ");
}

DEFUN("help-tutorial", help_tutorial)
/*+
Show a tutorial window.
+*/
{
	if (show_file(PATH_DATA "/TUTORIAL")) {
		pathbuffer_t *buf;
		buf = pathbuffer_create(0);
		cur_bp->flags = 0;
		pathbuffer_put(buf, getenv("HOME"));
		pathbuffer_append(buf, "/TUTORIAL");
		set_buffer_filename(cur_bp, pathbuffer_str(buf));

		pathbuffer_free(buf);
		return TRUE;
	}

	return FALSE;
}

/*
 * Fetch the documentation of a function or variable from the
 * AUTODOC automatically generated file.
 */
static astr get_funcvar_doc(char *name, astr defval, int isfunc)
{
	FILE *f;
	astr buf, match, doc;
	int reading_doc = 0;

	buf = astr_new();
	match = astr_new();
	doc = astr_new();

	if ((f = fopen(PATH_DATA "/AUTODOC", "r")) == NULL) {
		minibuf_error("Unable to read file `@b%s@@'",
			      PATH_DATA "/AUTODOC");
		return NULL;
	}

	if (isfunc)
		astr_fmt(match, "\fF_%s", name);
	else
		astr_fmt(match, "\fV_%s", name);

	while (astr_fgets(buf, f) != NULL)
		if (reading_doc) {
			if (astr_cstr(buf)[0] == '\f')
				break;
			if (isfunc || astr_size(defval) > 0) {
				astr_append(doc, buf);
				astr_append_char(doc, '\n');
			} else
				astr_assign(defval, buf);
		} else if (astr_eq(buf, match))
			reading_doc = 1;

	fclose(f);

	astr_delete(buf);
	astr_delete(match);

	if (!reading_doc) {
		minibuf_error("Cannot find documentation for `@f%s@@'", name);
		astr_delete(doc);
		return NULL;
	}
		
	return doc;
}

static void write_function_description(va_list ap)
{
	const char *name = va_arg(ap, const char *);
	astr doc = va_arg(ap, astr);

	bprintf("Function: %s\n\n"
		"Documentation:\n%s",
		name, astr_cstr(doc));
}

DEFUN("describe-function", describe_function)
/*+
Display the full documentation of a function.
+*/
{
	char *name;
	astr bufname, doc;

	name = minibuf_read_function_name("Describe function: ");
	if (name == NULL)
		return FALSE;

	if ((doc = get_funcvar_doc(name, NULL, TRUE)) == NULL)
		return FALSE;

	bufname = astr_new();
	astr_fmt(bufname, "*Help: function `%s'*", name);
	write_temp_buffer(astr_cstr(bufname), write_function_description,
			  name, doc);
	astr_delete(bufname);
	astr_delete(doc);

	return TRUE;
}

static void write_variable_description(va_list ap)
{
	char *name = va_arg(ap, char *);
	astr defval = va_arg(ap, astr);
	astr doc = va_arg(ap, astr);
	bprintf("Variable: %s\n\n"
		"Default value: %s\n"
		"Current value: %s\n\n"
		"Documentation:\n%s",
		name, astr_cstr(defval), get_variable(name), astr_cstr(doc));
}

DEFUN("describe-variable", describe_variable)
/*+
Display the full documentation of a variable.
+*/
{
	char *name;
	astr bufname, defval, doc;

	name = minibuf_read_variable_name("Describe variable: ");
	if (name == NULL)
		return FALSE;

	defval = astr_new();
	if ((doc = get_funcvar_doc(name, defval, FALSE)) == NULL)
		return FALSE;

	bufname = astr_new();
	astr_fmt(bufname, "*Help: variable `%s'*", name);
	write_temp_buffer(astr_cstr(bufname), write_variable_description,
			  name, defval, doc);
	astr_delete(bufname);
	astr_delete(doc);
	astr_delete(defval);

	return TRUE;
}

DEFUN("describe-key", describe_key)
/*+
Display documentation of the function invoked by a key sequence.
+*/
{
	char *name;
	astr bufname, doc;

	minibuf_write("Describe key:");
	if ((name = get_function_by_key_sequence()) == NULL) {
		minibuf_error("Key sequence is undefined");
		return FALSE;
	}

	minibuf_write("Key sequence is bound to `@f%s@@'", name);

	if ((doc = get_funcvar_doc(name, NULL, TRUE)) == NULL)
		return FALSE;

	bufname = astr_new();
	astr_fmt(bufname, "*Help: function `%s'*", name);
	write_temp_buffer(astr_cstr(bufname), write_function_description,
			  name, doc);
	astr_delete(bufname);
	astr_delete(doc);

	return TRUE;
}
