# Top-level Makefile.am
# Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
# Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Reuben Thomas.
# Copyright (c) 2004 David Capello.
# All rights reserved.
#
# This file is part of Zile.
#
# Zile is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2, or (at your option) any later
# version.
#
# Zile is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with Zile; see the file COPYING.  If not, write to the Free
# Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
# 02111-1301, USA.

## Process this file with automake to produce Makefile.in

SUBDIRS = doc src

EXTRA_DIST =					\
	AUTHORS					\
	COPYING					\
	INSTALL					\
	NEWS					\
	README					\
	THANKS

release: distcheck
	cvs diff && \
	cvs tag rel-`echo $(VERSION) | sed -e 's/\./-/g'` && \
	woger $(PACKAGE) $(VERSION) release-notes-$(VERSION)
