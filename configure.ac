AC_PREREQ([2.65])

AC_INIT(mpower-libs, 0.1, [http://www.openrobots.org/], mpower-libs)
AM_INIT_AUTOMAKE([dist-bzip2])

AC_CONFIG_MACRO_DIR([m4])
AM_CONFIG_HEADER([config.h])

AC_PROG_CC
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([LIBCURL], libcurl)

AC_OUTPUT([Makefile
	src/Makefile
	test/Makefile
	mpower-libs.pc])
