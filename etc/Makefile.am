## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.9 2004/11/15 01:01:20 rrt Exp $

noinst_PROGRAMS = mkdoc
mkdoc_SOURCES = mkdoc.c

mkdoc_LDADD = $(top_srcdir)/src/astr.o $(top_srcdir)/src/strrstr.o $(top_srcdir)/src/zmalloc.o $(top_srcdir)/src/vasprintf.o

ziledir = $(datadir)/zile
zile_DATA =					\
	AUTODOC					\
	FAQ					\
	HELP					\
	TUTORIAL				\
	zilerc.sample

EXTRA_DIST =					\
	FAQ					\
	rationale				\
	TUTORIAL				\
	zilerc.sample				\
	zlint

all-local: HELP AUTODOC

HELP: $(top_srcdir)/doc/zile.texi
	makeinfo --no-header --no-split -I$(top_srcdir)/doc $< >>$@

AUTODOCSRCS = $(top_srcdir)/src/*.c
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

AUTODOC: $(AUTODOCSRCS) mkdoc
	./mkdoc $(AUTODOCSRCS) >AUTODOC

clean-local:
	-rm -f mkdoc AUTODOC HELP *.o
