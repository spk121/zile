## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.11 2004/12/18 23:28:04 rrt Exp $

SUBDIRS = doc src

EXTRA_DIST =					\
	AUTHORS					\
	COPYING					\
	INSTALL					\
	NEWS					\
	README					\
	TODO					\
	zile.pkg

CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

dist-epoc:
	emxsis zile.pkg
