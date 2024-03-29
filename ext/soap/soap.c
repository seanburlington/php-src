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
  | Authors: Brad Lafountain <rodif_bl@yahoo.com>                        |
  |          Shane Caraveo <shane@caraveo.com>                           |
  |          Dmitry Stogov <dmitry@zend.com>                             |
  +----------------------------------------------------------------------+
*/
/* $Id: soap.c,v 1.264 2009/05/25 14:32:14 felipe Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php_soap.h"
#if HAVE_PHP_SESSION && !defined(COMPILE_DL_SESSION)
#include "ext/session/php_session.h"
#endif
#include "zend_exceptions.h"
#include "ext/libxml/php_libxml.h"

typedef struct _soapHeader {
	sdlFunctionPtr                    function;
	zval                              function_name;
	int                               mustUnderstand;
	int                               num_params;
	zval                            **parameters;
	zval                              retval;
	sdlSoapBindingFunctionHeaderPtr   hdr;
	struct _soapHeader               *next;
} soapHeader;

/* Local functions */
static void function_to_string(sdlFunctionPtr function, smart_str *buf);
static void type_to_string(sdlTypePtr type, smart_str *buf, int level);

static void set_soap_fault(zval *obj, char *fault_code_ns, char *fault_code, char *fault_string, char *fault_actor, zval *fault_detail, char *name TSRMLS_DC);
static void soap_server_fault(char* code, char* string, char *actor, zval* details, char *name TSRMLS_DC);
static void soap_server_fault_ex(sdlFunctionPtr function, zval* fault, soapHeader* hdr TSRMLS_DC);

static sdlParamPtr get_param(sdlFunctionPtr function, char *param_name, int index, int);
static sdlFunctionPtr get_function(sdlPtr sdl, const char *function_name);
static sdlFunctionPtr get_doc_function(sdlPtr sdl, xmlNodePtr node);

static sdlFunctionPtr deserialize_function_call(sdlPtr sdl, xmlDocPtr request, char* actor, zval *function_name, int *num_params, zval **parameters[], int *version, soapHeader **headers TSRMLS_DC);
static xmlDocPtr serialize_response_call(sdlFunctionPtr function, char *function_name,char *uri,zval *ret, soapHeader *headers, int version TSRMLS_DC);
static xmlDocPtr serialize_function_call(zval *this_ptr, sdlFunctionPtr function, char *function_name, char *uri, zval **arguments, int arg_count, int version, HashTable *soap_headers TSRMLS_DC);
static xmlNodePtr serialize_parameter(sdlParamPtr param,zval *param_val,int index,char *name, int style, xmlNodePtr parent TSRMLS_DC);
static xmlNodePtr serialize_zval(zval *val, sdlParamPtr param, char *paramName, int style, xmlNodePtr parent TSRMLS_DC);

static void soap_error_handler(int error_num, const char *error_filename, const uint error_lineno, const char *format, va_list args);

#define SOAP_SERVER_BEGIN_CODE() \
	zend_bool _old_handler = SOAP_GLOBAL(use_soap_error_handler);\
	char* _old_error_code = SOAP_GLOBAL(error_code);\
	zval* _old_error_object = SOAP_GLOBAL(error_object);\
	int _old_soap_version = SOAP_GLOBAL(soap_version);\
	SOAP_GLOBAL(use_soap_error_handler) = 1;\
	SOAP_GLOBAL(error_code) = "Server";\
	SOAP_GLOBAL(error_object) = this_ptr;

#define SOAP_SERVER_END_CODE() \
	SOAP_GLOBAL(use_soap_error_handler) = _old_handler;\
	SOAP_GLOBAL(error_code) = _old_error_code;\
	SOAP_GLOBAL(error_object) = _old_error_object;\
	SOAP_GLOBAL(soap_version) = _old_soap_version;

#define SOAP_CLIENT_BEGIN_CODE() \
	zend_bool _old_handler = SOAP_GLOBAL(use_soap_error_handler);\
	char* _old_error_code = SOAP_GLOBAL(error_code);\
	zval* _old_error_object = SOAP_GLOBAL(error_object);\
	int _old_soap_version = SOAP_GLOBAL(soap_version);\
	zend_bool _old_in_compilation = CG(in_compilation); \
	zend_bool _old_in_execution = EG(in_execution); \
	zend_execute_data *_old_current_execute_data = EG(current_execute_data); \
	int _bailout = 0;\
	SOAP_GLOBAL(use_soap_error_handler) = 1;\
	SOAP_GLOBAL(error_code) = "Client";\
	SOAP_GLOBAL(error_object) = this_ptr;\
	zend_try {

#define SOAP_CLIENT_END_CODE() \
	} zend_catch {\
		CG(in_compilation) = _old_in_compilation; \
		EG(in_execution) = _old_in_execution; \
		EG(current_execute_data) = _old_current_execute_data; \
		if (EG(exception) == NULL || \
		    Z_TYPE_P(EG(exception)) != IS_OBJECT || \
		    !instanceof_function(Z_OBJCE_P(EG(exception)), soap_fault_class_entry TSRMLS_CC)) {\
			_bailout = 1;\
		}\
	} zend_end_try();\
	SOAP_GLOBAL(use_soap_error_handler) = _old_handler;\
	SOAP_GLOBAL(error_code) = _old_error_code;\
	SOAP_GLOBAL(error_object) = _old_error_object;\
	SOAP_GLOBAL(soap_version) = _old_soap_version;\
	if (_bailout) {\
		zend_bailout();\
	}

#define HTTP_RAW_POST_DATA "HTTP_RAW_POST_DATA"

#define ZERO_PARAM() \
	if (zend_parse_parameters_none() == FAILURE) \
 		return;

static zend_class_entry* soap_client_class_entry;
static zend_class_entry* soap_server_class_entry;
static zend_class_entry* soap_fault_class_entry;
static zend_class_entry* soap_header_class_entry;
static zend_class_entry* soap_param_class_entry;
zend_class_entry* soap_var_class_entry;

ZEND_DECLARE_MODULE_GLOBALS(soap)

static void (*old_error_handler)(int, const char *, const uint, const char*, va_list);

#ifdef va_copy
#define call_old_error_handler(error_num, error_filename, error_lineno, format, args) \
{ \
	va_list copy; \
	va_copy(copy, args); \
	old_error_handler(error_num, error_filename, error_lineno, format, copy); \
	va_end(copy); \
}
#else
#define call_old_error_handler(error_num, error_filename, error_lineno, format, args) \
{ \
	old_error_handler(error_num, error_filename, error_lineno, format, args); \
}
#endif

#define PHP_SOAP_SERVER_CLASSNAME "SoapServer"
#define PHP_SOAP_CLIENT_CLASSNAME "SoapClient"
#define PHP_SOAP_VAR_CLASSNAME    "SoapVar"
#define PHP_SOAP_FAULT_CLASSNAME  "SoapFault"
#define PHP_SOAP_PARAM_CLASSNAME  "SoapParam"
#define PHP_SOAP_HEADER_CLASSNAME "SoapHeader"

PHP_RINIT_FUNCTION(soap);
PHP_MINIT_FUNCTION(soap);
PHP_MSHUTDOWN_FUNCTION(soap);
PHP_MINFO_FUNCTION(soap);

/*
  Registry Functions
  TODO: this!
*/
PHP_FUNCTION(soap_encode_to_xml);
PHP_FUNCTION(soap_encode_to_zval);
PHP_FUNCTION(use_soap_error_handler);
PHP_FUNCTION(is_soap_fault);


/* Server Functions */
PHP_METHOD(SoapServer, SoapServer);
PHP_METHOD(SoapServer, setClass);
PHP_METHOD(SoapServer, setObject);
PHP_METHOD(SoapServer, addFunction);
PHP_METHOD(SoapServer, getFunctions);
PHP_METHOD(SoapServer, handle);
PHP_METHOD(SoapServer, setPersistence);
PHP_METHOD(SoapServer, fault);
PHP_METHOD(SoapServer, addSoapHeader);

/* Client Functions */
PHP_METHOD(SoapClient, SoapClient);
PHP_METHOD(SoapClient, __call);
PHP_METHOD(SoapClient, __getLastRequest);
PHP_METHOD(SoapClient, __getLastResponse);
PHP_METHOD(SoapClient, __getLastRequestHeaders);
PHP_METHOD(SoapClient, __getLastResponseHeaders);
PHP_METHOD(SoapClient, __getFunctions);
PHP_METHOD(SoapClient, __getTypes);
PHP_METHOD(SoapClient, __doRequest);
PHP_METHOD(SoapClient, __setCookie);
PHP_METHOD(SoapClient, __getCookies);
PHP_METHOD(SoapClient, __setLocation);
PHP_METHOD(SoapClient, __setSoapHeaders);

/* SoapVar Functions */
PHP_METHOD(SoapVar, SoapVar);

/* SoapFault Functions */
PHP_METHOD(SoapFault, SoapFault);
PHP_METHOD(SoapFault, __toString);

/* SoapParam Functions */
PHP_METHOD(SoapParam, SoapParam);

/* SoapHeader Functions */
PHP_METHOD(SoapHeader, SoapHeader);

#define SOAP_CTOR(class_name, func_name, arginfo, flags) ZEND_FENTRY(__construct, ZEND_MN(class_name##_##func_name), arginfo, flags)

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_soapparam_soapparam, 0, 0, 2)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapheader_soapheader, 0, 0, 2)
	ZEND_ARG_INFO(0, namespace)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, mustunderstand)
	ZEND_ARG_INFO(0, actor)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapfault_soapfault, 0, 0, 2)
	ZEND_ARG_INFO(0, faultcode)
	ZEND_ARG_INFO(0, faultstring)
	ZEND_ARG_INFO(0, faultactor)
	ZEND_ARG_INFO(0, detail)
	ZEND_ARG_INFO(0, faultname)
	ZEND_ARG_INFO(0, headerfault)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapvar_soapvar, 0, 0, 2)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, encoding)
	ZEND_ARG_INFO(0, type_name)
	ZEND_ARG_INFO(0, type_namespace)
	ZEND_ARG_INFO(0, node_name)
	ZEND_ARG_INFO(0, node_namespace)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapserver_fault, 0, 0, 2)
	ZEND_ARG_INFO(0, code)
	ZEND_ARG_INFO(0, string)
	ZEND_ARG_INFO(0, actor)
	ZEND_ARG_INFO(0, details)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapserver_addsoapheader, 0, 0, 1)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapserver_soapserver, 0, 0, 1)
	ZEND_ARG_INFO(0, wsdl)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapserver_setpersistence, 0, 0, 1)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapserver_setclass, 0, 0, 1)
	ZEND_ARG_INFO(0, class_name)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapserver_setobject, 0, 0, 1)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_soapserver_getfunctions, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapserver_addfunction, 0, 0, 1)
	ZEND_ARG_INFO(0, functions)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapserver_handle, 0, 0, 0)
	ZEND_ARG_INFO(0, soap_request)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapclient_soapclient, 0, 0, 1)
	ZEND_ARG_INFO(0, wsdl)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapclient___call, 0, 0, 2)
	ZEND_ARG_INFO(0, function_name)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapclient___soapcall, 0, 0, 2)
	ZEND_ARG_INFO(0, function_name)
	ZEND_ARG_INFO(0, arguments)
	ZEND_ARG_INFO(0, options)
	ZEND_ARG_INFO(0, input_headers)
	ZEND_ARG_INFO(1, output_headers)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_soapclient___getfunctions, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_soapclient___gettypes, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_soapclient___getlastrequest, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_soapclient___getlastresponse, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_soapclient___getlastrequestheaders, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_soapclient___getlastresponseheaders, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapclient___dorequest, 0, 0, 4)
	ZEND_ARG_INFO(0, request)
	ZEND_ARG_INFO(0, location)
	ZEND_ARG_INFO(0, action)
	ZEND_ARG_INFO(0, version)
	ZEND_ARG_INFO(0, one_way)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapclient___setcookie, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_soapclient___getcookie, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapclient___setsoapheaders, 0, 0, 1)
	ZEND_ARG_INFO(0, soapheaders)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soapclient___setlocation, 0, 0, 0)
	ZEND_ARG_INFO(0, new_location)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soap_use_soap_error_handler, 0, 0, 0)
	ZEND_ARG_INFO(0, handler)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_soap_is_soap_fault, 0, 0, 1)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()
/* }}} */

static const zend_function_entry soap_functions[] = {
	PHP_FE(use_soap_error_handler, 	arginfo_soap_use_soap_error_handler)
	PHP_FE(is_soap_fault, 			arginfo_soap_is_soap_fault)
	{NULL, NULL, NULL}
};

static const zend_function_entry soap_fault_functions[] = {
	SOAP_CTOR(SoapFault, SoapFault, arginfo_soapfault_soapfault, 0)
	PHP_ME(SoapFault, __toString, NULL, 0)
	{NULL, NULL, NULL}
};

static const zend_function_entry soap_server_functions[] = {
	SOAP_CTOR(SoapServer, SoapServer, 	arginfo_soapserver_soapserver, 0)
	PHP_ME(SoapServer, setPersistence, 	arginfo_soapserver_setpersistence, 0)
	PHP_ME(SoapServer, setClass, 		arginfo_soapserver_setclass, 0)
	PHP_ME(SoapServer, setObject, 		arginfo_soapserver_setobject, 0)
	PHP_ME(SoapServer, addFunction, 	arginfo_soapserver_addfunction, 0)
	PHP_ME(SoapServer, getFunctions, 	arginfo_soapserver_getfunctions, 0)
	PHP_ME(SoapServer, handle, 			arginfo_soapserver_handle, 0)
	PHP_ME(SoapServer, fault, 			arginfo_soapserver_fault, 0)
	PHP_ME(SoapServer, addSoapHeader, 	arginfo_soapserver_addsoapheader, 0)
	{NULL, NULL, NULL}
};

static const zend_function_entry soap_client_functions[] = {
	SOAP_CTOR(SoapClient, SoapClient, arginfo_soapclient_soapclient, 0)
	PHP_ME(SoapClient, __call, arginfo_soapclient___call, 0)
	ZEND_FENTRY(__soapCall, ZEND_MN(SoapClient___call), arginfo_soapclient___soapcall, 0)
	PHP_ME(SoapClient, __getLastRequest, 			arginfo_soapclient___getlastrequest, 0)
	PHP_ME(SoapClient, __getLastResponse, 			arginfo_soapclient___getlastresponse, 0)
	PHP_ME(SoapClient, __getLastRequestHeaders,		arginfo_soapclient___getlastrequestheaders, 0)
	PHP_ME(SoapClient, __getLastResponseHeaders, 	arginfo_soapclient___getlastresponseheaders, 0)
	PHP_ME(SoapClient, __getFunctions, 				arginfo_soapclient___getfunctions, 0)
	PHP_ME(SoapClient, __getTypes, 					arginfo_soapclient___gettypes, 0)
	PHP_ME(SoapClient, __doRequest, 				arginfo_soapclient___dorequest, 0)
	PHP_ME(SoapClient, __setCookie, 				arginfo_soapclient___setcookie, 0)
	PHP_ME(SoapClient, __getCookies, 				arginfo_soapclient___getcookie, 0)
	PHP_ME(SoapClient, __setLocation, 				arginfo_soapclient___setlocation, 0)
	PHP_ME(SoapClient, __setSoapHeaders, 			arginfo_soapclient___setsoapheaders, 0)
	{NULL, NULL, NULL}
};

static const zend_function_entry soap_var_functions[] = {
	SOAP_CTOR(SoapVar, SoapVar, arginfo_soapvar_soapvar, 0)
	{NULL, NULL, NULL}
};

static const zend_function_entry soap_param_functions[] = {
	SOAP_CTOR(SoapParam, SoapParam, arginfo_soapparam_soapparam, 0)
	{NULL, NULL, NULL}
};

static const zend_function_entry soap_header_functions[] = {
	SOAP_CTOR(SoapHeader, SoapHeader, arginfo_soapheader_soapheader, 0)
	{NULL, NULL, NULL}
};

zend_module_entry soap_module_entry = {
#ifdef STANDARD_MODULE_HEADER
  STANDARD_MODULE_HEADER,
#endif
  "soap",
  soap_functions,
  PHP_MINIT(soap),
  PHP_MSHUTDOWN(soap),
  PHP_RINIT(soap),
  NULL,
  PHP_MINFO(soap),
#ifdef STANDARD_MODULE_HEADER
  NO_VERSION_YET,
#endif
  STANDARD_MODULE_PROPERTIES,
};

#ifdef COMPILE_DL_SOAP
ZEND_GET_MODULE(soap)
#endif

char* soap_unicode_to_string(UChar *ustr, int ustr_len TSRMLS_DC)
{
	int dummy;

	return php_libxml_unicode_to_string(ustr, ustr_len, &dummy TSRMLS_CC);
}

void soap_decode_string(zval *ret, char* str TSRMLS_DC)
{
	/* TODO: unicode support */
	ZVAL_STRING(ret, str, 1);
	zval_string_to_unicode_ex(ret, UG(utf8_conv) TSRMLS_CC);
}

char* soap_encode_string_ex(zend_uchar type, zstr data, int len TSRMLS_DC)
{
	char *str;
	int new_len;

	if (type == IS_UNICODE) {
		str = soap_unicode_to_string(data.u, len TSRMLS_CC);
	} else {
		str = estrndup(data.s, len);
		new_len = len;

		if (SOAP_GLOBAL(encoding) != NULL) {
			xmlBufferPtr in  = xmlBufferCreateStatic(str, new_len);
			xmlBufferPtr out = xmlBufferCreate();
			int n = xmlCharEncInFunc(SOAP_GLOBAL(encoding), out, in);

			if (n >= 0) {
				efree(str);
				str = estrdup((char*)xmlBufferContent(out));
			}
			xmlBufferFree(out);
			xmlBufferFree(in);
		}
		if (!php_libxml_xmlCheckUTF8(BAD_CAST(str))) {
			char *err = emalloc(new_len + 8);
			char c;
			int i;
		
			memcpy(err, str, new_len+1);
			for (i = 0; (c = err[i++]);) {
				if ((c & 0x80) == 0) {
				} else if ((c & 0xe0) == 0xc0) {
					if ((err[i] & 0xc0) != 0x80) {
						break;
					}
					i++;
				} else if ((c & 0xf0) == 0xe0) {
					if ((err[i] & 0xc0) != 0x80 || (err[i+1] & 0xc0) != 0x80) {
						break;
					}
					i += 2;
				} else if ((c & 0xf8) == 0xf0) {
					if ((err[i] & 0xc0) != 0x80 || (err[i+1] & 0xc0) != 0x80 || (err[i+2] & 0xc0) != 0x80) {
						break;
					}
					i += 3;
				} else {
					break;
				}
			}
			if (c) {
				err[i-1] = '\\';
				err[i++] = 'x';
				err[i++] = ((unsigned char)c >> 4) + ((((unsigned char)c >> 4) > 9) ? ('a' - 10) : '0');
				err[i++] = (c & 15) + (((c & 15) > 9) ? ('a' - 10) : '0');
				err[i++] = '.';
				err[i++] = '.';
				err[i++] = '.';
				err[i++] = 0;
			}

			soap_error1(E_ERROR,  "Encoding: string '%s' is not a valid utf-8 string", err);
		}
	}
	return str;
}

char* soap_encode_string(zval *data, int* len TSRMLS_DC)
{
	char *str;
	int new_len;

	if (Z_TYPE_P(data) == IS_UNICODE) {
		str = soap_unicode_to_string(Z_USTRVAL_P(data), Z_USTRLEN_P(data) TSRMLS_CC);
		new_len = strlen(str);
	} else {
		if (Z_TYPE_P(data) == IS_STRING) {
			str = estrndup(Z_STRVAL_P(data), Z_STRLEN_P(data));
			new_len = Z_STRLEN_P(data);
		} else {
			zval tmp = *data;

			zval_copy_ctor(&tmp);
			convert_to_string(&tmp);
			str = estrndup(Z_STRVAL(tmp), Z_STRLEN(tmp));
			new_len = Z_STRLEN(tmp);
			zval_dtor(&tmp);
		}

		if (SOAP_GLOBAL(encoding) != NULL) {
			xmlBufferPtr in  = xmlBufferCreateStatic(str, new_len);
			xmlBufferPtr out = xmlBufferCreate();
			int n = xmlCharEncInFunc(SOAP_GLOBAL(encoding), out, in);

			if (n >= 0) {
				efree(str);
				str = estrdup((char*)xmlBufferContent(out));
				new_len = n;
			}
			xmlBufferFree(out);
			xmlBufferFree(in);
		}
		if (!php_libxml_xmlCheckUTF8(BAD_CAST(str))) {
			char *err = emalloc(new_len + 8);
			char c;
			int i;
		
			memcpy(err, str, new_len+1);
			for (i = 0; (c = err[i++]);) {
				if ((c & 0x80) == 0) {
				} else if ((c & 0xe0) == 0xc0) {
					if ((err[i] & 0xc0) != 0x80) {
						break;
					}
					i++;
				} else if ((c & 0xf0) == 0xe0) {
					if ((err[i] & 0xc0) != 0x80 || (err[i+1] & 0xc0) != 0x80) {
						break;
					}
					i += 2;
				} else if ((c & 0xf8) == 0xf0) {
					if ((err[i] & 0xc0) != 0x80 || (err[i+1] & 0xc0) != 0x80 || (err[i+2] & 0xc0) != 0x80) {
						break;
					}
					i += 3;
				} else {
					break;
				}
			}
			if (c) {
				err[i-1] = '\\';
				err[i++] = 'x';
				err[i++] = ((unsigned char)c >> 4) + ((((unsigned char)c >> 4) > 9) ? ('a' - 10) : '0');
				err[i++] = (c & 15) + (((c & 15) > 9) ? ('a' - 10) : '0');
				err[i++] = '.';
				err[i++] = '.';
				err[i++] = '.';
				err[i++] = 0;
			}

			soap_error1(E_ERROR,  "Encoding: string '%s' is not a valid utf-8 string", err);
		}
	}
	if (len) {
		*len = new_len;
	}
	return str;
}

ZEND_INI_MH(OnUpdateCacheEnabled)
{
	if (OnUpdateBool(entry, new_value, new_value_length, mh_arg1, mh_arg2, mh_arg3, stage TSRMLS_CC) == FAILURE) {
		return FAILURE;
	}
	if (SOAP_GLOBAL(cache_enabled)) {
		SOAP_GLOBAL(cache) = SOAP_GLOBAL(cache_mode);
	} else {
		SOAP_GLOBAL(cache) = 0;
	}
	return SUCCESS;
}

ZEND_INI_MH(OnUpdateCacheMode)
{
	char *p;
#ifndef ZTS
	char *base = (char *) mh_arg2;
#else
	char *base = (char *) ts_resource(*((int *) mh_arg2));
#endif

	p = (char*) (base+(size_t) mh_arg1);

	*p = (char)atoi(new_value);

	if (SOAP_GLOBAL(cache_enabled)) {
		SOAP_GLOBAL(cache) = SOAP_GLOBAL(cache_mode);
	} else {
		SOAP_GLOBAL(cache) = 0;
	}
	return SUCCESS;
}

PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("soap.wsdl_cache_enabled",     "1", PHP_INI_ALL, OnUpdateCacheEnabled,
                  cache_enabled, zend_soap_globals, soap_globals)
STD_PHP_INI_ENTRY("soap.wsdl_cache_dir",         "/tmp", PHP_INI_ALL, OnUpdateString,
                  cache_dir, zend_soap_globals, soap_globals)
STD_PHP_INI_ENTRY("soap.wsdl_cache_ttl",         "86400", PHP_INI_ALL, OnUpdateLong,
                  cache_ttl, zend_soap_globals, soap_globals)
STD_PHP_INI_ENTRY("soap.wsdl_cache",             "1", PHP_INI_ALL, OnUpdateCacheMode,
                  cache_mode, zend_soap_globals, soap_globals)
STD_PHP_INI_ENTRY("soap.wsdl_cache_limit",       "5", PHP_INI_ALL, OnUpdateLong,
                  cache_limit, zend_soap_globals, soap_globals)
PHP_INI_END()

static HashTable defEnc, defEncIndex, defEncNs;

static void php_soap_prepare_globals()
{
	int i;
	encodePtr enc;

	zend_hash_init(&defEnc, 0, NULL, NULL, 1);
	zend_hash_init(&defEncIndex, 0, NULL, NULL, 1);
	zend_hash_init(&defEncNs, 0, NULL, NULL, 1);

	i = 0;
	do {
		enc = &defaultEncoding[i];

		/* If has a ns and a str_type then index it */
		if (defaultEncoding[i].details.type_str) {
			if (defaultEncoding[i].details.ns != NULL) {
				char *ns_type;
				spprintf(&ns_type, 0, "%s:%s", defaultEncoding[i].details.ns, defaultEncoding[i].details.type_str);
				zend_hash_add(&defEnc, ns_type, strlen(ns_type) + 1, &enc, sizeof(encodePtr), NULL);
				efree(ns_type);
			} else {
				zend_hash_add(&defEnc, defaultEncoding[i].details.type_str, strlen(defaultEncoding[i].details.type_str) + 1, &enc, sizeof(encodePtr), NULL);
			}
		}
		/* Index everything by number */
		if (!zend_hash_index_exists(&defEncIndex, defaultEncoding[i].details.type)) {
			zend_hash_index_update(&defEncIndex, defaultEncoding[i].details.type, &enc, sizeof(encodePtr), NULL);
		}
		i++;
	} while (defaultEncoding[i].details.type != END_KNOWN_TYPES);

	/* hash by namespace */
	zend_hash_add(&defEncNs, XSD_1999_NAMESPACE, sizeof(XSD_1999_NAMESPACE), XSD_NS_PREFIX, sizeof(XSD_NS_PREFIX), NULL);
	zend_hash_add(&defEncNs, XSD_NAMESPACE, sizeof(XSD_NAMESPACE), XSD_NS_PREFIX, sizeof(XSD_NS_PREFIX), NULL);
	zend_hash_add(&defEncNs, XSI_NAMESPACE, sizeof(XSI_NAMESPACE), XSI_NS_PREFIX, sizeof(XSI_NS_PREFIX), NULL);
	zend_hash_add(&defEncNs, XML_NAMESPACE, sizeof(XML_NAMESPACE), XML_NS_PREFIX, sizeof(XML_NS_PREFIX), NULL);
	zend_hash_add(&defEncNs, SOAP_1_1_ENC_NAMESPACE, sizeof(SOAP_1_1_ENC_NAMESPACE), SOAP_1_1_ENC_NS_PREFIX, sizeof(SOAP_1_1_ENC_NS_PREFIX), NULL);
	zend_hash_add(&defEncNs, SOAP_1_2_ENC_NAMESPACE, sizeof(SOAP_1_2_ENC_NAMESPACE), SOAP_1_2_ENC_NS_PREFIX, sizeof(SOAP_1_2_ENC_NS_PREFIX), NULL);
}

