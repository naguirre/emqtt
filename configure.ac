AC_INIT(ecore-mqtt, 0.1.0, aguirre.nicolas@gmail.com)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.ac)

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

version_info="0:1:0"
release_info="1"
AC_SUBST([version_info])
AC_SUBST([release_info])

AC_C_BIGENDIAN
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AC_FUNC_ALLOCA

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([ECORE_MQTT], [
   ecore >= 1.2,
   ecore-con >= 1.2
])

AC_OUTPUT([
Makefile
src/Makefile
src/lib/Makefile
])
