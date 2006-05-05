dnl
dnl $Id: config.m4,v 1.9 2006/05/05 20:56:21 andrei Exp $
dnl

PHP_SUBST(UNICODE_SHARED_LIBADD)
AC_DEFINE(HAVE_UNICODE, 1, [ ])
PHP_NEW_EXTENSION(unicode, unicode.c locale.c unicode_iterators.c collator.c property.c constants.c, $ext_shared)
