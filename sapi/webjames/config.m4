dnl
dnl $Id: config.m4,v 1.5.22.1 2007/06/03 20:27:16 sniper Exp $
dnl

AC_ARG_WITH(webjames,
[  --with-webjames=SRCDIR  Build PHP as a WebJames module (RISC OS only)],[
  PHP_WEBJAMES=$withval
],[
  PHP_WEBJAMES=no
])

AC_MSG_CHECKING(for webjames)
if test "$PHP_WEBJAMES" != "no"; then
  PHP_EXPAND_PATH($PHP_WEBJAMES, PHP_WEBJAMES)
  INSTALL_IT="\
    echo 'PHP_LIBS = -l$abs_srcdir/$SAPI_STATIC \$(PHP_LIBS) \$(EXTRA_LIBS)' > $PHP_WEBJAMES/build/php; \
    echo 'PHP_LDFLAGS = \$(NATIVE_RPATHS) \$(PHP_LDFLAGS)' >> $PHP_WEBJAMES/build/php; \
    echo 'PHP_CFLAGS = -DPHP \$(COMMON_FLAGS) \$(EXTRA_CFLAGS) -I$abs_srcdir/sapi/webjames' >> $PHP_WEBJAMES/build/php;"
  PHP_ADD_INCLUDE($PHP_WEBJAMES)
  PHP_SELECT_SAPI(webjames, static, webjames.c)
  AC_MSG_RESULT([yes, using $PHP_WEBJAMES])
else
  AC_MSG_RESULT(no)
fi
