dnl
dnl $Id: config.m4,v 1.2 2001/11/30 18:59:32 sniper Exp $
dnl 

PHP_ARG_ENABLE(dio, whether to enable direct I/O support,
[  --enable-dio            Enable direct I/O support])

if test "$PHP_DIO" != "no"; then
  PHP_EXTENSION(dio, $ext_shared)
fi
