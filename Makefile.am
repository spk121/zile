## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.12 2004/12/19 00:10:46 rrt Exp $

SUBDIRS = doc src

EXTRA_DIST =					\
	AUTHORS					\
	COPYING					\
	INSTALL					\
	NEWS					\
	README					\
	TODO					\
	THANKS					\
	zile.pkg

CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

dist-epoc:
	emxsis zile.pkg
