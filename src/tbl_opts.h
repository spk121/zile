/* Table of command-line options

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

/*
 * long name, short name, argument, argument docstring, docstring
 */

/* Options which take no argument have optional_argument, so that no
   arguments are signalled as extraneous, as in Emacs. */

X("no-init-file", 'q', optional_argument, "",
  "do not load ~/." PACKAGE)
X("load", 'l', required_argument, "FILE",
  "load " PACKAGE_NAME " Lisp FILE using the load function")
X("help", 'h', optional_argument, "",
  "display this help message and exit")
X("version", 'v', optional_argument, "",
  "display version information and exit")
