## Process this file with automake to produce Makefile.in

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
