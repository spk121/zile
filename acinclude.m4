dnl Copied from binutils 1.15 so probably covered by the GPL
dnl (Plus minor modifications to find non-gcc compilers)
dnl Richard Smith <richard@ex-parrot.com>
dnl
dnl Get a default for CC_FOR_BUILD to put into Makefile.
AC_DEFUN([BFD_CC_FOR_BUILD],
[# Put a plausible default for CC_FOR_BUILD in Makefile.
if test -z "$CC_FOR_BUILD"; then
  if test "x$cross_compiling" = "xno"; then
    CC_FOR_BUILD='$(CC)'
  else
    AC_CHECK_PROGS(CC_FOR_BUILD, cc gcc cl)
  fi
fi
AC_SUBST(CC_FOR_BUILD)
# Also set EXEEXT_FOR_BUILD.
if test "x$cross_compiling" = "xno"; then
  EXEEXT_FOR_BUILD='$(EXEEXT)'
else
  AC_CACHE_CHECK([for build system executable suffix], bfd_cv_build_exeext,
    [rm -f conftest*
     echo 'int main () { return 0; }' > conftest.c
     bfd_cv_build_exeext=
     ${CC_FOR_BUILD} -o conftest conftest.c 1>&5 2>&5
     for file in conftest.*; do
       case $file in
       *.c | *.o | *.obj | *.ilk | *.pdb) ;;
       *) bfd_cv_build_exeext=`echo $file | sed -e s/conftest//` ;;
       esac
     done
     rm -f conftest*
     test x"${bfd_cv_build_exeext}" = x && bfd_cv_build_exeext=no])
  EXEEXT_FOR_BUILD=""
  test x"${bfd_cv_build_exeext}" != xno && EXEEXT_FOR_BUILD=${bfd_cv_build_exeex
t}
fi
AC_SUBST(EXEEXT_FOR_BUILD)])dnl
