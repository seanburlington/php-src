dnl
dnl $Id: config.m4,v 1.3 2002/03/12 16:26:12 sas Exp $
dnl

if test "$OPENSSL_DIR"; then
  PHP_NEW_EXTENSION(openssl, openssl.c, $ext_shared)
  AC_DEFINE(HAVE_OPENSSL_EXT,1,[ ])
fi
