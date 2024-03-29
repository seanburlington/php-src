dnl
dnl $Id: config.m4,v 1.11 2005/11/01 00:46:44 bfrance Exp $
dnl

PHP_ARG_ENABLE(ftp,whether to enable FTP support,
[  --enable-ftp            Enable FTP support])

PHP_ARG_WITH(openssl-dir,OpenSSL dir for FTP,
[  --with-openssl-dir[=DIR]  FTP: openssl install prefix], no, no)

if test "$PHP_FTP" = "yes"; then
  AC_DEFINE(HAVE_FTP,1,[Whether you want FTP support])
  PHP_NEW_EXTENSION(ftp, php_ftp.c ftp.c, $ext_shared)

  dnl Empty variable means 'no'
  test -z "$PHP_OPENSSL" && PHP_OPENSSL=no

  if test "$PHP_OPENSSL" != "no" || test "$PHP_OPENSSL_DIR" != "no"; then
    PHP_SETUP_OPENSSL(FTP_SHARED_LIBADD)
    PHP_SUBST(FTP_SHARED_LIBADD)
  fi
fi
