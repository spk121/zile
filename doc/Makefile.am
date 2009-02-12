# Documentation Makefile.am
#
# Copyright (c) 2008, 2009 Free Software Foundation, Inc.
# Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
# Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Reuben Thomas.
# Copyright (c) 2004 David Capello.
#
# This file is part of GNU Zile.
#
# GNU Zile is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Zile is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Zile; see the file COPYING.  If not, write to the
# Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
# MA 02111-1301, USA.

info_TEXINFOS = zile.texi

ziledir = $(datadir)/zile
dist_zile_DATA =				\
	FAQ					\
	dotzile.sample

EXTRA_DIST =					\
	dotzile.texi				\
	zlint

dist_man_MANS = zile.1

$(srcdir)/zile.info: dotzile.texi

BUILT_TEXI = vars.texi

$(PACKAGE).info $(PACKAGE).dvi: $(BUILT_TEXI)

dotzile.texi: dotzile.sample
	@echo "@c Automatically generated file: DO NOT EDIT!" >$@
	@echo "@setfilename dotzile.info" >>$@
	@echo "@example" >>$@
	expand $? >>$@
	@echo "@end example" >>$@

LUA_BUILDTIME = mkvars.lua

LUA_RUNTIME =				\
		lib.lua			\
		texinfo.lua		\
		std.lua

dotzile.sample vars.texi: $(top_srcdir)/src/tbl_vars.h mkvars.lua $(LUA_RUNTIME)
	LUA_INIT= LUA_PATH="$(srcdir)/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' -e 'PACKAGE_NAME="$(PACKAGE_NAME)"' $(srcdir)/mkvars.lua $(top_srcdir)/src/tbl_vars.h

check-local:
	$(top_srcdir)/doc/zlint $(top_builddir)/src/tbl_funcs.h $(top_srcdir)/doc/zile.texi
