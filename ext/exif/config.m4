dnl
dnl $Id: config.m4,v 1.4 2002/03/12 16:07:39 sas Exp $
dnl

PHP_ARG_ENABLE(exif, whether to enable EXIF support,
[  --enable-exif           Enable EXIF support])

if test "$PHP_EXIF" != "no"; then
  AC_DEFINE(HAVE_EXIF, 1, [Whether you want exif support])
  PHP_NEW_EXTENSION(exif, exif.c, $ext_shared)
fi
