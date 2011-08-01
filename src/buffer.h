/* Buffer fields

   Copyright (c) 2009, 2011 Free Software Foundation, Inc.

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

/* Cope with bad definition in some system headers. */
#undef lines

/* Dynamically allocated string fields of Buffer. */
FIELD_STR(name)           /* The name of the buffer. */
FIELD_STR(filename)       /* The file being edited. */

/* Other fields of Buffer. */
FIELD(Buffer *, next)     /* Next buffer in buffer list. */
FIELD(size_t, goalc)      /* Goal column for previous/next-line commands. */
FIELD(Marker *, mark)     /* The mark. */
FIELD(Marker *, markers)  /* Markers list (updated whenever text is changed). */
FIELD(Undo *, last_undop) /* Most recent undo delta. */
FIELD(Undo *, next_undop) /* Next undo delta to apply. */
FIELD(Hash_table *, vars) /* Buffer-local variables. */
FIELD(bool, modified)     /* Modified flag. */
FIELD(bool, nosave)       /* The buffer need not be saved. */
FIELD(bool, needname)     /* On save, ask for a file name. */
FIELD(bool, temporary)    /* The buffer is a temporary buffer. */
FIELD(bool, readonly)     /* The buffer cannot be modified. */
FIELD(bool, backup)       /* The old file has already been backed up. */
FIELD(bool, noundo)       /* Do not record undo informations. */
FIELD(bool, autofill)     /* The buffer is in Auto Fill mode. */
FIELD(bool, isearch)      /* The buffer is in Isearch loop. */
FIELD(bool, mark_active)  /* The mark is active. */
FIELD(astr, dir)          /* The default directory. */
