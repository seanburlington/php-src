dnl
dnl $Id: config.m4,v 1.2 2001/11/30 18:59:49 sniper Exp $
dnl

PHP_ARG_ENABLE(overload,whether to enable user-space object overloading support,
[  --enable-overload       Enable user-space object overloading support.])

if test "$PHP_OVERLOAD" != "no"; then
	AC_DEFINE(HAVE_OVERLOAD, 1, [ ])
	PHP_EXTENSION(overload, $ext_shared)
fi
