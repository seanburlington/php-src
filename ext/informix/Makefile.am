# $Id: Makefile.am,v 1.9 1999/12/05 23:11:37 sas Exp $

noinst_LTLIBRARIES=libphpext_informix.la
libphpext_informix_la_SOURCES=ifx.c
libphpext_informix_la_LIBADD=@IFX_LIBS@

CLEANFILES = ifx.c

ifx.c: $(srcdir)/ifx.ec
	(if test -d @INFORMIXDIR@; then \
	   esql -e @IFX_ESQL_FLAGS@ $(srcdir)/ifx.ec; \
	 else \
	   touch ifx.c; \
	 fi)

