dnl
dnl $Id: config.m4,v 1.6 2001/11/30 18:59:34 sniper Exp $
dnl

PHP_ARG_ENABLE(ftp,whether to enable FTP support,
[  --enable-ftp            Enable FTP support])

if test "$PHP_FTP" = "yes"; then
  AC_DEFINE(HAVE_FTP,1,[Whether you want FTP support])
  PHP_EXTENSION(ftp, $ext_shared)
fi
