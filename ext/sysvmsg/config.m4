dnl $Id: config.m4,v 1.1 2002/04/25 23:14:43 wez Exp $

PHP_ARG_ENABLE(sysvmsg,whether to enable System V IPC support,
[  --enable-sysvmsg           Enable sysvmsg support])

if test "$PHP_SYSVMSG" != "no"; then
  AC_DEFINE(HAVE_SYSVMSG, 1, [ ])
  PHP_NEW_EXTENSION(sysvmsg, sysvmsg.c, $ext_shared)
fi
