dnl $Id: config.m4,v 1.1 2000/05/24 10:33:18 rasmus Exp $
dnl config.m4 for extension exif

PHP_ARG_ENABLE(exif, whether to enable exif support,
dnl Make sure that the comment is aligned:
[  --enable-exif           Enable exif support])

if test "$PHP_EXIF" != "no"; then
  AC_DEFINE(PHP_EXIF, 1, [Whether you want exif support])
  PHP_EXTENSION(exif, $ext_shared)
fi
