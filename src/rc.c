/*	$Id: rc.c,v 1.1 2001/01/19 22:02:43 ssigala Exp $	*/

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

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "zile.h"
#include "extern.h"

static FILE *rc_file;
static char *rc_name;
static int lineno = 1;

static void error(char *fmt, ...)
{
	va_list ap;
	char msg1[PATH_MAX+100], msg2[50];

	sprintf(msg1, "zile:%s:%d: ", rc_name, lineno);

	va_start(ap, fmt);
	vsprintf(msg2, fmt, ap);
	va_end(ap);

	minibuf_error("%s%s", msg1, msg2);

	waitkey_discard(3 * 1000);
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
	int i;

	id[i = 0] = tolower(c);
	while (isalnum(c = fgetc(rc_file)) || c == '-')
		id[++i] = tolower(c);
	id[++i] = '\0';
	if (skip_ws(c) != '=') {
		error("syntax error");
		return;
	}
	if (isalnum(c = skip_ws(fgetc(rc_file))) || c == '-') {
		value[i = 0] = tolower(c);
		while (isalnum(c = fgetc(rc_file)) || c == '-')
			value[++i] = tolower(c);
		value[++i] = '\0';
	} else if (c == '"') {
		i = 0;
		while ((c = fgetc(rc_file)) != '"') {
			value[i++] = c;
			if (c == '\\')
				value[i++] = fgetc(rc_file);
		}
		value[i] = '\0';
	} else
		error("syntax error");
	set_variable(id, value);
	if (c == '\n')
		++lineno;
}

static void parse_rc(void)
{
	int c;

	while ((c = skip_ws(fgetc(rc_file))) != EOF)
		if (c == '#') {
			while ((c = fgetc(rc_file)) != '\n')
				;
			++lineno;
		} else if (c == '\n')
			++lineno;
		else if (isalnum(c))
			parse_id_line(c);
		else
			error("unexpected character `%c'", c);
}

void read_rc_file(void)
{
	char buf[PATH_MAX];

	strcpy(buf, getenv("HOME"));
	strcat(buf, "/.zilerc");
	rc_name = buf;

	if ((rc_file = fopen(buf, "r")) == NULL)
		return;

	parse_rc();

	fclose(rc_file);
}
