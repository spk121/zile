## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.2 2004/01/29 03:05:19 dacap Exp $

info_TEXINFOS = zile.texi

EXTRA_DIST = zilerc.texi

zile.info: zilerc.texi

zilerc.texi: $(top_srcdir)/etc/zilerc.sample
	@echo "@c Automatically generated file: NO DOT EDIT!" >$@
	@echo "@setfilename zilerc.info" >>$@
	@echo "@example" >>$@
	expand $< >>$@
	@echo "@end example" >>$@

install-data-local:
	$(INSTALL_DATA) zilerc.texi $(DESTDIR)$(infodir)/zilerc.texi

uninstall-local:
	rm -f $(DESTDIR)$(infodir)/zilerc.texi
