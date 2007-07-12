dnl
dnl $Id: config.m4,v 1.11 2007/07/12 16:08:26 jani Exp $
dnl

PHP_NEW_EXTENSION(unicode, unicode.c locale.c unicode_iterators.c collator.c property.c constants.c transform.c, no)