static void php_soap_init_globals(zend_soap_globals *soap_globals TSRMLS_DC)
{
	soap_globals->defEnc = defEnc;
	soap_globals->defEncIndex = defEncIndex;
	soap_globals->defEncNs = defEncNs;
	soap_globals->typemap = NULL;
	soap_globals->use_soap_error_handler = 0;
	soap_globals->error_code = NULL;
	soap_globals->error_object = NULL;
	soap_globals->sdl = NULL;
	soap_globals->soap_version = SOAP_1_1;
	soap_globals->mem_cache = NULL;
	soap_globals->ref_map = NULL;
}

PHP_MSHUTDOWN_FUNCTION(soap)
{
	zend_error_cb = old_error_handler;
	zend_hash_destroy(&SOAP_GLOBAL(defEnc));
	zend_hash_destroy(&SOAP_GLOBAL(defEncIndex));
	zend_hash_destroy(&SOAP_GLOBAL(defEncNs));
	if (SOAP_GLOBAL(mem_cache)) {
		zend_hash_destroy(SOAP_GLOBAL(mem_cache));
		free(SOAP_GLOBAL(mem_cache));
	}
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

PHP_RINIT_FUNCTION(soap)
{
	SOAP_GLOBAL(typemap) = NULL;
	SOAP_GLOBAL(use_soap_error_handler) = 0;
	SOAP_GLOBAL(error_code) = NULL;
	SOAP_GLOBAL(error_object) = NULL;
	SOAP_GLOBAL(sdl) = NULL;
	SOAP_GLOBAL(soap_version) = SOAP_1_1;
	SOAP_GLOBAL(encoding) = NULL;
	SOAP_GLOBAL(class_map) = NULL;
	SOAP_GLOBAL(features) = 0;
	SOAP_GLOBAL(ref_map) = NULL;
	return SUCCESS;
}

static void soap_server_dtor(void *object, zend_object_handle handle TSRMLS_DC)
{
	soap_server_object *service = (soap_server_object*)object;

	if (service->soap_functions.ft) {
		zend_hash_destroy(service->soap_functions.ft);
		efree(service->soap_functions.ft);
	}

	if (service->typemap) {
		zend_hash_destroy(service->typemap);
		efree(service->typemap);
	}

	if (service->soap_class.argc) {
		int i;
		for (i = 0; i < service->soap_class.argc;i++) {
			zval_ptr_dtor(&service->soap_class.argv[i]);
		}
		efree(service->soap_class.argv);
	}

	if (service->actor) {
		efree(service->actor);
	}
	if (service->uri) {
		efree(service->uri);
	}
	if (service->sdl) {
		delete_sdl(service->sdl);
	}
	if (service->encoding) {
		xmlCharEncCloseFunc(service->encoding);
	}
	if (service->class_map) {
		zend_hash_destroy(service->class_map);
		FREE_HASHTABLE(service->class_map);
	}
	if (service->soap_object) {
		zval_ptr_dtor(&service->soap_object);
	}
	zend_object_std_dtor(object TSRMLS_CC);
	efree(object);
}

static zend_object_value soap_server_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value new_value;
	soap_server_object *obj = (soap_server_object*)emalloc(sizeof(soap_server_object));

	memset(obj, 0, sizeof(soap_server_object));

	zend_object_std_init(&obj->zo, ce TSRMLS_CC);

	obj->sdl = NULL;
	obj->uri = NULL;
	obj->typemap = NULL;
	obj->version = SOAP_1_1;
	obj->class_map = NULL;
	obj->features = 0;
	obj->encoding = NULL;

	obj->soap_functions.ft = NULL;
	obj->soap_functions.functions_all = 1;

	obj->soap_class.ce = NULL;
	obj->soap_class.argv = NULL;
	obj->soap_class.argc = 0;
	obj->soap_class.persistance = SOAP_PERSISTENCE_REQUEST;

	obj->type = SOAP_MAP_FUNCTION;
	obj->actor = NULL;
	obj->soap_headers_ptr = NULL;

	new_value.handle = zend_objects_store_put(obj, soap_server_dtor, NULL, NULL TSRMLS_CC);
	new_value.handlers = zend_get_std_object_handlers();

	return new_value;
}

static void soap_client_dtor(void *object, zend_object_handle handle TSRMLS_DC)
{
	soap_client_object *client = (soap_client_object*)object;

	if (client->typemap) {
		zend_hash_destroy(client->typemap);
		efree(client->typemap);
	}
	if (client->uri) {
		efree(client->uri);
	}
	if (client->sdl) {
		delete_sdl(client->sdl);
	}
	if (client->encoding) {
		xmlCharEncCloseFunc(client->encoding);
	}
	if (client->class_map) {
		zend_hash_destroy(client->class_map);
		FREE_HASHTABLE(client->class_map);
	}
	if (client->location) {
		efree(client->location);
	}
	if (client->login) {
		efree(client->login);
	}
	if (client->password) {
		efree(client->password);
	}
	if (client->digest_realm) {
		efree(client->digest_realm);
	}
	if (client->digest_algorithm) {
		efree(client->digest_algorithm);
	}
	if (client->digest_nonce) {
		efree(client->digest_nonce);
	}
	if (client->digest_qop) {
		efree(client->digest_qop);
	}
	if (client->digest_opaque) {
		efree(client->digest_opaque);
	}
	if (client->proxy_host) {
		efree(client->proxy_host);
	}
	if (client->proxy_login) {
		efree(client->proxy_login);
	}
	if (client->proxy_password) {
		efree(client->proxy_password);
	}
	if (client->user_agent) {
		efree(client->user_agent);
	}

	if (client->last_request_headers) {
		efree(client->last_request_headers);
	}
	if (client->last_request) {
		efree(client->last_request);
	}
	if (client->last_response_headers) {
		efree(client->last_response_headers);
	}
	if (client->last_response) {
		efree(client->last_response);
	}
	if (client->stream) {
		php_stream_close(client->stream);
	}
	if (client->url) {
		php_url_free(client->url);
	}
	if (client->default_headers) {
		zval_ptr_dtor(&client->default_headers);
	}
	if (client->cookies) {
		zval_ptr_dtor(&client->cookies);
	}
	if (client->fault) {
		zval_ptr_dtor(&client->fault);
	}

	zend_object_std_dtor(object TSRMLS_CC);
	efree(object);
}

static zend_object_value soap_client_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value new_value;
	soap_client_object *obj = (soap_client_object*)emalloc(sizeof(soap_client_object));

	memset(obj, 0, sizeof(soap_client_object));

	zend_object_std_init(&obj->zo, ce TSRMLS_CC);

	obj->sdl = NULL;
	obj->uri = NULL;
	obj->typemap = NULL;
	obj->version = SOAP_1_1;
	obj->class_map = NULL;
	obj->features = 0;
	obj->encoding = NULL;

	obj->use = SOAP_ENCODED;
	obj->style = SOAP_RPC;

	obj->exceptions = 1;

	new_value.handle = zend_objects_store_put(obj, soap_client_dtor, NULL, NULL TSRMLS_CC);
	new_value.handlers = zend_get_std_object_handlers();

	return new_value;
}

PHP_MINIT_FUNCTION(soap)
{
	zend_class_entry ce;

	/* TODO: add ini entry for always use soap errors */
	php_soap_prepare_globals();
	ZEND_INIT_MODULE_GLOBALS(soap, php_soap_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	/* Register SoapClient class */
	/* BIG NOTE : THIS EMITS AN COMPILATION WARNING UNDER ZE2 - handle_function_call deprecated.
		soap_call_function_handler should be of type struct _zend_function, not (*handle_function_call).
	*/
	{
		zend_internal_function fe;

		fe.type = ZEND_INTERNAL_FUNCTION;
		fe.handler = ZEND_MN(SoapClient___call);
		fe.function_name.v = NULL;
		fe.scope = NULL;
		fe.fn_flags = 0;
		fe.prototype = NULL;
		fe.num_args = 2;
		fe.arg_info = NULL;
		fe.pass_rest_by_reference = 0;

		INIT_OVERLOADED_CLASS_ENTRY(ce, PHP_SOAP_CLIENT_CLASSNAME, soap_client_functions,
			(zend_function *)&fe, NULL, NULL);
		soap_client_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
		soap_client_class_entry->create_object = soap_client_ctor;
	}

	/* Register SoapVar class */
	INIT_CLASS_ENTRY(ce, PHP_SOAP_VAR_CLASSNAME, soap_var_functions);
	soap_var_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	/* Register SoapServer class */
	INIT_CLASS_ENTRY(ce, PHP_SOAP_SERVER_CLASSNAME, soap_server_functions);
	soap_server_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	soap_server_class_entry->create_object = soap_server_ctor;

	/* Register SoapFault class */
	INIT_CLASS_ENTRY(ce, PHP_SOAP_FAULT_CLASSNAME, soap_fault_functions);
	soap_fault_class_entry = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);

	/* Register SoapParam class */
	INIT_CLASS_ENTRY(ce, PHP_SOAP_PARAM_CLASSNAME, soap_param_functions);
	soap_param_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, PHP_SOAP_HEADER_CLASSNAME, soap_header_functions);
	soap_header_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	REGISTER_LONG_CONSTANT("SOAP_1_1", SOAP_1_1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_1_2", SOAP_1_2, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SOAP_PERSISTENCE_SESSION", SOAP_PERSISTENCE_SESSION, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_PERSISTENCE_REQUEST", SOAP_PERSISTENCE_REQUEST, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_FUNCTIONS_ALL", SOAP_FUNCTIONS_ALL, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SOAP_ENCODED", SOAP_ENCODED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_LITERAL", SOAP_LITERAL, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SOAP_RPC", SOAP_RPC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_DOCUMENT", SOAP_DOCUMENT, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SOAP_ACTOR_NEXT", SOAP_ACTOR_NEXT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_ACTOR_NONE", SOAP_ACTOR_NONE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_ACTOR_UNLIMATERECEIVER", SOAP_ACTOR_UNLIMATERECEIVER, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SOAP_COMPRESSION_ACCEPT", SOAP_COMPRESSION_ACCEPT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_COMPRESSION_GZIP", SOAP_COMPRESSION_GZIP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_COMPRESSION_DEFLATE", SOAP_COMPRESSION_DEFLATE, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SOAP_AUTHENTICATION_BASIC", SOAP_AUTHENTICATION_BASIC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_AUTHENTICATION_DIGEST", SOAP_AUTHENTICATION_DIGEST, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("UNKNOWN_TYPE", UNKNOWN_TYPE, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("XSD_STRING", XSD_STRING, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_BOOLEAN", XSD_BOOLEAN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_DECIMAL", XSD_DECIMAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_FLOAT", XSD_FLOAT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_DOUBLE", XSD_DOUBLE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_DURATION", XSD_DURATION, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_DATETIME", XSD_DATETIME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_TIME", XSD_TIME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_DATE", XSD_DATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_GYEARMONTH", XSD_GYEARMONTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_GYEAR", XSD_GYEAR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_GMONTHDAY", XSD_GMONTHDAY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_GDAY", XSD_GDAY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_GMONTH", XSD_GMONTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_HEXBINARY", XSD_HEXBINARY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_BASE64BINARY", XSD_BASE64BINARY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ANYURI", XSD_ANYURI, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_QNAME", XSD_QNAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NOTATION", XSD_NOTATION, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NORMALIZEDSTRING", XSD_NORMALIZEDSTRING, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_TOKEN", XSD_TOKEN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_LANGUAGE", XSD_LANGUAGE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NMTOKEN", XSD_NMTOKEN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NAME", XSD_NAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NCNAME", XSD_NCNAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ID", XSD_ID, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_IDREF", XSD_IDREF, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_IDREFS", XSD_IDREFS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ENTITY", XSD_ENTITY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ENTITIES", XSD_ENTITIES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_INTEGER", XSD_INTEGER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NONPOSITIVEINTEGER", XSD_NONPOSITIVEINTEGER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NEGATIVEINTEGER", XSD_NEGATIVEINTEGER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_LONG", XSD_LONG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_INT", XSD_INT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_SHORT", XSD_SHORT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_BYTE", XSD_BYTE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NONNEGATIVEINTEGER", XSD_NONNEGATIVEINTEGER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_UNSIGNEDLONG", XSD_UNSIGNEDLONG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_UNSIGNEDINT", XSD_UNSIGNEDINT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_UNSIGNEDSHORT", XSD_UNSIGNEDSHORT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_UNSIGNEDBYTE", XSD_UNSIGNEDBYTE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_POSITIVEINTEGER", XSD_POSITIVEINTEGER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NMTOKENS", XSD_NMTOKENS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ANYTYPE", XSD_ANYTYPE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ANYXML", XSD_ANYXML, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("APACHE_MAP", APACHE_MAP, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SOAP_ENC_OBJECT", SOAP_ENC_OBJECT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_ENC_ARRAY", SOAP_ENC_ARRAY, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("XSD_1999_TIMEINSTANT", XSD_1999_TIMEINSTANT, CONST_CS | CONST_PERSISTENT);

	REGISTER_STRING_CONSTANT("XSD_NAMESPACE", XSD_NAMESPACE, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("XSD_1999_NAMESPACE", XSD_1999_NAMESPACE,  CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SOAP_SINGLE_ELEMENT_ARRAYS", SOAP_SINGLE_ELEMENT_ARRAYS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_WAIT_ONE_WAY_CALLS", SOAP_WAIT_ONE_WAY_CALLS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_USE_XSI_ARRAY_TYPE", SOAP_USE_XSI_ARRAY_TYPE, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("WSDL_CACHE_NONE",   WSDL_CACHE_NONE,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("WSDL_CACHE_DISK",   WSDL_CACHE_DISK,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("WSDL_CACHE_MEMORY", WSDL_CACHE_MEMORY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("WSDL_CACHE_BOTH",   WSDL_CACHE_BOTH,   CONST_CS | CONST_PERSISTENT);

	old_error_handler = zend_error_cb;
	zend_error_cb = soap_error_handler;

	return SUCCESS;
}

PHP_MINFO_FUNCTION(soap)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Soap Client", "enabled");
	php_info_print_table_row(2, "Soap Server", "enabled");
	php_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}

/* {{{ proto object SoapParam::SoapParam ( mixed data, string name) U
   SoapParam constructor */
PHP_METHOD(SoapParam, SoapParam)
{
	zval *data;
	zstr name;
	int name_length;
	zend_uchar name_type;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zt", &data, &name, &name_length, &name_type) == FAILURE) {
		return;
	}
	if (name_length == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid parameter name");
		return;
	}

	if (name_type == IS_STRING) {
		add_property_stringl(this_ptr, "param_name", name.s, name_length, 1);
	} else {
		add_property_unicodel(this_ptr, "param_name", name.u, name_length, 1);
	}
	add_property_zval(this_ptr, "param_data", data);
}
/* }}} */


/* {{{ proto object SoapHeader::SoapHeader ( string namespace, string name [, mixed data [, bool mustUnderstand [, mixed actor]]]) U
   SoapHeader constructor */
PHP_METHOD(SoapHeader, SoapHeader)
{
	zval *data = NULL, *actor = NULL;
	zstr name, ns;
	int name_len, ns_len;
	zend_uchar name_type, ns_type;
	zend_bool must_understand = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "tt|zbz",
			&ns, &ns_len, &ns_type, &name, &name_len, &name_type,
			&data, &must_understand, &actor) == FAILURE) {
		return;
	}
	if (ns_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid namespace");
		return;
	}
	if (name_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid header name");
		return;
	}

	if (ns_type == IS_STRING) {
		add_property_stringl(this_ptr, "namespace", ns.s, ns_len, 1);
	} else {
		add_property_unicodel(this_ptr, "namespace", ns.u, ns_len, 1);
	}
	if (name_type == IS_STRING) {
		add_property_stringl(this_ptr, "name", name.s, name_len, 1);
	} else {
		add_property_unicodel(this_ptr, "name", name.u, name_len, 1);
	}
	if (data) {
		add_property_zval(this_ptr, "data", data);
	}
	add_property_bool(this_ptr, "mustUnderstand", must_understand);
	if (actor == NULL) {
	} else if (Z_TYPE_P(actor) == IS_LONG &&
	  (Z_LVAL_P(actor) == SOAP_ACTOR_NEXT ||
	   Z_LVAL_P(actor) == SOAP_ACTOR_NONE ||
	   Z_LVAL_P(actor) == SOAP_ACTOR_UNLIMATERECEIVER)) {
		add_property_long(this_ptr, "actor", Z_LVAL_P(actor));
	} else if (Z_TYPE_P(actor) == IS_STRING && Z_STRLEN_P(actor) > 0) {
		add_property_stringl(this_ptr, "actor", Z_STRVAL_P(actor), Z_STRLEN_P(actor), 1);
	} else if (Z_TYPE_P(actor) == IS_UNICODE && Z_USTRLEN_P(actor) > 0) {
		add_property_unicodel(this_ptr, "actor", Z_USTRVAL_P(actor), Z_USTRLEN_P(actor), 1);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid actor");
		return;
	}
}

/* {{{ proto object SoapFault::SoapFault ( string faultcode, string faultstring [, string faultactor [, mixed detail [, string faultname [, mixed headerfault]]]]) U
   SoapFault constructor */
PHP_METHOD(SoapFault, SoapFault)
{
	zstr fault_string = NULL_ZSTR;
	zstr fault_actor = NULL_ZSTR;
	zstr name = NULL_ZSTR;
	char *fault_code = NULL, *fault_code_ns = NULL;
	int fault_string_len = 0, fault_actor_len = 0, name_len = 0;
	zval *code = NULL, *details = NULL, *headerfault = NULL;
	zend_uchar name_type = 0, fault_string_type = 0, fault_actor_type = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zt|t!z!t!z",
		&code,
		&fault_string, &fault_string_len, &fault_string_type,
		&fault_actor, &fault_actor_len, &fault_actor_type,
		&details,
		&name, &name_len, &name_type,
		&headerfault) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(code) == IS_NULL) {
	} else if (Z_TYPE_P(code) == IS_STRING || Z_TYPE_P(code) == IS_UNICODE) {
		fault_code = soap_encode_string(code, NULL TSRMLS_CC);
	} else if (Z_TYPE_P(code) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(code)) == 2) {
		zval **t_ns, **t_code;

		zend_hash_internal_pointer_reset(Z_ARRVAL_P(code));
		zend_hash_get_current_data(Z_ARRVAL_P(code), (void**)&t_ns);
		zend_hash_move_forward(Z_ARRVAL_P(code));
		zend_hash_get_current_data(Z_ARRVAL_P(code), (void**)&t_code);
		if ((Z_TYPE_PP(t_ns) == IS_STRING || Z_TYPE_PP(t_ns) == IS_UNICODE) &&
		    (Z_TYPE_PP(t_code) == IS_STRING || Z_TYPE_PP(t_code) == IS_UNICODE)) {
			fault_code_ns = soap_encode_string(*t_ns, NULL TSRMLS_CC);
			fault_code = soap_encode_string(*t_code, NULL TSRMLS_CC);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid fault code");
			return;
		}
	} else  {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid fault code");
		return;
	}
	if (fault_code != NULL && !fault_code[0]) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid fault code");
		return;
	}
	if (name.v != NULL && name_len == 0) {
		name.v = NULL;
	}
	if (name.v) {
		name.s = soap_encode_string_ex(name_type, name, name_len TSRMLS_CC);
	}
	if (fault_string.v) {
		fault_string.s = soap_encode_string_ex(fault_string_type, fault_string, fault_string_len TSRMLS_CC);
	}
	if (fault_actor.v) {
		fault_actor.s = soap_encode_string_ex(fault_actor_type, fault_actor, fault_actor_len TSRMLS_CC);
	}

	set_soap_fault(this_ptr, fault_code_ns, fault_code, fault_string.s, fault_actor.s, details, name.s TSRMLS_CC);
	if (headerfault != NULL) {
		add_property_zval(this_ptr, "headerfault", headerfault);
	}
	if (fault_code) {
		efree(fault_code);
	}
	if (fault_code_ns) {
		efree(fault_code_ns);
	}
	if (name.s) {
		efree(name.s);
	}
	if (fault_string.s) {
		efree(fault_string.s);
	}
	if (fault_actor.s) {
		efree(fault_actor.s);
	}
}
/* }}} */


/* {{{ proto object SoapFault::__toString () U
 */
PHP_METHOD(SoapFault, __toString)
{
	zval *faultcode, *faultstring, *file, *line, *trace;
	char *str;
	int len;
	zend_fcall_info fci;
	zval fname;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	faultcode   = zend_read_property(soap_fault_class_entry, this_ptr, "faultcode", sizeof("faultcode")-1, 1 TSRMLS_CC);
	faultstring = zend_read_property(soap_fault_class_entry, this_ptr, "faultstring", sizeof("faultstring")-1, 1 TSRMLS_CC);
	file = zend_read_property(soap_fault_class_entry, this_ptr, "file", sizeof("file")-1, 1 TSRMLS_CC);
	line = zend_read_property(soap_fault_class_entry, this_ptr, "line", sizeof("line")-1, 1 TSRMLS_CC);

	ZVAL_ASCII_STRINGL(&fname, "gettraceasstring", sizeof("gettraceasstring")-1, 1);

	fci.size = sizeof(fci);
	fci.function_table = &Z_OBJCE_P(getThis())->function_table;
	fci.function_name = &fname;
	fci.symbol_table = NULL;
	fci.object_ptr = getThis();
	fci.retval_ptr_ptr = &trace;
	fci.param_count = 0;
	fci.params = NULL;
	fci.no_separation = 1;

	zend_call_function(&fci, NULL TSRMLS_CC);
	zval_dtor(&fname);

	len = spprintf(&str, 0, "SoapFault exception: [%R] %R in %s:%ld\nStack trace:\n%s",
	               Z_TYPE_P(faultcode), Z_UNIVAL_P(faultcode),
	               Z_TYPE_P(faultstring), Z_UNIVAL_P(faultstring),
	               Z_STRVAL_P(file), Z_LVAL_P(line),
	               Z_STRLEN_P(trace) ? Z_STRVAL_P(trace) : "#0 {main}\n");

	zval_ptr_dtor(&trace);

	RETVAL_STRINGL(str, len, 0);
	zval_string_to_unicode(return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto object SoapVar::SoapVar ( mixed data, int encoding [, string type_name [, string type_namespace [, string node_name [, string node_namespace]]]]) U
   SoapVar constructor */
PHP_METHOD(SoapVar, SoapVar)
{
	zval *data, *type;
	zstr stype = NULL_ZSTR, ns = NULL_ZSTR, name = NULL_ZSTR, namens = NULL_ZSTR;
	int stype_len = 0, ns_len = 0, name_len = 0, namens_len = 0;
	zend_uchar stype_type = 0, ns_type = 0, name_type = 0, namens_type = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z!z|tttt",
			&data, &type,
			&stype, &stype_len, &stype_type,
			&ns, &ns_len, &ns_type,
			&name, &name_len, &name_type,
			&namens, &namens_len, &namens_type) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(type) == IS_NULL) {
		add_property_long(this_ptr, "enc_type", UNKNOWN_TYPE);
	} else {
		if (zend_hash_index_exists(&SOAP_GLOBAL(defEncIndex), Z_LVAL_P(type))) {
			add_property_long(this_ptr, "enc_type", Z_LVAL_P(type));
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid type ID");
			return;
		}
	}

	if (data) {
		add_property_zval(this_ptr, "enc_value", data);
	}

	if (stype.v && stype_len > 0) {
		if (stype_type == IS_STRING) {
			add_property_stringl(this_ptr, "enc_stype", stype.s, stype_len, 1);
		} else if (stype_type == IS_UNICODE) {
			add_property_unicodel(this_ptr, "enc_stype", stype.u, stype_len, 1);
		}
	}
	if (ns.v && ns_len > 0) {
		if (ns_type == IS_STRING) {
			add_property_stringl(this_ptr, "enc_ns", ns.s, ns_len, 1);
		} else if (ns_type == IS_UNICODE) {
			add_property_unicodel(this_ptr, "enc_ns", ns.u, ns_len, 1);
		}
	}
	if (name.v && name_len > 0) {
		if (name_type == IS_STRING) {
			add_property_stringl(this_ptr, "enc_name", name.s, name_len, 1);
		} else if (name_type == IS_UNICODE) {
			add_property_unicodel(this_ptr, "enc_name", name.u, name_len, 1);
		}
	}
	if (namens.v && namens_len > 0) {
		if (namens_type == IS_STRING) {
			add_property_stringl(this_ptr, "enc_namens", namens.s, namens_len, 1);
		} else if (namens_type == IS_UNICODE) {
			add_property_unicodel(this_ptr, "enc_namens", namens.u, namens_len, 1);
		}
	}
}
/* }}} */


