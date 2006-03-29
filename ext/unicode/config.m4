dnl
dnl $Id: config.m4,v 1.7 2006/03/29 01:20:43 pollita Exp $
dnl

PHP_SUBST(UNICODE_SHARED_LIBADD)
AC_DEFINE(HAVE_UNICODE, 1, [ ])
PHP_NEW_EXTENSION(unicode, unicode.c locale.c unicode_iterators.c collator.c, $ext_shared)
