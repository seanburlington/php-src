dnl $Id: config.m4,v 1.7 2001/04/20 19:33:22 andrei Exp $
dnl config.m4 for extension wddx

PHP_ARG_ENABLE(wddx,for WDDX support,
[  --disable-wddx           Disable WDDX support], yes)

if test "$PHP_WDDX" != "no"; then
  if test "$enable_xml" = "no"; then
    AC_MSG_WARN(Activating XML)
    enable_xml=yes
  fi
  AC_DEFINE(HAVE_WDDX, 1, [ ])
  PHP_EXTENSION(wddx, $ext_shared)
fi
