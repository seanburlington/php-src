dnl
dnl $Id: config.m4,v 1.4.2.1 2002/04/11 12:53:24 derick Exp $
dnl

PHP_ARG_ENABLE(overload,whether to enable user-space object overloading support,
[  --enable-overload       EXPERIMENTAL: Enable user-space object overloading support.], no)

if test "$PHP_OVERLOAD" != "no"; then
	AC_DEFINE(HAVE_OVERLOAD, 1, [ ])
	PHP_EXTENSION(overload, $ext_shared)
fi
