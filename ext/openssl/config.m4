dnl
dnl $Id: config.m4,v 1.2 2001/11/30 18:59:48 sniper Exp $
dnl

if test "$OPENSSL_DIR"; then
  PHP_EXTENSION(openssl, $ext_shared)
  AC_DEFINE(HAVE_OPENSSL_EXT,1,[ ])
fi
