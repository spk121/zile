## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.3 2004/10/16 20:36:50 rrt Exp $

noinst_PROGRAMS = mkdoc
mkdoc_SOURCES = mkdoc.c

mkdoc_LDADD = $(top_srcdir)/lib/libzile.a

ziledir = $(datadir)/zile
zile_DATA =					\
	AUTODOC					\
	FAQ					\
	HELP					\
	MINIHELP1				\
	MINIHELP2				\
	MINIHELP3				\
	TUTORIAL				\
	zilerc.sample

EXTRA_DIST =					\
	FAQ					\
	MINIHELP1				\
	MINIHELP2				\
	MINIHELP3				\
	rationale				\
	refcard.tex				\
	TUTORIAL				\
	zilerc.sample				\
	zlint

all-local: HELP AUTODOC refcard.ps

HELP: $(top_srcdir)/doc/zile.texi
	echo "**********************************************************" >$@
	echo "Please consider reading the Texinfo version of this manual" >>$@
	echo "(from the command line type \`info zile')." >>$@
	echo "**********************************************************" >>$@
	echo "" >>$@
	makeinfo --no-header --no-split -I$(top_srcdir)/doc $< >>$@

AUTODOCSRCS = $(top_srcdir)/src/*.c
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

AUTODOC: $(AUTODOCSRCS) mkdoc
	./mkdoc $(AUTODOCSRCS) >AUTODOC

$(top_srcdir)/lib/libzile.a:
	cd $(top_srcdir)/lib && make libzile.a

refcard.ps: refcard.tex
	latex refcard
	dvips -o refcard.ps -ta4 -tlandscape refcard.dvi

clean-local:
	-rm -f mkdoc AUTODOC HELP *.o
