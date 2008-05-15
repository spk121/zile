#! /bin/sh

touch ChangeLog
gnulib-tool --update
autoreconf -i
