AC_INIT([PACKAGE], [1.0], [anto.cardace@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_PROG_AR

AC_CHECK_LIB([math],[floor])
AC_CHECK_HEADERS([math.h stdlib.h unistd.h string.h])
AC_CHECK_HEADERS([stdio.h errno.h string.h stdint.h signal.h time.h unistd.h alsa/soundlib.h], [compile_example=yes])

AM_CONDITIONAL([WANT_EXAMPLE], [test "$compile_example" = yes])

LT_INIT
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile include/Makefile])
AC_OUTPUT
