## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.7 2004/11/14 21:39:48 rrt Exp $

noinst_PROGRAMS = mkdoc
mkdoc_SOURCES = mkdoc.c

mkdoc_LDADD = $(top_srcdir)/lib/libzile.a

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

$(top_srcdir)/lib/libzile.a:
	cd $(top_srcdir)/lib && make libzile.a

clean-local:
	-rm -f mkdoc AUTODOC HELP *.o *.dvi *.ps *.log *.aux
