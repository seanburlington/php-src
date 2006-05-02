dnl
dnl $Id: config.m4,v 1.8 2006/05/02 20:58:30 andrei Exp $
dnl

PHP_SUBST(UNICODE_SHARED_LIBADD)
AC_DEFINE(HAVE_UNICODE, 1, [ ])
PHP_NEW_EXTENSION(unicode, unicode.c locale.c unicode_iterators.c collator.c property.c, $ext_shared)
