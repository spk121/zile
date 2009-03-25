/* Completion fields

   Copyright (c) 2009 Free Software Foundation, Inc.

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

/* Dynamically allocated string fields of Completion. */
FIELD_STR(match)		/* The match buffer. */

/* Other fields of Completion. */
FIELD(Buffer *, old_bp)		/* The buffer from which the completion was invoked. */
FIELD(gl_list_t, completions)	/* The completions list. */
FIELD(gl_list_t, matches)	/* The matches list. */
FIELD(size_t, matchsize)	/* The match buffer size. */
FIELD(int, flags)		/* Completion flags. */
FIELD(astr, path)		/* Path for a filename completion. */
