dnl $Id: config.m4,v 1.2 1999/12/30 04:07:22 sas Exp $

AC_MSG_CHECKING(whether to include the bundled filePro support)
AC_ARG_WITH(filepro,
[  --with-filepro          Include the bundled read-only filePro support],[
  if test "$withval" != "no"; then
    AC_DEFINE(HAVE_FILEPRO, 1, [ ])
    AC_MSG_RESULT(yes)
    PHP_EXTENSION(filepro)
  else
    AC_DEFINE(HAVE_FILEPRO, 0, [ ])
    AC_MSG_RESULT(no)
  fi
],[
  AC_DEFINE(HAVE_FILEPRO, 0, [ ])
  AC_MSG_RESULT(no)
]) 
