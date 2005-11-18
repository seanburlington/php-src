dnl $Id: config.m4,v 1.1 2005/11/18 19:51:09 pollita Exp $
dnl config.m4 for extension hash

PHP_ARG_ENABLE(hash, whether to enable hash support,
[  --enable-hash           Enable hash support])

if test "$PHP_HASH" != "no"; then
  AC_DEFINE(HAVE_HASH_EXT,1,[Have HASH Extension])
  PHP_NEW_EXTENSION(hash, hash.c hash_md.c hash_sha.c hash_ripemd.c , $ext_shared)
fi
