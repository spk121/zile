## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.1 2004/01/23 01:45:50 dacap Exp $

info_TEXINFOS = zile.texi zilerc.texi

zile.info: zilerc.texi

zilerc.texi: $(top_srcdir)/etc/zilerc.sample
	@echo "@c Automatically generated file: NO DOT EDIT!" >$@
	@echo "@setfilename zilerc.info" >>$@
	@echo "@example" >>$@
	expand $< >>$@
	@echo "@end example" >>$@
