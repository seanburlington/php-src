dnl $Id: config.m4,v 1.1 1999/04/21 23:11:19 ssb Exp $

if test -n "$APACHE_INCLUDE"; then
    PHP_EXTENSION(apache)
fi
