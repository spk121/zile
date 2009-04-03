/* Buffer fields

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

/* Dynamically allocated string fields of Buffer. */
FIELD_STR(name)			/* The name of the buffer. */
FIELD_STR(filename)		/* The file being edited. */

/* Other fields of Buffer. */
FIELD(Buffer *, next)		/* Next buffer in buffer list. */
FIELD(char *, eol)		/* EOL string (up to 2 chars). */
FIELD(Line *, lines)		/* The lines of text. */
FIELD(size_t, last_line)	/* The number of the last line in the buffer. */
FIELD(Point, pt)		/* The point. */
FIELD(Marker *, mark)		/* The mark. */
FIELD(Marker *, markers)	/* Markers list (updated whenever text is changed). */
FIELD(Undo *, last_undop)	/* Most recent undo delta. */
FIELD(Undo *, next_undop)	/* Next undo delta to apply. */
FIELD(int, vars)                /* Buffer-local variables. */
FIELD(bool, modified)		/* Modified flag. */
FIELD(bool, nosave)		/* The buffer need not be saved. */
FIELD(bool, needname)		/* On save, ask for a file name. */
FIELD(bool, temporary)		/* The buffer is a temporary buffer. */
FIELD(bool, readonly)		/* The buffer cannot be modified. */
FIELD(bool, overwrite)		/* The buffer is in overwrite mode. */
FIELD(bool, backup)		/* The old file has already been backed up. */
FIELD(bool, noundo)		/* Do not record undo informations. */
FIELD(bool, autofill)		/* The buffer is in Auto Fill mode. */
FIELD(bool, isearch)		/* The buffer is in Isearch loop. */
FIELD(bool, mark_active)	/* The mark is active. */
