dnl $Id: config.m4,v 1.1 1999/04/21 23:28:00 ssb Exp $

AC_MSG_CHECKING(whether to enable bc style precision math functions)
AC_ARG_ENABLE(bcmath,
[  --disable-bcmath        Compile without bc style precision math functions. ],
[
  if test "$enableval" = "yes"; then
    AC_DEFINE(WITH_BCMATH, 1)
    AC_MSG_RESULT(yes)
    PHP_EXTENSION(bcmath)
  else
    AC_DEFINE(WITH_BCMATH, 0)
    AC_MSG_RESULT(no)
  fi
],[
  AC_DEFINE(WITH_BCMATH, 1)
  AC_MSG_RESULT(yes)
  PHP_EXTENSION(bcmath)
]) 
