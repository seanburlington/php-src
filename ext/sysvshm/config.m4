dnl
dnl $Id: config.m4,v 1.4 2001/11/30 19:00:00 sniper Exp $
dnl

PHP_ARG_ENABLE(sysvshm,whether to enable System V shared memory support,
[  --enable-sysvshm        Enable the System V shared memory support.])

if test "$PHP_SYSVSHM" != "no"; then
  AC_DEFINE(HAVE_SYSVSHM, 1, [ ])
  PHP_EXTENSION(sysvshm, $ext_shared)
fi
