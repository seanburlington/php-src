dnl
dnl $Id: config.m4,v 1.9 2005/05/13 21:38:17 magnus Exp $
dnl

PHP_ARG_ENABLE(posix,whether to enable POSIX-like functions,
[  --disable-posix         Disable POSIX-like functions], yes)

if test "$PHP_POSIX" = "yes"; then
  AC_DEFINE(HAVE_POSIX, 1, [whether to include POSIX-like functions])
  PHP_NEW_EXTENSION(posix, posix.c, $ext_shared)

  AC_CHECK_FUNCS(seteuid setegid setsid getsid setpgid getpgid ctermid mkfifo mknod getrlimit getlogin getgroups)
fi
