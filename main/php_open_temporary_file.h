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
   | Author: Zeev Suraski <zeev@zend.com>                                 |
   +----------------------------------------------------------------------+
*/

/* $Id: php_open_temporary_file.h,v 1.20 2009/03/10 23:39:53 helly Exp $ */

#ifndef PHP_OPEN_TEMPORARY_FILE_H
#define PHP_OPEN_TEMPORARY_FILE_H

BEGIN_EXTERN_C()
PHPAPI FILE *php_open_temporary_file(const char *dir, const char *pfx, char **opened_path_p TSRMLS_DC);
PHPAPI int php_open_temporary_fd(const char *dir, const char *pfx, char **opened_path_p TSRMLS_DC);
PHPAPI const char *php_get_temporary_directory(void);
PHPAPI void php_shutdown_temporary_directory(void);
END_EXTERN_C()

#endif /* PHP_OPEN_TEMPORARY_FILE_H */
