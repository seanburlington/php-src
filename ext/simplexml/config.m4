dnl $Id: config.m4,v 1.1 2003/05/18 20:33:26 sterling Exp $
dnl config.m4 for extension simplexml

PHP_ARG_WITH(simplexml, for simplexml support,
[  --with-simplexml             Include simplexml support])

if test "$PHP_SIMPLEXML" != "no"; then
  PHP_NEW_EXTENSION(simplexml, simplexml.c, $ext_shared)
fi
