# Copyright (C) 2008 Tim Felgentreff <timfelgentreff@gmail.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AC_INIT([dwm_edje],
        [0.0.1],
        [Tim Felgentreff timfelgentreff@gmail.com],
        [dwm_edje}])
AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(config.h)
AC_CHECK_HEADERS([pthread.h stdlib.h string.h unistd.h X11/Xdefs.h])
AM_INIT_AUTOMAKE([dist-bzip2])

if test "x${prefix}" = "xNONE"; then
   PACKAGE_BIN_DIR="${ac_default_prefix}/bin"
else
   PACKAGE_BIN_DIR="${prefix}/bin"
fi
AC_SUBST(PACKAGE_BIN_DIR)

AC_PROG_CC
AC_PROG_INSTALL
#AC_AIX
AC_ISC_POSIX
#AC_MINIX
AC_STDC_HEADERS
AC_PROG_RANLIB

AC_CONFIG_FILES([
   Makefile
   README
   doc/Makefile
   m4/Makefile
   src/Makefile
   src/edje/Makefile
   src/dwm/Makefile
   src/ruby/Makefile
])

AC_OUTPUT
