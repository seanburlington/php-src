# $Id: Makefile.am,v 1.3 1999/10/04 15:18:10 sas Exp $

INCLUDES=@INCLUDES@ -I@top_srcdir@ -I@top_srcdir@/libzend
noinst_LTLIBRARIES=libphpext_informix.la
libphpext_informix_la_SOURCES=ifx.c

ifx.c: ifx.ec
	(if test -d @INFORMIXDIR@; then \
	   esql -e @IFX_ESQL_FLAGS@ ifx.ec; \
	 else \
	   touch ifx.c; \
	 fi)

clean:
	-rm -f ifx.c
