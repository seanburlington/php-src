dnl
dnl $Id: config.m4,v 1.8.2.3 2002/03/19 01:08:15 ssb Exp $
dnl

PHP_ARG_WITH(iconv, for iconv support,
[  --with-iconv[=DIR]      Include iconv support])

if test "$PHP_ICONV" != "no"; then

  PHP_SETUP_ICONV(ICONV_SHARED_LIBADD, [
    PHP_EXTENSION(iconv, $ext_shared)
    PHP_SUBST(ICONV_SHARED_LIBADD)
  ], [
    AC_MSG_ERROR(Please reinstall the iconv library.)
  ])

fi
