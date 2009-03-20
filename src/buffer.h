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

/* type, name */

BUFFER_GETTER_AND_SETTER(bool, modified)
BUFFER_GETTER_AND_SETTER(bool, nosave)
BUFFER_GETTER_AND_SETTER(bool, needname)
BUFFER_GETTER_AND_SETTER(bool, temporary)
BUFFER_GETTER_AND_SETTER(bool, readonly)
BUFFER_GETTER_AND_SETTER(bool, overwrite)
BUFFER_GETTER_AND_SETTER(bool, backup)
BUFFER_GETTER_AND_SETTER(bool, noundo)
BUFFER_GETTER_AND_SETTER(bool, autofill)
BUFFER_GETTER_AND_SETTER(bool, isearch)
BUFFER_GETTER_AND_SETTER(bool, mark_active)
