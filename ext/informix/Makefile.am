# $Id: Makefile.am,v 1.1 1999/04/22 00:25:55 ssb Exp $

INCLUDES=@INCLUDES@ -I@top_srcdir@ -I@top_srcdir@/libzend
noinst_LIBRARIES=libphpext_informix.a
libphpext_informix_a_SOURCES=ifx.c

ifx.c: ifx.ec
	(if test -d @INFORMIXDIR@; then \
	   esql -e @IFX_ESQL_FLAGS@ ifx.ec; \
	 else \
	   touch ifx.c; \
	 fi)
