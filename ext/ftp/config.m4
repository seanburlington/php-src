dnl
dnl $Id: config.m4,v 1.8 2005/10/09 20:33:09 sniper Exp $
dnl

PHP_ARG_ENABLE(ftp,whether to enable FTP support,
[  --enable-ftp            Enable FTP support])

if test "$PHP_FTP" = "yes"; then
  AC_DEFINE(HAVE_FTP,1,[Whether you want FTP support])
  PHP_NEW_EXTENSION(ftp, php_ftp.c ftp.c, $ext_shared)
  PHP_SETUP_OPENSSL(FTP_SHARED_LIBADD)
  PHP_SUBST(FTP_SHARED_LIBADD)
fi
