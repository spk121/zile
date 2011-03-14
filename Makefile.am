# Top-level Makefile.am
#
# Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

SUBDIRS = lib src

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = BUGS

dist_pkgdata_DATA = FAQ

ChangeLog:
	git2cl > ChangeLog

release:
	agrep -d '^\* Note' $(VERSION) NEWS | tail -n +3 | head -n -1 > release-notes && \
	woger freshmeat $(PACKAGE) "$(PACKAGE_NAME)" $(VERSION) "the tiny Emacs clone" release-notes && \
	rm -f release-notes

CYCLO_SRCS = ${top_srcdir}/src/*.[ch]

cyclo-$(PACKAGE).html: $(CYCLO_SRCS)
	$(PMCCABE) $(CYCLO_SRCS) \
		| sort -nr \
		| $(AWK) -f ${top_srcdir}/build-aux/pmccabe2html \
			-v lang=html -v name="$(PACKAGE_NAME)" \
			-v vcurl="http://git.savannah.gnu.org/gitweb/?p=$(PACKAGE).git;a=blob;f=%FILENAME%;hb=HEAD" \
			-v url="http://www.gnu.org/software/$(PACKAGE)/" \
			-v css=${top_srcdir}/build-aux/pmccabe.css \
			-v cut_dir=${top_srcdir}/ \
			> $@-tmp
	mv $@-tmp $@
