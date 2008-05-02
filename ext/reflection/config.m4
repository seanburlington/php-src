dnl $Id: config.m4,v 1.8 2008/05/02 13:12:02 lstrojny Exp $
dnl config.m4 for extension reflection

PHP_NEW_EXTENSION(reflection, php_reflection.c, no)
AC_DEFINE(HAVE_REFLECTION, 1)
