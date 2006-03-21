dnl
dnl $Id: config.m4,v 1.5 2006/03/21 13:56:50 derick Exp $
dnl

PHP_SUBST(UNICODE_SHARED_LIBADD)
AC_DEFINE(HAVE_UNICODE, 1, [ ])
PHP_NEW_EXTENSION(unicode, unicode.c locale.c unicode_filter.c unicode_iterators.c, $ext_shared)
