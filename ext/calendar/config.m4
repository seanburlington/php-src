dnl
dnl $Id: config.m4,v 1.5 2001/11/30 18:59:27 sniper Exp $
dnl

PHP_ARG_ENABLE(calendar,whether to enable calendar conversion support,
[  --enable-calendar       Enable support for calendar conversion])

if test "$PHP_CALENDAR" = "yes"; then
  AC_DEFINE(HAVE_CALENDAR,1,[ ])
  PHP_EXTENSION(calendar, $ext_shared)
fi
