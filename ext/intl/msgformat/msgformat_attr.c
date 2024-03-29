/*
   +----------------------------------------------------------------------+
   | PHP Version 6                                                        |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Stanislav Malyshev <stas@zend.com>                          |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_intl.h"
#include "msgformat_class.h"
#include "msgformat_attr.h"

#include <unicode/ustring.h>


/* {{{ proto unicode MessageFormatter::getPattern( )
 * Get formatter pattern. }}} */
/* {{{ proto string msgfmt_get_pattern( MessageFormatter $mf )
 * Get formatter pattern.
 */
PHP_FUNCTION( msgfmt_get_pattern )
{
	MSG_FORMAT_METHOD_INIT_VARS;

	/* Parse parameters. */
	if( zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, MessageFormatter_ce_ptr ) == FAILURE )
	{
		intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR,	
			"msgfmt_get_pattern: unable to parse input params", 0 TSRMLS_CC );
		RETURN_FALSE;
	}

	/* Fetch the object. */
	MSG_FORMAT_METHOD_FETCH_OBJECT;

 	if(mfo->mf_data.orig_format) {
 		RETURN_UNICODEL(mfo->mf_data.orig_format, mfo->mf_data.orig_format_len, 1);
  	}
  
 	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool MessageFormatter::setPattern( string $pattern )
 * Set formatter pattern. }}} */
/* {{{ proto bool msgfmt_set_pattern( MessageFormatter $mf, string $pattern )
 * Set formatter pattern.
 */
PHP_FUNCTION( msgfmt_set_pattern )
{
	int         slength = 0;
	UChar*	    svalue  = NULL;
	int free_pattern = 0;
	MSG_FORMAT_METHOD_INIT_VARS;

	/* Parse parameters. */
	if( zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ou",
		&object, MessageFormatter_ce_ptr, &svalue, &slength ) == FAILURE )
	{
		intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR,	
			"msgfmt_set_pattern: unable to parse input params", 0 TSRMLS_CC);
		RETURN_FALSE;
	}

	MSG_FORMAT_METHOD_FETCH_OBJECT;

	if(mfo->mf_data.orig_format) {
 		efree(mfo->mf_data.orig_format);
 	}
 	mfo->mf_data.orig_format = eustrndup(svalue, slength);
 	mfo->mf_data.orig_format_len = slength;

	if(msgformat_fix_quotes(&svalue, &slength, &INTL_DATA_ERROR_CODE(mfo), &free_pattern) != SUCCESS) {
 		intl_error_set( NULL, U_INVALID_FORMAT_ERROR,
 			"msgfmt_set_pattern: error converting pattern to quote-friendly format", 0 TSRMLS_CC );
 		RETURN_FALSE;
 	}

	/* TODO: add parse error information */
	umsg_applyPattern(MSG_FORMAT_OBJECT(mfo), svalue, slength, NULL, &INTL_DATA_ERROR_CODE(mfo));
	if(free_pattern) {
		efree(svalue);
	}
	INTL_METHOD_CHECK_STATUS(mfo, "Error setting symbol value");


	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string MessageFormatter::getLocale()
 * Get formatter locale. }}} */
/* {{{ proto string msgfmt_get_locale(MessageFormatter $mf)
 * Get formatter locale.
 */
PHP_FUNCTION( msgfmt_get_locale )
{
	char *loc;
	MSG_FORMAT_METHOD_INIT_VARS;

	/* Parse parameters. */
	if( zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",
		&object, MessageFormatter_ce_ptr ) == FAILURE )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"msgfmt_get_locale: unable to parse input params", 0 TSRMLS_CC );

		RETURN_FALSE;
	}

	/* Fetch the object. */
	MSG_FORMAT_METHOD_FETCH_OBJECT;

	loc = (char *)umsg_getLocale(MSG_FORMAT_OBJECT(mfo));
	RETURN_STRING(loc, 1);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
