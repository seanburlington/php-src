dnl
dnl $Id: config.m4,v 1.5 2003/02/27 17:43:36 wez Exp $
dnl

if test "$PHP_OPENSSL" != "no"; then
  PHP_NEW_EXTENSION(openssl, openssl.c xp_ssl.c, $ext_openssl_shared)
  OPENSSL_SHARED_LIBADD="-lcrypto -lssl"
  PHP_SUBST(OPENSSL_SHARED_LIBADD)
  AC_DEFINE(HAVE_OPENSSL_EXT,1,[ ])
fi