static HashTable* soap_create_typemap(sdlPtr sdl, HashTable *ht TSRMLS_DC)
{
	zval **tmp;
	HashTable *ht2;
	HashPosition pos1, pos2;
	HashTable *typemap = NULL;
	
	zend_hash_internal_pointer_reset_ex(ht, &pos1);
	while (zend_hash_get_current_data_ex(ht, (void**)&tmp, &pos1) == SUCCESS) {
		char *type_name = NULL;
		char *type_ns = NULL;
		char *free_type_name = NULL;
		char *free_type_ns = NULL;
		zval *to_xml = NULL;
		zval *to_zval = NULL;
		encodePtr enc, new_enc;

		if (Z_TYPE_PP(tmp) != IS_ARRAY) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Wrong 'typemap' option");
			return NULL;
		}
		ht2 = Z_ARRVAL_PP(tmp);

		zend_hash_internal_pointer_reset_ex(ht2, &pos2);
		while (zend_hash_get_current_data_ex(ht2, (void**)&tmp, &pos2) == SUCCESS) {
			zend_uchar name_type;
			zstr name;
			unsigned int name_len;
			ulong index;

			name_type = zend_hash_get_current_key_ex(ht2, &name, &name_len, &index, 0, &pos2);
			if (name_type == IS_STRING || name_type == IS_UNICODE) {
				if (ZEND_U_EQUAL(name_type, name, name_len, "type_name", sizeof("type_name"))) {
					if (Z_TYPE_PP(tmp) == IS_STRING) {
						type_name = Z_STRVAL_PP(tmp);
					} else if (Z_TYPE_PP(tmp) == IS_UNICODE) {
						free_type_name = type_name = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);;
					} else if (Z_TYPE_PP(tmp) != IS_NULL) {
					}
				} else if (ZEND_U_EQUAL(name_type, name, name_len, "type_ns", sizeof("type_ns"))) {
					if (Z_TYPE_PP(tmp) == IS_STRING) {
						type_ns = Z_STRVAL_PP(tmp);
					} else if (Z_TYPE_PP(tmp) == IS_UNICODE) {
						free_type_ns = type_ns = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);;
					} else if (Z_TYPE_PP(tmp) != IS_NULL) {
					}
				} else if (ZEND_U_EQUAL(name_type, name, name_len, "to_xml", sizeof("to_xml"))) {
					to_xml = *tmp;
				} else if (ZEND_U_EQUAL(name_type, name, name_len, "from_xml", sizeof("from_xml"))) {
					to_zval = *tmp;
				}
			}
			zend_hash_move_forward_ex(ht2, &pos2);
		}		

		if (type_name) {
			smart_str nscat = {0};

			if (type_ns) {
				enc = get_encoder(sdl, type_ns, type_name);
			} else {
				enc = get_encoder_ex(sdl, type_name, strlen(type_name));
			}

			new_enc = emalloc(sizeof(encode));
			memset(new_enc, 0, sizeof(encode));

			if (enc) {
				new_enc->details.type = enc->details.type;
				new_enc->details.ns = estrdup(enc->details.ns);
				new_enc->details.type_str = estrdup(enc->details.type_str);
				new_enc->details.sdl_type = enc->details.sdl_type;
			} else {
				enc = get_conversion(UNKNOWN_TYPE);
				new_enc->details.type = enc->details.type;
				if (type_ns) {
					new_enc->details.ns = estrdup(type_ns);
				}
				new_enc->details.type_str = estrdup(type_name);
			}
			new_enc->to_xml = enc->to_xml;
			new_enc->to_zval = enc->to_zval;
			new_enc->details.map = emalloc(sizeof(soapMapping));
			memset(new_enc->details.map, 0, sizeof(soapMapping));			
			if (to_xml) {
				zval_add_ref(&to_xml);
				new_enc->details.map->to_xml = to_xml;
				new_enc->to_xml = to_xml_user;
			} else if (enc->details.map && enc->details.map->to_xml) {
				zval_add_ref(&enc->details.map->to_xml);
				new_enc->details.map->to_xml = enc->details.map->to_xml;
			}
			if (to_zval) {
				zval_add_ref(&to_zval);
				new_enc->details.map->to_zval = to_zval;
				new_enc->to_zval = to_zval_user;
			} else if (enc->details.map && enc->details.map->to_zval) {
				zval_add_ref(&enc->details.map->to_zval);
				new_enc->details.map->to_zval = enc->details.map->to_zval;
			}
			if (!typemap) {
				typemap = emalloc(sizeof(HashTable));
				zend_hash_init(typemap, 0, NULL, delete_encoder, 0);
			}

			if (type_ns) {
				smart_str_appends(&nscat, type_ns);
				smart_str_appendc(&nscat, ':');
			}
			smart_str_appends(&nscat, type_name);
			smart_str_0(&nscat);
			zend_hash_update(typemap, nscat.c, nscat.len + 1, &new_enc, sizeof(encodePtr), NULL);
			smart_str_free(&nscat);
		}

		if (free_type_name) {
			efree(free_type_name);
		}
		if (free_type_ns) {
			efree(free_type_ns);
		}

		zend_hash_move_forward_ex(ht, &pos1);
	}
	return typemap;
}


/* {{{ proto object SoapServer::SoapServer ( mixed wsdl [, array options]) U
   SoapServer constructor */
PHP_METHOD(SoapServer, SoapServer)
{
	soap_server_object *service;
	zval *options = NULL;
	zstr zwsdl = NULL_ZSTR;
	int zwsdl_len;
	zend_uchar zwsdl_type;
	char *wsdl = NULL;
	int version = SOAP_1_1;
	zend_bool cache_wsdl;
	HashTable *typemap_ht = NULL;

	SOAP_SERVER_BEGIN_CODE();

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "t!|a", &zwsdl, &zwsdl_len, &zwsdl_type, &options) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Invalid parameters");
	}

	if (zwsdl.v) {
		if (zwsdl_type == IS_STRING) {
			wsdl = estrndup(zwsdl.s, zwsdl_len);
		} else {
			wsdl = soap_unicode_to_string(zwsdl.u, zwsdl_len TSRMLS_CC);
		}
	}

	service = (soap_server_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	service->send_errors = 1;

	cache_wsdl = SOAP_GLOBAL(cache);

	if (options != NULL) {
		HashTable *ht = Z_ARRVAL_P(options);
		zval **tmp;

		if (zend_ascii_hash_find(ht, "soap_version", sizeof("soap_version"), (void**)&tmp) == SUCCESS) {
			if (Z_TYPE_PP(tmp) == IS_LONG ||
			    (Z_LVAL_PP(tmp) == SOAP_1_1 && Z_LVAL_PP(tmp) == SOAP_1_2)) {
				version = Z_LVAL_PP(tmp);
			}
		}

		if (zend_ascii_hash_find(ht, "uri", sizeof("uri"), (void**)&tmp) == SUCCESS) {
			if (Z_TYPE_PP(tmp) == IS_STRING) {
				service->uri = estrndup(Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
			} else if (Z_TYPE_PP(tmp) == IS_UNICODE) {
				service->uri = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
			}
		}

		if (zend_ascii_hash_find(ht, "actor", sizeof("actor"), (void**)&tmp) == SUCCESS) {
			if (Z_TYPE_PP(tmp) == IS_STRING) {
				service->actor = estrndup(Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
			} else if (Z_TYPE_PP(tmp) == IS_UNICODE) {
				service->actor = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
			}
		}

		if (zend_ascii_hash_find(ht, "encoding", sizeof("encoding"), (void**)&tmp) == SUCCESS &&
		    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
			xmlCharEncodingHandlerPtr encoding;
			char *str;

			if (Z_TYPE_PP(tmp) == IS_UNICODE) {
				str = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
			} else {
				str = Z_STRVAL_PP(tmp);
			}
			encoding = xmlFindCharEncodingHandler(str);
			if (encoding == NULL) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Invalid arguments. Invalid 'encoding' option - '%v'", Z_TYPE_PP(tmp), Z_UNIVAL_PP(tmp));
			} else {
				service->encoding = encoding;
			}
			if (Z_TYPE_PP(tmp) == IS_UNICODE) {
				efree(str);
			}
		}

		if (zend_ascii_hash_find(ht, "classmap", sizeof("classmap"), (void**)&tmp) == SUCCESS &&
			Z_TYPE_PP(tmp) == IS_ARRAY) {
			zval *ztmp;

			ALLOC_HASHTABLE(service->class_map);
			zend_u_hash_init(service->class_map, zend_hash_num_elements((*tmp)->value.ht), NULL, ZVAL_PTR_DTOR, 0, UG(unicode));
			zend_hash_copy(service->class_map, (*tmp)->value.ht, (copy_ctor_func_t) zval_add_ref, (void *) &ztmp, sizeof(zval *));
		}

		if (zend_ascii_hash_find(ht, "typemap", sizeof("typemap"), (void**)&tmp) == SUCCESS &&
			Z_TYPE_PP(tmp) == IS_ARRAY &&
			zend_hash_num_elements(Z_ARRVAL_PP(tmp)) > 0) {
			typemap_ht = Z_ARRVAL_PP(tmp);
		}

		if (zend_ascii_hash_find(ht, "features", sizeof("features"), (void**)&tmp) == SUCCESS &&
			Z_TYPE_PP(tmp) == IS_LONG) {
			service->features = Z_LVAL_PP(tmp);
		}

		if (zend_ascii_hash_find(ht, "cache_wsdl", sizeof("cache_wsdl"), (void**)&tmp) == SUCCESS &&
		    Z_TYPE_PP(tmp) == IS_LONG) {
			cache_wsdl = Z_LVAL_PP(tmp);
		}

		if (zend_ascii_hash_find(ht, "send_errors", sizeof("send_errors"), (void**)&tmp) == SUCCESS &&
		    (Z_TYPE_PP(tmp) == IS_BOOL || Z_TYPE_PP(tmp) == IS_LONG)) {
			service->send_errors = Z_LVAL_PP(tmp);
		}
	}

	if (wsdl == NULL && service->uri == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "'uri' option is required in nonWSDL mode");
	}


	service->version = version;
	service->type = SOAP_FUNCTIONS;
	service->soap_functions.functions_all = FALSE;
	service->soap_functions.ft = emalloc(sizeof(HashTable));
	zend_u_hash_init(service->soap_functions.ft, 0, NULL, ZVAL_PTR_DTOR, 0, UG(unicode));

	if (wsdl) {
		service->sdl = get_sdl(NULL, wsdl, cache_wsdl TSRMLS_CC);
		if (service->uri == NULL) {
			if (service->sdl->target_ns) {
				service->uri = estrdup(service->sdl->target_ns);
			} else {
				/*FIXME*/
				service->uri = estrdup("http://unknown-uri/");
			}
		}
		efree(wsdl);
	}

	if (typemap_ht) {
		service->typemap = soap_create_typemap(service->sdl, typemap_ht TSRMLS_CC);
	}

	SOAP_SERVER_END_CODE();
}
/* }}} */


/* {{{ proto object SoapServer::setPersistence ( int mode ) U
   Sets persistence mode of SoapServer */
PHP_METHOD(SoapServer, setPersistence)
{
	soap_server_object *service;
	long value;

	SOAP_SERVER_BEGIN_CODE();

	service = (soap_server_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &value) != FAILURE) {
		if (service->type == SOAP_CLASS) {
			if (value == SOAP_PERSISTENCE_SESSION ||
				value == SOAP_PERSISTENCE_REQUEST) {
				service->soap_class.persistance = value;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Tried to set persistence with bogus value (%ld)", value);
				return;
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Tried to set persistence when you are using you SOAP SERVER in function mode, no persistence needed");
			return;
		}
	}

	SOAP_SERVER_END_CODE();
}
/* }}} */


/* {{{ proto void SoapServer::setClass(string class_name [, mixed args]) U
   Sets class which will handle SOAP requests */
PHP_METHOD(SoapServer, setClass)
{
	soap_server_object *service;
	zend_class_entry **ce;
	int found, argc;
	zval ***argv;

	SOAP_SERVER_BEGIN_CODE();

	service = (soap_server_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);

	argc = ZEND_NUM_ARGS();
	argv = safe_emalloc(argc, sizeof(zval **), 0);

	if (argc < 1 || zend_get_parameters_array_ex(argc, argv) == FAILURE) {
		efree(argv);
		WRONG_PARAM_COUNT;
	}

	if (Z_TYPE_PP(argv[0]) == IS_STRING || Z_TYPE_PP(argv[0]) == IS_UNICODE) {
		found = zend_u_lookup_class(Z_TYPE_PP(argv[0]), Z_UNIVAL_PP(argv[0]), Z_UNILEN_PP(argv[0]), &ce TSRMLS_CC);
		if (found != FAILURE) {
			service->type = SOAP_CLASS;
			service->soap_class.ce = *ce;
			service->soap_class.persistance = SOAP_PERSISTENCE_REQUEST;
			service->soap_class.argc = argc - 1;
			if (service->soap_class.argc > 0) {
				int i;
				service->soap_class.argv = safe_emalloc(sizeof(zval), service->soap_class.argc, 0);
				for (i = 0;i < service->soap_class.argc;i++) {
					service->soap_class.argv[i] = *(argv[i + 1]);
					zval_add_ref(&service->soap_class.argv[i]);
				}
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Tried to set a non existant class (%s)", Z_STRVAL_PP(argv[0]));
			return;
		}
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "You must pass in a string");
		return;
	}

	efree(argv);

	SOAP_SERVER_END_CODE();
}
/* }}} */


/* {{{ proto void SoapServer::setObject(object)
   Sets object which will handle SOAP requests */
PHP_METHOD(SoapServer, setObject)
{
	soap_server_object *service;
	zval *obj;

	SOAP_SERVER_BEGIN_CODE();
	service = (soap_server_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &obj) == FAILURE) {
		return;
	}

	service->type = SOAP_OBJECT;

	MAKE_STD_ZVAL(service->soap_object);
	*service->soap_object = *obj;
	zval_copy_ctor(service->soap_object);
	INIT_PZVAL(service->soap_object);

	SOAP_SERVER_END_CODE();
}
/* }}} */


/* {{{ proto array SoapServer::getFunctions(void) U
   Returns list of defined functions */
PHP_METHOD(SoapServer, getFunctions)
{
	soap_server_object *service;
	HashTable *ft = NULL;

	SOAP_SERVER_BEGIN_CODE();

	ZERO_PARAM()
	service = (soap_server_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);

	array_init(return_value);
	if (service->type == SOAP_OBJECT) {
		ft = &(Z_OBJCE_P(service->soap_object)->function_table);
	} else if (service->type == SOAP_CLASS) {
		ft = &service->soap_class.ce->function_table;
	} else if (service->soap_functions.functions_all == TRUE) {
		ft = EG(function_table);
	} else if (service->soap_functions.ft != NULL) {
		zval **name;
		HashPosition pos;

		zend_hash_internal_pointer_reset_ex(service->soap_functions.ft, &pos);
		while (zend_hash_get_current_data_ex(service->soap_functions.ft, (void **)&name, &pos) != FAILURE) {
			add_next_index_unicode(return_value, Z_UNIVAL_PP(name).u, 1);
			zend_hash_move_forward_ex(service->soap_functions.ft, &pos);
		}
	}
	if (ft != NULL) {
		zend_function *f;
		HashPosition pos;
		zend_hash_internal_pointer_reset_ex(ft, &pos);
		while (zend_hash_get_current_data_ex(ft, (void **)&f, &pos) != FAILURE) {
			if ((service->type != SOAP_OBJECT && service->type != SOAP_CLASS) || (f->common.fn_flags & ZEND_ACC_PUBLIC)) {
				add_next_index_unicode(return_value, f->common.function_name.u, 1);
			}
			zend_hash_move_forward_ex(ft, &pos);
		}
	}

	SOAP_SERVER_END_CODE();
}
/* }}} */


/* {{{ proto void SoapServer::addFunction(mixed functions) U
   Adds one or several functions those will handle SOAP requests */
PHP_METHOD(SoapServer, addFunction)
{
	soap_server_object *service;
	zval *function_name, *function_copy;
	HashPosition pos;

	SOAP_SERVER_BEGIN_CODE();

	service = (soap_server_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &function_name) == FAILURE) {
		return;
	}

	/* TODO: could use zend_is_callable here */

	if (function_name->type == IS_ARRAY) {
		if (service->type == SOAP_FUNCTIONS) {
			zval **tmp_function, *function_copy;

			if (service->soap_functions.ft == NULL) {
				service->soap_functions.functions_all = FALSE;
				service->soap_functions.ft = emalloc(sizeof(HashTable));
				zend_u_hash_init(service->soap_functions.ft, 0, NULL, ZVAL_PTR_DTOR, 0, UG(unicode));
			}

			zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(function_name), &pos);
			while (zend_hash_get_current_data_ex(Z_ARRVAL_P(function_name), (void **)&tmp_function, &pos) != FAILURE) {
				zstr key;
				unsigned int key_len;
				zend_function *f;

				if (Z_TYPE_PP(tmp_function) != IS_STRING &&
				    Z_TYPE_PP(tmp_function) != IS_UNICODE) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Tried to add a function that isn't a string");
					return;
				}

				key = zend_u_str_case_fold(Z_TYPE_PP(tmp_function), Z_UNIVAL_PP(tmp_function), Z_UNILEN_PP(tmp_function), 0, &key_len);

				if (zend_u_hash_find(EG(function_table), Z_TYPE_PP(tmp_function), key, key_len+1, (void**)&f) == FAILURE) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Tried to add a non existant function '%s'", Z_STRVAL_PP(tmp_function));
					return;
				}

				MAKE_STD_ZVAL(function_copy);
				ZVAL_UNICODE(function_copy, f->common.function_name.u, 1);
				zend_u_hash_update(service->soap_functions.ft, Z_TYPE_PP(tmp_function), key, key_len+1, &function_copy, sizeof(zval *), NULL);

				efree(key.v);
				zend_hash_move_forward_ex(Z_ARRVAL_P(function_name), &pos);
			}
		}
	} else if (Z_TYPE_P(function_name) == IS_STRING ||
	           Z_TYPE_P(function_name) == IS_UNICODE) {
		zstr key;
		unsigned int key_len;
		zend_function *f;

		key = zend_u_str_case_fold(Z_TYPE_P(function_name), Z_UNIVAL_P(function_name), Z_UNILEN_P(function_name), 0, &key_len);
		if (zend_u_hash_find(EG(function_table), Z_TYPE_P(function_name), key, key_len+1, (void**)&f) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Tried to add a non existant function '%s'", Z_STRVAL_P(function_name));
			return;
		}
		if (service->soap_functions.ft == NULL) {
			service->soap_functions.functions_all = FALSE;
			service->soap_functions.ft = emalloc(sizeof(HashTable));
			zend_u_hash_init(service->soap_functions.ft, 0, NULL, ZVAL_PTR_DTOR, 0, UG(unicode));
		}

		MAKE_STD_ZVAL(function_copy);
		ZVAL_UNICODE(function_copy, f->common.function_name.u, 1);
		zend_u_hash_update(service->soap_functions.ft, Z_TYPE_P(function_name), key, key_len+1, &function_copy, sizeof(zval *), NULL);
		efree(key.v);
	} else if (function_name->type == IS_LONG) {
		if (Z_LVAL_P(function_name) == SOAP_FUNCTIONS_ALL) {
			if (service->soap_functions.ft != NULL) {
				zend_hash_destroy(service->soap_functions.ft);
				efree(service->soap_functions.ft);
				service->soap_functions.ft = NULL;
			}
			service->soap_functions.functions_all = TRUE;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid value passed");
			return;
		}
	}

	SOAP_SERVER_END_CODE();
}
/* }}} */


/* {{{ proto void SoapServer::handle ( [string soap_request]) U
   Handles a SOAP request */
PHP_METHOD(SoapServer, handle)
{
	int soap_version, old_soap_version;
	sdlPtr old_sdl = NULL;
	soap_server_object *service;
	xmlDocPtr doc_request=NULL, doc_return;
	zval function_name, **params, **raw_post, *soap_obj, *retval;
	char *fn_name, cont_len[30];
	int num_params = 0, size, i, call_status = 0;
	xmlChar *buf;
	HashTable *function_table;
	soapHeader *soap_headers = NULL;
	sdlFunctionPtr function;
	zstr arg = NULL_ZSTR;
	int arg_len = 0;
	xmlCharEncodingHandlerPtr old_encoding;
	HashTable *old_class_map, *old_typemap;
	int old_features;
	zend_uchar arg_type;

	SOAP_SERVER_BEGIN_CODE();

	service = (soap_server_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	SOAP_GLOBAL(soap_version) = service->version;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|t", &arg, &arg_len, &arg_type) == FAILURE) {
		return;
	}

	if (SG(request_info).request_method &&
	    strcmp(SG(request_info).request_method, "GET") == 0 &&
	    SG(request_info).query_string &&
	    stricmp(SG(request_info).query_string, "wsdl") == 0) {

		if (service->sdl) {
/*
			char *hdr = emalloc(sizeof("Location: ")+strlen(service->sdl->source));
			strcpy(hdr,"Location: ");
			strcat(hdr,service->sdl->source);
			sapi_add_header(hdr, sizeof("Location: ")+strlen(service->sdl->source)-1, 1);
			efree(hdr);
*/
			zval readfile, readfile_ret, *param;

			INIT_ZVAL(readfile);
			INIT_ZVAL(readfile_ret);
			MAKE_STD_ZVAL(param);

			sapi_add_header("Content-Type: text/xml; charset=utf-8", sizeof("Content-Type: text/xml; charset=utf-8")-1, 1);
			ZVAL_STRING(param, service->sdl->source, 1);
			ZVAL_STRING(&readfile, "readfile", 1);
			if (call_user_function(EG(function_table), NULL, &readfile, &readfile_ret, 1, &param  TSRMLS_CC) == FAILURE) {
				soap_server_fault("Server", "Couldn't find WSDL", NULL, NULL, NULL TSRMLS_CC);
			}

			zval_ptr_dtor(&param);
			zval_dtor(&readfile);
			zval_dtor(&readfile_ret);

			SOAP_SERVER_END_CODE();
			return;
		} else {
			soap_server_fault("Server", "WSDL generation is not supported yet", NULL, NULL, NULL TSRMLS_CC);
/*
			sapi_add_header("Content-Type: text/xml; charset=utf-8", sizeof("Content-Type: text/xml; charset=utf-8"), 1);
			PUTS("<?xml version=\"1.0\" ?>\n<definitions\n");
			PUTS("    xmlns=\"http://schemas.xmlsoap.org/wsdl/\"\n");
			PUTS("    targetNamespace=\"");
			PUTS(service->uri);
			PUTS("\">\n");
			PUTS("</definitions>");
*/
			SOAP_SERVER_END_CODE();
			return;
		}
	}

	ALLOC_INIT_ZVAL(retval);

	if (php_output_start_default(TSRMLS_C) != SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,"ob_start failed");
	}

	if (ZEND_NUM_ARGS() == 0) {
		if (zend_ascii_hash_find(&EG(symbol_table), HTTP_RAW_POST_DATA, sizeof(HTTP_RAW_POST_DATA), (void **) &raw_post) != FAILURE &&
			(Z_TYPE_PP(raw_post) == IS_STRING ||
			 Z_TYPE_PP(raw_post) == IS_UNICODE)) {
			if (Z_TYPE_PP(raw_post) == IS_STRING) {
				zval **server_vars, **encoding;

				zend_is_auto_global("_SERVER", sizeof("_SERVER")-1 TSRMLS_CC);
				if (zend_ascii_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void **) &server_vars) == SUCCESS &&
				    Z_TYPE_PP(server_vars) == IS_ARRAY &&
				    zend_ascii_hash_find(Z_ARRVAL_PP(server_vars), "HTTP_CONTENT_ENCODING", sizeof("HTTP_CONTENT_ENCODING"), (void **) &encoding)==SUCCESS &&
				    (Z_TYPE_PP(encoding) == IS_STRING || Z_TYPE_PP(encoding) == IS_UNICODE)) {
					zval func;
					zval retval;
					zval param;
					zval *params[1];

					if ((ZEND_U_EQUAL(Z_TYPE_PP(encoding), Z_UNIVAL_PP(encoding), Z_UNILEN_PP(encoding), "gzip", sizeof("gzip")-1) ||
					     ZEND_U_EQUAL(Z_TYPE_PP(encoding), Z_UNIVAL_PP(encoding), Z_UNILEN_PP(encoding), "x-gzip", sizeof("x-gzip")-1)) &&
					    zend_hash_exists(EG(function_table), "gzinflate", sizeof("gzinflate"))) {
						ZVAL_STRING(&func, "gzinflate", 0);
						params[0] = &param;
						ZVAL_STRINGL(params[0], Z_STRVAL_PP(raw_post)+10, Z_STRLEN_PP(raw_post)-10, 0);
						INIT_PZVAL(params[0]);
					} else if (ZEND_U_EQUAL(Z_TYPE_PP(encoding), Z_UNIVAL_PP(encoding), Z_UNILEN_PP(encoding), "deflate", sizeof("deflate")-1) &&
					           zend_hash_exists(EG(function_table), "gzuncompress", sizeof("gzuncompress"))) {
						ZVAL_STRING(&func, "gzuncompress", 0);
						params[0] = &param;
						ZVAL_STRINGL(params[0], Z_STRVAL_PP(raw_post), Z_STRLEN_PP(raw_post), 0);
						INIT_PZVAL(params[0]);
					} else {
						php_error_docref(NULL TSRMLS_CC, E_WARNING,"Request is compressed with unknown compression '%s'",Z_STRVAL_PP(encoding));
						return;
					}
					if (call_user_function(CG(function_table), (zval**)NULL, &func, &retval, 1, params TSRMLS_CC) == SUCCESS &&
					    Z_TYPE(retval) == IS_STRING) {
						doc_request = soap_xmlParseMemory(Z_STRVAL(retval),Z_STRLEN(retval));
						zval_dtor(&retval);
					} else {
						php_error_docref(NULL TSRMLS_CC, E_WARNING,"Can't uncompress compressed request");
						return;
					}
				} else {
					doc_request = soap_xmlParseMemory(Z_STRVAL_PP(raw_post),Z_STRLEN_PP(raw_post));
				}
			} else {
				/* unicode */
				/* TODO: remove unicode->string conversion */
				int str_req_len;
				char *str_req = soap_encode_string(*raw_post, &str_req_len TSRMLS_CC);
				doc_request = soap_xmlParseMemory(str_req, str_req_len);
				efree(str_req);

			}
		} else {
			if (SG(request_info).request_method &&
	    		strcmp(SG(request_info).request_method, "POST") == 0) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "PHP-SOAP requires 'always_populate_raw_post_data' to be on please check your php.ini file");
				return;
			}
			soap_server_fault("Server", "Bad Request. Can't find HTTP_RAW_POST_DATA", NULL, NULL, NULL TSRMLS_CC);
			return;
		}
	} else if (arg_type == IS_UNICODE) {
		/* TODO: remove unicode->string conversion */
		char *str_req = soap_unicode_to_string(arg.u, arg_len TSRMLS_CC);

		doc_request = soap_xmlParseMemory(str_req, strlen(str_req));
		efree(str_req);
	} else {
		doc_request = soap_xmlParseMemory(arg.s, arg_len);
	}

	if (doc_request == NULL) {
		soap_server_fault("Client", "Bad Request", NULL, NULL, NULL TSRMLS_CC);
	}
	if (xmlGetIntSubset(doc_request) != NULL) {
		xmlNodePtr env = get_node(doc_request->children,"Envelope");
		if (env && env->ns) {
			if (strcmp((char*)env->ns->href, SOAP_1_1_ENV_NAMESPACE) == 0) {
				SOAP_GLOBAL(soap_version) = SOAP_1_1;
			} else if (strcmp((char*)env->ns->href,SOAP_1_2_ENV_NAMESPACE) == 0) {
				SOAP_GLOBAL(soap_version) = SOAP_1_2;
			}
		}
		xmlFreeDoc(doc_request);
		soap_server_fault("Server", "DTD are not supported by SOAP", NULL, NULL, NULL TSRMLS_CC);
	}

	old_sdl = SOAP_GLOBAL(sdl);
	SOAP_GLOBAL(sdl) = service->sdl;
	old_encoding = SOAP_GLOBAL(encoding);
	SOAP_GLOBAL(encoding) = service->encoding;
	old_class_map = SOAP_GLOBAL(class_map);
	SOAP_GLOBAL(class_map) = service->class_map;
	old_typemap = SOAP_GLOBAL(typemap);
	SOAP_GLOBAL(typemap) = service->typemap;
	old_features = SOAP_GLOBAL(features);
	SOAP_GLOBAL(features) = service->features;
	old_soap_version = SOAP_GLOBAL(soap_version);
	function = deserialize_function_call(service->sdl, doc_request, service->actor, &function_name, &num_params, &params, &soap_version, &soap_headers TSRMLS_CC);
	xmlFreeDoc(doc_request);

