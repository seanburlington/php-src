dnl
dnl $Id: config.m4,v 1.19 2008/09/07 13:13:53 lbarnaud Exp $
dnl

if test "$PHP_MHASH" != "no"; then
	PHP_NEW_EXTENSION(mhash, mhash.c, $ext_shared)
	PHP_SUBST(MHASH_SHARED_LIBADD)
	PHP_ADD_EXTENSION_DEP(mhash, hash, true)
fi
