dnl $Id: config.m4,v 1.14 2007/12/18 05:32:10 cellog Exp $
dnl config.m4 for extension phar

PHP_ARG_ENABLE(phar, for phar support/phar zlib support,
[  --enable-phar           Enable phar support, use --with-zlib-dir if zlib detection fails])

if test "$PHP_PHAR" != "no"; then
  PHP_NEW_EXTENSION(phar, phar.c phar_object.c phar_path_check.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(phar, zlib, true)
  PHP_ADD_EXTENSION_DEP(phar, bz2, true)
  PHP_ADD_EXTENSION_DEP(phar, spl, true)
  PHP_ADD_EXTENSION_DEP(phar, gnupg, true)
  PHP_ADD_MAKEFILE_FRAGMENT
fi
