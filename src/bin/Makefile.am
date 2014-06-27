MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)\" \
-I$(top_builddir)/src/lib \
@EMQTT_CFLAGS@

bin_PROGRAMS = emqtt_sn_broker emqtt_sn_sub

emqtt_sn_broker_SOURCES = \
emqtt_sn_broker.c

emqtt_sn_broker_LDADD = \
@EMQTT_LIBS@ \
$(top_builddir)/src/lib/libemqtt.la

emqtt_sn_sub_SOURCES = \
emqtt_sn_sub.c

emqtt_sn_sub_LDADD = \
@EMQTT_LIBS@ \
$(top_builddir)/src/lib/libemqtt.la
