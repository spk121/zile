## Process this file with automake to produce Makefile.in

info_TEXINFOS = zile.texi

ziledir = $(datadir)/zile
zile_DATA =					\
	AUTODOC					\
	FAQ					\
	HELP					\
	TUTORIAL				\
	dotzile.sample

EXTRA_DIST =					\
	FAQ					\
	TUTORIAL				\
	dotzile.sample				\
	dotzile.texi				\
	zlint					\
	zile.1.in				\
	mkdoc.c					\
	config.h

man_MANS = zile.1

HELP: zile.info
	makeinfo --no-header --no-split -I$(top_srcdir)/doc $(top_srcdir)/doc/zile.texi >$@

AUTODOCSRCS = $(top_srcdir)/src/*.c
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

mkdoc$(EXEEXT_FOR_BUILD): mkdoc.o
	$(CC_FOR_BUILD) $(CFLAGS) $(LDFLAGS) -o $@ mkdoc.o

mkdoc.o: mkdoc.c config.h $(top_srcdir)/src/tbl_funcs.h $(top_srcdir)/src/tbl_vars.h \
         $(top_srcdir)/src/strrstr.c $(top_srcdir)/src/vasprintf.c $(top_srcdir)/src/zmalloc.c $(top_srcdir)/src/astr.c
	$(CC_FOR_BUILD) -c -I$(top_srcdir)/doc -I$(top_srcdir) -I$(top_srcdir)/src $(CFLAGS) $(srcdir)/mkdoc.c

AUTODOC: $(AUTODOCSRCS) mkdoc$(EXEEXT_FOR_BUILD)
	./mkdoc $(AUTODOCSRCS) >AUTODOC

zile.info: dotzile.texi

dotzile.texi: dotzile.sample
	@echo "@c Automatically generated file: NO DOT EDIT!" >$@
	@echo "@setfilename dotzile.info" >>$@
	@echo "@example" >>$@
	expand $? >>$@
	@echo "@end example" >>$@

zile.1: zile.1.in
	rm -f $@
	sed s+'@'pkgdatadir@+$(pkgdatadir)+g $? > $@

all-local: HELP AUTODOC

clean-local:
	rm -f mkdoc$(EXEEXT_FOR_BUILD) AUTODOC HELP *.o
	rm -f zile.1

check-local:
	cd $(top_srcdir)/doc && $(top_srcdir)/doc/zlint
