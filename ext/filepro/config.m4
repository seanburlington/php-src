dnl $Id: config.m4,v 1.1 1999/04/21 22:02:46 ssb Exp $

AC_MSG_CHECKING(whether to include the bundled filePro support)
AC_ARG_WITH(filepro,
[  --with-filepro          Include the bundled read-only filePro support],[
  if test "$withval" != "no"; then
    AC_DEFINE(HAVE_FILEPRO, 1)
    AC_MSG_RESULT(yes)
    PHP_EXTENSION(filepro)
  else
    AC_DEFINE(HAVE_FILEPRO, 0)
    AC_MSG_RESULT(no)
  fi
],[
  AC_DEFINE(HAVE_FILEPRO, 0)
  AC_MSG_RESULT(no)
]) 
