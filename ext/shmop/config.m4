dnl $Id: config.m4,v 1.1 2000/10/01 15:06:04 derick Exp $
PHP_ARG_ENABLE(shmop, whether to enable shmop support, 
[  --enable-shmop           Enable shmop support])

if test "$PHP_SHMOP" != "no"; then
  AC_DEFINE(HAVE_SHMOP, 1, [ ])
  PHP_EXTENSION(shmop, $ext_shared)
fi
