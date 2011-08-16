# Top-level Makefile.am
#
# Copyright (c) 1997-2011 Free Software Foundation, Inc.
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

CLOC = cloc --force-lang="C",h --force-lang="C",in

SUBDIRS = lib .

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

DISTCLEANFILES =
MAINTAINERCLEANFILES =

PERL_BUILDTIME =					\
	build-aux/Zile.pm				\
	build-aux/mkfuncs.pl				\
	build-aux/mkvars.pl

EXTRA_DIST = 						\
	FAQ						\
	build-aux/zile-help2man-wrapper			\
	$(PERL_BUILDTIME)

doc_DATA = AUTHORS FAQ NEWS

include doc/Makefile.am
include src/Makefile.am
include tests/Makefile.am

ChangeLog:
	git2cl > ChangeLog

PRODUCTIONSOURCES =					\
	configure.ac					\
	Makefile.am					\
	$(PERL_BUILDTIME)				\
	doc/Makefile.am					\
	src/Makefile.am					\
	$(src_zile_base_SOURCE_FILES)			\
	src/tbl_opts.h.in				\
	src/dotzile-extra.el

loc:
	cd $(srcdir) &&					\
	$(CLOC) $(PRODUCTIONSOURCES)

loc-all:
	cd $(srcdir) &&					\
	$(CLOC) $(PRODUCTIONSOURCES) $(LISP_ALL_TESTS)

release:
	agrep -d '^\* Note' $(VERSION) NEWS | tail -n +3 | head -n -1 | grep -v '^\*\*' > release-notes && \
	woger freshmeat,gnu $(PACKAGE) "$(PACKAGE_NAME)" $(VERSION) "the tiny Emacs clone" release-notes && \
	rm -f release-notes

CYCLO_SRCS = $(srcdir)/src/*.[ch]

cyclo-$(PACKAGE).html: $(CYCLO_SRCS)
	$(PMCCABE) $(CYCLO_SRCS) \
		| sort -nr \
		| $(AWK) -f $(srcdir)/build-aux/pmccabe2html \
			-v lang=html -v name="$(PACKAGE_NAME)" \
			-v vcurl="http://git.savannah.gnu.org/gitweb/?p=$(PACKAGE).git;a=blob;f=%FILENAME%;hb=HEAD" \
			-v url="http://www.gnu.org/software/$(PACKAGE)/" \
			-v css=$(srcdir)/build-aux/pmccabe.css \
			-v cut_dir=$(srcdir)/ \
			> $@-tmp
	mv $@-tmp $@
