top_srcdir   = /local/projects/php/php4
top_builddir = /local/projects/php/php4
srcdir       = /local/projects/php/php4/ext/msession
builddir     = /local/projects/php/php4/ext/msession
VPATH        = /local/projects/php/php4/ext/msession
# $Id: Makefile,v 1.1 2001/09/19 13:14:23 mlwmohawk Exp $

LTLIBRARY_NAME        = libmsession.la
LTLIBRARY_SOURCES     = msession.c
LTLIBRARY_SHARED_NAME = msession.la

LTLIBRARY_SHARED_LIBADD = $(PHOENIX_LIB)

EXTRA_INCLUDES = $(PHOENIX_INCLUDE)

include $(top_srcdir)/build/dynlib.mk
