## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.13 2004/12/20 13:55:24 rrt Exp $

SUBDIRS = doc src

EXTRA_DIST =					\
	AUTHORS					\
	COPYING					\
	INSTALL					\
	NEWS					\
	README					\
	THANKS					\
	zile.pkg

CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

dist-epoc:
	emxsis zile.pkg
