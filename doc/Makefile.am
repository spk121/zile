## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.8 2005/01/03 03:43:02 rrt Exp $

info_TEXINFOS = zile.texi

ziledir = $(datadir)/zile
zile_DATA =					\
	AUTODOC					\
	FAQ					\
	HELP					\
	TUTORIAL				\
	zilerc.sample

EXTRA_DIST =					\
	FAQ					\
	TUTORIAL				\
	zilerc.sample				\
	zlint					\
	zilerc.texi				\
	mkdoc.c					\
	config.h

HELP: $(top_srcdir)/doc/zile.texi
	makeinfo --no-header --no-split -I$(top_srcdir)/doc $< >>$@

AUTODOCSRCS = $(top_srcdir)/src/*.c
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

mkdoc$(EXEEXT_FOR_BUILD): mkdoc.o config.h
	$(CC_FOR_BUILD) $(CFLAGS) $(LDFLAGS) -o $@ mkdoc.o

mkdoc.o: mkdoc.c
	$(CC_FOR_BUILD) -c -I. -I.. -I../src $(CFLAGS) $(srcdir)/mkdoc.c

AUTODOC: $(AUTODOCSRCS) mkdoc
	./mkdoc $(AUTODOCSRCS) >AUTODOC

zile.info: zilerc.texi

zilerc.texi: zilerc.sample
	@echo "@c Automatically generated file: NO DOT EDIT!" >$@
	@echo "@setfilename zilerc.info" >>$@
	@echo "@example" >>$@
	expand $< >>$@
	@echo "@end example" >>$@

all-local: HELP AUTODOC

clean-local:
	-rm -f mkdoc$(EXEEXT_FOR_BUILD) AUTODOC HELP *.o
