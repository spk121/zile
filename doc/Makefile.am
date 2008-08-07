# Documentation Makefile.am
#
# Copyright (c) 2008 Free Software Foundation, Inc.
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
zile_DATA =					\
	AUTODOC					\
	FAQ					\
	HELP					\
	TUTORIAL				\
	dotzile.sample

EXTRA_DIST =					\
	FAQ					\
	TUTORIAL				\
	dotzile.sample				\
	dotzile.texi				\
	zlint					\
	zile.1.in				\
	mkdoc.c

man_MANS = zile.1

HELP: zile.info
	makeinfo --no-header --no-split -I$(top_srcdir)/doc $(top_srcdir)/doc/zile.texi >$@

AUTODOCSRCS = $(top_srcdir)/src/*.c

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib -I$(top_srcdir)/src -DPATH_DATA="\"$(pkgdatadir)\""
LDADD = $(top_builddir)/lib/libgnu.a

EXTRA_PROGRAMS = mkdoc

mkdoc_SOURCES = mkdoc.c

AUTODOC: $(AUTODOCSRCS) mkdoc$(EXEEXT_FOR_BUILD)
	./mkdoc $(AUTODOCSRCS) >AUTODOC

zile.info: dotzile.texi

dotzile.texi: dotzile.sample
	@echo "@c Automatically generated file: NO DOT EDIT!" >$@
	@echo "@setfilename dotzile.info" >>$@
	@echo "@example" >>$@
	expand $? >>$@
	@echo "@end example" >>$@

zile.1: zile.1.in
	rm -f $@
	sed s+'@'pkgdatadir@+$(pkgdatadir)+g $? > $@

all-local: HELP AUTODOC

clean-local:
	rm -f mkdoc$(EXEEXT_FOR_BUILD) AUTODOC HELP *.o
	rm -f zile.1

check-local:
	cd $(top_srcdir)/doc && $(top_srcdir)/doc/zlint
