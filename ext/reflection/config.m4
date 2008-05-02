dnl $Id: config.m4,v 1.4.2.3.2.1.2.2 2008/05/02 13:11:45 lstrojny Exp $
dnl config.m4 for extension reflection

PHP_NEW_EXTENSION(reflection, php_reflection.c, no)
AC_DEFINE(HAVE_REFLECTION, 1)
