dnl $Id: config.m4,v 1.1 2001/06/12 08:54:25 jason Exp $
dnl config.m4 for extension pcntl

dnl Process Control (pcntl) extentsion --EXPERIMENTAL--
dnl TODO - Add platform checks 
PHP_ARG_ENABLE(pcntl, whether to enable pcntl support,
[  --enable-pcntl           Enable experimental pcntl support (CGI ONLY!)])

if test "$PHP_PCNTL" != "no"; then
  if test "$PHP_SAPI" != "cgi"; then
      AC_MSG_ERROR(pcntl currently only accepts the CGI SAPI, this will save you from harming your webserver.)
  else
      PHP_EXTENSION(pcntl, $ext_shared)
  fi
fi