#ifdef ZEND_ENGINE_2
	if (EG(exception)) {
		php_output_discard(TSRMLS_C);
		if (Z_TYPE_P(EG(exception)) == IS_OBJECT &&
		    instanceof_function(Z_OBJCE_P(EG(exception)), soap_fault_class_entry TSRMLS_CC)) {
			soap_server_fault_ex(function, EG(exception), NULL TSRMLS_CC);
		}
		goto fail;
	}
#endif

	service->soap_headers_ptr = &soap_headers;

	soap_obj = NULL;
	if (service->type == SOAP_OBJECT) {
		soap_obj = service->soap_object;
		function_table = &((Z_OBJCE_P(soap_obj))->function_table);
	} else if (service->type == SOAP_CLASS) {
#if HAVE_PHP_SESSION && !defined(COMPILE_DL_SESSION)
		/* If persistent then set soap_obj from from the previous created session (if available) */
		if (service->soap_class.persistance == SOAP_PERSISTENCE_SESSION) {
			zval **tmp_soap;

			if (PS(session_status) != php_session_active &&
			    PS(session_status) != php_session_disabled) {
				php_session_start(TSRMLS_C);
			}

			/* Find the soap object and assign */
			if (zend_ascii_hash_find(Z_ARRVAL_P(PS(http_session_vars)), "_bogus_session_name", sizeof("_bogus_session_name"), (void **) &tmp_soap) == SUCCESS &&
			    Z_TYPE_PP(tmp_soap) == IS_OBJECT &&
			    Z_OBJCE_PP(tmp_soap) == service->soap_class.ce) {
				soap_obj = *tmp_soap;
			}
		}
#endif
		/* If new session or something wierd happned */
		if (soap_obj == NULL) {
			zval *tmp_soap;
			zend_function *constructor_fn;

			MAKE_STD_ZVAL(tmp_soap);
			object_init_ex(tmp_soap, service->soap_class.ce);

			/* Call constructor */
			if ((constructor_fn = zend_std_get_constructor(tmp_soap TSRMLS_CC))) {
				zval c_ret, constructor;

				INIT_ZVAL(c_ret);
				INIT_ZVAL(constructor);

				ZVAL_UNICODE(&constructor, constructor_fn->common.function_name.u, 1);
				if (call_user_function(NULL, &tmp_soap, &constructor, &c_ret, service->soap_class.argc, service->soap_class.argv TSRMLS_CC) == FAILURE) {
					php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error calling constructor");
				}
				if (EG(exception)) {
					php_output_discard(TSRMLS_C);
					if (Z_TYPE_P(EG(exception)) == IS_OBJECT &&
					    instanceof_function(Z_OBJCE_P(EG(exception)), soap_fault_class_entry TSRMLS_CC)) {
						soap_server_fault_ex(function, EG(exception), NULL TSRMLS_CC);
					}
					zval_dtor(&constructor);
					zval_dtor(&c_ret);
					zval_ptr_dtor(&tmp_soap);
					goto fail;
				}
				zval_dtor(&constructor);
				zval_dtor(&c_ret);
			} else {
				/* FIXME: Unicode support??? */
				int class_name_len = strlen(service->soap_class.ce->name.s);
				char *class_name = emalloc(class_name_len+1);

				memcpy(class_name, service->soap_class.ce->name.s, class_name_len+1);
				if (zend_hash_exists(&Z_OBJCE_P(tmp_soap)->function_table, php_strtolower(class_name, class_name_len), class_name_len+1)) {
					zval c_ret, constructor;

					INIT_ZVAL(c_ret);
					INIT_ZVAL(constructor);

					ZVAL_UNICODE(&constructor, service->soap_class.ce->name.u, 1);
					if (call_user_function(NULL, &tmp_soap, &constructor, &c_ret, service->soap_class.argc, service->soap_class.argv TSRMLS_CC) == FAILURE) {
						php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error calling constructor");
					}
					if (EG(exception)) {
						php_output_discard(TSRMLS_C);
						if (Z_TYPE_P(EG(exception)) == IS_OBJECT &&
						    instanceof_function(Z_OBJCE_P(EG(exception)), soap_fault_class_entry TSRMLS_CC)) {
							soap_server_fault_ex(function, EG(exception), NULL TSRMLS_CC);
						}
						zval_dtor(&constructor);
						zval_dtor(&c_ret);
						efree(class_name);
						zval_ptr_dtor(&tmp_soap);
						goto fail;
					}
					zval_dtor(&constructor);
					zval_dtor(&c_ret);
				}
				efree(class_name);
			}
#if HAVE_PHP_SESSION && !defined(COMPILE_DL_SESSION)
			/* If session then update session hash with new object */
			if (service->soap_class.persistance == SOAP_PERSISTENCE_SESSION) {
				zval **tmp_soap_pp;
				if (zend_ascii_hash_update(Z_ARRVAL_P(PS(http_session_vars)), "_bogus_session_name", sizeof("_bogus_session_name"), &tmp_soap, sizeof(zval *), (void **)&tmp_soap_pp) == SUCCESS) {
					soap_obj = *tmp_soap_pp;
				}
			} else {
				soap_obj = tmp_soap;
			}
#else
			soap_obj = tmp_soap;
#endif

		}
		function_table = &((Z_OBJCE_P(soap_obj))->function_table);
	} else {
		if (service->soap_functions.functions_all == TRUE) {
			function_table = EG(function_table);
		} else {
			function_table = service->soap_functions.ft;
		}
	}

	doc_return = NULL;

	/* Process soap headers */
	if (soap_headers != NULL) {
		soapHeader *header = soap_headers;
		while (header != NULL) {
			soapHeader *h = header;

			header = header->next;
			if (service->sdl && !h->function && !h->hdr) {
				if (h->mustUnderstand) {
					soap_server_fault("MustUnderstand","Header not understood", NULL, NULL, NULL TSRMLS_CC);
				} else {
					continue;
				}
			}

			fn_name = estrndup(Z_STRVAL(h->function_name),Z_STRLEN(h->function_name));
			if (zend_hash_exists(function_table, php_strtolower(fn_name, Z_STRLEN(h->function_name)), Z_STRLEN(h->function_name) + 1) ||
			    ((service->type == SOAP_CLASS || service->type == SOAP_OBJECT) &&
			     zend_hash_exists(function_table, ZEND_CALL_FUNC_NAME, sizeof(ZEND_CALL_FUNC_NAME)))) {
				if (service->type == SOAP_CLASS || service->type == SOAP_OBJECT) {
					call_status = call_user_function(NULL, &soap_obj, &h->function_name, &h->retval, h->num_params, h->parameters TSRMLS_CC);
				} else {
					call_status = call_user_function(EG(function_table), NULL, &h->function_name, &h->retval, h->num_params, h->parameters TSRMLS_CC);
				}
				if (call_status != SUCCESS) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Function '%s' call failed", Z_STRVAL(h->function_name));
					return;
				}
				if (Z_TYPE(h->retval) == IS_OBJECT &&
				    instanceof_function(Z_OBJCE(h->retval), soap_fault_class_entry TSRMLS_CC)) {
					zval *headerfault = NULL, **tmp;

					if (zend_hash_find(Z_OBJPROP(h->retval), "headerfault", sizeof("headerfault"), (void**)&tmp) == SUCCESS &&
					    Z_TYPE_PP(tmp) != IS_NULL) {
						headerfault = *tmp;
					}
					php_output_discard(TSRMLS_C);
					soap_server_fault_ex(function, &h->retval, h TSRMLS_CC);
					efree(fn_name);
					if (service->type == SOAP_CLASS && soap_obj) {zval_ptr_dtor(&soap_obj);}
					goto fail;
				} else if (EG(exception)) {
					php_output_discard(TSRMLS_C);
					if (Z_TYPE_P(EG(exception)) == IS_OBJECT &&
					    instanceof_function(Z_OBJCE_P(EG(exception)), soap_fault_class_entry TSRMLS_CC)) {
						zval *headerfault = NULL, **tmp;

						if (zend_hash_find(Z_OBJPROP_P(EG(exception)), "headerfault", sizeof("headerfault"), (void**)&tmp) == SUCCESS &&
						    Z_TYPE_PP(tmp) != IS_NULL) {
							headerfault = *tmp;
						}
						soap_server_fault_ex(function, EG(exception), h TSRMLS_CC);
					}
					efree(fn_name);
					if (service->type == SOAP_CLASS && soap_obj) {zval_ptr_dtor(&soap_obj);}
					goto fail;
				}
			} else if (h->mustUnderstand) {
				soap_server_fault("MustUnderstand","Header not understood", NULL, NULL, NULL TSRMLS_CC);
			}
			efree(fn_name);
		}
	}

	fn_name = estrndup(Z_STRVAL(function_name),Z_STRLEN(function_name));
	if (zend_hash_exists(function_table, php_strtolower(fn_name, Z_STRLEN(function_name)), Z_STRLEN(function_name) + 1) ||
	    ((service->type == SOAP_CLASS || service->type == SOAP_OBJECT) &&
	     zend_hash_exists(function_table, ZEND_CALL_FUNC_NAME, sizeof(ZEND_CALL_FUNC_NAME)))) {
		if (service->type == SOAP_CLASS || service->type == SOAP_OBJECT) {
			call_status = call_user_function(NULL, &soap_obj, &function_name, retval, num_params, params TSRMLS_CC);
			if (service->type == SOAP_CLASS) {
#if HAVE_PHP_SESSION && !defined(COMPILE_DL_SESSION)
				if (service->soap_class.persistance != SOAP_PERSISTENCE_SESSION) {
					zval_ptr_dtor(&soap_obj);
					soap_obj = NULL;
				}
#else
				zval_ptr_dtor(&soap_obj);
				soap_obj = NULL;
#endif
			}
		} else {
			call_status = call_user_function(EG(function_table), NULL, &function_name, retval, num_params, params TSRMLS_CC);
		}
	} else {
		php_error(E_ERROR, "Function '%s' doesn't exist", Z_STRVAL(function_name));
	}
	efree(fn_name);

	if (EG(exception)) {
		php_output_discard(TSRMLS_C);
		if (Z_TYPE_P(EG(exception)) == IS_OBJECT &&
		    instanceof_function(Z_OBJCE_P(EG(exception)), soap_fault_class_entry TSRMLS_CC)) {
			soap_server_fault_ex(function, EG(exception), NULL TSRMLS_CC);
		}
		if (service->type == SOAP_CLASS) {
#if HAVE_PHP_SESSION && !defined(COMPILE_DL_SESSION)
			if (soap_obj && service->soap_class.persistance != SOAP_PERSISTENCE_SESSION) {
#else
			if (soap_obj) {
#endif
			  zval_ptr_dtor(&soap_obj);
			}
		}
		goto fail;
	}
	if (call_status == SUCCESS) {
		char *response_name;

		if (Z_TYPE_P(retval) == IS_OBJECT &&
		    instanceof_function(Z_OBJCE_P(retval), soap_fault_class_entry TSRMLS_CC)) {
			php_output_discard(TSRMLS_C);
			soap_server_fault_ex(function, retval, NULL TSRMLS_CC);
			goto fail;
		}

		if (function && function->responseName) {
			response_name = estrdup(function->responseName);
		} else {
			response_name = emalloc(Z_STRLEN(function_name) + sizeof("Response"));
			memcpy(response_name,Z_STRVAL(function_name),Z_STRLEN(function_name));
			memcpy(response_name+Z_STRLEN(function_name),"Response",sizeof("Response"));
		}
		doc_return = serialize_response_call(function, response_name, service->uri, retval, soap_headers, soap_version TSRMLS_CC);
		efree(response_name);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Function '%s' call failed", Z_STRVAL(function_name));
		return;
	}

#ifdef ZEND_ENGINE_2
	if (EG(exception)) {
		php_output_discard(TSRMLS_C);
		if (Z_TYPE_P(EG(exception)) == IS_OBJECT &&
		    instanceof_function(Z_OBJCE_P(EG(exception)), soap_fault_class_entry TSRMLS_CC)) {
			soap_server_fault_ex(function, EG(exception), NULL TSRMLS_CC);
		}
		if (service->type == SOAP_CLASS) {
#if HAVE_PHP_SESSION && !defined(COMPILE_DL_SESSION)
			if (soap_obj && service->soap_class.persistance != SOAP_PERSISTENCE_SESSION) {
#else
			if (soap_obj) {
#endif
			  zval_ptr_dtor(&soap_obj);
			}
		}
		goto fail;
	}
#endif

	/* Flush buffer */
	php_output_discard(TSRMLS_C);

	if (doc_return) {
		UConverter *old_runtime_conv, *old_output_conv;

		/* xmlDocDumpMemoryEnc(doc_return, &buf, &size, XML_CHAR_ENCODING_UTF8); */
		xmlDocDumpMemory(doc_return, &buf, &size);

		if (size == 0) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Dump memory failed");
		}

		if (soap_version == SOAP_1_2) {
			sapi_add_header("Content-Type: application/soap+xml; charset=utf-8", sizeof("Content-Type: application/soap+xml; charset=utf-8")-1, 1);
		} else {
			sapi_add_header("Content-Type: text/xml; charset=utf-8", sizeof("Content-Type: text/xml; charset=utf-8")-1, 1);
		}

		xmlFreeDoc(doc_return);
		old_runtime_conv = UG(runtime_encoding_conv);
		old_output_conv = UG(output_encoding_conv);
		UG(runtime_encoding_conv) = UG(utf8_conv);
		UG(output_encoding_conv) = UG(utf8_conv);

		if (zend_ini_long("zlib.output_compression", sizeof("zlib.output_compression"), 0)) {
			sapi_add_header("Connection: close", sizeof("Connection: close")-1, 1);
		} else {
			snprintf(cont_len, sizeof(cont_len), "Content-Length: %d", size);
			sapi_add_header(cont_len, strlen(cont_len), 1);
		}
		php_write(buf, size TSRMLS_CC);
		xmlFree(buf);

		UG(runtime_encoding_conv) = old_runtime_conv;
		UG(output_encoding_conv) = old_output_conv;
	} else {
		sapi_add_header("HTTP/1.1 202 Accepted", sizeof("HTTP/1.1 202 Accepted")-1, 1);
		sapi_add_header("Content-Length: 0", sizeof("Content-Length: 0")-1, 1);
	}

fail:
	SOAP_GLOBAL(soap_version) = old_soap_version;
	SOAP_GLOBAL(encoding) = old_encoding;
	SOAP_GLOBAL(sdl) = old_sdl;
	SOAP_GLOBAL(class_map) = old_class_map;
	SOAP_GLOBAL(typemap) = old_typemap;
	SOAP_GLOBAL(features) = old_features;

	/* Free soap headers */
	zval_ptr_dtor(&retval);
	while (soap_headers != NULL) {
		soapHeader *h = soap_headers;
		int i;

		soap_headers = soap_headers->next;
		if (h->parameters) {
			i = h->num_params;
			while (i > 0) {
				zval_ptr_dtor(&h->parameters[--i]);
			}
			efree(h->parameters);
		}
		zval_dtor(&h->function_name);
		zval_dtor(&h->retval);
		efree(h);
	}
	service->soap_headers_ptr = NULL;

	/* Free Memory */
	if (num_params > 0) {
		for (i = 0; i < num_params;i++) {
			zval_ptr_dtor(&params[i]);
		}
		efree(params);
	}
	zval_dtor(&function_name);

	SOAP_SERVER_END_CODE();
}
/* }}} */


/* {{{ proto SoapServer::fault ( staring code, string string [, string actor [, mixed details [, string name]]] ) U
   Issue SoapFault indicating an error */
PHP_METHOD(SoapServer, fault)
{
	zstr code, string;
	zstr actor=NULL_ZSTR;
	zstr name=NULL_ZSTR;
	int code_len, string_len, actor_len = 0, name_len = 0;
	zend_uchar code_type, string_type, actor_type = 0, name_type = 0;
	zval* details = NULL;

	SOAP_SERVER_BEGIN_CODE();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "tt|tzt",
	    &code, &code_len, &code_type,
	    &string, &string_len, &string_type,
	    &actor, &actor_len, &actor_type,
	    &details,
	    &name, &name_len, &name_type) == FAILURE) {
		return;
	}

	if (code.v) {
		code.s = soap_encode_string_ex(code_type, code, code_len TSRMLS_CC);
	}
	if (name.v) {
		name.s = soap_encode_string_ex(name_type, name, name_len TSRMLS_CC);
	}
	if (string.v) {
		string.s = soap_encode_string_ex(string_type, string, string_len TSRMLS_CC);
	}
	if (actor.v) {
		actor.s = soap_encode_string_ex(actor_type, actor, actor_len TSRMLS_CC);
	}
	soap_server_fault(code.s, string.s, actor.s, details, name.s TSRMLS_CC);
	if (code.s) {
		efree(code.s);
	}
	if (name.s) {
		efree(name.s);
	}
	if (string.s) {
		efree(string.s);
	}
	if (actor.s) {
		efree(actor.s);
	}
	SOAP_SERVER_END_CODE();
}
/* }}} */

/* {{{ proto SoapServer::addSoapHeader ( SoapHeader header ) U
   Adds one SOAP header into response */
PHP_METHOD(SoapServer, addSoapHeader)
{
	soap_server_object *service;
	zval *fault;
	soapHeader **p;

	SOAP_SERVER_BEGIN_CODE();

	service = (soap_server_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);

	if (!service || !service->soap_headers_ptr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The SoapServer::addSoapHeader function may be called only during SOAP request processing");
		return;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &fault, soap_header_class_entry) == FAILURE) {
		return;
	}

	p = service->soap_headers_ptr;
	while (*p != NULL) {
		p = &(*p)->next;
	}
	*p = emalloc(sizeof(soapHeader));
	memset(*p, 0, sizeof(soapHeader));
	ZVAL_NULL(&(*p)->function_name);
	(*p)->retval = *fault;
	zval_copy_ctor(&(*p)->retval);

	SOAP_SERVER_END_CODE();
}
/* }}} */

static void soap_server_fault_ex(sdlFunctionPtr function, zval* fault, soapHeader *hdr TSRMLS_DC)
{
	int soap_version;
	xmlChar *buf;
	char cont_len[30];
	int size;
	xmlDocPtr doc_return;
	UConverter *old_runtime_conv, *old_output_conv;
	zval **agent_name;
	int use_http_error_status = 1;

	soap_version = SOAP_GLOBAL(soap_version);

	doc_return = serialize_response_call(function, NULL, NULL, fault, hdr, soap_version TSRMLS_CC);

	xmlDocDumpMemory(doc_return, &buf, &size);

	zend_is_auto_global("_SERVER", sizeof("_SERVER") - 1 TSRMLS_CC);
	if (PG(http_globals)[TRACK_VARS_SERVER] &&
		zend_ascii_hash_find(PG(http_globals)[TRACK_VARS_SERVER]->value.ht, "HTTP_USER_AGENT", sizeof("HTTP_USER_AGENT"), (void **) &agent_name) == SUCCESS &&
		Z_TYPE_PP(agent_name) == IS_STRING) {
		if (strncmp(Z_STRVAL_PP(agent_name), "Shockwave Flash", sizeof("Shockwave Flash")-1) == 0) {
			use_http_error_status = 0;
		}
	}
	/*
	   Want to return HTTP 500 but apache wants to over write
	   our fault code with their own handling... Figure this out later
	*/
	if (use_http_error_status) {
		sapi_add_header("HTTP/1.1 500 Internal Service Error", sizeof("HTTP/1.1 500 Internal Service Error")-1, 1);
	}
	if (zend_ini_long("zlib.output_compression", sizeof("zlib.output_compression"), 0)) {
		sapi_add_header("Connection: close", sizeof("Connection: close")-1, 1);
	} else {
		snprintf(cont_len, sizeof(cont_len), "Content-Length: %d", size);
		sapi_add_header(cont_len, strlen(cont_len), 1);
	}
	if (soap_version == SOAP_1_2) {
		sapi_add_header("Content-Type: application/soap+xml; charset=utf-8", sizeof("Content-Type: application/soap+xml; charset=utf-8")-1, 1);
	} else {
		sapi_add_header("Content-Type: text/xml; charset=utf-8", sizeof("Content-Type: text/xml; charset=utf-8")-1, 1);
	}

	old_runtime_conv = UG(runtime_encoding_conv);
	old_output_conv = UG(output_encoding_conv);
	UG(runtime_encoding_conv) = UG(utf8_conv);
	UG(output_encoding_conv) = UG(utf8_conv);
	php_write(buf, size TSRMLS_CC);
	UG(runtime_encoding_conv) = old_runtime_conv;
	UG(output_encoding_conv) = old_output_conv;

	xmlFreeDoc(doc_return);
	xmlFree(buf);
	zend_clear_exception(TSRMLS_C);
}

static void soap_server_fault(char* code, char* string, char *actor, zval* details, char* name TSRMLS_DC)
{
	zval ret;

	INIT_ZVAL(ret);

	set_soap_fault(&ret, NULL, code, string, actor, details, name TSRMLS_CC);
	/* TODO: Which function */
	soap_server_fault_ex(NULL, &ret, NULL TSRMLS_CC);
	zend_bailout();
}

static void soap_error_handler(int error_num, const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
	zend_bool _old_in_compilation, _old_in_execution;
	zend_execute_data *_old_current_execute_data;
	int _old_http_response_code;
	char *_old_http_status_line;
	TSRMLS_FETCH();

	_old_in_compilation = CG(in_compilation);
	_old_in_execution = EG(in_execution);
	_old_current_execute_data = EG(current_execute_data);
	_old_http_response_code = SG(sapi_headers).http_response_code;
	_old_http_status_line = SG(sapi_headers).http_status_line;

	if (!SOAP_GLOBAL(use_soap_error_handler)) {
		call_old_error_handler(error_num, error_filename, error_lineno, format, args);
		return;
	}

	if (SOAP_GLOBAL(error_object) &&
	    Z_TYPE_P(SOAP_GLOBAL(error_object)) == IS_OBJECT &&
	    instanceof_function(Z_OBJCE_P(SOAP_GLOBAL(error_object)), soap_client_class_entry TSRMLS_CC)) {
		soap_client_object *client;

		client = (soap_client_object*)zend_object_store_get_object(SOAP_GLOBAL(error_object) TSRMLS_CC);
		if ((error_num == E_USER_ERROR ||
		     error_num == E_COMPILE_ERROR ||
		     error_num == E_CORE_ERROR ||
		     error_num == E_ERROR ||
		     error_num == E_PARSE) &&
		    client->exceptions) {
			zval *fault, *exception;
			char* code = SOAP_GLOBAL(error_code);
			char *buffer;
			int buffer_len;
			zval outbuf, outbuflen;
#ifdef va_copy
			va_list argcopy;
#endif
			zend_object_store_bucket *old_objects;
			int old = PG(display_errors);

			INIT_ZVAL(outbuf);
			INIT_ZVAL(outbuflen);
#ifdef va_copy
			va_copy(argcopy, args);
			buffer_len = vspprintf(&buffer, 0, format, argcopy);
			va_end(argcopy);
#else
			buffer_len = vspprintf(&buffer, 0, format, args);
#endif

			if (code == NULL) {
				code = "Client";
			}
			fault = add_soap_fault(SOAP_GLOBAL(error_object), code, buffer, NULL, NULL TSRMLS_CC);
			efree(buffer);
			MAKE_STD_ZVAL(exception);
			*exception = *fault;
			zval_copy_ctor(exception);
			INIT_PZVAL(exception);
			zend_throw_exception_object(exception TSRMLS_CC);

			old_objects = EG(objects_store).object_buckets;
			EG(objects_store).object_buckets = NULL;
			PG(display_errors) = 0;
			SG(sapi_headers).http_status_line = NULL;
			zend_try {
				call_old_error_handler(error_num, error_filename, error_lineno, format, args);
			} zend_catch {
				CG(in_compilation) = _old_in_compilation;
				EG(in_execution) = _old_in_execution;
				EG(current_execute_data) = _old_current_execute_data;
				if (SG(sapi_headers).http_status_line) {
					efree(SG(sapi_headers).http_status_line);
				}
				SG(sapi_headers).http_status_line = _old_http_status_line;
				SG(sapi_headers).http_response_code = _old_http_response_code;
			} zend_end_try();
			EG(objects_store).object_buckets = old_objects;
			PG(display_errors) = old;
			zend_bailout();
		} else if (!client->exceptions ||
		           !SOAP_GLOBAL(error_code) ||
		           strcmp(SOAP_GLOBAL(error_code),"WSDL") != 0) {
			/* Ignore libxml warnings during WSDL parsing */
			call_old_error_handler(error_num, error_filename, error_lineno, format, args);
		}
	} else {
		int old = PG(display_errors);
		int fault = 0;
		zval fault_obj;
#ifdef va_copy
		va_list argcopy;
#endif

		if (error_num == E_USER_ERROR ||
		    error_num == E_COMPILE_ERROR ||
		    error_num == E_CORE_ERROR ||
		    error_num == E_ERROR ||
		    error_num == E_PARSE) {

			char* code = SOAP_GLOBAL(error_code);
			soap_server_object *server;
			char *buffer;
			zval *outbuf = NULL;

			if (code == NULL) {
				code = "Server";
			}

			if (SOAP_GLOBAL(error_object) &&
			    Z_TYPE_P(SOAP_GLOBAL(error_object)) == IS_OBJECT &&
			    instanceof_function(Z_OBJCE_P(SOAP_GLOBAL(error_object)), soap_server_class_entry TSRMLS_CC) &&
				(server = (soap_server_object*)zend_object_store_get_object(SOAP_GLOBAL(error_object) TSRMLS_CC)) &&
				!server->send_errors) {
				buffer = estrdup("Internal Error");
			} else {
				int buffer_len;
				zval outbuflen;

				INIT_ZVAL(outbuflen);

#ifdef va_copy
				va_copy(argcopy, args);
				buffer_len = vspprintf(&buffer, 0, format, argcopy);
				va_end(argcopy);
#else
				buffer_len = vspprintf(&buffer, 0, format, args);
#endif

				/* Get output buffer and send as fault detials */
				if (php_output_get_length(&outbuflen TSRMLS_CC) != FAILURE && Z_LVAL(outbuflen) != 0) {
					ALLOC_INIT_ZVAL(outbuf);
					php_output_get_contents(outbuf TSRMLS_CC);
				}
				php_output_discard(TSRMLS_C);
			}

			INIT_ZVAL(fault_obj);
			set_soap_fault(&fault_obj, NULL, code, buffer, NULL, outbuf, NULL TSRMLS_CC);
			efree(buffer);
			fault = 1;
		}

		PG(display_errors) = 0;
		SG(sapi_headers).http_status_line = NULL;
		zend_try {
			call_old_error_handler(error_num, error_filename, error_lineno, format, args);
		} zend_catch {
			CG(in_compilation) = _old_in_compilation;
			EG(in_execution) = _old_in_execution;
			EG(current_execute_data) = _old_current_execute_data;
			if (SG(sapi_headers).http_status_line) {
				efree(SG(sapi_headers).http_status_line);
			}
			SG(sapi_headers).http_status_line = _old_http_status_line;
			SG(sapi_headers).http_response_code = _old_http_response_code;
		} zend_end_try();
		PG(display_errors) = old;

		if (fault) {
			soap_server_fault_ex(NULL, &fault_obj, NULL TSRMLS_CC);
			zend_bailout();
		}
	}
}

