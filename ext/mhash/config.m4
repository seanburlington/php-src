dnl
dnl $Id: config.m4,v 1.12.4.1 2003/09/30 22:38:24 iliaa Exp $
dnl

PHP_ARG_WITH(mhash, for mhash support,
[  --with-mhash[=DIR]      Include mhash support.])

if test "$PHP_MHASH" != "no"; then
  for i in $PHP_MHASH /usr/local /usr /opt/mhash; do
    if test -f $i/include/mhash.h; then
      MHASH_DIR=$i
    fi
  done

  if test -z "$MHASH_DIR"; then
    AC_MSG_ERROR(Please reinstall libmhash - I cannot find mhash.h)
  fi
  PHP_ADD_INCLUDE($MHASH_DIR/include)
  PHP_ADD_LIBRARY_WITH_PATH(mhash, $MHASH_DIR/lib, MHASH_SHARED_LIBADD)
  PHP_SUBST(MHASH_SHARED_LIBADD)

  AC_DEFINE(HAVE_LIBMHASH,1,[ ])

  PHP_NEW_EXTENSION(mhash, mhash.c, $ext_shared)
fi
