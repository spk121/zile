# Top-level Makefile.am
#
# Copyright (c) 1997-2012 Free Software Foundation, Inc.
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

CLOC = cloc --force-lang="C",h --autoconf

SUBDIRS = lib .

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

DISTCLEANFILES =
MAINTAINERCLEANFILES =

PERL_BUILDTIME =					\
	build-aux/Zile.pm				\
	build-aux/mkfuncs.pl				\
	build-aux/mkvars.pl

EXTRA_DIST =						\
	FAQ						\
	build-aux/zile-help2man-wrapper			\
	$(PERL_BUILDTIME)

doc_DATA = AUTHORS FAQ NEWS

include doc/Makefile.am
include src/Makefile.am
include tests/Makefile.am

PRODUCTIONSOURCES =					\
	configure.ac					\
	Makefile.am					\
	$(PERL_BUILDTIME)				\
	doc/Makefile.am					\
	src/Makefile.am					\
	$(src_zile_base_SOURCE_FILES)			\
	src/tbl_opts.h.in

loc:
	cd $(srcdir) &&					\
	$(CLOC) $(PRODUCTIONSOURCES)

loc-all:
	cd $(srcdir) &&					\
	$(CLOC) $(PRODUCTIONSOURCES) $(LISP_ALL_TESTS) $(srcdir)/tests/run-lisp-tests.pl

prerelease:
	{ $(MAKE) -k maintainer-clean || { ./configure && $(MAKE) maintainer-clean; } } && \
	git checkout master && \
	git pull origin master && \
	git diff --exit-code && \
	git push origin master && \
	./bootstrap && ./configure && \
	$(MAKE) "DISTCHECK_CONFIGURE_FLAGS=--enable-valgrind" check distcheck

## ------------- ##
## Distribution. ##
## ------------- ##

gitlog_to_changelog	= $(srcdir)/build-aux/gitlog-to-changelog
git_log_fix		= $(srcdir)/build-aux/git-log-fix
changelog		= $(srcdir)/ChangeLog

# Changes before this date are from before Lua was used for any of Zile.
changelog_start_date = 2009-03-30
$(changelog): .FORCE
	$(AM_V_GEN)if test -d '$(srcdir)/.git'; then \
	  $(gitlog_to_changelog) --amend=$(git_log_fix) \
	      --since=$(changelog_start_date) > '$@T'; \
	  rm -f '$@'; mv '$@T' '$@'; \
	fi

dist-hook: $(changelog)

distcheck-hook: syntax-check

dorelease:
	agrep -d '^\* Note' $(VERSION) NEWS | tail -n +3 | head -n -1 | grep -v '^\*\*' > release-notes && \
	woger gnu,freecode upload=ftp.gnu.org package=$(PACKAGE) package_name="$(PACKAGE_NAME)" version=$(VERSION) description="the tiny Emacs clone" notes=release-notes email=~/announce-$(PACKAGE)-$(VERSION) dist_type="tar.gz" && \
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

## An empty target to depend on when a rule needs to always run
## whenever it is visited.
.FORCE:
