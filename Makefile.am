## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.14 2004/12/21 00:21:05 rrt Exp $

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

dist-epoc: zile.zip

zile.zip:
	emxsis zile.pkg
	cp README Readme.txt
	zip zile.zip zile.sis Readme.txt
	rm Readme.txt
