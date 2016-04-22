lib_LTLIBRARIES = libmpower.la

libmpower_la_SOURCES = \
	cJSON.c \
	cJSON.h \
	mpower.c

LIBS = $(LIBCURL_LIBS) -lm

AM_CPPFLAGS = -I$(top_srcdir) $(LIBCURL_CFLAGS)
AM_CFLAGS = -g -Wall

libmpowerincludedir = $(includedir)/mpower
libmpowerinclude_HEADERS = $(top_srcdir)/mpower/mpower.h
