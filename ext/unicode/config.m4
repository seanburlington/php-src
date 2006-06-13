dnl
dnl $Id: config.m4,v 1.10 2006/06/13 23:46:04 andrei Exp $
dnl

PHP_SUBST(UNICODE_SHARED_LIBADD)
AC_DEFINE(HAVE_UNICODE, 1, [ ])
PHP_NEW_EXTENSION(unicode, unicode.c locale.c unicode_iterators.c collator.c property.c constants.c transform.c, $ext_shared)
