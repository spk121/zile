## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.3 2004/12/18 22:09:25 rrt Exp $

info_TEXINFOS = zile.texi

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
	TUTORIAL				\
	zilerc.sample				\
	zlint					\
	zilerc.texi

HELP: $(top_srcdir)/doc/zile.texi
	makeinfo --no-header --no-split -I$(top_srcdir)/doc $< >>$@

AUTODOCSRCS = $(top_srcdir)/src/*.c
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

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
	-rm -f mkdoc AUTODOC HELP *.o

install-data-local:
	$(INSTALL_DATA) zilerc.texi $(DESTDIR)$(infodir)/zilerc.texi

uninstall-local:
	rm -f $(DESTDIR)$(infodir)/zilerc.texi