/* {{{ proto bool use_soap_error_handler ( [bool on] ) U
   Enable or disable SOAP's error handler, that translates PHP errors into 
   SOAP faults */
PHP_FUNCTION(use_soap_error_handler)
{
	zend_bool handler = 1;

	ZVAL_BOOL(return_value, SOAP_GLOBAL(use_soap_error_handler));
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &handler) == SUCCESS) {
		SOAP_GLOBAL(use_soap_error_handler) = handler;
	}
}
/* }}} */

/* {{{ proto bool is_soap_fault ( mixed object ) U
   Checks if given value is a SoapFault object */
PHP_FUNCTION(is_soap_fault)
{
	zval *fault;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &fault) == SUCCESS &&
	    Z_TYPE_P(fault) == IS_OBJECT &&
	    instanceof_function(Z_OBJCE_P(fault), soap_fault_class_entry TSRMLS_CC)) {
		RETURN_TRUE;
	}
	RETURN_FALSE
}
/* }}} */

/* SoapClient functions */

/* {{{ proto object SoapClient::SoapClient ( mixed wsdl [, array options]) U
   SoapClient constructor */
PHP_METHOD(SoapClient, SoapClient)
{
	zstr zwsdl;
	int zwsdl_len;
	zend_uchar zwsdl_type;
	char *wsdl = NULL;
	zval *options = NULL;
	int  soap_version = SOAP_1_1;
	php_stream_context *context = NULL;
	zend_bool cache_wsdl;
	HashTable *typemap_ht = NULL;
	soap_client_object *client;

	SOAP_CLIENT_BEGIN_CODE();

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t!|a", &zwsdl, &zwsdl_len, &zwsdl_type, &options) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Invalid parameters");
	}

	if (zwsdl.v) {
		if (zwsdl_type == IS_STRING) {
			wsdl = estrndup(zwsdl.s, zwsdl_len);
		} else {
			wsdl = soap_unicode_to_string(zwsdl.u, zwsdl_len TSRMLS_CC);
		}
	}

	cache_wsdl = SOAP_GLOBAL(cache);

	if (options != NULL) {
		HashTable *ht = Z_ARRVAL_P(options);
		zval **tmp;

		if (wsdl == NULL) {
			/* Fetching non-WSDL mode options */
			if (zend_ascii_hash_find(ht, "uri", sizeof("uri"), (void**)&tmp) == SUCCESS &&
				(Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
				if (Z_TYPE_PP(tmp) == IS_STRING) {
					client->uri = estrndup(Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
				} else {
					client->uri = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
				}
			} else {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "'uri' option is required in nonWSDL mode");
			}

			if (zend_ascii_hash_find(ht, "style", sizeof("style"), (void**)&tmp) == SUCCESS &&
					Z_TYPE_PP(tmp) == IS_LONG &&
					(Z_LVAL_PP(tmp) == SOAP_RPC || Z_LVAL_PP(tmp) == SOAP_DOCUMENT)) {
				client->style = Z_LVAL_PP(tmp);
			}

			if (zend_ascii_hash_find(ht, "use", sizeof("use"), (void**)&tmp) == SUCCESS &&
					Z_TYPE_PP(tmp) == IS_LONG &&
					(Z_LVAL_PP(tmp) == SOAP_LITERAL || Z_LVAL_PP(tmp) == SOAP_ENCODED)) {
				client->use = Z_LVAL_PP(tmp);
			}
		}

		if (zend_ascii_hash_find(ht, "stream_context", sizeof("stream_context"), (void**)&tmp) == SUCCESS &&
				Z_TYPE_PP(tmp) == IS_RESOURCE) {
			context = php_stream_context_from_zval(*tmp, 1);
			zend_list_addref(context->rsrc_id);
		}

		if (zend_ascii_hash_find(ht, "location", sizeof("location"), (void**)&tmp) == SUCCESS &&
		    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
			if (Z_TYPE_PP(tmp) == IS_STRING) {
				client->location = estrndup(Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
			} else {
				client->location = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
			}
		} else if (wsdl == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "'location' option is required in nonWSDL mode");
		}

		if (zend_ascii_hash_find(ht, "soap_version", sizeof("soap_version"), (void**)&tmp) == SUCCESS) {
			if (Z_TYPE_PP(tmp) == IS_LONG ||
			    (Z_LVAL_PP(tmp) == SOAP_1_1 && Z_LVAL_PP(tmp) == SOAP_1_2)) {
				soap_version = Z_LVAL_PP(tmp);
			}
		}
		if (zend_ascii_hash_find(ht, "login", sizeof("login"), (void**)&tmp) == SUCCESS &&
		    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
		    if (Z_TYPE_PP(tmp) == IS_STRING) {
		    	client->login = estrndup(Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
			} else {
		    	client->login = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
			}
			if (zend_ascii_hash_find(ht, "password", sizeof("password"), (void**)&tmp) == SUCCESS &&
			    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
			    if (Z_TYPE_PP(tmp) == IS_STRING) {
			    	client->password = estrndup(Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
				} else {
			    	client->password = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
				}
			}
			if (zend_ascii_hash_find(ht, "authentication", sizeof("authentication"), (void**)&tmp) == SUCCESS &&
			    Z_TYPE_PP(tmp) == IS_LONG &&
			    Z_LVAL_PP(tmp) == SOAP_AUTHENTICATION_DIGEST) {
			    client->digest = 1;
			}
		}
		if (zend_ascii_hash_find(ht, "proxy_host", sizeof("proxy_host"), (void**)&tmp) == SUCCESS &&
		    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
		    if (Z_TYPE_PP(tmp) == IS_STRING) {
		    	client->proxy_host = estrndup(Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
			} else {
		    	client->proxy_host = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
		    }
			if (zend_ascii_hash_find(ht, "proxy_port", sizeof("proxy_port"), (void**)&tmp) == SUCCESS) {
				convert_to_long(*tmp); 
				client->proxy_port = Z_LVAL_PP(tmp);
			}
			if (zend_ascii_hash_find(ht, "proxy_login", sizeof("proxy_login"), (void**)&tmp) == SUCCESS &&
			    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
			    if (Z_TYPE_PP(tmp) == IS_STRING) {
			    	client->proxy_login = estrndup(Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
				} else {
			    	client->proxy_login = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
				}
				if (zend_ascii_hash_find(ht, "proxy_password", sizeof("proxy_password"), (void**)&tmp) == SUCCESS &&
				    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
				    if (Z_TYPE_PP(tmp) == IS_STRING) {
				    	client->proxy_password = estrndup(Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
					} else {
				    	client->proxy_password = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
					}
				}
			}
		}
		if (zend_ascii_hash_find(ht, "local_cert", sizeof("local_cert"), (void**)&tmp) == SUCCESS &&
		    Z_TYPE_PP(tmp) == IS_STRING) {
			if (!context) {
				context = php_stream_context_alloc();
			}
 			php_stream_context_set_option(context, "ssl", "local_cert", *tmp);
			if (zend_ascii_hash_find(ht, "passphrase", sizeof("passphrase"), (void**)&tmp) == SUCCESS &&
			    Z_TYPE_PP(tmp) == IS_STRING) {
				php_stream_context_set_option(context, "ssl", "passphrase", *tmp);
			}
		}
		if (zend_ascii_hash_find(ht, "trace", sizeof("trace"), (void**)&tmp) == SUCCESS &&
		    (Z_TYPE_PP(tmp) == IS_BOOL || Z_TYPE_PP(tmp) == IS_LONG) &&
				Z_LVAL_PP(tmp) == 1) {
			client->trace = 1;
		}
		if (zend_ascii_hash_find(ht, "exceptions", sizeof("exceptions"), (void**)&tmp) == SUCCESS &&
		    (Z_TYPE_PP(tmp) == IS_BOOL || Z_TYPE_PP(tmp) == IS_LONG) &&
				Z_LVAL_PP(tmp) == 0) {
			client->exceptions = 0;
		}
		if (zend_ascii_hash_find(ht, "compression", sizeof("compression"), (void**)&tmp) == SUCCESS &&
		    Z_TYPE_PP(tmp) == IS_LONG &&
		    zend_hash_exists(EG(function_table), "gzinflate", sizeof("gzinflate")) &&
		    zend_hash_exists(EG(function_table), "gzdeflate", sizeof("gzdeflate")) &&
		    zend_hash_exists(EG(function_table), "gzuncompress", sizeof("gzuncompress")) &&
		    zend_hash_exists(EG(function_table), "gzcompress", sizeof("gzcompress")) &&
		    zend_hash_exists(EG(function_table), "gzencode", sizeof("gzencode"))) {
			client->compression = Z_LVAL_PP(tmp);
		}
		if (zend_ascii_hash_find(ht, "encoding", sizeof("encoding"), (void**)&tmp) == SUCCESS &&
		    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
			xmlCharEncodingHandlerPtr encoding;
			char *str;

			if (Z_TYPE_PP(tmp) == IS_UNICODE) {
				str = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
			} else {
				str = Z_STRVAL_PP(tmp);
			}
			encoding = xmlFindCharEncodingHandler(str);
			if (encoding == NULL) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Invalid arguments. Invalid 'encoding' option - '%v'", Z_TYPE_PP(tmp), Z_UNIVAL_PP(tmp));
			} else {
				client->encoding = encoding;
			}
			if (Z_TYPE_PP(tmp) == IS_UNICODE) {
				efree(str);
			}
		}
		if (zend_ascii_hash_find(ht, "classmap", sizeof("classmap"), (void**)&tmp) == SUCCESS &&
			Z_TYPE_PP(tmp) == IS_ARRAY) {
			zval *ztmp;

			ALLOC_HASHTABLE(client->class_map);
			zend_u_hash_init(client->class_map, 0, NULL, ZVAL_PTR_DTOR, 0, UG(unicode));
			zend_hash_copy(client->class_map, (*tmp)->value.ht, (copy_ctor_func_t) zval_add_ref, (void *) &ztmp, sizeof(zval *));
		}

		if (zend_ascii_hash_find(ht, "typemap", sizeof("typemap"), (void**)&tmp) == SUCCESS &&
			Z_TYPE_PP(tmp) == IS_ARRAY &&
			zend_hash_num_elements(Z_ARRVAL_PP(tmp)) > 0) {
			typemap_ht = Z_ARRVAL_PP(tmp);
		}

		if (zend_ascii_hash_find(ht, "features", sizeof("features"), (void**)&tmp) == SUCCESS &&
			Z_TYPE_PP(tmp) == IS_LONG) {
			client->features = Z_LVAL_PP(tmp);
	    }

		if (zend_ascii_hash_find(ht, "connection_timeout", sizeof("connection_timeout"), (void**)&tmp) == SUCCESS) {
			convert_to_long(*tmp);
			if(Z_LVAL_PP(tmp) >= 0) {
		    		client->connection_timeout = Z_LVAL_PP(tmp);
			}
		}

		if (context) {
			client->stream_context = context;
		}

		if (zend_ascii_hash_find(ht, "cache_wsdl", sizeof("cache_wsdl"), (void**)&tmp) == SUCCESS &&
		    Z_TYPE_PP(tmp) == IS_LONG) {
			cache_wsdl = Z_LVAL_PP(tmp);
		}

		if (zend_ascii_hash_find(ht, "user_agent", sizeof("user_agent"), (void**)&tmp) == SUCCESS &&
		    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
		    if (Z_TYPE_PP(tmp) == IS_STRING) {
		    	client->user_agent = estrndup(Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
			} else {
		    	client->user_agent = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
			}
		}

	} else if (wsdl == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "'location' and 'uri' options are required in nonWSDL mode");
	}

	client->version = soap_version;

	if (wsdl) {
		int    old_soap_version;
		sdlPtr sdl;

		old_soap_version = SOAP_GLOBAL(soap_version);
		SOAP_GLOBAL(soap_version) = soap_version;

		sdl = get_sdl(this_ptr, wsdl, cache_wsdl TSRMLS_CC);

		client->sdl = sdl;

		SOAP_GLOBAL(soap_version) = old_soap_version;
		efree(wsdl);
	}

	if (typemap_ht) {
		client->typemap = soap_create_typemap(client->sdl, typemap_ht TSRMLS_CC);
	}

	SOAP_CLIENT_END_CODE();
}
/* }}} */

static int do_request(zval *this_ptr, xmlDoc *request, char *location, char *action, int version, int one_way, zval *response TSRMLS_DC)
{
	int    ret = TRUE;
	char  *buf;
	int    buf_size;
	zval   func, param0, param1, param2, param3, param4;
	zval  *params[5];
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	INIT_ZVAL(*response);

	xmlDocDumpMemory(request, (xmlChar**)&buf, &buf_size);
	if (!buf) {
		add_soap_fault(this_ptr, "HTTP", "Error build soap request", NULL, NULL TSRMLS_CC);
		return FALSE;
	}

	if (client->trace) {
		if (client->last_request) {
			efree(client->last_request);
		}
		client->last_request = estrndup(buf, buf_size);
	}

	INIT_ZVAL(func);
	ZVAL_STRINGL(&func,"__doRequest",sizeof("__doRequest")-1,0);
	INIT_ZVAL(param0);
	params[0] = &param0;
	ZVAL_STRINGL(params[0], buf, buf_size, 1);
   	zval_string_to_unicode_ex(params[0], UG(utf8_conv) TSRMLS_CC);

	INIT_ZVAL(param1);
	params[1] = &param1;
	if (location == NULL) {
		ZVAL_NULL(params[1]);
	} else {
		ZVAL_STRING(params[1], location, 1);
   		zval_string_to_unicode_ex(params[1], UG(utf8_conv) TSRMLS_CC);
	}
	INIT_ZVAL(param2);
	params[2] = &param2;
	if (action == NULL) {
		ZVAL_NULL(params[2]);
	} else {
		ZVAL_STRING(params[2], action, 1);
   		zval_string_to_unicode_ex(params[2], UG(utf8_conv) TSRMLS_CC);
	}
	INIT_ZVAL(param3);
	params[3] = &param3;
	ZVAL_LONG(params[3], version);

	INIT_ZVAL(param4);
	params[4] = &param4;
	ZVAL_LONG(params[4], one_way);

	if (call_user_function(NULL, &this_ptr, &func, response, 5, params TSRMLS_CC) != SUCCESS) {
		add_soap_fault(this_ptr, "Client", "SoapClient::__doRequest() failed", NULL, NULL TSRMLS_CC);
		ret = FALSE;
	} else if (Z_TYPE_P(response) != IS_STRING && Z_TYPE_P(response) != IS_UNICODE) {
		if (!client->fault) {
			add_soap_fault(this_ptr, "Client", "SoapClient::__doRequest() returned non string value", NULL, NULL TSRMLS_CC);
		}
		ret = FALSE;
	} else {
	    if (Z_TYPE_P(response) == IS_UNICODE) {
	    	zval_unicode_to_string_ex(response, UG(utf8_conv) TSRMLS_CC);
	    }
		if (client->trace) {
			if (client->last_response) {
				efree(client->last_response);
			}
			client->last_response = estrndup(Z_STRVAL_P(response), Z_STRLEN_P(response));
		}
	}
	xmlFree(buf);
	zval_dtor(params[0]);
	zval_dtor(params[1]);
	zval_dtor(params[2]);
	if (ret && client->fault) {
		return FALSE;
	}
	return ret;
}

static void do_soap_call(zval* this_ptr,
                         char* function,
                         int function_len,
                         int arg_count,
                         zval** real_args,
                         zval* return_value,
                         char* location,
                         char* soap_action,
                         char* call_uri,
                         HashTable* soap_headers,
                         zval* output_headers
                         TSRMLS_DC)
{
 	sdlPtr sdl = NULL;
 	sdlPtr old_sdl = NULL;
 	sdlFunctionPtr fn;
	xmlDocPtr request = NULL;
	int ret = FALSE;
	int soap_version;
	zval response;
	xmlCharEncodingHandlerPtr old_encoding;
	HashTable *old_class_map, *old_typemap;
	int old_features;
	soap_client_object *client;

	SOAP_CLIENT_BEGIN_CODE();

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);

	if (client->trace) {
		if (client->last_request) {
			efree(client->last_request);
			client->last_request = NULL;
		}
		if (client->last_response) {
			efree(client->last_response);
			client->last_response = NULL;
		}
	}

	soap_version = client->version;

	if (location == NULL) {
		location = client->location;
	}

	sdl = client->sdl;

	if (client->fault) {
		zval_ptr_dtor(&client->fault);
		client->fault = NULL;
	}

	SOAP_GLOBAL(soap_version) = soap_version;
	old_sdl = SOAP_GLOBAL(sdl);
	SOAP_GLOBAL(sdl) = sdl;
	old_encoding = SOAP_GLOBAL(encoding);
	SOAP_GLOBAL(encoding) = client->encoding;
	old_class_map = SOAP_GLOBAL(class_map);
	SOAP_GLOBAL(class_map) = client->class_map;
	old_typemap = SOAP_GLOBAL(typemap);
	SOAP_GLOBAL(typemap) = client->typemap;
	old_features = SOAP_GLOBAL(features);
	SOAP_GLOBAL(features) = client->features;

 	if (sdl != NULL) {
 		fn = get_function(sdl, function);
 		if (fn != NULL) {
			sdlBindingPtr binding = fn->binding;
			int one_way = 0;

			if (fn->responseName == NULL &&
			    fn->responseParameters == NULL &&
			    soap_headers == NULL) {
				one_way = 1;
			}

			if (location == NULL) {
				location = binding->location;
			}
			if (binding->bindingType == BINDING_SOAP) {
				sdlSoapBindingFunctionPtr fnb = (sdlSoapBindingFunctionPtr)fn->bindingAttributes;
 				request = serialize_function_call(this_ptr, fn, NULL, fnb->input.ns, real_args, arg_count, soap_version, soap_headers TSRMLS_CC);
 				ret = do_request(this_ptr, request, location, fnb->soapAction, soap_version, one_way, &response TSRMLS_CC);
 			}	else {
 				request = serialize_function_call(this_ptr, fn, NULL, sdl->target_ns, real_args, arg_count, soap_version, soap_headers TSRMLS_CC);
 				ret = do_request(this_ptr, request, location, NULL, soap_version, one_way, &response TSRMLS_CC);
 			}

			xmlFreeDoc(request);

			if (ret && Z_TYPE(response) == IS_STRING) {
				encode_reset_ns();
				ret = parse_packet_soap(this_ptr, Z_STRVAL(response), Z_STRLEN(response), fn, NULL, return_value, output_headers TSRMLS_CC);
				encode_finish();
			}

			zval_dtor(&response);

 		} else {
 			smart_str error = {0};
 			smart_str_appends(&error,"Function (\"");
 			smart_str_appends(&error,function);
 			smart_str_appends(&error,"\") is not a valid method for this service");
 			smart_str_0(&error);
			add_soap_fault(this_ptr, "Client", error.c, NULL, NULL TSRMLS_CC);
			smart_str_free(&error);
		}
	} else {
		smart_str action = {0};

		if (!client->uri) {
			add_soap_fault(this_ptr, "Client", "Error finding \"uri\" property", NULL, NULL TSRMLS_CC);
		} else if (location == NULL) {
			add_soap_fault(this_ptr, "Client", "Error could not find \"location\" property", NULL, NULL TSRMLS_CC);
		} else {
			if (call_uri == NULL) {
				call_uri = client->uri;
			}
	 		request = serialize_function_call(this_ptr, NULL, function, call_uri, real_args, arg_count, soap_version, soap_headers TSRMLS_CC);

	 		if (soap_action == NULL) {
				smart_str_appends(&action, call_uri);
				smart_str_appendc(&action, '#');
				smart_str_appends(&action, function);
			} else {
				smart_str_appends(&action, soap_action);
			}
			smart_str_0(&action);

			ret = do_request(this_ptr, request, location, action.c, soap_version, 0, &response TSRMLS_CC);

	 		smart_str_free(&action);
			xmlFreeDoc(request);

			if (ret && Z_TYPE(response) == IS_STRING) {
				encode_reset_ns();
				ret = parse_packet_soap(this_ptr, Z_STRVAL(response), Z_STRLEN(response), NULL, function, return_value, output_headers TSRMLS_CC);
				encode_finish();
			}

			zval_dtor(&response);
		}
 	}

	if (!ret) {
		if (client->fault) {
			*return_value = *client->fault;
			zval_copy_ctor(return_value);
			INIT_PZVAL(return_value);
		} else {
			*return_value = *add_soap_fault(this_ptr, "Client", "Unknown Error", NULL, NULL TSRMLS_CC);
			zval_copy_ctor(return_value);
			INIT_PZVAL(return_value);
		}
	} else {
		if (client->fault) {
			*return_value = *client->fault;
			zval_copy_ctor(return_value);
			INIT_PZVAL(return_value);
		}
	}
	if (!EG(exception) &&
	    Z_TYPE_P(return_value) == IS_OBJECT &&
	    instanceof_function(Z_OBJCE_P(return_value), soap_fault_class_entry TSRMLS_CC) &&
        client->exceptions) {
		zval *exception;

		MAKE_STD_ZVAL(exception);
		*exception = *return_value;
		zval_copy_ctor(exception);
		INIT_PZVAL(exception);
		zend_throw_exception_object(exception TSRMLS_CC);
	}
	SOAP_GLOBAL(features) = old_features;
	SOAP_GLOBAL(typemap) = old_typemap;
	SOAP_GLOBAL(class_map) = old_class_map;
	SOAP_GLOBAL(encoding) = old_encoding;
	SOAP_GLOBAL(sdl) = old_sdl;
	SOAP_CLIENT_END_CODE();
}

static void verify_soap_headers_array(HashTable *ht TSRMLS_DC)
{
	zval **tmp;

	zend_hash_internal_pointer_reset(ht);
	while (zend_hash_get_current_data(ht, (void**)&tmp) == SUCCESS) {
		if (Z_TYPE_PP(tmp) != IS_OBJECT ||
		    !instanceof_function(Z_OBJCE_PP(tmp), soap_header_class_entry TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Invalid SOAP header");
		}
		zend_hash_move_forward(ht);
	}
}


/* {{{ proto mixed SoapClient::__call ( string function_name, array arguments [, array options [, array input_headers [, array output_headers]]]) U
   Calls a SOAP function */
PHP_METHOD(SoapClient, __call)
{
	zstr function;
	zend_uchar function_type;
	char *location=NULL, *soap_action = NULL, *uri = NULL;
	int function_len, i = 0;
	HashTable* soap_headers = NULL;
	zval *options = NULL;
	zval *headers = NULL;
	zval *output_headers = NULL;
	zval *args;
	zval **real_args = NULL;
	zval **param;
	int arg_count;
	zval **tmp;
	zend_bool free_soap_headers = 0, free_location = 0, free_soap_action = 0, free_uri = 0;
	HashPosition pos;
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ta|a!zz",
		&function, &function_len, &function_type, &args, &options, &headers, &output_headers) == FAILURE) {
		return;
	}

	if (options) {
		if (Z_TYPE_P(options) == IS_ARRAY) {
			HashTable *ht = Z_ARRVAL_P(options);
			if (zend_ascii_hash_find(ht, "location", sizeof("location"), (void**)&tmp) == SUCCESS &&
			    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
				if (Z_TYPE_PP(tmp) == IS_STRING) {
					location = Z_STRVAL_PP(tmp);
				} else if (Z_TYPE_PP(tmp) == IS_UNICODE) {
					free_location = 1;
					location = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
				}
			}

			if (zend_ascii_hash_find(ht, "soapaction", sizeof("soapaction"), (void**)&tmp) == SUCCESS &&
			    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
				if (Z_TYPE_PP(tmp) == IS_STRING) {
					soap_action = Z_STRVAL_PP(tmp);
				} else if (Z_TYPE_PP(tmp) == IS_UNICODE) {
					free_soap_action = 1;
					soap_action = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
				}
			}

			if (zend_ascii_hash_find(ht, "uri", sizeof("uri"), (void**)&tmp) == SUCCESS &&
			    (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
				if (Z_TYPE_PP(tmp) == IS_STRING) {
					uri = Z_STRVAL_PP(tmp);
				} else if (Z_TYPE_PP(tmp) == IS_UNICODE) {
					free_uri = 1;
					uri = soap_unicode_to_string(Z_USTRVAL_PP(tmp), Z_USTRLEN_PP(tmp) TSRMLS_CC);
				}
			}
		}
	}

	if (headers == NULL || Z_TYPE_P(headers) == IS_NULL) {
	} else if (Z_TYPE_P(headers) == IS_ARRAY) {
		soap_headers = Z_ARRVAL_P(headers);
		verify_soap_headers_array(soap_headers TSRMLS_CC);
		free_soap_headers = 0;
	} else if (Z_TYPE_P(headers) == IS_OBJECT &&
	           instanceof_function(Z_OBJCE_P(headers), soap_header_class_entry TSRMLS_CC)) {
	    soap_headers = emalloc(sizeof(HashTable));
		zend_hash_init(soap_headers, 0, NULL, ZVAL_PTR_DTOR, 0);
		zend_hash_next_index_insert(soap_headers, &headers, sizeof(zval*), NULL);
		Z_ADDREF_P(headers);
		free_soap_headers = 1;
	} else{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid SOAP header");
	}

	/* Add default headers */
	if (client->default_headers) {
		if (soap_headers) {
			HashTable *default_headers = Z_ARRVAL_P(client->default_headers);

			if (!free_soap_headers) {
				HashTable *tmp =  emalloc(sizeof(HashTable));
				zend_hash_init(tmp, 0, NULL, ZVAL_PTR_DTOR, 0);
				zend_hash_copy(tmp, soap_headers, (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *));
				soap_headers = tmp;
				free_soap_headers = 1;
			}
			zend_hash_internal_pointer_reset(default_headers);
			while (zend_hash_get_current_data(default_headers, (void**)&tmp) == SUCCESS) {
				Z_ADDREF_P(*tmp);
				zend_hash_next_index_insert(soap_headers, tmp, sizeof(zval *), NULL);
				zend_hash_move_forward(default_headers);
			}
		} else {
			soap_headers = Z_ARRVAL_P(client->default_headers);
			free_soap_headers = 0;
		}
	}

	arg_count = zend_hash_num_elements(Z_ARRVAL_P(args));

	if (arg_count > 0) {
		real_args = safe_emalloc(sizeof(zval *), arg_count, 0);
		for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(args), &pos);
			zend_hash_get_current_data_ex(Z_ARRVAL_P(args), (void **) &param, &pos) == SUCCESS;
			zend_hash_move_forward_ex(Z_ARRVAL_P(args), &pos)) {
				/*zval_add_ref(param);*/
				real_args[i++] = *param;
		}
	}
	if (output_headers) {
		array_init(output_headers);
	}

	function.s = soap_encode_string_ex(function_type, function, function_len TSRMLS_CC);
	function_len = strlen(function.s);
	do_soap_call(this_ptr, function.s, function_len, arg_count, real_args, return_value, location, soap_action, uri, soap_headers, output_headers TSRMLS_CC);
	efree(function.s);

	if (arg_count > 0) {
		efree(real_args);
	}

	if (soap_headers && free_soap_headers) {
		zend_hash_destroy(soap_headers);
		efree(soap_headers);
	}

	if (free_location) {
		efree(location);
	}
	if (free_soap_action) {
		efree(soap_action);
	}
	if (free_uri) {
		efree(uri);
	}
}
/* }}} */


/* {{{ proto array SoapClient::__getFunctions ( void ) U
   Returns list of SOAP functions */
PHP_METHOD(SoapClient, __getFunctions)
{
	sdlPtr sdl;
	HashPosition pos;
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	sdl = client->sdl;
	if (sdl) {
		smart_str buf = {0};
		sdlFunctionPtr *function;

		array_init(return_value);
 		zend_hash_internal_pointer_reset_ex(&sdl->functions, &pos);
		while (zend_hash_get_current_data_ex(&sdl->functions, (void **)&function, &pos) != FAILURE) {
			function_to_string((*function), &buf);
			add_next_index_rt_stringl(return_value, buf.c, buf.len, 1);
			smart_str_free(&buf);
			zend_hash_move_forward_ex(&sdl->functions, &pos);
		}
	}
}
/* }}} */


/* {{{ proto array SoapClient::__getTypes ( void ) U
   Returns list of SOAP types */
PHP_METHOD(SoapClient, __getTypes)
{
	sdlPtr sdl;
	HashPosition pos;
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	sdl = client->sdl;
	if (sdl) {
		sdlTypePtr *type;
		smart_str buf = {0};

		array_init(return_value);
		if (sdl->types) {
			zend_hash_internal_pointer_reset_ex(sdl->types, &pos);
			while (zend_hash_get_current_data_ex(sdl->types, (void **)&type, &pos) != FAILURE) {
				type_to_string((*type), &buf, 0);
				add_next_index_rt_stringl(return_value, buf.c, buf.len, 1);
				smart_str_free(&buf);
				zend_hash_move_forward_ex(sdl->types, &pos);
			}
		}
	}
}
/* }}} */


/* {{{ proto string SoapClient::__getLastRequest ( void ) U
   Returns last SOAP request */
PHP_METHOD(SoapClient, __getLastRequest)
{
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	if (client->last_request) {
		soap_decode_string(return_value, client->last_request TSRMLS_CC);
		return;
	}
	RETURN_NULL();
}
/* }}} */


/* {{{ proto object SoapClient::__getLastResponse ( void ) U
   Returns last SOAP response */
PHP_METHOD(SoapClient, __getLastResponse)
{
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	if (client->last_response) {
		soap_decode_string(return_value, client->last_response TSRMLS_CC);
		return;
	}
	RETURN_NULL();
}
/* }}} */


/* {{{ proto string SoapClient::__getLastRequestHeaders(void) U
   Returns last SOAP request headers */
PHP_METHOD(SoapClient, __getLastRequestHeaders)
{
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	if (client->last_request_headers) {
		soap_decode_string(return_value, client->last_request_headers TSRMLS_CC);
		return;
	}
	RETURN_NULL();
}
/* }}} */


/* {{{ proto string SoapClient::__getLastResponseHeaders(void) U
   Returns last SOAP response headers */
PHP_METHOD(SoapClient, __getLastResponseHeaders)
{
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	if (client->last_response_headers) {
		soap_decode_string(return_value, client->last_response_headers TSRMLS_CC);
		return;
	}
	RETURN_NULL();
}
/* }}} */


/* {{{ proto string SoapClient::__doRequest() U
   SoapClient::__doRequest() */
PHP_METHOD(SoapClient, __doRequest)
{
  zstr  buf, location, action;
  int   buf_size, location_size, action_size;
  zend_uchar buf_type, location_type, action_type;
  long  version;
  long  one_way = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "tttl|l",
	    &buf, &buf_size, &buf_type,
	    &location, &location_size, &location_type,
	    &action, &action_size, &action_type,
	    &version, &one_way) == FAILURE) {
		return;
	}
	if (buf_type == IS_UNICODE) {
		buf.s = soap_unicode_to_string(buf.u, buf_size TSRMLS_CC);
		buf_size = strlen(buf.s);
	}
	if (location_type == IS_UNICODE) {
		location.s = soap_unicode_to_string(location.u, location_size TSRMLS_CC);
	}
	if (action_type == IS_UNICODE) {
		action.s = soap_unicode_to_string(action.u, action_size TSRMLS_CC);
	}
	if (SOAP_GLOBAL(features) & SOAP_WAIT_ONE_WAY_CALLS) {
		one_way = 0;
	}
	if (one_way) {
		if (make_http_soap_request(this_ptr, buf.s, buf_size, location.s, action.s, version, NULL, NULL TSRMLS_CC)) {
			RETVAL_EMPTY_STRING();
		}
	} else if (make_http_soap_request(this_ptr, buf.s, buf_size, location.s, action.s, version,
	    &Z_STRVAL_P(return_value), &Z_STRLEN_P(return_value) TSRMLS_CC)) {
		return_value->type = IS_STRING;
		zval_string_to_unicode_ex(return_value, UG(utf8_conv) TSRMLS_CC);
	}
	if (buf_type == IS_UNICODE) {
		efree(buf.s);
	}
	if (location_type == IS_UNICODE) {
		efree(location.s);
	}
	if (action_type == IS_UNICODE) {
		efree(action.s);
	}
}
/* }}} */

