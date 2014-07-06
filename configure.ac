AC_INIT(ecore-mqtt, 0.1.0, aguirre.nicolas@gmail.com)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.ac)
AC_CONFIG_MACRO_DIR([m4])

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

PKG_CHECK_MODULES([EMQTT], [
   ecore >= 1.2
])

### Checks for linker characteristics
lt_enable_auto_import=""
case "${host_os}" in
   mingw*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
   ;;
esac
AC_SUBST([lt_enable_auto_import])

EFL_CHECK_COMPILER_FLAGS([EMQTT], [-Wall -Wextra -Wno-unused-parameter -Wshadow -Wpointer-arith -Wno-missing-field-initializers -fvisibility=hidden -fdata-sections -ffunction-sections])

AC_OUTPUT([
emqtt.pc
Makefile
src/Makefile
src/lib/Makefile
src/bin/Makefile
])
