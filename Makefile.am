AUTOMAKE_OPTIONS     = 1.4 foreign

SUBDIRS = src

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in acconfig.h depcomp config.h.in~

EXTRA_DIST= \
AUTHORS \
COPYING \
README \
autogen.sh \
emqtt.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = emqtt.pc
