dnl $Id: config.m4,v 1.2 1999/09/13 17:17:50 ssb Exp $

AC_MSG_CHECKING(for MCAL support)
AC_ARG_WITH(mcal,
[  --with-mcal[=DIR]       Include MCAL support.],
[
  if test "$withval" != "no"; then
    if test "$withval" = "yes"; then
      MCAL_DIR=/usr/local
    else
      MCAL_DIR=$withval
    fi
    
    AC_ADD_INCLUDE($MCAL_DIR)
    AC_ADD_LIBRARY_WITH_PATH(mcal, $MCAL_DIR)
    AC_DEFINE(HAVE_MCAL)
    PHP_EXTENSION(mcal)
    AC_MSG_RESULT(yes)
  else
    AC_MSG_ERROR(no)
  fi
],[
  AC_MSG_RESULT(no)
])

