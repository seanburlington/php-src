dnl $Id: config.m4,v 1.5 2001/04/04 00:06:49 sniper Exp $
dnl config.m4 for extension sockets

PHP_ARG_ENABLE(sockets, whether to enable sockets support,
[  --enable-sockets        Enable sockets support])

if test "$PHP_SOCKETS" != "no"; then

  AC_CHECK_HEADERS(netdb.h netinet/tcp.h sys/un.h errno.h)
  AC_DEFINE(HAVE_SOCKETS, 1, [ ])

  PHP_EXTENSION(sockets, $ext_shared)
fi
