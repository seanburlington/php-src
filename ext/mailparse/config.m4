dnl $Id: config.m4,v 1.2 2001/09/19 09:17:54 sas Exp $
dnl config.m4 for extension mailparse

PHP_ARG_ENABLE(mailparse, whether to enable mailparse support,
[  --enable-mailparse      Enable mailparse support])

if test "$PHP_MAILPARSE" != "no"; then
  if test "$ext_shared" != "yes" && test "$enable_mbstring" != "yes"; then
	  AC_MSG_WARN(Activating mbstring)
	  enable_mbstring=yes
  fi
  PHP_EXTENSION(mailparse, $ext_shared)
fi
