dnl $Id: config.m4,v 1.2 1999/12/30 04:07:26 sas Exp $
dnl config.m4 for extension ftp
dnl don't forget to call PHP_EXTENSION(ftp)

AC_MSG_CHECKING(for FTP support)
AC_ARG_WITH(ftp,
[  --with-ftp              Include FTP support.],
[
  if test "$withval" != "no"; then
    AC_DEFINE(HAVE_FTP,,[ ])
    PHP_EXTENSION(ftp)
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi
],[
  AC_MSG_RESULT(no)
])
