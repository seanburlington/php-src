dnl $Id: config.m4,v 1.3 2000/04/20 15:13:29 sas Exp $

PHP_ARG_ENABLE(calendar,whether to enable calendar conversion support,
[  --enable-calendar       Enable support for calendar conversion])

if test "$PHP_CALENDAR" = "yes"; then
  AC_DEFINE(HAVE_CALENDAR,1,[ ])
  PHP_EXTENSION(calendar)
fi
