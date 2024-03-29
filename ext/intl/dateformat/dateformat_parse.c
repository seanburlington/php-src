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
   | Authors: Kirti Velankar <kirtig@yahoo-inc.com>                       |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unicode/ustring.h>

#include "php_intl.h"
#include "intl_convert.h"
#include "dateformat.h"
#include "dateformat_class.h"
#include "dateformat_parse.h"
#include "dateformat_data.h"

/* {{{ 
 * Internal function which calls the udat_parse
 * param int store_error acts like a boolean 
 *	if set to 1 - store any error encountered  in the parameter parse_error  
 *	if set to 0 - no need to store any error encountered  in the parameter parse_error  
*/
static void internal_parse_to_timestamp(IntlDateFormatter_object *dfo, char* text_to_parse , int32_t text_len, int parse_pos , zval *return_value TSRMLS_DC){
	long	result =  0;
	UDate 	timestamp   =0;
	UChar* 	text_utf16  = NULL;
	int32_t text_utf16_len = 0;

	/* Convert timezone to UTF-16. */
	intl_convert_utf8_to_utf16(&text_utf16 , &text_utf16_len , text_to_parse , text_len, &INTL_DATA_ERROR_CODE(dfo));
	INTL_METHOD_CHECK_STATUS(dfo, "Error converting timezone to UTF-16" );

	timestamp = udat_parse( DATE_FORMAT_OBJECT(dfo), text_utf16 , text_utf16_len , &parse_pos , &INTL_DATA_ERROR_CODE(dfo));
	if( text_utf16 ){
		efree(text_utf16);
	}

	INTL_METHOD_CHECK_STATUS( dfo, "Date parsing failed" );
	
	/* Since return is in  sec. */
	result = (long )( timestamp / 1000 );
	if( result != (timestamp/1000) ) {
		intl_error_set( NULL, U_BUFFER_OVERFLOW_ERROR,
			"datefmt_parse: parsing of input parametrs resulted in value larger than data type long can handle.\nThe valid range of a timestamp is typically from Fri, 13 Dec 1901 20:45:54 GMT to Tue, 19 Jan 2038 03:14:07 GMT.", 0 TSRMLS_CC );
	}
	RETURN_LONG( result );
}
/* }}} */

static void add_to_localtime_arr( IntlDateFormatter_object *dfo, zval* return_value ,UCalendar parsed_calendar , long calendar_field , char* key_name TSRMLS_DC){
	long calendar_field_val = ucal_get( parsed_calendar , calendar_field , &INTL_DATA_ERROR_CODE(dfo));	
	INTL_METHOD_CHECK_STATUS( dfo, "Date parsing - localtime failed : could not get a field from calendar" );

	if( strcmp(key_name , CALENDAR_YEAR )==0 ){
		/* since tm_year is years from 1900 */
		add_assoc_long( return_value, key_name ,( calendar_field_val-1900) ); 
	}else if( strcmp(key_name , CALENDAR_WDAY )==0 ){
		/* since tm_wday starts from 0 whereas ICU WDAY start from 1 */
		add_assoc_long( return_value, key_name ,( calendar_field_val-1) ); 
	}else{
		add_assoc_long( return_value, key_name , calendar_field_val ); 
	}
}

