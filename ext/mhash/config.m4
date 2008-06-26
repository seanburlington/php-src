dnl
dnl $Id: config.m4,v 1.15.4.1.2.1 2008/06/26 22:33:16 scottmac Exp $
dnl

PHP_ARG_WITH(mhash, for mhash support,
[  --with-mhash[=DIR]      Include mhash support])

if test "$PHP_MHASH" != "no"; then
  PHP_NEW_EXTENSION(mhash, mhash.c, $ext_shared)
  PHP_SUBST(MHASH_SHARED_LIBADD)
  PHP_ADD_EXTENSION_DEP(mhash, hash, true)
fi
