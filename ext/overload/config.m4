dnl $Id: config.m4,v 1.1 2001/10/15 20:32:56 andrei Exp $
dnl config.m4 for extension overload

PHP_ARG_ENABLE(overload,for user-space object overloading support,
[  --enable-overload       Enable user-space object overloading support])

if test "$PHP_OVERLOAD" != "no"; then
	AC_DEFINE(HAVE_OVERLOAD, 1, [ ])
	PHP_EXTENSION(overload, $ext_shared)
fi