/* {{{
 * Internal function which calls the udat_parseCalendar
*/
static void internal_parse_to_localtime(IntlDateFormatter_object *dfo, char* text_to_parse , int32_t text_len, int parse_pos , zval *return_value TSRMLS_DC){
	UCalendar* 	parsed_calendar = NULL;
	UChar*  	text_utf16  = NULL;
	int32_t 	text_utf16_len = 0;
	long 		isInDST = 0;

	/* Convert timezone to UTF-16. */
	intl_convert_utf8_to_utf16(&text_utf16 , &text_utf16_len , text_to_parse , text_len, &INTL_DATA_ERROR_CODE(dfo));
	INTL_METHOD_CHECK_STATUS(dfo, "Error converting timezone to UTF-16" );

	parsed_calendar = ucal_open(NULL, -1, NULL, UCAL_GREGORIAN, &INTL_DATA_ERROR_CODE(dfo));
	udat_parseCalendar( DATE_FORMAT_OBJECT(dfo), parsed_calendar , text_utf16 , text_utf16_len , &parse_pos , &INTL_DATA_ERROR_CODE(dfo));
	if( text_utf16 ){
		efree(text_utf16);
	}

	INTL_METHOD_CHECK_STATUS( dfo, "Date parsing failed" );

	array_init( return_value );
	/* Add  entries from various fields of the obtained parsed_calendar */
	add_to_localtime_arr( dfo , return_value , parsed_calendar , UCAL_SECOND , CALENDAR_SEC TSRMLS_CC);
	add_to_localtime_arr( dfo , return_value , parsed_calendar , UCAL_MINUTE , CALENDAR_MIN TSRMLS_CC);
	add_to_localtime_arr( dfo , return_value , parsed_calendar , UCAL_HOUR_OF_DAY , CALENDAR_HOUR TSRMLS_CC);
	add_to_localtime_arr( dfo , return_value , parsed_calendar , UCAL_YEAR , CALENDAR_YEAR TSRMLS_CC); 
	add_to_localtime_arr( dfo , return_value , parsed_calendar , UCAL_DAY_OF_MONTH , CALENDAR_MDAY TSRMLS_CC);
	add_to_localtime_arr( dfo , return_value , parsed_calendar , UCAL_DAY_OF_WEEK  , CALENDAR_WDAY TSRMLS_CC);
	add_to_localtime_arr( dfo , return_value , parsed_calendar , UCAL_DAY_OF_YEAR  , CALENDAR_YDAY TSRMLS_CC);
	add_to_localtime_arr( dfo , return_value , parsed_calendar , UCAL_MONTH , CALENDAR_MON TSRMLS_CC);

	/* Is in DST? */
	isInDST = ucal_inDaylightTime(parsed_calendar	 , &INTL_DATA_ERROR_CODE(dfo));
	INTL_METHOD_CHECK_STATUS( dfo, "Date parsing - localtime failed : while checking if currently in DST." );
	add_assoc_long( return_value, CALENDAR_ISDST ,(isInDST==1?1:0)); 
}
/* }}} */


/* {{{ proto integer IntlDateFormatter::parse( string $text_to_parse  , int $parse_pos )
 * Parse the string $value starting at parse_pos to a Unix timestamp -int }}}*/
/* {{{ proto integer datefmt_parse( IntlDateFormatter $fmt, string $text_to_parse , int $parse_pos )
 * Parse the string $value starting at parse_pos to a Unix timestamp -int }}}*/
PHP_FUNCTION(datefmt_parse)
{
	char*           text_to_parse = NULL;
	int32_t         text_len =0;
	long         	parse_pos =0;

	DATE_FORMAT_METHOD_INIT_VARS;

	/* Parse parameters. */
	if( zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|l",
		&object, IntlDateFormatter_ce_ptr,  &text_to_parse ,  &text_len , &parse_pos ) == FAILURE ){
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR, "datefmt_parse: unable to parse input params", 0 TSRMLS_CC );
		RETURN_FALSE;
	}

	/* Fetch the object. */
	DATE_FORMAT_METHOD_FETCH_OBJECT;

	internal_parse_to_timestamp( dfo, text_to_parse ,  text_len , 
	parse_pos , 
	return_value TSRMLS_CC);

}
/* }}} */

/* {{{ proto integer IntlDateFormatter::localtime( string $text_to_parse, int $parse_pos ))
 * Parse the string $value to a localtime array  }}}*/
/* {{{ proto integer datefmt_localtime( IntlDateFormatter $fmt, string $text_to_parse, int $parse_pos ))
 * Parse the string $value to a localtime array  }}}*/
PHP_FUNCTION(datefmt_localtime)
{

	char*           text_to_parse = NULL;
	int32_t         text_len =0;
	long         	parse_pos =0;

	DATE_FORMAT_METHOD_INIT_VARS;

	/* Parse parameters. */
	if( zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osl",
		&object, IntlDateFormatter_ce_ptr,  &text_to_parse ,  &text_len , &parse_pos ) == FAILURE ){
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR, "datefmt_parse_to_localtime: unable to parse input params", 0 TSRMLS_CC );
		RETURN_FALSE;
	}

	/* Fetch the object. */
	DATE_FORMAT_METHOD_FETCH_OBJECT;

	internal_parse_to_localtime( dfo, text_to_parse ,  text_len , 
	parse_pos, 
	return_value TSRMLS_CC);
}
/* }}} */

