#! /bin/sh

aclocal
autoheader
automake -a --foreign
autoconf
rm -f config.cache
