## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.6 2004/02/14 22:19:03 ssigala Exp $

SUBDIRS = term_ncurses
INCLUDES = -I$(top_srcdir)/lib

if ENABLE_C_MODE
fontlock_c = fontlock_c.o
endif

if ENABLE_CPP_MODE
fontlock_cpp = fontlock_cpp.o
endif

if ENABLE_CSHARP_MODE
fontlock_csharp = fontlock_csharp.o
endif

if ENABLE_JAVA_MODE
fontlock_java = fontlock_java.o
endif

bin_PROGRAMS = zile
zile_SOURCES =					\
	basic.c					\
	bind.c					\
	buffer.c				\
	completion.c				\
	editfns.c				\
	editfns.h				\
	extern.h				\
	file.c					\
	fontlock.c				\
	funcs.c					\
	glue.c					\
	help.c					\
	history.c				\
	keys.c					\
	killring.c				\
	line.c					\
	lua.c					\
	macro.c					\
	main.c					\
	marker.c				\
	minibuf.c				\
	point.c					\
	rc.c					\
	redisplay.c				\
	registers.c				\
	search.c				\
	tbl_funcs.h				\
	tbl_vars.h				\
	undo.c					\
	variables.c				\
	window.c				\
	zile.h

zile_LDADD =					\
	$(top_srcdir)/lib/libzile.a		\
	term_ncurses/libterm_ncurses.a		\
	$(fontlock_c)				\
	$(fontlock_cpp)				\
	$(fontlock_csharp)			\
	$(fontlock_java)

man_MANS = zile.1

EXTRA_DIST =					\
	fontlock_c.gperf			\
	fontlock_cpp.gperf			\
	fontlock_csharp.gperf			\
	fontlock_java.gperf			\
	zile.1.in

zile.1: zile.1.in
	rm -f $@
	sed s+'@'pkgdatadir@+$(pkgdatadir)+g $< > $@

clean-local:
	rm -f zile.1

distclean-local:
	rm -f paths.h \
	      fontlock_c.c fontlock_cpp.c fontlock_csharp.c fontlock_java.c

help.o: paths.h

paths.h:
	@echo "/* Automatically generated file.  NO NOT EDIT! */" >paths.h.tmp
	@echo "" >>paths.h.tmp
	@echo '#define PATH_PREFIX "$(prefix)"' >>paths.h.tmp
	@echo '#define PATH_DATA "$(pkgdatadir)"' >>paths.h.tmp
	if test ! -f paths.h -o				\
	        ! "cmp paths.h paths.h.tmp -s" ; then	\
		mv paths.h.tmp paths.h ;		\
	fi

if ENABLE_C_MODE
fontlock_c.c: fontlock_c.gperf
	echo "#include \"config.h\"" >$@
	echo "#if ENABLE_C_MODE" >>$@
	$(GPERF) -LANSI-C -ac -N is_c_keyword $< >>$@
	echo "#endif" >>$@
endif

if ENABLE_CPP_MODE
fontlock_cpp.c: fontlock_cpp.gperf
	echo "#include \"config.h\"" >$@
	echo "#if ENABLE_CPP_MODE" >>$@
	$(GPERF) -LANSI-C -ac -k 1,2,3 -N is_cpp_keyword $< >>$@
	echo "#endif" >>$@
endif

if ENABLE_CSHARP_MODE
fontlock_csharp.c: fontlock_csharp.gperf
	echo "#include \"config.h\"" >$@
	echo "#if ENABLE_CSHARP_MODE" >>$@
	$(GPERF) -LANSI-C -ac -k 1,2,4 -N is_csharp_keyword $< >>$@
	echo "#endif" >>$@
endif

if ENABLE_JAVA_MODE
fontlock_java.c: fontlock_java.gperf
	echo "#include \"config.h\"" >$@
	echo "#if ENABLE_JAVA_MODE" >>$@
	$(GPERF) -LANSI-C -ac -k 1,2 -N is_java_keyword $< >>$@
	echo "#endif" >>$@
endif
