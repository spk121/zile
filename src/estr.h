/* Dynamically allocated encoded strings

   Copyright (c) 2011-2012 Free Software Foundation, Inc.

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

/* String with encoding */
typedef struct estr estr;  /* FIXME: Should really be opaque. */
struct estr
{
  astr as;			/* String. */
  const char *eol;		/* EOL type. */
};

extern estr estr_empty;

extern const char *coding_eol_lf;
extern const char *coding_eol_crlf;
extern const char *coding_eol_cr;

void estr_init (void);

/*
 * Make estr from astr, determining EOL type from astr's contents.
 */
estr estr_new_astr (castr as);

_GL_ATTRIBUTE_PURE size_t estr_prev_line (estr es, size_t o);
_GL_ATTRIBUTE_PURE size_t estr_next_line (estr es, size_t o);
_GL_ATTRIBUTE_PURE size_t estr_start_of_line (estr es, size_t o);
_GL_ATTRIBUTE_PURE size_t estr_end_of_line (estr es, size_t o);
_GL_ATTRIBUTE_PURE size_t estr_line_len (estr es, size_t o);
_GL_ATTRIBUTE_PURE size_t estr_lines (estr es);
estr estr_replace_estr (estr es, size_t pos, estr src);
estr estr_cat (estr es, estr src);

#define estr_len(es, eol_type) astr_len (es.as) +  estr_lines (es) * (strlen (eol_type) - strlen (es.eol))

/*
 * Read file contents into an estr.
 * The `as' member is NULL if the file doesn't exist, or other error.
 */
estr estr_readf (const char *filename);