/* {{{ proto void SoapClient::__setCookie(string name [, strung value]) U
   Sets cookie thet will sent with SOAP request.
   The call to this function will effect all folowing calls of SOAP methods.
   If value is not specified cookie is removed. */
PHP_METHOD(SoapClient, __setCookie)
{
	zstr name;
	zstr val = NULL_ZSTR;
	int  name_len, val_len = 0;
	zend_uchar name_type, val_type = 0;
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t|t",
	    &name, &name_len, &name_type, &val, &val_len, &val_type) == FAILURE) {
		return;
	}

	if (val.v == NULL) {
		if (client->cookies) {
			zend_u_hash_del(Z_ARRVAL_P(client->cookies), name_type, name, name_len+1);
		}
	} else {
		zval *zcookie;

		if (!client->cookies) {
			MAKE_STD_ZVAL(client->cookies);
			array_init(client->cookies);
		}

		ALLOC_INIT_ZVAL(zcookie);
		array_init(zcookie);
		if (val_type == IS_STRING) {
			add_index_stringl(zcookie, 0, val.s, val_len, 1);
		} else {
			add_index_unicodel(zcookie, 0, val.u, val_len, 1);
		}
		add_u_assoc_zval_ex(client->cookies, name_type, name, name_len+1, zcookie);
	}
}
/* }}} */

/* {{{ proto array SoapClient::__getCookies() U
   Returns array of cookies. */
PHP_METHOD(SoapClient, __getCookies)
{
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);

	if (!client->cookies) {
		array_init(return_value);
	} else {
		*return_value = *client->cookies;
		zval_copy_ctor(return_value);
		INIT_PZVAL(return_value);
	}
}
/* }}} */

/* {{{ proto void SoapClient::__setSoapHeaders(array SoapHeaders) U
   Sets SOAP headers for subsequent calls (replaces any previous
   values).
   If no value is specified, all of the headers are removed. */
PHP_METHOD(SoapClient, __setSoapHeaders)
{
	zval *headers = NULL;
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &headers) == FAILURE) {
		return;
	}

	if (client->default_headers) {
		zval_ptr_dtor(&client->default_headers);
		client->default_headers = NULL;
	}
	if (headers == NULL || Z_TYPE_P(headers) == IS_NULL) {
	} else if (Z_TYPE_P(headers) == IS_ARRAY) {
		verify_soap_headers_array(Z_ARRVAL_P(headers) TSRMLS_CC);
		ALLOC_ZVAL(client->default_headers);
		*client->default_headers = *headers;
		INIT_PZVAL(client->default_headers);
		zval_copy_ctor(client->default_headers);
	} else if (Z_TYPE_P(headers) == IS_OBJECT &&
	           instanceof_function(Z_OBJCE_P(headers), soap_header_class_entry TSRMLS_CC)) {
		ALLOC_INIT_ZVAL(client->default_headers);
		array_init(client->default_headers);
		Z_ADDREF_P(headers);
		add_next_index_zval(client->default_headers, headers);
	} else{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid SOAP header");
	}
	RETURN_TRUE;
}
/* }}} */



/* {{{ proto string SoapClient::__setLocation([string new_location]) U
   Sets the location option (the endpoint URL that will be touched by the
   following SOAP requests).
   If new_location is not specified or null then SoapClient will use endpoint
   from WSDL file.
   The function returns old value of location options. */
PHP_METHOD(SoapClient, __setLocation)
{
	void* location = NULL;
	int  location_len = 0;
	zend_uchar location_type = 0;
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|t",
	    &location, &location_len, &location_type) == FAILURE) {
		return;
	}
	if (client->location) {
		RETVAL_STRING(client->location, 1);
		zval_string_to_unicode_ex(return_value, UG(utf8_conv) TSRMLS_CC);
		efree(client->location);
		client->location = NULL;
	} else {
	  RETVAL_NULL();
	}
	if (location && location_len) {
		if (location_type == IS_STRING) {
			client->location = estrndup(location, location_len);
		} else {
			client->location = soap_unicode_to_string(location, location_len TSRMLS_CC);
		}
	}
}
/* }}} */

zval* add_soap_fault(zval *obj, char *fault_code, char *fault_string, char *fault_actor, zval *fault_detail TSRMLS_DC)
{
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(obj TSRMLS_CC);
	if (!client->fault) {
		ALLOC_INIT_ZVAL(client->fault);
		set_soap_fault(client->fault, NULL, fault_code, fault_string, fault_actor, fault_detail, NULL TSRMLS_CC);
	}
	return client->fault;
}

static void set_soap_fault(zval *obj, char *fault_code_ns, char *fault_code, char *fault_string, char *fault_actor, zval *fault_detail, char *name TSRMLS_DC)
{
	zval *tmp;

	if (Z_TYPE_P(obj) != IS_OBJECT) {
		object_init_ex(obj, soap_fault_class_entry);
	}
	if (fault_string != NULL) {
		MAKE_STD_ZVAL(tmp);
		soap_decode_string(tmp, fault_string TSRMLS_CC);
		Z_DELREF_P(tmp);
		add_property_zval(obj, "faultstring", tmp);
		zend_update_property(zend_exception_get_default(TSRMLS_C), obj, "message", sizeof("message")-1, tmp TSRMLS_CC);
	}
	if (fault_code != NULL) {
		int soap_version = SOAP_GLOBAL(soap_version);

		if (fault_code_ns) {
			MAKE_STD_ZVAL(tmp);
			soap_decode_string(tmp, fault_code TSRMLS_CC);
			Z_DELREF_P(tmp);
			add_property_zval(obj, "faultcode", tmp);
			MAKE_STD_ZVAL(tmp);
			soap_decode_string(tmp, fault_code_ns TSRMLS_CC);
			Z_DELREF_P(tmp);
			add_property_zval(obj, "faultcodens", tmp);
		} else {
			if (soap_version == SOAP_1_1) {
				add_property_ascii_string(obj, "faultcode", fault_code, 1);
				if (strcmp(fault_code,"Client") == 0 ||
				    strcmp(fault_code,"Server") == 0 ||
				    strcmp(fault_code,"VersionMismatch") == 0 ||
			  	  strcmp(fault_code,"MustUnderstand") == 0) {
					add_property_ascii_string(obj, "faultcodens", SOAP_1_1_ENV_NAMESPACE, 1);
				}
			} else if (soap_version == SOAP_1_2) {
				if (strcmp(fault_code,"Client") == 0) {
					add_property_ascii_string(obj, "faultcode", "Sender", 1);
					add_property_ascii_string(obj, "faultcodens", SOAP_1_2_ENV_NAMESPACE, 1);
				} else if (strcmp(fault_code,"Server") == 0) {
					add_property_ascii_string(obj, "faultcode", "Receiver", 1);
					add_property_ascii_string(obj, "faultcodens", SOAP_1_2_ENV_NAMESPACE, 1);
				} else {
					if (strcmp(fault_code,"VersionMismatch") == 0 ||
					           strcmp(fault_code,"MustUnderstand") == 0 ||
					           strcmp(fault_code,"DataEncodingUnknown") == 0) {
						add_property_ascii_string(obj, "faultcodens", SOAP_1_2_ENV_NAMESPACE, 1);
					}
					MAKE_STD_ZVAL(tmp);
					soap_decode_string(tmp, fault_code TSRMLS_CC);
					Z_DELREF_P(tmp);
					add_property_zval(obj, "faultcode", tmp);
				}
			}
		}
	}
	if (fault_actor != NULL) {
		MAKE_STD_ZVAL(tmp);
		soap_decode_string(tmp, fault_actor TSRMLS_CC);
		Z_DELREF_P(tmp);
		add_property_zval(obj, "faultactor", tmp);
	}
	if (fault_detail != NULL) {
		add_property_zval(obj, "detail", fault_detail);
	}
	if (name != NULL) {
		add_property_string(obj, "_name", name, 1);
	}
}

static void deserialize_parameters(xmlNodePtr params, sdlFunctionPtr function, int *num_params, zval ***parameters)
{
	int cur_param = 0,num_of_params = 0;
	zval **tmp_parameters = NULL;

	if (function != NULL) {
		sdlParamPtr *param;
		xmlNodePtr val;
		int	use_names = 0;

		if (function->requestParameters == NULL) {
			return;
		}
		num_of_params = zend_hash_num_elements(function->requestParameters);
		zend_hash_internal_pointer_reset(function->requestParameters);
		while (zend_hash_get_current_data(function->requestParameters, (void **)&param) == SUCCESS) {
			if (get_node(params, (*param)->paramName) != NULL) {
				use_names = 1;
			}
			zend_hash_move_forward(function->requestParameters);
		}
		if (use_names) {
			tmp_parameters = safe_emalloc(num_of_params, sizeof(zval *), 0);
			zend_hash_internal_pointer_reset(function->requestParameters);
			while (zend_hash_get_current_data(function->requestParameters, (void **)&param) == SUCCESS) {
				val = get_node(params, (*param)->paramName);
				if (!val) {
					/* TODO: may be "nil" is not OK? */
					MAKE_STD_ZVAL(tmp_parameters[cur_param]);
					ZVAL_NULL(tmp_parameters[cur_param]);
				} else {
					tmp_parameters[cur_param] = master_to_zval((*param)->encode, val);
				}
				cur_param++;

				zend_hash_move_forward(function->requestParameters);
			}
			(*parameters) = tmp_parameters;
			(*num_params) = num_of_params;
			return;
		}
	}
	if (params) {
		xmlNodePtr trav;

		num_of_params = 0;
		trav = params;
		while (trav != NULL) {
			if (trav->type == XML_ELEMENT_NODE) {
				num_of_params++;
			}
			trav = trav->next;
		}

		if (num_of_params == 1 &&
		    function &&
		    function->binding &&
		    function->binding->bindingType == BINDING_SOAP &&
		    ((sdlSoapBindingFunctionPtr)function->bindingAttributes)->style == SOAP_DOCUMENT &&
		    (function->requestParameters == NULL ||
		     zend_hash_num_elements(function->requestParameters) == 0) &&
		    strcmp(params->name, function->functionName) == 0) {
			num_of_params = 0;
		} else if (num_of_params > 0) {
			tmp_parameters = safe_emalloc(num_of_params, sizeof(zval *), 0);

			trav = params;
			while (trav != 0 && cur_param < num_of_params) {
				if (trav->type == XML_ELEMENT_NODE) {
					encodePtr enc;
					sdlParamPtr *param = NULL;
					if (function != NULL &&
					    zend_hash_index_find(function->requestParameters, cur_param, (void **)&param) == FAILURE) {
						TSRMLS_FETCH();
						soap_server_fault("Client", "Error cannot find parameter", NULL, NULL, NULL TSRMLS_CC);
					}
					if (param == NULL) {
						enc = NULL;
					} else {
						enc = (*param)->encode;
					}
					tmp_parameters[cur_param] = master_to_zval(enc, trav);
					cur_param++;
				}
				trav = trav->next;
			}
		}
	}
	if (num_of_params > cur_param) {
		TSRMLS_FETCH();
		soap_server_fault("Client","Missing parameter", NULL, NULL, NULL TSRMLS_CC);
	}
	(*parameters) = tmp_parameters;
	(*num_params) = num_of_params;
}

static sdlFunctionPtr find_function(sdlPtr sdl, xmlNodePtr func, zval* function_name)
{
	sdlFunctionPtr function;

	function = get_function(sdl, (char*)func->name);
	if (function && function->binding && function->binding->bindingType == BINDING_SOAP) {
		sdlSoapBindingFunctionPtr fnb = (sdlSoapBindingFunctionPtr)function->bindingAttributes;
		if (fnb->style == SOAP_DOCUMENT) {
			if (func->children != NULL ||
			    (function->requestParameters != NULL &&
			     zend_hash_num_elements(function->requestParameters) > 0)) {
				function = NULL;
			}
		}
	}
	if (sdl != NULL && function == NULL) {
		function = get_doc_function(sdl, func);
	}

	INIT_ZVAL(*function_name);
	if (function != NULL) {
		ZVAL_STRING(function_name, (char *)function->functionName, 1);
	} else {
		ZVAL_STRING(function_name, (char *)func->name, 1);
	}

	return function;
}

static sdlFunctionPtr deserialize_function_call(sdlPtr sdl, xmlDocPtr request, char* actor, zval *function_name, int *num_params, zval ***parameters, int *version, soapHeader **headers TSRMLS_DC)
{
	char* envelope_ns = NULL;
	xmlNodePtr trav,env,head,body,func;
	xmlAttrPtr attr;
	sdlFunctionPtr function;

	encode_reset_ns();

	/* Get <Envelope> element */
	env = NULL;
	trav = request->children;
	while (trav != NULL) {
		if (trav->type == XML_ELEMENT_NODE) {
			if (env == NULL && node_is_equal_ex(trav,"Envelope",SOAP_1_1_ENV_NAMESPACE)) {
				env = trav;
				*version = SOAP_1_1;
				envelope_ns = SOAP_1_1_ENV_NAMESPACE;
				SOAP_GLOBAL(soap_version) = SOAP_1_1;
			} else if (env == NULL && node_is_equal_ex(trav,"Envelope",SOAP_1_2_ENV_NAMESPACE)) {
				env = trav;
				*version = SOAP_1_2;
				envelope_ns = SOAP_1_2_ENV_NAMESPACE;
				SOAP_GLOBAL(soap_version) = SOAP_1_2;
			} else {
				soap_server_fault("VersionMismatch", "Wrong Version", NULL, NULL, NULL TSRMLS_CC);
			}
		}
		trav = trav->next;
	}
	if (env == NULL) {
		soap_server_fault("Client", "looks like we got XML without \"Envelope\" element", NULL, NULL, NULL TSRMLS_CC);
	}

	attr = env->properties;
	while (attr != NULL) {
		if (attr->ns == NULL) {
			soap_server_fault("Client", "A SOAP Envelope element cannot have non Namespace qualified attributes", NULL, NULL, NULL TSRMLS_CC);
		} else if (attr_is_equal_ex(attr,"encodingStyle",SOAP_1_2_ENV_NAMESPACE)) {
			if (*version == SOAP_1_2) {
				soap_server_fault("Client", "encodingStyle cannot be specified on the Envelope", NULL, NULL, NULL TSRMLS_CC);
			} else if (strcmp((char*)attr->children->content,SOAP_1_1_ENC_NAMESPACE) != 0) {
				soap_server_fault("Client", "Unknown data encoding style", NULL, NULL, NULL TSRMLS_CC);
			}
		}
		attr = attr->next;
	}

	/* Get <Header> element */
	head = NULL;
	trav = env->children;
	while (trav != NULL && trav->type != XML_ELEMENT_NODE) {
		trav = trav->next;
	}
	if (trav != NULL && node_is_equal_ex(trav,"Header",envelope_ns)) {
		head = trav;
		trav = trav->next;
	}

	/* Get <Body> element */
	body = NULL;
	while (trav != NULL && trav->type != XML_ELEMENT_NODE) {
		trav = trav->next;
	}
	if (trav != NULL && node_is_equal_ex(trav,"Body",envelope_ns)) {
		body = trav;
		trav = trav->next;
	}
	while (trav != NULL && trav->type != XML_ELEMENT_NODE) {
		trav = trav->next;
	}
	if (body == NULL) {
		soap_server_fault("Client", "Body must be present in a SOAP envelope", NULL, NULL, NULL TSRMLS_CC);
	}
	attr = body->properties;
	while (attr != NULL) {
		if (attr->ns == NULL) {
			if (*version == SOAP_1_2) {
				soap_server_fault("Client", "A SOAP Body element cannot have non Namespace qualified attributes", NULL, NULL, NULL TSRMLS_CC);
			}
		} else if (attr_is_equal_ex(attr,"encodingStyle",SOAP_1_2_ENV_NAMESPACE)) {
			if (*version == SOAP_1_2) {
				soap_server_fault("Client", "encodingStyle cannot be specified on the Body", NULL, NULL, NULL TSRMLS_CC);
			} else if (strcmp((char*)attr->children->content,SOAP_1_1_ENC_NAMESPACE) != 0) {
				soap_server_fault("Client", "Unknown data encoding style", NULL, NULL, NULL TSRMLS_CC);
			}
		}
		attr = attr->next;
	}

	if (trav != NULL && *version == SOAP_1_2) {
		soap_server_fault("Client", "A SOAP 1.2 envelope can contain only Header and Body", NULL, NULL, NULL TSRMLS_CC);
	}

	func = NULL;
	trav = body->children;
	while (trav != NULL) {
		if (trav->type == XML_ELEMENT_NODE) {
/*
			if (func != NULL) {
				soap_server_fault("Client", "looks like we got \"Body\" with several functions call", NULL, NULL, NULL TSRMLS_CC);
			}
*/
			func = trav;
			break; /* FIXME: the rest of body is ignored */
		}
		trav = trav->next;
	}
	if (func == NULL) {
		function = get_doc_function(sdl, NULL);
		if (function != NULL) {
			INIT_ZVAL(*function_name);
			ZVAL_STRING(function_name, (char *)function->functionName, 1);
		} else {
			soap_server_fault("Client", "looks like we got \"Body\" without function call", NULL, NULL, NULL TSRMLS_CC);
		}
	} else {
		if (*version == SOAP_1_1) {
			attr = get_attribute_ex(func->properties,"encodingStyle",SOAP_1_1_ENV_NAMESPACE);
			if (attr && strcmp((char*)attr->children->content,SOAP_1_1_ENC_NAMESPACE) != 0) {
				soap_server_fault("Client","Unknown Data Encoding Style", NULL, NULL, NULL TSRMLS_CC);
			}
		} else {
			attr = get_attribute_ex(func->properties,"encodingStyle",SOAP_1_2_ENV_NAMESPACE);
			if (attr && strcmp((char*)attr->children->content,SOAP_1_2_ENC_NAMESPACE) != 0) {
				soap_server_fault("DataEncodingUnknown","Unknown Data Encoding Style", NULL, NULL, NULL TSRMLS_CC);
			}
		}
		function = find_function(sdl, func, function_name);
		if (sdl != NULL && function == NULL) {
			if (*version == SOAP_1_2) {
				soap_server_fault("rpc:ProcedureNotPresent","Procedure not present", NULL, NULL, NULL TSRMLS_CC);
			} else {
				php_error(E_ERROR, "Procedure '%s' not present", func->name);
			}
		}
	}

	*headers = NULL;
	if (head) {
		soapHeader *h, *last = NULL;

		attr = head->properties;
		while (attr != NULL) {
			if (attr->ns == NULL) {
				soap_server_fault("Client", "A SOAP Header element cannot have non Namespace qualified attributes", NULL, NULL, NULL TSRMLS_CC);
			} else if (attr_is_equal_ex(attr,"encodingStyle",SOAP_1_2_ENV_NAMESPACE)) {
				if (*version == SOAP_1_2) {
					soap_server_fault("Client", "encodingStyle cannot be specified on the Header", NULL, NULL, NULL TSRMLS_CC);
				} else if (strcmp((char*)attr->children->content,SOAP_1_1_ENC_NAMESPACE) != 0) {
					soap_server_fault("Client", "Unknown data encoding style", NULL, NULL, NULL TSRMLS_CC);
				}
			}
			attr = attr->next;
		}
		trav = head->children;
		while (trav != NULL) {
			if (trav->type == XML_ELEMENT_NODE) {
				xmlNodePtr hdr_func = trav;
				xmlAttrPtr attr;
				int mustUnderstand = 0;

				if (*version == SOAP_1_1) {
					attr = get_attribute_ex(hdr_func->properties,"encodingStyle",SOAP_1_1_ENV_NAMESPACE);
					if (attr && strcmp((char*)attr->children->content,SOAP_1_1_ENC_NAMESPACE) != 0) {
						soap_server_fault("Client","Unknown Data Encoding Style", NULL, NULL, NULL TSRMLS_CC);
					}
					attr = get_attribute_ex(hdr_func->properties,"actor",envelope_ns);
					if (attr != NULL) {
						if (strcmp((char*)attr->children->content,SOAP_1_1_ACTOR_NEXT) != 0 &&
						    (actor == NULL || strcmp((char*)attr->children->content,actor) != 0)) {
						  goto ignore_header;
						}
					}
				} else if (*version == SOAP_1_2) {
					attr = get_attribute_ex(hdr_func->properties,"encodingStyle",SOAP_1_2_ENV_NAMESPACE);
					if (attr && strcmp((char*)attr->children->content,SOAP_1_2_ENC_NAMESPACE) != 0) {
						soap_server_fault("DataEncodingUnknown","Unknown Data Encoding Style", NULL, NULL, NULL TSRMLS_CC);
					}
					attr = get_attribute_ex(hdr_func->properties,"role",envelope_ns);
					if (attr != NULL) {
						if (strcmp((char*)attr->children->content,SOAP_1_2_ACTOR_UNLIMATERECEIVER) != 0 &&
						    strcmp((char*)attr->children->content,SOAP_1_2_ACTOR_NEXT) != 0 &&
						    (actor == NULL || strcmp((char*)attr->children->content,actor) != 0)) {
						  goto ignore_header;
						}
					}
				}
				attr = get_attribute_ex(hdr_func->properties,"mustUnderstand",envelope_ns);
				if (attr) {
					if (strcmp((char*)attr->children->content,"1") == 0 ||
					    strcmp((char*)attr->children->content,"true") == 0) {
						mustUnderstand = 1;
					} else if (strcmp((char*)attr->children->content,"0") == 0 ||
					           strcmp((char*)attr->children->content,"false") == 0) {
						mustUnderstand = 0;
					} else {
						soap_server_fault("Client","mustUnderstand value is not boolean", NULL, NULL, NULL TSRMLS_CC);
					}
				}
				h = emalloc(sizeof(soapHeader));
				memset(h, 0, sizeof(soapHeader));
				h->mustUnderstand = mustUnderstand;
				h->function = find_function(sdl, hdr_func, &h->function_name);
				if (!h->function && sdl && function && function->binding && function->binding->bindingType == BINDING_SOAP) {
					sdlSoapBindingFunctionHeaderPtr *hdr;
					sdlSoapBindingFunctionPtr fnb = (sdlSoapBindingFunctionPtr)function->bindingAttributes;
					if (fnb->input.headers) {
						smart_str key = {0};

						if (hdr_func->ns) {
							smart_str_appends(&key, (char*)hdr_func->ns->href);
							smart_str_appendc(&key, ':');
						}
						smart_str_appendl(&key, Z_STRVAL(h->function_name), Z_STRLEN(h->function_name));
						smart_str_0(&key);
						if (zend_hash_find(fnb->input.headers, key.c, key.len+1, (void**)&hdr) == SUCCESS) {
							h->hdr = *hdr;
						}
						smart_str_free(&key);
					}
				}
				if (h->hdr) {
					h->num_params = 1;
					h->parameters = emalloc(sizeof(zval*));
					h->parameters[0] = master_to_zval(h->hdr->encode, hdr_func);
				} else {
					if (h->function && h->function->binding && h->function->binding->bindingType == BINDING_SOAP) {
						sdlSoapBindingFunctionPtr fnb = (sdlSoapBindingFunctionPtr)h->function->bindingAttributes;
						if (fnb->style == SOAP_RPC) {
							hdr_func = hdr_func->children;
						}
					}
					deserialize_parameters(hdr_func, h->function, &h->num_params, &h->parameters);
				}
				INIT_ZVAL(h->retval);
				if (last == NULL) {
					*headers = h;
				} else {
					last->next = h;
				}
				last = h;
			}
ignore_header:
			trav = trav->next;
		}
	}

	if (function && function->binding && function->binding->bindingType == BINDING_SOAP) {
		sdlSoapBindingFunctionPtr fnb = (sdlSoapBindingFunctionPtr)function->bindingAttributes;
		if (fnb->style == SOAP_RPC) {
			func = func->children;
		}
	} else {
		func = func->children;
	}
	deserialize_parameters(func, function, num_params, parameters);
	
	encode_finish();

	return function;
}

