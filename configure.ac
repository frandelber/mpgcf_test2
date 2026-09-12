dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([mpgcf], [1.0.0], [https://github.com/frandelber/mpgcf])
AC_CONFIG_SRCDIR([src/mpgcf.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.14 foreign subdir-objects -Wall -Werror])
AM_SILENT_RULES([yes])

dnl ---- toolchain --------------------------------------------------------
AC_PROG_CC
AM_PROG_AR
LT_INIT

dnl ---- headers / types ----------------------------------------------------
AC_CHECK_HEADERS([stdlib.h string.h ctype.h])
AC_TYPE_SIZE_T

dnl ---- GMP dependency ------------------------------------------------------
AC_ARG_WITH([gmp],
  [AS_HELP_STRING([--with-gmp=DIR], [location of GMP installation])],
  [
    CPPFLAGS="-I$withval/include $CPPFLAGS"
    LDFLAGS="-L$withval/lib $LDFLAGS"
  ], [])

AC_CHECK_HEADER([gmp.h], [],
  [AC_MSG_ERROR([gmp.h not found; install libgmp-dev / gmp-devel, or pass --with-gmp=DIR])])

AC_CHECK_LIB([gmp], [__gmpq_init], [],
  [AC_MSG_ERROR([libgmp not found; install libgmp-dev / gmp-devel, or pass --with-gmp=DIR])])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  tests/Makefile
  tools/Makefile
  doc/Makefile
  mpgcf.pc
])

AC_OUTPUT

AC_MSG_NOTICE([])
AC_MSG_NOTICE([mpgcf $VERSION configured:])
AC_MSG_NOTICE([  CC       = $CC])
AC_MSG_NOTICE([  CPPFLAGS = $CPPFLAGS])
AC_MSG_NOTICE([  LDFLAGS  = $LDFLAGS])
AC_MSG_NOTICE([])
