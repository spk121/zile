/* Zile configuration file reader
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

/*	$Id: rc.c,v 1.20 2005/01/16 13:07:44 rrt Exp $	*/

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
  char *msg2;

  msg1 = astr_new();
  astr_afmt(msg1, "zile:%s:%d: ", rc_name, lineno);

  va_start(ap, fmt);
  vasprintf(&msg2, fmt, ap);
  va_end(ap);

  minibuf_error("%s%s", astr_cstr(msg1), msg2);

  waitkey(WAITKEY_DEFAULT);
  astr_delete(msg1);
  free(msg2);
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
  size_t i = 0;
  int c2;

  /* Read variable name.  */
  id[i] = tolower(c);
  while (isalnum(c = fgetc(rc_file)) || c == '-') {
    /* Prevent overflow.  */
    if (i < sizeof(id) - 2)
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
      if (i < sizeof(value) - 2)
        value[++i] = tolower(c);
    }
    value[++i] = '\0';
  }
  /* Quoted value.  */
  else if (c == '"') {
    i = 0;
    while ((c = fgetc(rc_file)) != '"' && c != EOF) {
      /* Prevent overflow.  */
      if (i < sizeof(value) - 2) { /* prevent "\\\0" string */
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

#if DEBUG
  ZTRACE(("'%s' = '%s'\n", id, value));
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

void read_rc_file(void)
{
  astr buf = get_home_dir();

  rc_file = NULL;

  astr_cat_cstr(buf, "/.zilerc");
  rc_name = astr_cstr(buf);
  rc_file = fopen(rc_name, "r");
  astr_delete(buf);

  if (rc_file != NULL) {
    parse_rc();
    fclose(rc_file);
  }
}
