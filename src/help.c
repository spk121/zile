/*	$Id: help.c,v 1.1 2001/01/19 22:02:23 ssigala Exp $	*/

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

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "zile.h"
#include "extern.h"
#include "paths.h"
#include "version.h"

DEFUN("zile-version", zile_version)
/*+
Show the zile version.
+*/
{
	minibuf_write("Zile " ZILE_VERSION " of " CONFIGURE_DATE " on " CONFIGURE_HOST);

	return TRUE;
}

DEFUN("toggle-minihelp-window", toggle_minihelp_window)
/*+
Toggle the mini help window.
+*/
{
	char *bname = "*Mini Help*";
	bufferp bp;
	windowp wp;

	if ((wp = find_window(bname)) != NULL) {
		cur_wp = wp;
		cur_bp = wp->bp;
		FUNCALL(delete_window);
	} else {
		if (!exist_file(PATH_DATA "/HELPWIN")) {
			minibuf_error("%FCUnable to read file %FY`%s'%E",
				      PATH_DATA "/HELPWIN");
			return FALSE;
		}

		FUNCALL(delete_other_windows);
		FUNCALL(split_window);
		cur_wp = head_wp;
		if ((bp = find_buffer(bname, FALSE)) == NULL) {
			bp = find_buffer(bname, TRUE);
			bp->flags = BFLAG_NOUNDO | BFLAG_READONLY
				| BFLAG_NOSAVE | BFLAG_NEEDNAME
				| BFLAG_TEMPORARY | BFLAG_MODIFIED
				| BFLAG_NOEOB;
			set_temporary_buffer(bp);
			switch_to_buffer(bp);
			if (lookup_bool_variable("alternative-bindings"))
				read_from_disk(PATH_DATA "/HELPWINALT");
			else
				read_from_disk(PATH_DATA "/HELPWIN");
		} else {
			switch_to_buffer(bp);
			gotobob();
		}
		cur_wp = head_wp->next;
		cur_bp = head_wp->next->bp;
		while (head_wp->eheight > head_wp->bp->num_lines + 1)
			FUNCALL(enlarge_window);
	}

	return TRUE;
}

static int show_file(char *filename)
{
	if (!exist_file(filename)) {
		minibuf_error("%FCUnable to read file %FY`%s'%E", filename);
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
		char buf[PATH_MAX];

		cur_bp->flags = 0;
		strcpy(buf, getenv("HOME"));
		strcat(buf, "/TUTORIAL");
		set_buffer_filename(cur_bp, buf);

		return TRUE;
	}

	return FALSE;
}

/*
 * Fetch the documentation of a function or variable from the
 * AUTODOC automatically generated file.
 */
static char *get_funcvar_doc(char *name, char *defval, int isfunc)
{
	FILE *f;
	char buf[1024], match[128], *doc;
	size_t size, maxsize;
	int reading_doc = 0;

	if ((f = fopen(PATH_DATA "/AUTODOC", "r")) == NULL) {
		minibuf_error("%FCUnable to read file %FY`%s'%E",
			      PATH_DATA "/AUTODOC");
		return NULL;
	}

	if (isfunc)
		sprintf(match, "\fF_%s\n", name);
	else
		sprintf(match, "\fV_%s\n", name);
	size = 1;
	maxsize = 1024;
	doc = (char *)xmalloc(maxsize);
	*doc = '\0';
	if (!isfunc)
		*defval = '\0';

	while (fgets(buf, 1024, f) != NULL) {
		if (reading_doc) {
			if (buf[0] == '\f')
				break;
			if (isfunc || *defval != '\0') {
				size += strlen(buf);
				if (size > maxsize) {
					maxsize = size + 256;
					doc = (char *)xrealloc(doc, maxsize);
				}
				strcat(doc, buf);
			} else {
				strcpy(defval, buf);
				defval[strlen(defval) - 1] = '\0';
			}
		} else if (!strcmp(buf, match))
			reading_doc = 1;
	}

	fclose(f);

	if (!reading_doc) {
		minibuf_error("%FCCannot find documentation for %FY`%s'%E", name);
		free(doc);
		return NULL;
	}
		
	return doc;
}

static void write_function_description(void *p1, void *p2, void *p3, void *p4)
{
	char *name = (char *)p1, *doc = (char *)p2;

	bprintf("Function: %s\n\n"
		"Documentation:\n%s",
		name, doc);
}

DEFUN("describe-function", describe_function)
/*+
Display the full documentation of a function.
+*/
{
	char bufname[128], *name, *doc;

	name = minibuf_read_function_name("Describe function: ");
	if (name == NULL)
		return FALSE;

	if ((doc = get_funcvar_doc(name, NULL, TRUE)) == NULL)
		return FALSE;

	sprintf(bufname, "*Help: function `%s'*", name);
	write_to_temporary_buffer(bufname, write_function_description,
				  name, doc, NULL, NULL);
	free(doc);

	return TRUE;
}

static void write_variable_description(void *p1, void *p2, void *p3, void *p4)
{
	char *name = (char *)p1, *defval = (char *)p2, *doc = (char *)p3;
	bprintf("Variable: %s\n\n"
		"Default value: %s\n"
		"Current value: %s\n\n"
		"Documentation:\n%s",
		name, defval, get_variable(name), doc);
}

DEFUN("describe-variable", describe_variable)
/*+
Display the full documentation of a variable.
+*/
{
	char bufname[128], defval[128], *name, *doc;

	name = minibuf_read_variable_name("Describe variable: ");
	if (name == NULL)
		return FALSE;

	if ((doc = get_funcvar_doc(name, defval, FALSE)) == NULL)
		return FALSE;

	sprintf(bufname, "*Help: variable `%s'*", name);
	write_to_temporary_buffer(bufname, write_variable_description,
				  name, defval, doc, NULL);
	free(doc);

	return TRUE;
}

DEFUN("describe-key", describe_key)
/*+
Display documentation of the function invoked by a key sequence.
+*/
{
	char bufname[128], *name, *doc;

	minibuf_write("Describe key:");
	if ((name = get_function_by_key_sequence()) == NULL) {
		minibuf_error("%FCKey sequence is undefined%E");
		return FALSE;
	}

	minibuf_write("Key sequence is bound to %FY`%s'%E", name);

	if ((doc = get_funcvar_doc(name, NULL, TRUE)) == NULL)
		return FALSE;

	sprintf(bufname, "*Help: function `%s'*", name);
	write_to_temporary_buffer(bufname, write_function_description,
				  name, doc, NULL, NULL);
	free(doc);

	return TRUE;
}
