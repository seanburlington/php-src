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
   | Author: Hartmut Holzgraefe <hholzgra@php.net>                        |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_ctype.h"
#include "SAPI.h"
#include "ext/standard/info.h"

#include <ctype.h>

#if HAVE_CTYPE

static PHP_MINFO_FUNCTION(ctype);

static PHP_FUNCTION(ctype_alnum);
static PHP_FUNCTION(ctype_alpha);
static PHP_FUNCTION(ctype_cntrl);
static PHP_FUNCTION(ctype_digit);
static PHP_FUNCTION(ctype_lower);
static PHP_FUNCTION(ctype_graph);
static PHP_FUNCTION(ctype_print);
static PHP_FUNCTION(ctype_punct);
static PHP_FUNCTION(ctype_space);
static PHP_FUNCTION(ctype_upper);
static PHP_FUNCTION(ctype_xdigit);

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_ctype_alnum, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ctype_alpha, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ctype_cntrl, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ctype_digit, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ctype_lower, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ctype_graph, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ctype_print, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ctype_punct, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ctype_space, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ctype_upper, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ctype_xdigit, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ ctype_functions[]
 * Every user visible function must have an entry in ctype_functions[].
 */
static const zend_function_entry ctype_functions[] = {
	PHP_FE(ctype_alnum,	arginfo_ctype_alnum)
	PHP_FE(ctype_alpha,	arginfo_ctype_alpha)
	PHP_FE(ctype_cntrl,	arginfo_ctype_cntrl)
	PHP_FE(ctype_digit,	arginfo_ctype_digit)
	PHP_FE(ctype_lower,	arginfo_ctype_lower)
	PHP_FE(ctype_graph,	arginfo_ctype_graph)
	PHP_FE(ctype_print,	arginfo_ctype_print)
	PHP_FE(ctype_punct,	arginfo_ctype_punct)
	PHP_FE(ctype_space,	arginfo_ctype_space)
	PHP_FE(ctype_upper,	arginfo_ctype_upper)
	PHP_FE(ctype_xdigit,	arginfo_ctype_xdigit)
	{NULL, NULL, NULL}	/* Must be the last line in ctype_functions[] */
};
/* }}} */

/* {{{ ctype_module_entry
 */
zend_module_entry ctype_module_entry = {
	STANDARD_MODULE_HEADER,
	"ctype",
	ctype_functions,
	NULL,
	NULL,
	NULL,
	NULL,
	PHP_MINFO(ctype),
    NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CTYPE
ZEND_GET_MODULE(ctype)
#endif

/* {{{ PHP_MINFO_FUNCTION
 */
static PHP_MINFO_FUNCTION(ctype)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "ctype functions", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ ctype
 */
#define CTYPE(iswhat) \
	zval *c, tmp; \
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &c) == FAILURE) \
		return; \
 	switch (Z_TYPE_P(c)) { \
	case IS_LONG: \
		RETURN_BOOL(u_##iswhat(Z_LVAL_P(c))); \
		break; \
	case IS_UNICODE: \
		{ \
			int ofs = 0; \
			while (ofs < Z_USTRLEN_P(c)) { \
				UChar32 ch; \
				U16_GET(Z_USTRVAL_P(c), 0, ofs, Z_USTRLEN_P(c), ch); \
				if (!u_##iswhat(ch)) { \
					RETURN_FALSE; \
				} \
				U16_FWD_1(Z_USTRVAL_P(c), ofs, Z_USTRLEN_P(c)); \
			} \
			RETURN_TRUE; \
		} \
	case IS_STRING: \
		{ \
			char *p = Z_STRVAL_P(c), *e = Z_STRVAL_P(c) + Z_STRLEN_P(c); \
			if (e == p) {	\
				if (c == &tmp) zval_dtor(&tmp); \
				RETURN_FALSE;	\
			}	\
			while (p < e) { \
				if(!iswhat((int)*(unsigned char *)(p++))) { \
					if (c == &tmp) zval_dtor(&tmp); \
					RETURN_FALSE; \
				} \
			} \
			if (c == &tmp) zval_dtor(&tmp); \
			RETURN_TRUE; \
		} \
	default: \
		break; \
	} \
	RETURN_FALSE; 
 
/* }}} */

/* {{{ proto bool ctype_alnum(mixed c) U
   Checks for alphanumeric character(s) */
static PHP_FUNCTION(ctype_alnum)
{
	CTYPE(isalnum);
}
/* }}} */

/* {{{ proto bool ctype_alpha(mixed c) U
   Checks for alphabetic character(s) */
static PHP_FUNCTION(ctype_alpha)
{
	CTYPE(isalpha);
}
/* }}} */

/* {{{ proto bool ctype_cntrl(mixed c) U
   Checks for control character(s) */
static PHP_FUNCTION(ctype_cntrl)
{
	CTYPE(iscntrl);
}
/* }}} */

/* {{{ proto bool ctype_digit(mixed c) U
   Checks for numeric character(s) */
static PHP_FUNCTION(ctype_digit)
{
	CTYPE(isdigit);
}
/* }}} */

/* {{{ proto bool ctype_lower(mixed c) U
   Checks for lowercase character(s)  */
static PHP_FUNCTION(ctype_lower)
{
	CTYPE(islower);
}
/* }}} */

/* {{{ proto bool ctype_graph(mixed c) U
   Checks for any printable character(s) except space */
static PHP_FUNCTION(ctype_graph)
{
	CTYPE(isgraph);
}
/* }}} */

/* {{{ proto bool ctype_print(mixed c) U
   Checks for printable character(s) */
static PHP_FUNCTION(ctype_print)
{
	CTYPE(isprint);
}
/* }}} */

/* {{{ proto bool ctype_punct(mixed c) U
   Checks for any printable character which is not whitespace or an alphanumeric character */
static PHP_FUNCTION(ctype_punct)
{
	CTYPE(ispunct);
}
/* }}} */

/* {{{ proto bool ctype_space(mixed c) U
   Checks for whitespace character(s)*/
static PHP_FUNCTION(ctype_space)
{
	CTYPE(isspace);
}
/* }}} */

/* {{{ proto bool ctype_upper(mixed c) U
   Checks for uppercase character(s) */
static PHP_FUNCTION(ctype_upper)
{
	CTYPE(isupper);
}
/* }}} */

/* {{{ proto bool ctype_xdigit(mixed c) U
   Checks for character(s) representing a hexadecimal digit */
static PHP_FUNCTION(ctype_xdigit)
{
	CTYPE(isxdigit);
}
/* }}} */

#endif	/* HAVE_CTYPE */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
