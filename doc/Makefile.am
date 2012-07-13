# Source Makefile.am
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

edit = sed \
	-e 's|@pkgdatadir[@]|$(pkgdatadir)|g'

MAINTAINERCLEANFILES += $(srcdir)/doc/zile.1.in doc/zile.1
DISTCLEANFILES += doc/zile.1
EXTRA_DIST += doc/man-extras doc/zile.1.in
man_MANS = doc/zile.1

doc/zile.1: $(srcdir)/doc/zile.1.in doc/Makefile.am config.status
	@test -d doc || mkdir doc
	rm -f $@ $@.tmp
	$(edit) $(abs_srcdir)/doc/zile.1.in >$@.tmp
	mv $@.tmp $@

$(srcdir)/doc/zile.1.in: doc/man-extras build-aux/zile-help2man-wrapper configure.ac $(builddir)/src/zile$(EXEEXT)
	@test -d doc || mkdir doc
## Exit gracefully if zile.1.in is not writeable, such as during distcheck!
	$(AM_V_GEN)if ( touch $@.w && rm -f $@.w; ) >/dev/null 2>&1; then \
	  $(srcdir)/build-aux/missing --run $(HELP2MAN) --output=$@ --no-info --name="Zile Is Lossy Emacs" --include $(srcdir)/doc/man-extras $(srcdir)/build-aux/zile-help2man-wrapper; \
	fi
