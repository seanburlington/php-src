dnl
dnl $Id: config.m4,v 1.7 2001/11/30 18:59:58 sniper Exp $
dnl

PHP_ARG_ENABLE(sockets, whether to enable sockets support,
[  --enable-sockets        Enable sockets support])

if test "$PHP_SOCKETS" != "no"; then

  AC_CHECK_FUNCS(hstrerror)
  AC_CHECK_HEADERS(netdb.h netinet/tcp.h sys/un.h errno.h)
  AC_DEFINE(HAVE_SOCKETS, 1, [ ])

  PHP_EXTENSION(sockets, $ext_shared)
fi
