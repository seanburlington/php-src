dnl
dnl $Id: config.m4,v 1.3 2001/11/30 18:59:57 sniper Exp $
dnl

PHP_ARG_ENABLE(shmop, whether to enable shmop support, 
[  --enable-shmop          Enable shmop support])

if test "$PHP_SHMOP" != "no"; then
  AC_DEFINE(HAVE_SHMOP, 1, [ ])
  PHP_EXTENSION(shmop, $ext_shared)
fi
