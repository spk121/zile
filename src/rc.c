/*	$Id: rc.c,v 1.8 2004/02/08 04:39:26 dacap Exp $	*/

/*
 * Copyright (c) 1997-2003 Sandro Sigala.  All rights reserved.
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

#include <ctype.h>
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "zile.h"
#include "extern.h"

static FILE *rc_file;
static const char *rc_name;
static int lineno = 1;

static void error(char *fmt, ...)
{
	va_list ap;
	astr msg1;
	char msg2[50];

	msg1 = astr_new();
	astr_fmt(msg1, "zile:%s:%d: ", rc_name, lineno);

	va_start(ap, fmt);
	vsprintf(msg2, fmt, ap);
	va_end(ap);

	minibuf_error("%s%s", astr_cstr(msg1), msg2);

	waitkey_discard(3 * 1000);
	astr_delete(msg1);
}

static int skip_ws(int c)
{
	while (c == ' ' || c == '\t' || c == '\v' || c == '\f')
		c = fgetc(rc_file); /* eat whitespace */
	return c;
}

static void parse_id_line(int c)
{
	char id[50], value[50];
	int i, c2;

	/* Read variable name.  */
	id[i = 0] = tolower(c);
	while (isalnum(c = fgetc(rc_file)) || c == '-') {
		/* Prevent overflow.  */
		if (i < sizeof(id)-2)
			id[++i] = tolower(c);
	}
	id[++i] = '\0';

	/* Search the '=' character.  */
	if (skip_ws(c) != '=') {
		error("syntax error");
		return;
	}

	/* Read value without quotes.  */
	if (isalnum(c = skip_ws(fgetc(rc_file))) || c == '-') {
		value[i = 0] = tolower(c);
		while (isalnum(c = fgetc(rc_file)) || c == '-') {
			/* Prevent overflow.  */
			if (i < sizeof(value)-2)
				value[++i] = tolower(c);
		}
		value[++i] = '\0';
	}
	/* Quoted value.  */
	else if (c == '"') {
		i = 0;
		while ((c = fgetc(rc_file)) != '"' && c != EOF) {
			/* Prevent overflow.  */
			if (i < sizeof(value)-2) { /* prevent "\\\0" string */
				value[i++] = c;
				if (c == '\\') {
					c2 = fgetc(rc_file);
					/* Prevent overflow.  */
					if (i < sizeof(value)-1)
						value[i++] = c2;
				}
			}
		}
		value[i] = '\0';
	} else
		error("syntax error");

#if 0
	fprintf(stderr, "'%s' = '%s'\n", id, value);
#endif

	/* Change variable's value.  */
	set_variable(id, value); 

	if (c == '\n')
		++lineno;
}

static void parse_rc(void)
{
	int c;

	while ((c = skip_ws(fgetc(rc_file))) != EOF)
		if (c == '#') {
			while ((c = fgetc(rc_file)) != '\n' && c != EOF)
				;
			++lineno;
		} else if (c == '\n')
			++lineno;
		else if (isalnum(c))
			parse_id_line(c);
		else
			error("unexpected character `%c'", c);
}

void read_rc_file(const char *filename)
{
	astr buf;

	buf = astr_new();
	rc_file = NULL;

	if (filename != NULL) {
		rc_name = (char *)filename;
		if ((rc_file = fopen(filename, "r")) == NULL) {
			minibuf_error("Cannot open configuration file %s", filename);
			waitkey_discard(3 * 1000);
		}
	}

	if (rc_file == NULL) {
		astr_assign_cstr(buf, getenv("HOME"));
		astr_append_cstr(buf, "/.zilerc");
		rc_name = astr_cstr(buf);
		rc_file = fopen(rc_name, "r");
	}

	if (rc_file != NULL) {
		parse_rc();
		fclose(rc_file);
	}

	astr_delete(buf);
}
