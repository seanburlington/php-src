/* 
	+----------------------------------------------------------------------+
	| PHP Version 6                                                        |
	+----------------------------------------------------------------------+
	| Copyright (c) 1997-2009 The PHP Group                                |
	+----------------------------------------------------------------------+
	| This source file is subject to version 3.01 of the PHP license,      |
	| that is bundled with this package in the file LICENSE, and is        |
	| available through the world-wide-web at the following url:           |
	| http://www.php.net/license/3_01.txt                                  |
	| If you did not receive a copy of the PHP license and are unable to   |
	| obtain it through the world-wide-web, please send a note to          |
	| license@php.net so we can mail you a copy immediately.               |
	+----------------------------------------------------------------------+
	| Authors: Andi Gutmans <andi@zend.com>                                |
	|          Zeev Suraski <zeev@zend.com>                                |
	+----------------------------------------------------------------------+
*/

/* $Id: internal_functions_win32.c,v 1.95 2009/03/10 23:39:53 helly Exp $ */

/* {{{ includes
 */
#include "php.h"
#include "php_main.h"
#include "zend_modules.h"
#include "zend_compile.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef ZEND_ENGINE_2
#error HEAD does not work with ZendEngine1 anymore
#endif

#include "ext/standard/dl.h"
#include "ext/standard/file.h"
#include "ext/standard/fsock.h"
#include "ext/standard/head.h"
#include "ext/standard/pack.h"
#include "ext/standard/php_browscap.h"
#include "ext/standard/php_crypt.h"
#include "ext/standard/php_dir.h"
#include "ext/standard/php_filestat.h"
#include "ext/standard/php_mail.h"
#include "ext/standard/php_ext_syslog.h"
#include "ext/standard/php_standard.h"
#include "ext/standard/php_lcg.h"
#include "ext/standard/php_array.h"
#include "ext/standard/php_assert.h"
#if HAVE_BCMATH
#include "ext/bcmath/php_bcmath.h"
#endif
#if HAVE_CALENDAR
#include "ext/calendar/php_calendar.h"
#endif
#if HAVE_CTYPE
#include "ext/ctype/php_ctype.h"
#endif
#if HAVE_FTP
#include "ext/ftp/php_ftp.h"
#endif
#if HAVE_PCRE || HAVE_BUNDLED_PCRE
#include "ext/pcre/php_pcre.h"
#endif
#if HAVE_UODBC
#include "ext/odbc/php_odbc.h"
#endif
#if HAVE_PHP_SESSION
#include "ext/session/php_session.h"
#endif
#if HAVE_MBSTRING
#include "ext/mbstring/mbstring.h"
#endif
#if HAVE_TOKENIZER
#include "ext/tokenizer/php_tokenizer.h"
#endif
#if HAVE_ZLIB
#include "ext/zlib/php_zlib.h"
#endif
#if HAVE_LIBXML
#include "ext/libxml/php_libxml.h"
#if HAVE_DOM
#include "ext/dom/php_dom.h"
#endif
#if HAVE_SIMPLEXML
#include "ext/simplexml/php_simplexml.h"
#endif
#endif
#if HAVE_XML
#include "ext/xml/php_xml.h"
#endif
#if HAVE_XML && HAVE_WDDX
#include "ext/wddx/php_wddx.h"
#endif
#ifdef HAVE_SQLITE
#include "ext/sqlite/php_sqlite.h"
#endif
#include "ext/com_dotnet/php_com_dotnet.h"
/* }}} */

/* {{{ php_builtin_extensions[]
 */
static zend_module_entry *php_builtin_extensions[] = {
	phpext_standard_ptr
#if HAVE_BCMATH
	,phpext_bcmath_ptr
#endif
#if HAVE_CALENDAR
	,phpext_calendar_ptr
#endif
	,phpext_com_dotnet_ptr
#if HAVE_CTYPE
	,phpext_ctype_ptr
#endif
#if HAVE_FTP
	,phpext_ftp_ptr
#endif
#if HAVE_MBSTRING
	,phpext_mbstring_ptr
#endif
#if HAVE_UODBC
	,phpext_odbc_ptr
#endif
#if HAVE_PCRE || HAVE_BUNDLED_PCRE
	,phpext_pcre_ptr
#endif
#if HAVE_PHP_SESSION
	,phpext_session_ptr
#endif
#if HAVE_TOKENIZER
	,phpext_tokenizer_ptr
#endif
#if HAVE_ZLIB
	,phpext_zlib_ptr
#endif
#if HAVE_LIBXML
	,phpext_libxml_ptr
#if HAVE_DOM
	,phpext_dom_ptr
#endif
#if HAVE_SIMPLEXML
	,phpext_simplexml_ptr
#endif
#endif
#if HAVE_XML
	,phpext_xml_ptr
#endif
#if HAVE_XML && HAVE_WDDX
	,phpext_wddx_ptr
#endif
#if HAVE_SQLITE
	,phpext_sqlite_ptr
#endif
};
/* }}} */

#define EXTCOUNT (sizeof(php_builtin_extensions)/sizeof(zend_module_entry *))
	
PHPAPI int php_register_internal_extensions(TSRMLS_D)
{
	return php_register_extensions(php_builtin_extensions, EXTCOUNT TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
