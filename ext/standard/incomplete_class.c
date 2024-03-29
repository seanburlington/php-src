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
   | Author:  Sascha Schumann <sascha@schumann.cx>                        |
   +----------------------------------------------------------------------+
*/

/* $Id: incomplete_class.c,v 1.42 2009/05/25 14:32:15 felipe Exp $ */

#include "php.h"
#include "basic_functions.h"
#include "php_incomplete_class.h"

#define INCOMPLETE_CLASS_MSG \
		"The script tried to execute a method or "  \
		"access a property of an incomplete object. " \
		"Please ensure that the class definition \"%s\" of the object " \
		"you are trying to operate on was loaded _before_ " \
		"unserialize() gets called or provide a __autoload() function " \
		"to load the class definition "

static zend_object_handlers php_incomplete_object_handlers;

/* {{{ incomplete_class_message
 */
static void incomplete_class_message(zval *object, int error_type TSRMLS_DC)
{
	zstr class_name;
	zend_bool class_name_alloced = 1;

	class_name = php_lookup_class_name(object, NULL);
	
	/* FIXME: Unicode support??? */
	if (!class_name.s) {
		class_name_alloced = 0;
		class_name.s = "unknown";
	}
	
	php_error_docref(NULL TSRMLS_CC, error_type, INCOMPLETE_CLASS_MSG, class_name);

	if (class_name_alloced) {
		efree(class_name.v);
	}
}
/* }}} */

static zval *incomplete_class_get_property(zval *object, zval *member, int type TSRMLS_DC) /* {{{ */
{
	incomplete_class_message(object, E_NOTICE TSRMLS_CC);
	
	if (type == BP_VAR_W || type == BP_VAR_RW) {
		return EG(error_zval_ptr);
	} else {
		return EG(uninitialized_zval_ptr);
	}
}
/* }}} */

static void incomplete_class_write_property(zval *object, zval *member, zval *value TSRMLS_DC) /* {{{ */
{
	incomplete_class_message(object, E_NOTICE TSRMLS_CC);
}
/* }}} */
	
static zval **incomplete_class_get_property_ptr_ptr(zval *object, zval *member TSRMLS_DC) /* {{{ */
{
	incomplete_class_message(object, E_NOTICE TSRMLS_CC);
	return &EG(error_zval_ptr);
}
/* }}} */

static void incomplete_class_unset_property(zval *object, zval *member TSRMLS_DC) /* {{{ */
{
	incomplete_class_message(object, E_NOTICE TSRMLS_CC);
}
/* }}} */

static int incomplete_class_has_property(zval *object, zval *member, int check_empty TSRMLS_DC) /* {{{ */
{
	incomplete_class_message(object, E_NOTICE TSRMLS_CC);
	return 0;
}
/* }}} */

static union _zend_function *incomplete_class_get_method(zval **object, zstr method, int method_len TSRMLS_DC) /* {{{ */
{
	incomplete_class_message(*object, E_ERROR TSRMLS_CC);
	return NULL;
}
/* }}} */

/* {{{ php_create_incomplete_class
 */
static zend_object_value php_create_incomplete_object(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object *object;
	zend_object_value value;
	
	value = zend_objects_new(&object, class_type TSRMLS_CC);
	value.handlers = &php_incomplete_object_handlers;
	
	ALLOC_HASHTABLE(object->properties);
	zend_u_hash_init(object->properties, 0, NULL, ZVAL_PTR_DTOR, 0, UG(unicode));
	return value;
}

PHPAPI zend_class_entry *php_create_incomplete_class(TSRMLS_D)
{
	zend_class_entry incomplete_class;

	INIT_CLASS_ENTRY(incomplete_class, INCOMPLETE_CLASS, NULL);
	incomplete_class.create_object = php_create_incomplete_object;

	memcpy(&php_incomplete_object_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	php_incomplete_object_handlers.read_property = incomplete_class_get_property;
	php_incomplete_object_handlers.has_property = incomplete_class_has_property;
	php_incomplete_object_handlers.unset_property = incomplete_class_unset_property;
	php_incomplete_object_handlers.write_property = incomplete_class_write_property;
	php_incomplete_object_handlers.get_property_ptr_ptr = incomplete_class_get_property_ptr_ptr;
    php_incomplete_object_handlers.get_method = incomplete_class_get_method;
	
	return zend_register_internal_class(&incomplete_class TSRMLS_CC);
}
/* }}} */

/* {{{ php_lookup_class_name
 */
PHPAPI zstr php_lookup_class_name(zval *object, zend_uint *nlen)
{
	zval **val;
	zstr retval = NULL_ZSTR;
	HashTable *object_properties;
	TSRMLS_FETCH();

	object_properties = Z_OBJPROP_P(object);

	if (zend_hash_find(object_properties, MAGIC_MEMBER, sizeof(MAGIC_MEMBER), (void **) &val) == SUCCESS) {
		retval.u = eustrndup(Z_USTRVAL_PP(val), Z_USTRLEN_PP(val));

		if (nlen) {
			*nlen = Z_UNILEN_PP(val);
		}
	}

	return retval;
}
/* }}} */

/* {{{ php_store_class_name
 */
PHPAPI void php_store_class_name(zval *object, zstr name, zend_uint len)
{
	zval *val;
	TSRMLS_FETCH();

	MAKE_STD_ZVAL(val);
	ZVAL_UNICODEL(val, name.u, len, 1);

	zend_hash_update(Z_OBJPROP_P(object), MAGIC_MEMBER, sizeof(MAGIC_MEMBER), &val, sizeof(val), NULL);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