static int serialize_response_call2(xmlNodePtr body, sdlFunctionPtr function, char *function_name, char *uri, zval *ret, int version, int main TSRMLS_DC)
{
	xmlNodePtr method = NULL, param;
	sdlParamPtr parameter = NULL;
	int param_count;
	int style, use;
	xmlNsPtr ns = NULL;

	if (function != NULL && function->binding->bindingType == BINDING_SOAP) {
		sdlSoapBindingFunctionPtr fnb = (sdlSoapBindingFunctionPtr)function->bindingAttributes;

		style = fnb->style;
		use = fnb->output.use;
		if (style == SOAP_RPC) {
			ns = encode_add_ns(body, fnb->output.ns);
			if (function->responseName) {
				method = xmlNewChild(body, ns, BAD_CAST(function->responseName), NULL);
			} else if (function->responseParameters) {
				method = xmlNewChild(body, ns, BAD_CAST(function->functionName), NULL);
			}
		}
	} else {
		style = main?SOAP_RPC:SOAP_DOCUMENT;
		use = main?SOAP_ENCODED:SOAP_LITERAL;
		if (style == SOAP_RPC) {
			ns = encode_add_ns(body, uri);
			method = xmlNewChild(body, ns, BAD_CAST(function_name), NULL);
		}
	}

	if (function != NULL) {
		if (function->responseParameters) {
			param_count = zend_hash_num_elements(function->responseParameters);
		} else {
		  param_count = 0;
		}
	} else {
	  param_count = 1;
	}

	if (param_count == 1) {
		parameter = get_param(function, NULL, 0, TRUE);

		if (style == SOAP_RPC) {
		  xmlNode *rpc_result;
			if (main && version == SOAP_1_2) {
				xmlNs *rpc_ns = xmlNewNs(body, BAD_CAST(RPC_SOAP12_NAMESPACE), BAD_CAST(RPC_SOAP12_NS_PREFIX));
				rpc_result = xmlNewChild(method, rpc_ns, BAD_CAST("result"), NULL);
				param = serialize_parameter(parameter, ret, 0, "return", use, method TSRMLS_CC);
				xmlNodeSetContent(rpc_result,param->name);
			} else {
				param = serialize_parameter(parameter, ret, 0, "return", use, method TSRMLS_CC);
			}
		} else {
			param = serialize_parameter(parameter, ret, 0, "return", use, body TSRMLS_CC);
			if (function && function->binding->bindingType == BINDING_SOAP) {
				if (parameter && parameter->element) {
					ns = encode_add_ns(param, parameter->element->namens);
					xmlNodeSetName(param, BAD_CAST(parameter->element->name));
					xmlSetNs(param, ns);
				}
			} else if (strcmp((char*)param->name,"return") == 0) {
				ns = encode_add_ns(param, uri);
				xmlNodeSetName(param, BAD_CAST(function_name));
				xmlSetNs(param, ns);
			}
		}
	} else if (param_count > 1 && Z_TYPE_P(ret) == IS_ARRAY) {
		HashPosition pos;
		zval **data;
		int i = 0;

		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(ret), &pos);
		while (zend_hash_get_current_data_ex(Z_ARRVAL_P(ret), (void **)&data, &pos) != FAILURE) {
			zend_uchar param_name_type;
			zstr param_name = NULL_ZSTR;
			unsigned int param_name_len;
			ulong param_index = i;

			param_name_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(ret), &param_name, &param_name_len, &param_index, 0, &pos);
			param_name.s = soap_encode_string_ex(param_name_type, param_name, param_name_len TSRMLS_CC);
			param_name_len = strlen(param_name.s);
			parameter = get_param(function, param_name.s, param_index, TRUE);
			if (style == SOAP_RPC) {
				param = serialize_parameter(parameter, *data, i, param_name.s, use, method TSRMLS_CC);
			} else {
				param = serialize_parameter(parameter, *data, i, param_name.s, use, body TSRMLS_CC);
				if (function && function->binding->bindingType == BINDING_SOAP) {
					if (parameter && parameter->element) {
						ns = encode_add_ns(param, parameter->element->namens);
						xmlNodeSetName(param, BAD_CAST(parameter->element->name));
						xmlSetNs(param, ns);
					}
				}
			}
			efree(param_name.s);

			zend_hash_move_forward_ex(Z_ARRVAL_P(ret), &pos);
			i++;
		}
	}
	if (use == SOAP_ENCODED && version == SOAP_1_2 && method != NULL) {
		xmlSetNsProp(method, body->ns, BAD_CAST("encodingStyle"), BAD_CAST(SOAP_1_2_ENC_NAMESPACE));
	}
	return use;
}

static xmlDocPtr serialize_response_call(sdlFunctionPtr function, char *function_name, char *uri, zval *ret, soapHeader* headers, int version TSRMLS_DC)
{
	xmlDocPtr doc;
	xmlNodePtr envelope = NULL, body, param;
	xmlNsPtr ns = NULL;
	int use = SOAP_LITERAL;
	xmlNodePtr head = NULL;

	encode_reset_ns();

	doc = xmlNewDoc(BAD_CAST("1.0"));
	doc->charset = XML_CHAR_ENCODING_UTF8;
	doc->encoding = xmlCharStrdup("UTF-8");

	if (version == SOAP_1_1) {
		envelope = xmlNewDocNode(doc, NULL, BAD_CAST("Envelope"), NULL);
		ns = xmlNewNs(envelope, BAD_CAST(SOAP_1_1_ENV_NAMESPACE), BAD_CAST(SOAP_1_1_ENV_NS_PREFIX));
		xmlSetNs(envelope,ns);
	} else if (version == SOAP_1_2) {
		envelope = xmlNewDocNode(doc, NULL, BAD_CAST("Envelope"), NULL);
		ns = xmlNewNs(envelope, BAD_CAST(SOAP_1_2_ENV_NAMESPACE), BAD_CAST(SOAP_1_2_ENV_NS_PREFIX));
		xmlSetNs(envelope,ns);
	} else {
		soap_server_fault("Server", "Unknown SOAP version", NULL, NULL, NULL TSRMLS_CC);
	}
	xmlDocSetRootElement(doc, envelope);

	if (Z_TYPE_P(ret) == IS_OBJECT &&
	    instanceof_function(Z_OBJCE_P(ret), soap_fault_class_entry TSRMLS_CC)) {
	  char *detail_name;
		HashTable* prop;
		zval **tmp;
		sdlFaultPtr fault = NULL;
		char *fault_ns = NULL;

		prop = Z_OBJPROP_P(ret);

		if (headers &&
		    zend_hash_find(prop, "headerfault", sizeof("headerfault"), (void**)&tmp) == SUCCESS) {
			xmlNodePtr head;
			encodePtr hdr_enc = NULL;
			int hdr_use = SOAP_LITERAL;
			zval *hdr_ret  = *tmp;
			char *hdr_ns   = headers->hdr?headers->hdr->ns:NULL;
			char *hdr_name = Z_STRVAL(headers->function_name);
			int free_ns = 0, free_name = 0;

			head = xmlNewChild(envelope, ns, BAD_CAST("Header"), NULL);
			if (Z_TYPE_P(hdr_ret) == IS_OBJECT &&
			    instanceof_function(Z_OBJCE_P(hdr_ret), soap_header_class_entry TSRMLS_CC)) {
				HashTable* ht = Z_OBJPROP_P(hdr_ret);
				zval **tmp;
				sdlSoapBindingFunctionHeaderPtr *hdr;
				smart_str key = {0};

				if (zend_hash_find(ht, "namespace", sizeof("namespace"), (void**)&tmp) == SUCCESS) {
					if (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE) {
						char *str = soap_encode_string(*tmp, NULL TSRMLS_CC);

						smart_str_appends(&key, str);
						smart_str_appendc(&key, ':');
						hdr_ns = str;
						free_ns = 1;
					}
				}
				if (zend_hash_find(ht, "name", sizeof("name"), (void**)&tmp) == SUCCESS) {
					if (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE) {
						char *str = soap_encode_string(*tmp, NULL TSRMLS_CC);

						smart_str_appends(&key, str);
						hdr_name = str;
						free_name = 1;
					}
				}
				smart_str_0(&key);
				if (headers->hdr && headers->hdr->headerfaults &&
				    zend_hash_find(headers->hdr->headerfaults, key.c, key.len+1, (void**)&hdr) == SUCCESS) {
					hdr_enc = (*hdr)->encode;
					hdr_use = (*hdr)->use;
				}
				smart_str_free(&key);
				if (zend_hash_find(ht, "data", sizeof("data"), (void**)&tmp) == SUCCESS) {
					hdr_ret = *tmp;
				} else {
					hdr_ret = NULL;
				}
			}

			if (headers->function) {
				if (serialize_response_call2(head, headers->function, Z_STRVAL(headers->function_name), uri, hdr_ret, version, 0 TSRMLS_CC) == SOAP_ENCODED) {
					use = SOAP_ENCODED;
				}
			} else {
				xmlNodePtr xmlHdr = master_to_xml(hdr_enc, hdr_ret, hdr_use, head);
				if (hdr_name) {
					xmlNodeSetName(xmlHdr, BAD_CAST(hdr_name));
				}
				if (hdr_ns) {
					xmlNsPtr nsptr = encode_add_ns(xmlHdr, hdr_ns);
					xmlSetNs(xmlHdr, nsptr);
				}
			}

			if (free_ns) {
				efree(hdr_ns);
			}
			if (free_name) {
				efree(hdr_name);
			}
		}

		body = xmlNewChild(envelope, ns, BAD_CAST("Body"), NULL);
		param = xmlNewChild(body, ns, BAD_CAST("Fault"), NULL);

		if (zend_hash_find(prop, "faultcodens", sizeof("faultcodens"), (void**)&tmp) == SUCCESS &&
			(Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE)) {
			fault_ns = soap_encode_string(*tmp, NULL TSRMLS_CC);
		}
		use = SOAP_LITERAL;
		if (zend_hash_find(prop, "_name", sizeof("_name"), (void**)&tmp) == SUCCESS && Z_TYPE_PP(tmp) == IS_STRING) {
			sdlFaultPtr *tmp_fault;
			if (function && function->faults &&
			    zend_hash_find(function->faults, Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp)+1, (void**)&tmp_fault) == SUCCESS) {
			  fault = *tmp_fault;
				if (function->binding &&
				    function->binding->bindingType == BINDING_SOAP &&
				    fault->bindingAttributes) {
					sdlSoapBindingFunctionFaultPtr fb = (sdlSoapBindingFunctionFaultPtr)fault->bindingAttributes;
					use = fb->use;
					if (fault_ns == NULL) {
						fault_ns = estrdup(fb->ns);
					}
				}
			}
		} else if (function && function->faults &&
		           zend_hash_num_elements(function->faults) == 1) {

			zend_hash_internal_pointer_reset(function->faults);
			zend_hash_get_current_data(function->faults, (void**)&fault);
			fault = *(sdlFaultPtr*)fault;
			if (function->binding &&
			    function->binding->bindingType == BINDING_SOAP &&
			    fault->bindingAttributes) {
				sdlSoapBindingFunctionFaultPtr fb = (sdlSoapBindingFunctionFaultPtr)fault->bindingAttributes;
				use = fb->use;
				if (fault_ns == NULL) {
				  fault_ns = estrdup(fb->ns);
				}
			}
		}

		if (fault_ns == NULL &&
		    fault &&
		    fault->details &&
		    zend_hash_num_elements(fault->details) == 1) {
			sdlParamPtr sparam;

			zend_hash_internal_pointer_reset(fault->details);
			zend_hash_get_current_data(fault->details, (void**)&sparam);
			sparam = *(sdlParamPtr*)sparam;
			if (sparam->element) {
				fault_ns = estrdup(sparam->element->namens);
			}
		}

		if (version == SOAP_1_1) {
			if (zend_hash_find(prop, "faultcode", sizeof("faultcode"), (void**)&tmp) == SUCCESS) {
				int new_len;
				xmlNodePtr node = xmlNewNode(NULL, BAD_CAST("faultcode"));
				char *str = soap_encode_string(*tmp, &new_len TSRMLS_CC);
				xmlAddChild(param, node);
				if (fault_ns) {
					xmlNsPtr nsptr = encode_add_ns(node, fault_ns);
					xmlChar *code = xmlBuildQName(BAD_CAST(str), nsptr->prefix, NULL, 0);
					xmlNodeSetContent(node, code);
					xmlFree(code);
				} else {
					xmlNodeSetContentLen(node, BAD_CAST(str), new_len);
				}
				efree(str);
			}
			if (zend_hash_find(prop, "faultstring", sizeof("faultstring"), (void**)&tmp) == SUCCESS) {
				xmlNodePtr node = master_to_xml(get_conversion(IS_STRING), *tmp, SOAP_LITERAL, param);
				xmlNodeSetName(node, BAD_CAST("faultstring"));
			}
			if (zend_hash_find(prop, "faultactor", sizeof("faultactor"), (void**)&tmp) == SUCCESS) {
				xmlNodePtr node = master_to_xml(get_conversion(IS_STRING), *tmp, SOAP_LITERAL, param);
				xmlNodeSetName(node, BAD_CAST("faultactor"));
			}
			detail_name = "detail";
		} else {
			if (zend_hash_find(prop, "faultcode", sizeof("faultcode"), (void**)&tmp) == SUCCESS) {
				int new_len;
				xmlNodePtr node = xmlNewChild(param, ns, BAD_CAST("Code"), NULL);
				char *str = soap_encode_string(*tmp, &new_len TSRMLS_CC);
				node = xmlNewChild(node, ns, BAD_CAST("Value"), NULL);
				if (fault_ns) {
					xmlNsPtr nsptr = encode_add_ns(node, fault_ns);
					xmlChar *code = xmlBuildQName(BAD_CAST(str), nsptr->prefix, NULL, 0);
					xmlNodeSetContent(node, code);
					xmlFree(code);
				} else {
					xmlNodeSetContentLen(node, BAD_CAST(str), new_len);
				}
				efree(str);
			}
			if (zend_hash_find(prop, "faultstring", sizeof("faultstring"), (void**)&tmp) == SUCCESS) {
				xmlNodePtr node = xmlNewChild(param, ns, BAD_CAST("Reason"), NULL);
				node = master_to_xml(get_conversion(IS_STRING), *tmp, SOAP_LITERAL, node);
				xmlNodeSetName(node, BAD_CAST("Text"));
				xmlSetNs(node, ns);
			}
			detail_name = SOAP_1_2_ENV_NS_PREFIX":Detail";
		}
		if (fault_ns) {
			efree(fault_ns);
		}
		if (fault && fault->details && zend_hash_num_elements(fault->details) == 1) {
			xmlNodePtr node;
			zval *detail = NULL;
			sdlParamPtr sparam;
			xmlNodePtr x;

			if (zend_hash_find(prop, "detail", sizeof("detail"), (void**)&tmp) == SUCCESS &&
			    Z_TYPE_PP(tmp) != IS_NULL) {
				detail = *tmp;
			}
			node = xmlNewNode(NULL, BAD_CAST(detail_name));
			xmlAddChild(param, node);

			zend_hash_internal_pointer_reset(fault->details);
			zend_hash_get_current_data(fault->details, (void**)&sparam);
			sparam = *(sdlParamPtr*)sparam;

			if (detail &&
			    Z_TYPE_P(detail) == IS_OBJECT &&
			    sparam->element &&
			    zend_hash_num_elements(Z_OBJPROP_P(detail)) == 1 &&
			    zend_hash_find(Z_OBJPROP_P(detail), sparam->element->name, strlen(sparam->element->name)+1, (void**)&tmp) == SUCCESS) {
				detail = *tmp;
			}

			x = serialize_parameter(sparam, detail, 1, NULL, use, node TSRMLS_CC);

			if (function &&
			    function->binding &&
			    function->binding->bindingType == BINDING_SOAP &&
			    function->bindingAttributes) {
				sdlSoapBindingFunctionPtr fnb = (sdlSoapBindingFunctionPtr)function->bindingAttributes;
				if (fnb->style == SOAP_RPC && !sparam->element) {
				  if (fault->bindingAttributes) {
						sdlSoapBindingFunctionFaultPtr fb = (sdlSoapBindingFunctionFaultPtr)fault->bindingAttributes;
						if (fb->ns) {
							xmlNsPtr ns = encode_add_ns(x, fb->ns);
							xmlSetNs(x, ns);
						}
					}
				} else {
					if (sparam->element) {
						xmlNsPtr ns = encode_add_ns(x, sparam->element->namens);
						xmlNodeSetName(x, BAD_CAST(sparam->element->name));
						xmlSetNs(x, ns);
					}
				}
			}
			if (use == SOAP_ENCODED && version == SOAP_1_2) {
				xmlSetNsProp(x, envelope->ns, BAD_CAST("encodingStyle"), BAD_CAST(SOAP_1_2_ENC_NAMESPACE));
			}
		} else if (zend_hash_find(prop, "detail", sizeof("detail"), (void**)&tmp) == SUCCESS &&
		    Z_TYPE_PP(tmp) != IS_NULL) {
			serialize_zval(*tmp, NULL, detail_name, use, param TSRMLS_CC);
		}
	} else {

		if (headers) {
			soapHeader *h;

			head = xmlNewChild(envelope, ns, BAD_CAST("Header"), NULL);
			h = headers;
			while (h != NULL) {
				if (Z_TYPE(h->retval) != IS_NULL) {
					encodePtr hdr_enc = NULL;
					int hdr_use = SOAP_LITERAL;
					zval *hdr_ret = &h->retval;
					char *hdr_ns   = h->hdr?h->hdr->ns:NULL;
					char *hdr_name = Z_STRVAL(h->function_name);
					int free_ns = 0, free_name = 0;

					if (Z_TYPE(h->retval) == IS_OBJECT &&
					    instanceof_function(Z_OBJCE(h->retval), soap_header_class_entry TSRMLS_CC)) {
						HashTable* ht = Z_OBJPROP(h->retval);
						zval **tmp;
						sdlSoapBindingFunctionHeaderPtr *hdr;
						smart_str key = {0};

						if (zend_hash_find(ht, "namespace", sizeof("namespace"), (void**)&tmp) == SUCCESS) {
					    	if (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE) {
								char *str = soap_encode_string(*tmp, NULL TSRMLS_CC);

								smart_str_appends(&key, str);
								smart_str_appendc(&key, ':');
								hdr_ns = str;
								free_ns = 1;
							}
						}
						if (zend_hash_find(ht, "name", sizeof("name"), (void**)&tmp) == SUCCESS) {
							if (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE) {
								char *str = soap_encode_string(*tmp, NULL TSRMLS_CC);

								smart_str_appends(&key, str);
								hdr_name = str;
								free_name = 1;
							}
						}
						smart_str_0(&key);
						if (function && function->binding && function->binding->bindingType == BINDING_SOAP) {
							sdlSoapBindingFunctionPtr fnb = (sdlSoapBindingFunctionPtr)function->bindingAttributes;

							if (fnb->output.headers &&
							    zend_hash_find(fnb->output.headers, key.c, key.len+1, (void**)&hdr) == SUCCESS) {
								hdr_enc = (*hdr)->encode;
								hdr_use = (*hdr)->use;
							}
						}
						smart_str_free(&key);
						if (zend_hash_find(ht, "data", sizeof("data"), (void**)&tmp) == SUCCESS) {
							hdr_ret = *tmp;
						} else {
							hdr_ret = NULL;
						}
					}

					if (h->function) {
						if (serialize_response_call2(head, h->function, Z_STRVAL(h->function_name), uri, hdr_ret, version, 0 TSRMLS_CC) == SOAP_ENCODED) {
							use = SOAP_ENCODED;
						}
					} else {
						xmlNodePtr xmlHdr = master_to_xml(hdr_enc, hdr_ret, hdr_use, head);
						if (hdr_name) {
							xmlNodeSetName(xmlHdr, BAD_CAST(hdr_name));
						}
						if (hdr_ns) {
							xmlNsPtr nsptr = encode_add_ns(xmlHdr,hdr_ns);
							xmlSetNs(xmlHdr, nsptr);
						}
					}

					if (free_ns) {
						efree(hdr_ns);
					}
					if (free_name) {
						efree(hdr_name);
					}
				}
				h = h->next;
			}

			if (head->children == NULL) {
				xmlUnlinkNode(head);
				xmlFreeNode(head);
			}
		}

		body = xmlNewChild(envelope, ns, BAD_CAST("Body"), NULL);

		if (serialize_response_call2(body, function, function_name, uri, ret, version, 1 TSRMLS_CC) == SOAP_ENCODED) {
			use = SOAP_ENCODED;
		}

	}

	if (use == SOAP_ENCODED) {
		xmlNewNs(envelope, BAD_CAST(XSD_NAMESPACE), BAD_CAST(XSD_NS_PREFIX));
		if (version == SOAP_1_1) {
			xmlNewNs(envelope, BAD_CAST(SOAP_1_1_ENC_NAMESPACE), BAD_CAST(SOAP_1_1_ENC_NS_PREFIX));
			xmlSetNsProp(envelope, envelope->ns, BAD_CAST("encodingStyle"), BAD_CAST(SOAP_1_1_ENC_NAMESPACE));
		} else if (version == SOAP_1_2) {
			xmlNewNs(envelope, BAD_CAST(SOAP_1_2_ENC_NAMESPACE), BAD_CAST(SOAP_1_2_ENC_NS_PREFIX));
		}
	}

	encode_finish();

	if (function && function->responseName == NULL &&
	    body->children == NULL && head == NULL) {
		xmlFreeDoc(doc);
		return NULL;
	}
	return doc;
}

