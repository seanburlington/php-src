dnl $Id: config.m4,v 1.1 1999/04/21 23:11:20 ssb Exp $

AC_MSG_CHECKING(for Hyperwave support)
AC_ARG_WITH(hyperwave,
[  --with-hyperwave        Include Hyperwave support],
[
  if test "$withval" != "no"; then
    AC_DEFINE(HYPERWAVE,1)
    AC_MSG_RESULT(yes)
    PHP_EXTENSION(hyperwave)
  else
    AC_DEFINE(HYPERWAVE,0)
    AC_MSG_RESULT(no)
  fi
],[
  AC_DEFINE(HYPERWAVE,0)
  AC_MSG_RESULT(no)
])
