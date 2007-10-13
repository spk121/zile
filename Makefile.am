## Process this file with automake to produce Makefile.in

SUBDIRS = doc src

EXTRA_DIST =					\
	AUTHORS					\
	COPYING					\
	INSTALL					\
	NEWS					\
	README					\
	THANKS

# FIXME: Do cvs tag rel-$VERSION (need to s/\./-/)
release: distcheck
	@read -p "User: " USER && \
	read -p "Password: " PASSWORD && \
	read "Release focus:\n\
  Initial freshmeat announcement  1\n\
  Documentation                   2\n\
  Code cleanup                    3\n\
  Minor feature enhancements      4\n\
  Major feature enhancements      5\n\
  Minor bug fixes                 6\n\
  Major bug fixes                 7\n\
  Minor security fixes            8\n\
  Major security fixes            9" FOCUS && \
	$VISUAL release.notes && \
	copher -u $USER -P $PASSWORD -p $PACKAGE -k $PACKAGE -r $VERSION -N release.notes $PACKAGE-$VERSION.tar.gz && \
	fm-submit --no-stdin -p $PACKAGE -v $VERSION -c "`cat release-notes`" -r $FOCUS
