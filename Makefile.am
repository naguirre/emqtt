AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in acconfig.h depcomp config.h.in~

bin_PROGRAMS = ecore-mqtt

ecore_mqtt_LDADD = @ECORE_MQTT_LIBS@

AM_CPPFLAGS = \
@ECORE_MQTT_CFLAGS@

ecore_mqtt_SOURCES = \
main.c


