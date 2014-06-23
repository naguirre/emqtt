6MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" \
@ECORE_MQTT_CFLAGS@

includes_HEADERS = Ecore_Mqtt.h
includesdir = $(includedir)/ecore-mqtt

lib_LTLIBRARIES = libecore_mqtt.la

libecore_mqtt_la_SOURCES = \
Ecore_Mqtt.h \
ecore_mqtt_main.c

libecore_mqtt_la_LIBADD = @ECORE_MQTT_LIBS@

libecore_mqtt_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@
