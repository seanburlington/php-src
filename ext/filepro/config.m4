dnl
dnl $Id: config.m4,v 1.6 2002/03/12 16:17:41 sas Exp $
dnl

AC_ARG_WITH(filepro,[],[enable_filepro=$withval])

PHP_ARG_ENABLE(filepro,whether to enable the bundled filePro support,
[  --enable-filepro        Enable the bundled read-only filePro support.])

if test "$PHP_FILEPRO" = "yes"; then
  AC_DEFINE(HAVE_FILEPRO, 1, [ ])
  PHP_NEW_EXTENSION(filepro, filepro.c, $ext_shared)
fi
