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
  | Author: Christian Stocker <chregu@php.net>                           |
  +----------------------------------------------------------------------+
*/

/* $Id: php_xsl.c,v 1.48 2009/03/10 23:39:52 helly Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_xsl.h"

zend_class_entry *xsl_xsltprocessor_class_entry;
static zend_object_handlers xsl_object_handlers;

/* {{{ xsl_functions[]
 *
 * Every user visible function must have an entry in xsl_functions[].
 */
const zend_function_entry xsl_functions[] = {
	{NULL, NULL, NULL}  /* Must be the last line in xsl_functions[] */
};
/* }}} */

static const zend_module_dep xsl_deps[] = {
	ZEND_MOD_REQUIRED("libxml")
	{NULL, NULL, NULL}
};

/* {{{ xsl_module_entry
 */
zend_module_entry xsl_module_entry = {
#if ZEND_MODULE_API_NO >= 20050617
	STANDARD_MODULE_HEADER_EX, NULL,
	xsl_deps,
#elif ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"xsl",
	xsl_functions,
	PHP_MINIT(xsl),
	PHP_MSHUTDOWN(xsl),
	PHP_RINIT(xsl),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(xsl),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(xsl),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_XSL
ZEND_GET_MODULE(xsl)
#endif

/* {{{ xsl_objects_free_storage */
void xsl_objects_free_storage(void *object TSRMLS_DC)
{
	xsl_object *intern = (xsl_object *)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	zend_hash_destroy(intern->parameter);
	FREE_HASHTABLE(intern->parameter);
	
	zend_hash_destroy(intern->registered_phpfunctions);
	FREE_HASHTABLE(intern->registered_phpfunctions);
	
	if (intern->node_list) {
		zend_hash_destroy(intern->node_list);
		FREE_HASHTABLE(intern->node_list);
	}

	if (intern->doc) {
		php_libxml_decrement_doc_ref(intern->doc TSRMLS_CC);
		efree(intern->doc);
	}

	if (intern->ptr) {
		/* free wrapper */
		if (((xsltStylesheetPtr) intern->ptr)->_private != NULL) {
			((xsltStylesheetPtr) intern->ptr)->_private = NULL;   
		}

		xsltFreeStylesheet((xsltStylesheetPtr) intern->ptr);
		intern->ptr = NULL;
	}
	if (intern->profiling) {
		efree(intern->profiling);
	}
	efree(object);
}
/* }}} */

/* {{{ xsl_objects_new */
zend_object_value xsl_objects_new(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	xsl_object *intern;
	zval *tmp;

	intern = emalloc(sizeof(xsl_object));
	intern->ptr = NULL;
	intern->prop_handler = NULL;
	intern->parameter = NULL;
	intern->hasKeys = 0;
	intern->registerPhpFunctions = 0;
	intern->registered_phpfunctions = NULL;
	intern->node_list = NULL;
	intern->doc = NULL;
	intern->profiling = NULL;

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
	ALLOC_HASHTABLE(intern->parameter);
	zend_hash_init(intern->parameter, 0, NULL, ZVAL_PTR_DTOR, 0);
	ALLOC_HASHTABLE(intern->registered_phpfunctions);
	zend_u_hash_init(intern->registered_phpfunctions, 0, NULL, ZVAL_PTR_DTOR, 0, UG(unicode));
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) xsl_objects_free_storage, NULL TSRMLS_CC);
	intern->handle = retval.handle;
	retval.handlers = &xsl_object_handlers;
	return retval;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(xsl)
{
	
	zend_class_entry ce;
	
	memcpy(&xsl_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	xsl_object_handlers.clone_obj = NULL;

	REGISTER_XSL_CLASS(ce, "XSLTProcessor", NULL, php_xsl_xsltprocessor_class_functions, xsl_xsltprocessor_class_entry);
#if HAVE_XSL_EXSLT
	exsltRegisterAll();
#endif
 
	xsltRegisterExtModuleFunction ((const xmlChar *) "functionString",
				   (const xmlChar *) "http://php.net/xsl",
				   xsl_ext_function_string_php);
	xsltRegisterExtModuleFunction ((const xmlChar *) "function",
				   (const xmlChar *) "http://php.net/xsl",
				   xsl_ext_function_object_php);

	REGISTER_LONG_CONSTANT("XSL_CLONE_AUTO",      0,     CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSL_CLONE_NEVER",    -1,     CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSL_CLONE_ALWAYS",    1,     CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBXSLT_VERSION",           LIBXSLT_VERSION,            CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("LIBXSLT_DOTTED_VERSION",  LIBXSLT_DOTTED_VERSION,     CONST_CS | CONST_PERSISTENT);

#if HAVE_XSL_EXSLT
	REGISTER_LONG_CONSTANT("LIBEXSLT_VERSION",           LIBEXSLT_VERSION,            CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("LIBEXSLT_DOTTED_VERSION",  LIBEXSLT_DOTTED_VERSION,     CONST_CS | CONST_PERSISTENT);
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ xsl_object_get_data */
zval *xsl_object_get_data(void *obj)
{
	zval *dom_wrapper;
	dom_wrapper = ((xsltStylesheetPtr) obj)->_private;
	return dom_wrapper;
}
/* }}} */

/* {{{ xsl_object_set_data */
static void xsl_object_set_data(void *obj, zval *wrapper TSRMLS_DC)
{
	((xsltStylesheetPtr) obj)->_private = wrapper;
}
/* }}} */

/* {{{ php_xsl_set_object */
void php_xsl_set_object(zval *wrapper, void *obj TSRMLS_DC)
{
	xsl_object *object;

	object = (xsl_object *)zend_objects_get_address(wrapper TSRMLS_CC);
	object->ptr = obj;
	xsl_object_set_data(obj, wrapper TSRMLS_CC);
}
/* }}} */

/* {{{ php_xsl_create_object */
zval *php_xsl_create_object(xsltStylesheetPtr obj, int *found, zval *wrapper_in, zval *return_value  TSRMLS_DC)
{
	zval *wrapper;
	zend_class_entry *ce;

	*found = 0;

	if (!obj) {
		if(!wrapper_in) {
			ALLOC_ZVAL(wrapper);
		} else {
			wrapper = wrapper_in;
		}
		ZVAL_NULL(wrapper);
		return wrapper;
	}

	if ((wrapper = (zval *) xsl_object_get_data((void *) obj))) {
		zval_add_ref(&wrapper);
		*found = 1;
		return wrapper;
	}

	if(!wrapper_in) {
		wrapper = return_value;
	} else {
		wrapper = wrapper_in;
	}

	
	ce = xsl_xsltprocessor_class_entry;

	if(!wrapper_in) {
		object_init_ex(wrapper, ce);
	}
	php_xsl_set_object(wrapper, (void *) obj TSRMLS_CC);
	return (wrapper);
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(xsl)
{
	xsltUnregisterExtModuleFunction ((const xmlChar *) "functionString",
				   (const xmlChar *) "http://php.net/xsl");
	xsltUnregisterExtModuleFunction ((const xmlChar *) "function",
				   (const xmlChar *) "http://php.net/xsl");

	xsltCleanupGlobals();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(xsl)
{
	xsltSetGenericErrorFunc(NULL, php_libxml_error_handler);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(xsl)
{
	xsltSetGenericErrorFunc(NULL, NULL);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(xsl)
{
	php_info_print_table_start();
	{
		char buffer[128];
		int major, minor, subminor;

		php_info_print_table_row(2, "XSL", "enabled");
		major = xsltLibxsltVersion/10000;
		minor = (xsltLibxsltVersion - major * 10000) / 100;
		subminor = (xsltLibxsltVersion - major * 10000 - minor * 100);
		snprintf(buffer, 128, "%d.%d.%d", major, minor, subminor);
		php_info_print_table_row(2, "libxslt Version", buffer);
		major = xsltLibxmlVersion/10000;
		minor = (xsltLibxmlVersion - major * 10000) / 100;
		subminor = (xsltLibxmlVersion - major * 10000 - minor * 100);
		snprintf(buffer, 128, "%d.%d.%d", major, minor, subminor);
		php_info_print_table_row(2, "libxslt compiled against libxml Version", buffer);
	}
#if HAVE_XSL_EXSLT
	php_info_print_table_row(2, "EXSLT", "enabled");
	php_info_print_table_row(2, "libexslt Version", LIBEXSLT_DOTTED_VERSION);
#endif
	php_info_print_table_end();
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
