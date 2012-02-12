#!/bin/sh

for i in tests/*.el tests/interactive/*.el; do
  cat $i | sed 's/(point)//g' \
      | sed 's/(mark)//g' \
      | sed 's/other-window 1/other-window/' \
      | sed 's/other-window -1/other-window/' \
      | sed 's/\\/\\\\/g' \
      | sed 's/open-line 1/open-line/' \
      > `dirname $i`/`basename $i .el`.scm
done
