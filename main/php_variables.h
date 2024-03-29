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
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id: php_variables.h,v 1.32 2009/05/23 18:03:27 andrei Exp $ */

#ifndef PHP_VARIABLES_H
#define PHP_VARIABLES_H

#include "php.h"
#include "SAPI.h"

#define PARSE_POST 0
#define PARSE_GET 1
#define PARSE_COOKIE 2
#define PARSE_STRING 3
#define PARSE_ENV 4
#define PARSE_SERVER 5
#define PARSE_SESSION 6

BEGIN_EXTERN_C()
void php_treat_data(int arg, char *str, zval* destArray TSRMLS_DC);
void php_startup_auto_globals(TSRMLS_D);
extern PHPAPI void (*php_import_environment_variables)(zval *array_ptr TSRMLS_DC);
PHPAPI void php_register_variable(char *var, char *val, zval *track_vars_array TSRMLS_DC);
/* binary-safe version */
PHPAPI void php_register_variable_safe(zend_uchar type, zstr var, zstr strval, int str_len, zval *track_vars_array TSRMLS_DC);
PHPAPI void php_register_variable_ex(char *var, zval *val, zval *track_vars_array TSRMLS_DC);
PHPAPI void php_u_register_variable_ex(UChar *var, zval *val, zval *track_vars_array TSRMLS_DC);
PHPAPI int php_register_variable_with_conv(UConverter *conv, char *var, int var_len, char *val, int val_len, zval *array_ptr, int input_type  TSRMLS_DC);

int php_hash_environment(TSRMLS_D);
END_EXTERN_C()

#define NUM_TRACK_VARS	6

#endif /* PHP_VARIABLES_H */
