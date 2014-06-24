MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)\" \
-I$(top_builddir)/src/lib \
@ECORE_MQTT_CFLAGS@

bin_PROGRAMS = ecore_mqtt_sn_broker

ecore_mqtt_sn_broker_SOURCES = \
ecore_mqtt_sn_broker.c

ecore_mqtt_sn_broker_LDADD = \
@ECORE_MQTT_LIBS@ \
$(top_builddir)/src/lib/libecore_mqtt.la
