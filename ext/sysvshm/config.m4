dnl $Id: config.m4,v 1.1 1999/04/22 02:48:27 ssb Exp $

AC_MSG_CHECKING(whether to enable System V shared memory support)
AC_ARG_ENABLE(sysvshm,
[  --enable-sysvshm        Enable the System V shared memory support],[
  if test "$enableval" = "yes"; then
    AC_DEFINE(HAVE_SYSVSHM, 1)
    AC_MSG_RESULT(yes)
    PHP_EXTENSION(sysvshm)
  else
    AC_DEFINE(HAVE_SYSVSHM, 0)
    AC_MSG_RESULT(no)
  fi
],[
  AC_DEFINE(HAVE_SYSVSHM, 0)
  AC_MSG_RESULT(no)
]) 
