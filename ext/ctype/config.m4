dnl
dnl $Id: config.m4,v 1.4 2002/03/02 15:44:08 hholzgra Exp $
dnl

PHP_ARG_ENABLE(ctype, whether to enable ctype functions,
[  --disable-ctype         Disable ctype functions], yes)

if test "$PHP_CTYPE" != "no"; then
  AC_DEFINE(HAVE_CTYPE, 1, [ ])
  PHP_EXTENSION(ctype, $ext_shared)
fi
