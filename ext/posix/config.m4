dnl $Id: config.m4,v 1.3 2000/05/01 21:36:52 sas Exp $
dnl config.m4 for extension posix
dnl don't forget to call PHP_EXTENSION(posix)



PHP_ARG_ENABLE(posix,whether to include POSIX-like functions,
[  --disable-posix         Disable POSIX-like functions], yes)

if test "$PHP_POSIX" = "yes"; then
  AC_DEFINE(HAVE_POSIX, 1, [whether to include POSIX-like functions])
  PHP_EXTENSION(posix, $ext_shared)
fi
