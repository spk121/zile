## Process this file with automake to produce Makefile.in

SUBDIRS = doc src

EXTRA_DIST =					\
	AUTHORS					\
	COPYING					\
	INSTALL					\
	NEWS					\
	README					\
	THANKS

release: distcheck
	cvs diff && \ # Stop if there are outstanding changes
	cvs tag rel-`echo $(VERSION) | sed -e 's/\./-/g'` && \
	woger $(PACKAGE) $(VERSION) release-notes.$(VERSION)
