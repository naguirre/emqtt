MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" \
@EMQTT_CFLAGS@

includes_HEADERS = EMqtt.h
includesdir = $(includedir)/emqtt

lib_LTLIBRARIES = libemqtt.la

libemqtt_la_SOURCES = \
EMqtt.h \
emqtt_main.c

libemqtt_la_LIBADD = @EMQTT_LIBS@

libemqtt_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@
