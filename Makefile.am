## Process this file with automake to produce Makefile.in

SUBDIRS = doc src

EXTRA_DIST =					\
	AUTHORS					\
	COPYING					\
	INSTALL					\
	NEWS					\
	README					\
	THANKS

NOTES = release-notes

release: distcheck
	cvs tag rel-`echo $(VERSION) | sed -e 's/\./-/g'` && \
	read -e -p "SourceForge login: " USER && \
	read -r -s -p "Password: " PASSWORD && \
	echo -e "Release focus:\n\
  Initial FreshMeat announcement  1\n\
  Documentation                   2\n\
  Code cleanup                    3\n\
  Minor feature enhancements      4\n\
  Major feature enhancements      5\n\
  Minor bug fixes                 6\n\
  Major bug fixes                 7\n\
  Minor security fixes            8\n\
  Major security fixes            9" && \
	read -e -p "Choice: " FOCUS && \
	$$VISUAL $(NOTES) && \
	copher -u $$USER -P $$PASSWORD -p $(PACKAGE) -k $(PACKAGE) -r $(VERSION) -N $(NOTES) $(PACKAGE)-$(VERSION).tar.gz && \
	fm-submit --no-stdin -p $(PACKAGE) -v $(VERSION) -c "`cat $(NOTES)`" -r $$FOCUS
