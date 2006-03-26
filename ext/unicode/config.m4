dnl
dnl $Id: config.m4,v 1.6 2006/03/26 11:06:24 derick Exp $
dnl

PHP_SUBST(UNICODE_SHARED_LIBADD)
AC_DEFINE(HAVE_UNICODE, 1, [ ])
PHP_NEW_EXTENSION(unicode, unicode.c locale.c unicode_filter.c unicode_iterators.c collator.c, $ext_shared)
