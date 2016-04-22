bin_PROGRAMS = mpowerTest

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall

mpowerTest_SOURCES = mpowerTest.c
mpowerTest_LDADD = $(top_builddir)/src/libmpower.la $(CURL_LIBS) -lm
