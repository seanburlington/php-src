dnl
dnl $Id: config.m4,v 1.2 2001/11/30 18:59:32 sniper Exp $
dnl

PHP_ARG_ENABLE(dbx,whether to enable dbx support,
[  --enable-dbx            Enable dbx])

if test "$PHP_DBX" != "no"; then
  PHP_EXTENSION(dbx, $ext_shared)
fi
