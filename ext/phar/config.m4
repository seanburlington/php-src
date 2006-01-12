dnl $Id: config.m4,v 1.6 2006/01/12 21:16:27 helly Exp $
dnl config.m4 for extension phar

PHP_ARG_ENABLE(phar, for phar support/phar zlib support,
[  --enable-phar           Enable phar support, use --with-zlib-dir if zlib detection fails])

if test "$PHP_PHAR" != "no"; then
  PHP_NEW_EXTENSION(phar, phar.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(phar, zlib, true)
  PHP_ADD_EXTENSION_DEP(phar, bz2, true)
fi
