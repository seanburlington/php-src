dnl
dnl $Id: config.m4,v 1.8 2002/03/12 16:38:59 sas Exp $
dnl

PHP_ARG_ENABLE(yp,whether to include YP support,
[  --enable-yp             Include YP support.])

if test "$PHP_YP" != "no"; then
  AC_DEFINE(HAVE_YP,1,[ ])
  PHP_NEW_EXTENSION(yp, yp.c, $ext_shared)
  case $host_alias in
  *solaris*)
    AC_DEFINE(SOLARIS_YP,1,[ ]) ;;
  esac
fi
