dnl
dnl $Id: config.m4,v 1.9 2002/05/12 07:22:47 jason Exp $
dnl

PHP_ARG_ENABLE(sockets, whether to enable sockets support,
[  --enable-sockets        Enable sockets support])

if test "$PHP_SOCKETS" != "no"; then

  AC_CHECK_FUNCS(hstrerror)
  AC_CHECK_HEADERS(netdb.h netinet/tcp.h sys/un.h errno.h)
  AC_CHECK_MEMBER(struct msghdr.msg_flags, , AC_DEFINE(MISSING_MSGHDR_MSGFLAGS, 1, [ ]))
  AC_DEFINE(HAVE_SOCKETS, 1, [ ])

  PHP_NEW_EXTENSION(sockets, sockets.c, $ext_shared)
fi