static xmlDocPtr serialize_function_call(zval *this_ptr, sdlFunctionPtr function, char *function_name, char *uri, zval **arguments, int arg_count, int version, HashTable *soap_headers TSRMLS_DC)
{
	xmlDoc *doc;
	xmlNodePtr envelope = NULL, body, method = NULL, head = NULL;
	xmlNsPtr ns = NULL;
	int i, style, use;
	HashTable *hdrs = NULL;
	soap_client_object *client;

	client = (soap_client_object*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	encode_reset_ns();

	doc = xmlNewDoc(BAD_CAST("1.0"));
	doc->encoding = xmlCharStrdup("UTF-8");
	doc->charset = XML_CHAR_ENCODING_UTF8;
	if (version == SOAP_1_1) {
		envelope = xmlNewDocNode(doc, NULL, BAD_CAST("Envelope"), NULL);
		ns = xmlNewNs(envelope, BAD_CAST(SOAP_1_1_ENV_NAMESPACE), BAD_CAST(SOAP_1_1_ENV_NS_PREFIX));
		xmlSetNs(envelope, ns);
	} else if (version == SOAP_1_2) {
		envelope = xmlNewDocNode(doc, NULL, BAD_CAST("Envelope"), NULL);
		ns = xmlNewNs(envelope, BAD_CAST(SOAP_1_2_ENV_NAMESPACE), BAD_CAST(SOAP_1_2_ENV_NS_PREFIX));
		xmlSetNs(envelope, ns);
	} else {
		soap_error0(E_ERROR, "Unknown SOAP version");
	}
	xmlDocSetRootElement(doc, envelope);

	if (soap_headers) {
		head = xmlNewChild(envelope, ns, BAD_CAST("Header"), NULL);
	}

	body = xmlNewChild(envelope, ns, BAD_CAST("Body"), NULL);

	if (function && function->binding->bindingType == BINDING_SOAP) {
		sdlSoapBindingFunctionPtr fnb = (sdlSoapBindingFunctionPtr)function->bindingAttributes;

		hdrs = fnb->input.headers;
		style = fnb->style;
		/*FIXME: how to pass method name if style is SOAP_DOCUMENT */
		/*style = SOAP_RPC;*/
		use = fnb->input.use;
		if (style == SOAP_RPC) {
			ns = encode_add_ns(body, fnb->input.ns);
			if (function->requestName) {
				method = xmlNewChild(body, ns, BAD_CAST(function->requestName), NULL);
			} else {
				method = xmlNewChild(body, ns, BAD_CAST(function->functionName), NULL);
			}
		}
	} else {
		style = client->style;
		/*FIXME: how to pass method name if style is SOAP_DOCUMENT */
		/*style = SOAP_RPC;*/
		if (style == SOAP_RPC) {
			ns = encode_add_ns(body, uri);
			if (function_name) {
				method = xmlNewChild(body, ns, BAD_CAST(function_name), NULL);
			} else if (function && function->requestName) {
				method = xmlNewChild(body, ns, BAD_CAST(function->requestName), NULL);
			} else if (function && function->functionName) {
				method = xmlNewChild(body, ns, BAD_CAST(function->functionName), NULL);
			} else {
				method = body;
			}
		} else {
			method = body;
		}

		if (client->use == SOAP_LITERAL) {
			use = SOAP_LITERAL;
		} else {
			use = SOAP_ENCODED;
		}
	}

	for (i = 0;i < arg_count;i++) {
		xmlNodePtr param;
		sdlParamPtr parameter = get_param(function, NULL, i, FALSE);

		if (style == SOAP_RPC) {
			param = serialize_parameter(parameter, arguments[i], i, NULL, use, method TSRMLS_CC);
		} else if (style == SOAP_DOCUMENT) {
			param = serialize_parameter(parameter, arguments[i], i, NULL, use, body TSRMLS_CC);
			if (function && function->binding->bindingType == BINDING_SOAP) {
				if (parameter && parameter->element) {
					ns = encode_add_ns(param, parameter->element->namens);
					xmlNodeSetName(param, BAD_CAST(parameter->element->name));
					xmlSetNs(param, ns);
				}
			}
		}
	}

	if (function && function->requestParameters) {
		int n = zend_hash_num_elements(function->requestParameters);

		if (n > arg_count) {
			for (i = arg_count; i < n; i++) {
				xmlNodePtr param;
				sdlParamPtr parameter = get_param(function, NULL, i, FALSE);

				if (style == SOAP_RPC) {
					param = serialize_parameter(parameter, NULL, i, NULL, use, method TSRMLS_CC);
				} else if (style == SOAP_DOCUMENT) {
					param = serialize_parameter(parameter, NULL, i, NULL, use, body TSRMLS_CC);
					if (function && function->binding->bindingType == BINDING_SOAP) {
						if (parameter && parameter->element) {
							ns = encode_add_ns(param, parameter->element->namens);
							xmlNodeSetName(param, BAD_CAST(parameter->element->name));
							xmlSetNs(param, ns);
						}
					}
				}
			}
		}
	}

	if (head) {
		zval** header;

		zend_hash_internal_pointer_reset(soap_headers);
		while (zend_hash_get_current_data(soap_headers,(void**)&header) == SUCCESS) {
			HashTable *ht = Z_OBJPROP_PP(header);
			zval **name, **ns, **tmp;

			if (zend_hash_find(ht, "name", sizeof("name"), (void**)&name) == SUCCESS &&
			    (Z_TYPE_PP(name) == IS_STRING || Z_TYPE_PP(name) == IS_UNICODE) &&
			    zend_hash_find(ht, "namespace", sizeof("namespace"), (void**)&ns) == SUCCESS &&
			    (Z_TYPE_PP(ns) == IS_STRING || Z_TYPE_PP(ns) == IS_UNICODE)) {
				xmlNodePtr h;
				xmlNsPtr nsptr = NULL;
				int hdr_use = SOAP_LITERAL;
				encodePtr enc = NULL;
				char *name_str = NULL, *ns_str = NULL;

				if (hdrs) {
					smart_str key = {0};
					sdlSoapBindingFunctionHeaderPtr *hdr;

					ns_str = soap_encode_string(*ns, NULL TSRMLS_CC);
					smart_str_appends(&key, ns_str);

					smart_str_appendc(&key, ':');

					name_str = soap_encode_string(*name, NULL TSRMLS_CC);
					
					smart_str_appends(&key, name_str);
					smart_str_0(&key);

					if (zend_hash_find(hdrs, key.c, key.len+1,(void**)&hdr) == SUCCESS) {
						hdr_use = (*hdr)->use;
						enc = (*hdr)->encode;
						if (hdr_use == SOAP_ENCODED) {
							use = SOAP_ENCODED;
						}
					}
					smart_str_free(&key);
				} else {
					ns_str = soap_encode_string(*ns, NULL TSRMLS_CC);
					name_str = soap_encode_string(*name, NULL TSRMLS_CC);
				}

				if (zend_hash_find(ht, "data", sizeof("data"), (void**)&tmp) == SUCCESS) {
					h = master_to_xml(enc, *tmp, hdr_use, head);
					xmlNodeSetName(h, BAD_CAST(name_str?name_str:Z_STRVAL_PP(name)));
				} else {
					h = xmlNewNode(NULL, BAD_CAST(name_str?name_str:Z_STRVAL_PP(name)));
					xmlAddChild(head, h);
				}

				nsptr = encode_add_ns(h, ns_str?ns_str:Z_STRVAL_PP(ns));
				xmlSetNs(h, nsptr);

				if (name_str) {
					efree(name_str);
				}
				if (ns_str) {
					efree(ns_str);
				}

				if (zend_hash_find(ht, "mustUnderstand", sizeof("mustUnderstand"), (void**)&tmp) == SUCCESS &&
				    Z_TYPE_PP(tmp) == IS_BOOL && Z_LVAL_PP(tmp)) {
					if (version == SOAP_1_1) {
						xmlSetProp(h, BAD_CAST(SOAP_1_1_ENV_NS_PREFIX":mustUnderstand"), BAD_CAST("1"));
					} else {
						xmlSetProp(h, BAD_CAST(SOAP_1_2_ENV_NS_PREFIX":mustUnderstand"), BAD_CAST("true"));
					}
				}
				if (zend_hash_find(ht, "actor", sizeof("actor"), (void**)&tmp) == SUCCESS) {
					if (Z_TYPE_PP(tmp) == IS_STRING || Z_TYPE_PP(tmp) == IS_UNICODE) {
						char *str = soap_encode_string(*tmp, NULL TSRMLS_CC);

						if (version == SOAP_1_1) {
							xmlSetProp(h, BAD_CAST(SOAP_1_1_ENV_NS_PREFIX":actor"), BAD_CAST(str));
						} else {
							xmlSetProp(h, BAD_CAST(SOAP_1_2_ENV_NS_PREFIX":role"), BAD_CAST(str));
						}
						efree(str);
					} else if (Z_TYPE_PP(tmp) == IS_LONG) {
						if (version == SOAP_1_1) {
							if (Z_LVAL_PP(tmp) == SOAP_ACTOR_NEXT) {
								xmlSetProp(h, BAD_CAST(SOAP_1_1_ENV_NS_PREFIX":actor"), BAD_CAST(SOAP_1_1_ACTOR_NEXT));
							}
						} else {
							if (Z_LVAL_PP(tmp) == SOAP_ACTOR_NEXT) {
								xmlSetProp(h, BAD_CAST(SOAP_1_2_ENV_NS_PREFIX":role"), BAD_CAST(SOAP_1_2_ACTOR_NEXT));
							} else if (Z_LVAL_PP(tmp) == SOAP_ACTOR_NONE) {
								xmlSetProp(h, BAD_CAST(SOAP_1_2_ENV_NS_PREFIX":role"), BAD_CAST(SOAP_1_2_ACTOR_NONE));
							} else if (Z_LVAL_PP(tmp) == SOAP_ACTOR_UNLIMATERECEIVER) {
								xmlSetProp(h, BAD_CAST(SOAP_1_2_ENV_NS_PREFIX":role"), BAD_CAST(SOAP_1_2_ACTOR_UNLIMATERECEIVER));
							}
						}
					}
				}
			}
			zend_hash_move_forward(soap_headers);
		}
	}

	if (use == SOAP_ENCODED) {
		xmlNewNs(envelope, BAD_CAST(XSD_NAMESPACE), BAD_CAST(XSD_NS_PREFIX));
		if (version == SOAP_1_1) {
			xmlNewNs(envelope, BAD_CAST(SOAP_1_1_ENC_NAMESPACE), BAD_CAST(SOAP_1_1_ENC_NS_PREFIX));
			xmlSetNsProp(envelope, envelope->ns, BAD_CAST("encodingStyle"), BAD_CAST(SOAP_1_1_ENC_NAMESPACE));
		} else if (version == SOAP_1_2) {
			xmlNewNs(envelope, BAD_CAST(SOAP_1_2_ENC_NAMESPACE), BAD_CAST(SOAP_1_2_ENC_NS_PREFIX));
			if (method) {
				xmlSetNsProp(method, envelope->ns, BAD_CAST("encodingStyle"), BAD_CAST(SOAP_1_2_ENC_NAMESPACE));
			}
		}
	}

	encode_finish();

	return doc;
}

static xmlNodePtr serialize_parameter(sdlParamPtr param, zval *param_val, int index, char *name, int style, xmlNodePtr parent TSRMLS_DC)
{
	char *paramName;
	xmlNodePtr xmlParam;
	char paramNameBuf[10];
	int free_name = 0;

	if (param_val &&
	    Z_TYPE_P(param_val) == IS_OBJECT &&
	    Z_OBJCE_P(param_val) == soap_param_class_entry) {
		zval **param_name;
		zval **param_data;

		if (zend_hash_find(Z_OBJPROP_P(param_val), "param_name", sizeof("param_name"), (void **)&param_name) == SUCCESS &&
			(Z_TYPE_PP(param_name) == IS_STRING || Z_TYPE_PP(param_name) == IS_UNICODE) &&
		    zend_hash_find(Z_OBJPROP_P(param_val), "param_data", sizeof("param_data"), (void **)&param_data) == SUCCESS) {
			param_val = *param_data;
			name = soap_encode_string(*param_name, NULL TSRMLS_CC);
			free_name = 1;
		}
	}

	if (param != NULL && param->paramName != NULL) {
		paramName = param->paramName;
	} else {
		if (name == NULL) {
			paramName = paramNameBuf;
			snprintf(paramName, sizeof(paramNameBuf), "param%d",index);
		} else {
			paramName = name;
		}
	}

	xmlParam = serialize_zval(param_val, param, paramName, style, parent TSRMLS_CC);

	if (free_name) {
		efree(name);
	}

	return xmlParam;
}

static xmlNodePtr serialize_zval(zval *val, sdlParamPtr param, char *paramName, int style, xmlNodePtr parent TSRMLS_DC)
{
	xmlNodePtr xmlParam;
	encodePtr enc;
	zval defval;

	if (param != NULL) {
		enc = param->encode;
		if (val == NULL) {
			if (param->element) {
				if (param->element->fixed) {
					ZVAL_STRING(&defval, param->element->fixed, 0);
					val = &defval;
				} else if (param->element->def && !param->element->nillable) {
					ZVAL_STRING(&defval, param->element->def, 0);
					val = &defval;
				}
			}
		}
	} else {
		enc = NULL;
	}
	xmlParam = master_to_xml(enc, val, style, parent);
	if (!strcmp((char*)xmlParam->name, "BOGUS")) {
		xmlNodeSetName(xmlParam, BAD_CAST(paramName));
	}
	return xmlParam;
}

static sdlParamPtr get_param(sdlFunctionPtr function, char *param_name, int index, int response)
{
	sdlParamPtr *tmp;
	HashTable   *ht;

	if (function == NULL) {
		return NULL;
	}

	if (response == FALSE) {
		ht = function->requestParameters;
	} else {
		ht = function->responseParameters;
	}

	if (ht == NULL) {
	  return NULL;
	}

	if (param_name != NULL) {
		if (zend_hash_find(ht, param_name, strlen(param_name), (void **)&tmp) != FAILURE) {
			return *tmp;
		} else {
			HashPosition pos;

			zend_hash_internal_pointer_reset_ex(ht, &pos);
			while (zend_hash_get_current_data_ex(ht, (void **)&tmp, &pos) != FAILURE) {
				if ((*tmp)->paramName && strcmp(param_name, (*tmp)->paramName) == 0) {
					return *tmp;
				}
				zend_hash_move_forward_ex(ht, &pos);
			}
		}
	} else {
		if (zend_hash_index_find(ht, index, (void **)&tmp) != FAILURE) {
			return (*tmp);
		}
	}
	return NULL;
}

static sdlFunctionPtr get_function(sdlPtr sdl, const char *function_name)
{
	sdlFunctionPtr *tmp;

	int len = strlen(function_name);
	char *str = estrndup(function_name,len);
	php_strtolower(str,len);
	if (sdl != NULL) {
		if (zend_hash_find(&sdl->functions, str, len+1, (void **)&tmp) != FAILURE) {
			efree(str);
			return (*tmp);
		} else if (sdl->requests != NULL && zend_hash_find(sdl->requests, str, len+1, (void **)&tmp) != FAILURE) {
			efree(str);
			return (*tmp);
		}
	}
	efree(str);
	return NULL;
}

static sdlFunctionPtr get_doc_function(sdlPtr sdl, xmlNodePtr params)
{
	if (sdl) {
		sdlFunctionPtr *tmp;
		sdlParamPtr    *param;

		zend_hash_internal_pointer_reset(&sdl->functions);
		while (zend_hash_get_current_data(&sdl->functions, (void**)&tmp) == SUCCESS) {
			if ((*tmp)->binding && (*tmp)->binding->bindingType == BINDING_SOAP) {
				sdlSoapBindingFunctionPtr fnb = (sdlSoapBindingFunctionPtr)(*tmp)->bindingAttributes;
				if (fnb->style == SOAP_DOCUMENT) {
					if (params == NULL) {
						if ((*tmp)->requestParameters == NULL ||
						    zend_hash_num_elements((*tmp)->requestParameters) == 0) {
						  return *tmp;
						}
					} else if ((*tmp)->requestParameters != NULL &&
					           zend_hash_num_elements((*tmp)->requestParameters) > 0) {
						int ok = 1;
						xmlNodePtr node = params;

						zend_hash_internal_pointer_reset((*tmp)->requestParameters);
						while (zend_hash_get_current_data((*tmp)->requestParameters, (void**)&param) == SUCCESS) {
							if ((*param)->element) {
								if (strcmp((*param)->element->name, (char*)node->name) != 0) {
									ok = 0;
									break;
								}
								if ((*param)->element->namens != NULL && node->ns != NULL) {
									if (strcmp((*param)->element->namens, (char*)node->ns->href) != 0) {
										ok = 0;
										break;
									}
								} else if ((void*)(*param)->element->namens != (void*)node->ns) {
									ok = 0;
									break;
								}
							} else if (strcmp((*param)->paramName, (char*)node->name) != 0) {
								ok = 0;
								break;
							}
							zend_hash_move_forward((*tmp)->requestParameters);
							node = node->next;
						}
						if (ok /*&& node == NULL*/) {
							return (*tmp);
						}
					}
				}
			}
			zend_hash_move_forward(&sdl->functions);
		}
	}
	return NULL;
}

static void function_to_string(sdlFunctionPtr function, smart_str *buf)
{
	int i = 0;
	HashPosition pos;
	sdlParamPtr *param;

	if (function->responseParameters &&
	    zend_hash_num_elements(function->responseParameters) > 0) {
		if (zend_hash_num_elements(function->responseParameters) == 1) {
			zend_hash_internal_pointer_reset(function->responseParameters);
			zend_hash_get_current_data(function->responseParameters, (void**)&param);
			if ((*param)->encode && (*param)->encode->details.type_str) {
				smart_str_appendl(buf, (*param)->encode->details.type_str, strlen((*param)->encode->details.type_str));
				smart_str_appendc(buf, ' ');
			} else {
				smart_str_appendl(buf, "UNKNOWN ", 8);
			}
		} else {
			i = 0;
			smart_str_appendl(buf, "list(", 5);
			zend_hash_internal_pointer_reset_ex(function->responseParameters, &pos);
			while (zend_hash_get_current_data_ex(function->responseParameters, (void **)&param, &pos) != FAILURE) {
				if (i > 0) {
					smart_str_appendl(buf, ", ", 2);
				}
				if ((*param)->encode && (*param)->encode->details.type_str) {
					smart_str_appendl(buf, (*param)->encode->details.type_str, strlen((*param)->encode->details.type_str));
				} else {
					smart_str_appendl(buf, "UNKNOWN", 7);
				}
				smart_str_appendl(buf, " $", 2);
				smart_str_appendl(buf, (*param)->paramName, strlen((*param)->paramName));
				zend_hash_move_forward_ex(function->responseParameters, &pos);
				i++;
			}
			smart_str_appendl(buf, ") ", 2);
		}
	} else {
		smart_str_appendl(buf, "void ", 5);
	}

	smart_str_appendl(buf, function->functionName, strlen(function->functionName));

	smart_str_appendc(buf, '(');
	if (function->requestParameters) {
		i = 0;
		zend_hash_internal_pointer_reset_ex(function->requestParameters, &pos);
		while (zend_hash_get_current_data_ex(function->requestParameters, (void **)&param, &pos) != FAILURE) {
			if (i > 0) {
				smart_str_appendl(buf, ", ", 2);
			}
			if ((*param)->encode && (*param)->encode->details.type_str) {
				smart_str_appendl(buf, (*param)->encode->details.type_str, strlen((*param)->encode->details.type_str));
			} else {
				smart_str_appendl(buf, "UNKNOWN", 7);
			}
			smart_str_appendl(buf, " $", 2);
			smart_str_appendl(buf, (*param)->paramName, strlen((*param)->paramName));
			zend_hash_move_forward_ex(function->requestParameters, &pos);
			i++;
		}
	}
	smart_str_appendc(buf, ')');
	smart_str_0(buf);
}

static void model_to_string(sdlContentModelPtr model, smart_str *buf, int level)
{
	int i;

	switch (model->kind) {
		case XSD_CONTENT_ELEMENT:
			type_to_string(model->u.element, buf, level);
			smart_str_appendl(buf, ";\n", 2);
			break;
		case XSD_CONTENT_ANY:
			for (i = 0;i < level;i++) {
				smart_str_appendc(buf, ' ');
			}
			smart_str_appendl(buf, "<anyXML> any;\n", sizeof("<anyXML> any;\n")-1);
			break;
		case XSD_CONTENT_SEQUENCE:
		case XSD_CONTENT_ALL:
		case XSD_CONTENT_CHOICE: {
			sdlContentModelPtr *tmp;

			zend_hash_internal_pointer_reset(model->u.content);
			while (zend_hash_get_current_data(model->u.content, (void**)&tmp) == SUCCESS) {
				model_to_string(*tmp, buf, level);
				zend_hash_move_forward(model->u.content);
			}
			break;
		}
		case XSD_CONTENT_GROUP:
			model_to_string(model->u.group->model, buf, level);
		default:
		  break;
	}
}

static void type_to_string(sdlTypePtr type, smart_str *buf, int level)
{
	int i;
	smart_str spaces = {0};
	HashPosition pos;

	for (i = 0;i < level;i++) {
		smart_str_appendc(&spaces, ' ');
	}
	smart_str_appendl(buf, spaces.c, spaces.len);

	switch (type->kind) {
		case XSD_TYPEKIND_SIMPLE:
			if (type->encode) {
				smart_str_appendl(buf, type->encode->details.type_str, strlen(type->encode->details.type_str));
				smart_str_appendc(buf, ' ');
			} else {
				smart_str_appendl(buf, "anyType ", sizeof("anyType ")-1);
			}
			smart_str_appendl(buf, type->name, strlen(type->name));
			break;
		case XSD_TYPEKIND_LIST:
			smart_str_appendl(buf, "list ", 5);
			smart_str_appendl(buf, type->name, strlen(type->name));
			if (type->elements) {
				sdlTypePtr *item_type;

				smart_str_appendl(buf, " {", 2);
				zend_hash_internal_pointer_reset_ex(type->elements, &pos);
				if (zend_hash_get_current_data_ex(type->elements, (void **)&item_type, &pos) != FAILURE) {
					smart_str_appendl(buf, (*item_type)->name, strlen((*item_type)->name));
				}
				smart_str_appendc(buf, '}');
			}
			break;
		case XSD_TYPEKIND_UNION:
			smart_str_appendl(buf, "union ", 6);
			smart_str_appendl(buf, type->name, strlen(type->name));
			if (type->elements) {
				sdlTypePtr *item_type;
				int first = 0;

				smart_str_appendl(buf, " {", 2);
				zend_hash_internal_pointer_reset_ex(type->elements, &pos);
				while (zend_hash_get_current_data_ex(type->elements, (void **)&item_type, &pos) != FAILURE) {
					if (!first) {
						smart_str_appendc(buf, ',');
						first = 0;
					}
					smart_str_appendl(buf, (*item_type)->name, strlen((*item_type)->name));
					zend_hash_move_forward_ex(type->elements, &pos);
				}
				smart_str_appendc(buf, '}');
			}
			break;
		case XSD_TYPEKIND_COMPLEX:
		case XSD_TYPEKIND_RESTRICTION:
		case XSD_TYPEKIND_EXTENSION:
			if (type->encode &&
			    (type->encode->details.type == IS_ARRAY ||
			     type->encode->details.type == SOAP_ENC_ARRAY)) {
			  sdlAttributePtr *attr;
			  sdlExtraAttributePtr *ext;

				if (type->attributes &&
				    zend_hash_find(type->attributes, SOAP_1_1_ENC_NAMESPACE":arrayType",
				      sizeof(SOAP_1_1_ENC_NAMESPACE":arrayType"),
				      (void **)&attr) == SUCCESS &&
				      zend_hash_find((*attr)->extraAttributes, WSDL_NAMESPACE":arrayType", sizeof(WSDL_NAMESPACE":arrayType"), (void **)&ext) == SUCCESS) {
					char *end = strchr((*ext)->val, '[');
					int len;
					if (end == NULL) {
						len = strlen((*ext)->val);
					} else {
						len = end-(*ext)->val;
					}
					if (len == 0) {
						smart_str_appendl(buf, "anyType", sizeof("anyType")-1);
					} else {
						smart_str_appendl(buf, (*ext)->val, len);
					}
					smart_str_appendc(buf, ' ');
					smart_str_appendl(buf, type->name, strlen(type->name));
					if (end != NULL) {
						smart_str_appends(buf, end);
					}
				} else {
					sdlTypePtr elementType;
					if (type->attributes &&
					    zend_hash_find(type->attributes, SOAP_1_2_ENC_NAMESPACE":itemType",
					      sizeof(SOAP_1_2_ENC_NAMESPACE":itemType"),
					      (void **)&attr) == SUCCESS &&
					      zend_hash_find((*attr)->extraAttributes, WSDL_NAMESPACE":itemType", sizeof(WSDL_NAMESPACE":arrayType"), (void **)&ext) == SUCCESS) {
						smart_str_appends(buf, (*ext)->val);
						smart_str_appendc(buf, ' ');
					} else if (type->elements &&
					           zend_hash_num_elements(type->elements) == 1 &&
					           (zend_hash_internal_pointer_reset(type->elements),
					            zend_hash_get_current_data(type->elements, (void**)&elementType) == SUCCESS) &&
					           (elementType = *(sdlTypePtr*)elementType) != NULL &&
					           elementType->encode && elementType->encode->details.type_str) {
						smart_str_appends(buf, elementType->encode->details.type_str);
						smart_str_appendc(buf, ' ');
					} else {
						smart_str_appendl(buf, "anyType ", 8);
					}
					smart_str_appendl(buf, type->name, strlen(type->name));
					if (type->attributes &&
					    zend_hash_find(type->attributes, SOAP_1_2_ENC_NAMESPACE":arraySize",
					      sizeof(SOAP_1_2_ENC_NAMESPACE":arraySize"),
					      (void **)&attr) == SUCCESS &&
					      zend_hash_find((*attr)->extraAttributes, WSDL_NAMESPACE":itemType", sizeof(WSDL_NAMESPACE":arraySize"), (void **)&ext) == SUCCESS) {
						smart_str_appendc(buf, '[');
						smart_str_appends(buf, (*ext)->val);
						smart_str_appendc(buf, ']');
					} else {
						smart_str_appendl(buf, "[]", 2);
					}
				}
			} else {
				smart_str_appendl(buf, "struct ", 7);
				smart_str_appendl(buf, type->name, strlen(type->name));
				smart_str_appendc(buf, ' ');
				smart_str_appendl(buf, "{\n", 2);
				if ((type->kind == XSD_TYPEKIND_RESTRICTION ||
				     type->kind == XSD_TYPEKIND_EXTENSION) && type->encode) {
					encodePtr enc = type->encode;
					while (enc && enc->details.sdl_type &&
					       enc != enc->details.sdl_type->encode &&
					       enc->details.sdl_type->kind != XSD_TYPEKIND_SIMPLE &&
					       enc->details.sdl_type->kind != XSD_TYPEKIND_LIST &&
					       enc->details.sdl_type->kind != XSD_TYPEKIND_UNION) {
						enc = enc->details.sdl_type->encode;
					}
					if (enc) {
						smart_str_appendl(buf, spaces.c, spaces.len);
						smart_str_appendc(buf, ' ');
						smart_str_appendl(buf, type->encode->details.type_str, strlen(type->encode->details.type_str));
						smart_str_appendl(buf, " _;\n", 4);
					}
				}
				if (type->model) {
					model_to_string(type->model, buf, level+1);
				}
				if (type->attributes) {
					sdlAttributePtr *attr;

					zend_hash_internal_pointer_reset_ex(type->attributes, &pos);
					while (zend_hash_get_current_data_ex(type->attributes, (void **)&attr, &pos) != FAILURE) {
						smart_str_appendl(buf, spaces.c, spaces.len);
						smart_str_appendc(buf, ' ');
						if ((*attr)->encode && (*attr)->encode->details.type_str) {
							smart_str_appends(buf, (*attr)->encode->details.type_str);
							smart_str_appendc(buf, ' ');
						} else {
							smart_str_appendl(buf, "UNKNOWN ", 8);
						}
						smart_str_appends(buf, (*attr)->name);
						smart_str_appendl(buf, ";\n", 2);
						zend_hash_move_forward_ex(type->attributes, &pos);
					}
				}
				smart_str_appendl(buf, spaces.c, spaces.len);
				smart_str_appendc(buf, '}');
			}
			break;
		default:
			break;
	}
	smart_str_free(&spaces);
	smart_str_0(buf);
}
