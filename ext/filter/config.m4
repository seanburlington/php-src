dnl $Id: config.m4,v 1.1 2005/02/23 22:41:43 rasmus Exp $
dnl config.m4 for input filtering extension

PHP_ARG_ENABLE(filter, whether to enable input filter support,
[  --enable-filter           Enable input filter support])

if test "$PHP_FILTER" != "no"; then
  PHP_SUBST(FILTER_SHARED_LIBADD)
  PHP_NEW_EXTENSION(filter, filter.c, $ext_shared)
  CPPFLAGS="$CPPFLAGS -Wall"
fi
