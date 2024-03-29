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
   | Author: Andrew Sitnikov <sitnikov@infonet.ee>                        |
   +----------------------------------------------------------------------+
*/

/* $Id: ftok.c,v 1.24 2009/03/10 23:39:39 helly Exp $ */

#include "php.h"
#include "ext/standard/file.h"

#include <sys/types.h>                                                                                                        

#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif

#if HAVE_FTOK
/* {{{ proto int ftok(string pathname, string proj) U
   Convert a pathname and a project identifier to a System V IPC key */
PHP_FUNCTION(ftok)
{
	zval **pp_pathname;
	char *proj, *pathname;
	int proj_len, pathname_len;
	key_t k;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Zs", &pp_pathname, &proj, &proj_len) == FAILURE ||
		php_stream_path_param_encode(pp_pathname, &pathname, &pathname_len, REPORT_ERRORS, FG(default_context)) == FAILURE) {
		return;
	}

	if (pathname_len == 0){
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Pathname is invalid");
		RETURN_LONG(-1);
	}

	if (proj_len != 1){
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Project identifier is invalid");
		RETURN_LONG(-1);
    }

	k = ftok(pathname, proj[0]);
	if (k == -1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "ftok() failed - %s", strerror(errno));
	}

	RETURN_LONG(k);
}
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
