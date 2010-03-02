#! /bin/sh

touch ChangeLog

if [ -z "$GNULIB_SRCDIR" ]; then
    gnulib-tool --update
else
    $GNULIB_SRCDIR/gnulib-tool --update
fi

autoreconf -i
