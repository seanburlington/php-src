dnl $Id: config.m4,v 1.1 2002/03/24 23:00:47 hholzgra Exp $
dnl config.m4 for extension mime_magic

PHP_ARG_ENABLE(mime_magic, whether to enable mime_magic support,
[  --enable-mime_magic           Enable mime_magic support])

if test "$PHP_MIME_MAGIC" != "no"; then
  dnl PHP_SUBST(MIME_MAGIC_SHARED_LIBADD)

  PHP_NEW_EXTENSION(mime_magic, mime_magic.c, $ext_shared)
fi
