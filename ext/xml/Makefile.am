# $Id: Makefile.am,v 1.3 1999/07/24 00:56:56 sas Exp $

phplibdir=$(libdir)/php

SRC=xml.c
INCLUDES=@INCLUDES@ -I@top_srcdir@ -I@top_srcdir@/libzend
noinst_LIBRARIES=libphpext_xml.a
phplib_LTLIBRARIES=libphpext_xml.la
libphpext_xml_a_SOURCES=$(SRC)
libphpext_xml_la_SOURCES=$(SRC)
libphpext_xml_la_LIBADD=@XML_LIBS@
EXTRA_LIBS=
