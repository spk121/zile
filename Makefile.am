## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.16 2005/01/03 11:06:39 rrt Exp $

SUBDIRS = doc src

EXTRA_DIST =					\
	AUTHORS					\
	COPYING					\
	INSTALL					\
	NEWS					\
	README					\
	THANKS					\
	zile.pkg

CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src

dist-epoc:
	emxsis zile.pkg
	cp README Readme.txt
	zip zile.zip zile.sis Readme.txt
	rm Readme.txt
