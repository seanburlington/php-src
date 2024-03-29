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
   | Authors: Vadim Savchuk <vsavchuk@productengine.com>                  |
   |          Dmitry Lakhtyuk <dlakhtyuk@productengine.com>               |
   +----------------------------------------------------------------------+
 */

#include "collator_class.h"
#include "php_intl.h"
#include "collator_attr.h"
#include "collator_compare.h"
#include "collator_sort.h"
#include "collator_convert.h"
#include "collator_locale.h"
#include "collator_create.h"
#include "collator_error.h"
#include "intl_error.h"

#include <unicode/ucol.h>

zend_class_entry *Collator_ce_ptr = NULL;

/*
 * Auxiliary functions needed by objects of 'Collator' class
 */

/* {{{ Collator_objects_dtor */
static void Collator_objects_dtor(
	void *object,
	zend_object_handle handle TSRMLS_DC )
{
	zend_objects_destroy_object( object, handle TSRMLS_CC );
}
/* }}} */

/* {{{ Collator_objects_free */
void Collator_objects_free( zend_object *object TSRMLS_DC )
{
	Collator_object* co = (Collator_object*)object;

	zend_object_std_dtor( &co->zo TSRMLS_CC );

	collator_object_destroy( co TSRMLS_CC );

	efree( co );
}
/* }}} */

/* {{{ Collator_object_create */
zend_object_value Collator_object_create(
	zend_class_entry *ce TSRMLS_DC )
{
	zend_object_value    retval;
	Collator_object*     intern;

	intern = ecalloc( 1, sizeof(Collator_object) );
	intl_error_init( COLLATOR_ERROR_P( intern ) TSRMLS_CC );
	zend_object_std_init( &intern->zo, ce TSRMLS_CC );

	retval.handle = zend_objects_store_put(
		intern,
		Collator_objects_dtor,
		(zend_objects_free_object_storage_t)Collator_objects_free,
		NULL TSRMLS_CC );

	retval.handlers = zend_get_std_object_handlers();

	return retval;
}
/* }}} */

/*
 * 'Collator' class registration structures & functions
 */

/* {{{ Collator methods arguments info */
/* NOTE: modifying 'collator_XX_args' do not forget to
       modify approptiate 'collator_XX_args' for
       the procedural API.
*/

ZEND_BEGIN_ARG_INFO_EX( collator_0_args, 0, 0, 0 )
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX( collator_1_arg, 0, 0, 1 )
	ZEND_ARG_INFO( 0, arg1 )
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX( collator_2_args, 0, 0, 2 )
	ZEND_ARG_INFO( 0, arg1 )
	ZEND_ARG_INFO( 0, arg2 )
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX( collator_sort_args, 0, 0, 1 )
	ZEND_ARG_ARRAY_INFO( 1, arr, 0 )
	ZEND_ARG_INFO( 0, flags )
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ Collator_class_functions
 * Every 'Collator' class method has an entry in this table
 */

function_entry Collator_class_functions[] = {
	PHP_ME( Collator, __construct, collator_1_arg, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR )
	ZEND_FENTRY( create, ZEND_FN( collator_create ), collator_1_arg, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC )
	PHP_NAMED_FE( compare, ZEND_FN( collator_compare ), collator_2_args )
	PHP_NAMED_FE( sort, ZEND_FN( collator_sort ), collator_sort_args )
	PHP_NAMED_FE( sortWithSortKeys, ZEND_FN( collator_sort_with_sort_keys ), collator_sort_args )
	PHP_NAMED_FE( asort, ZEND_FN( collator_asort ), collator_sort_args )
	PHP_NAMED_FE( getAttribute, ZEND_FN( collator_get_attribute ), collator_1_arg )
	PHP_NAMED_FE( setAttribute, ZEND_FN( collator_set_attribute ), collator_2_args )
	PHP_NAMED_FE( getStrength, ZEND_FN( collator_get_strength ), collator_0_args )
	PHP_NAMED_FE( setStrength, ZEND_FN( collator_set_strength ), collator_1_arg )
	PHP_NAMED_FE( getLocale, ZEND_FN( collator_get_locale ), collator_1_arg )
	PHP_NAMED_FE( getErrorCode, ZEND_FN( collator_get_error_code ), collator_0_args )
	PHP_NAMED_FE( getErrorMessage, ZEND_FN( collator_get_error_message ), collator_0_args )
	{ NULL, NULL, NULL }
};
/* }}} */

/* {{{ collator_register_Collator_class
 * Initialize 'Collator' class
 */
void collator_register_Collator_class( TSRMLS_D )
{
	zend_class_entry ce;

	/* Create and register 'Collator' class. */
	INIT_CLASS_ENTRY( ce, "Collator", Collator_class_functions );
	ce.create_object = Collator_object_create;
	Collator_ce_ptr = zend_register_internal_class( &ce TSRMLS_CC );

	/* Declare 'Collator' class properties. */
	if( !Collator_ce_ptr )
	{
		zend_error( E_ERROR,
			"Collator: attempt to create properties "
			"on a non-registered class." );
		return;
	}
}
/* }}} */

/* {{{ void collator_object_init( Collator_object* co )
 * Initialize internals of Collator_object.
 * Must be called before any other call to 'collator_object_...' functions.
 */
void collator_object_init( Collator_object* co TSRMLS_DC )
{
	if( !co )
		return;

	intl_error_init( COLLATOR_ERROR_P( co ) TSRMLS_CC );
}
/* }}} */

/* {{{ void collator_object_destroy( Collator_object* co )
 * Clean up mem allocted by internals of Collator_object
 */
void collator_object_destroy( Collator_object* co TSRMLS_DC )
{
	if( !co )
		return;

	if( co->ucoll )
	{
		ucol_close( co->ucoll );
		co->ucoll = NULL;
	}

	intl_error_reset( COLLATOR_ERROR_P( co ) TSRMLS_CC );
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
