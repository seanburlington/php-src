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
   | Authors: Timm Friebe <thekid@thekid.de>                              |
   |          George Schlossnagle <george@omniti.com>                     |
   |          Andrei Zmievski <andrei@gravitonic.com>                     |
   |          Marcus Boerger <helly@php.net>                              |
   |          Johannes Schlueter <johannes@php.net>                       |
   +----------------------------------------------------------------------+
*/

/* $Id: php_reflection.c,v 1.345 2009/05/25 14:32:14 felipe Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_reflection.h"
#include "ext/standard/info.h"

#include "zend.h"
#include "zend_API.h"
#include "zend_exceptions.h"
#include "zend_operators.h"
#include "zend_constants.h"
#include "zend_ini.h"
#include "zend_interfaces.h"
#include "zend_closures.h"

/* Class entry pointers */
PHPAPI zend_class_entry *reflector_ptr;
PHPAPI zend_class_entry *reflection_exception_ptr;
PHPAPI zend_class_entry *reflection_ptr;
PHPAPI zend_class_entry *reflection_function_abstract_ptr;
PHPAPI zend_class_entry *reflection_function_ptr;
PHPAPI zend_class_entry *reflection_parameter_ptr;
PHPAPI zend_class_entry *reflection_class_ptr;
PHPAPI zend_class_entry *reflection_object_ptr;
PHPAPI zend_class_entry *reflection_method_ptr;
PHPAPI zend_class_entry *reflection_property_ptr;
PHPAPI zend_class_entry *reflection_extension_ptr;

#if MBO_0
ZEND_BEGIN_MODULE_GLOBALS(reflection)
	int dummy;
ZEND_END_MODULE_GLOBALS(reflection)

#ifdef ZTS
# define REFLECTION_G(v) \
	TSRMG(reflection_globals_id, zend_reflection_globals*, v)
extern int reflection_globals_id;
#else
# define REFLECTION_G(v) (reflection_globals.v)
extern zend_reflection_globals reflectionglobals;
#endif

ZEND_DECLARE_MODULE_GLOBALS(reflection)
#endif /* MBO_0 */

/* Method macros */

#define METHOD_NOTSTATIC(ce)                                                                                \
	if (!this_ptr || !instanceof_function(Z_OBJCE_P(this_ptr), ce TSRMLS_CC)) {                             \
		zend_error(E_ERROR, "%v() cannot be called statically", get_active_function_name(TSRMLS_C));        \
		return;                                                                                             \
	}                                                                                                       \

/* Exception throwing macro */
#define _DO_THROW(msg)                                                                                      \
	zend_throw_exception(reflection_exception_ptr, msg, 0 TSRMLS_CC);                                       \
	return;                                                                                                 \

#define RETURN_ON_EXCEPTION                                                                                 \
	if (EG(exception) && Z_OBJCE_P(EG(exception)) == reflection_exception_ptr) {                            \
		return;                                                                                             \
	}

#define GET_REFLECTION_OBJECT_PTR(target)                                                                   \
	intern = (reflection_object *) zend_object_store_get_object(getThis() TSRMLS_CC);                       \
	if (intern == NULL || intern->ptr == NULL) {                                                            \
		RETURN_ON_EXCEPTION                                                                                 \
		zend_error(E_ERROR, "Internal error: Failed to retrieve the reflection object");                    \
	}                                                                                                       \
	target = intern->ptr;                                                                                   \

/* Class constants */
#define REGISTER_REFLECTION_CLASS_CONST_LONG(class_name, const_name, value)                                        \
	zend_declare_class_constant_long(reflection_ ## class_name ## _ptr, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC);

/* {{{ Smart string functions */
typedef struct _string {
	char *string;
	int len;
	int alloced;
} string;

static void string_init(string *str)
{
	str->string = (char *) emalloc(1024);
	str->len = 1;
	str->alloced = 1024;
	*str->string = '\0';
}
	
static string *string_printf(string *str, const char *format, ...)
{
	int len;
	va_list arg;
	char *s_tmp;

	va_start(arg, format);
	len = zend_vspprintf(&s_tmp, 0, format, arg);
	if (len) {
		register int nlen = (str->len + len + (1024 - 1)) & ~(1024 - 1);
		if (str->alloced < nlen) {
			str->alloced = nlen;
			str->string = erealloc(str->string, str->alloced);
		}
		memcpy(str->string + str->len - 1, s_tmp, len + 1);
		str->len += len;
	}
	efree(s_tmp);
	va_end(arg);
	return str;
}

static string *string_write(string *str, char *buf, int len)
{
	register int nlen = (str->len + len + (1024 - 1)) & ~(1024 - 1);
	if (str->alloced < nlen) {
		str->alloced = nlen;
		str->string = erealloc(str->string, str->alloced);
	}
	memcpy(str->string + str->len - 1, buf, len);
	str->len += len;
	str->string[str->len - 1] = '\0';
	return str;
}

static string *string_append(string *str, string *append)
{
	if (append->len > 1) {
		string_write(str, append->string, append->len - 1);
	}
	return str;
}

static void string_free(string *str)
{
	efree(str->string);
	str->len = 0;
	str->alloced = 0;
	str->string = NULL;
}
/* }}} */

/* Struct for properties */
typedef struct _property_reference {
	zend_class_entry *ce;
	zend_property_info prop;
	unsigned int ignore_visibility:1;
} property_reference;

/* Struct for parameters */
typedef struct _parameter_reference {
	zend_uint offset;
	zend_uint required;
	struct _zend_arg_info *arg_info;
	zend_function *fptr;
} parameter_reference;

typedef enum {
	REF_TYPE_OTHER,      /* Must be 0 */
	REF_TYPE_FUNCTION,
	REF_TYPE_PARAMETER,
	REF_TYPE_PROPERTY,
} reflection_type_t;

/* Struct for reflection objects */
typedef struct {
	zend_object zo;
	void *ptr;
	reflection_type_t ref_type;
	zval *obj;
	zend_class_entry *ce;
} reflection_object;

static zend_object_handlers reflection_object_handlers;

static void _default_get_entry(zval *object, char *name, int name_len, zval *return_value TSRMLS_DC) /* {{{ */
{
	zval **value;

	if (zend_ascii_hash_find(Z_OBJPROP_P(object), name, name_len, (void **) &value) == FAILURE) {
		RETURN_FALSE;
	}

	*return_value = **value;
	zval_copy_ctor(return_value);
	INIT_PZVAL(return_value);
}
/* }}} */

static void _default_lookup_entry(zval *object, char *name, int name_len, zval **return_value TSRMLS_DC) /* {{{ */
{
	zval **value;

	if (zend_ascii_hash_find(Z_OBJPROP_P(object), name, name_len, (void **) &value) == FAILURE) {
		*return_value = NULL;
	} else {
		*return_value = *value;
	}
}
/* }}} */

static void reflection_register_implement(zend_class_entry *class_entry, zend_class_entry *interface_entry TSRMLS_DC) /* {{{ */
{
	zend_uint num_interfaces = ++class_entry->num_interfaces;

	class_entry->interfaces = (zend_class_entry **) realloc(class_entry->interfaces, sizeof(zend_class_entry *) * num_interfaces);
	class_entry->interfaces[num_interfaces - 1] = interface_entry;
}
/* }}} */

static zend_function *_copy_function(zend_function *fptr TSRMLS_DC) /* {{{ */
{
	if (fptr
		&& fptr->type == ZEND_INTERNAL_FUNCTION
		&& (fptr->internal_function.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) != 0)
	{
		zend_function *copy_fptr;
		copy_fptr = emalloc(sizeof(zend_function));
		memcpy(copy_fptr, fptr, sizeof(zend_function));
		copy_fptr->internal_function.function_name.u = eustrdup(fptr->internal_function.function_name.u);
		return copy_fptr;
	} else {
		/* no copy needed */
		return fptr;
	}
}
/* }}} */

static void _free_function(zend_function *fptr TSRMLS_DC) /* {{{ */
{
	if (fptr
		&& fptr->type != ZEND_USER_FUNCTION
		&& (fptr->internal_function.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) != 0)
	{
		efree(fptr->internal_function.function_name.v);
		efree(fptr);
	}
}
/* }}} */

static void reflection_free_objects_storage(void *object TSRMLS_DC) /* {{{ */
{
	reflection_object *intern = (reflection_object *) object;
	parameter_reference *reference;

	if (intern->ptr) {
		switch (intern->ref_type) {
		case REF_TYPE_PARAMETER:
			reference = (parameter_reference*)intern->ptr;
			_free_function(reference->fptr TSRMLS_CC);
			efree(intern->ptr);
			break;
		case REF_TYPE_FUNCTION:
			_free_function(intern->ptr TSRMLS_CC);
			break;
		case REF_TYPE_PROPERTY:
			efree(intern->ptr);
			break;
		case REF_TYPE_OTHER:
			break;
		}
	}
	intern->ptr = NULL;
	if (intern->obj) {
		zval_ptr_dtor(&intern->obj);
	}
	zend_objects_free_object_storage(object TSRMLS_CC);
}
/* }}} */

static zend_object_value reflection_objects_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
	zval tmp;
	zend_object_value retval;
	reflection_object *intern;

	intern = ecalloc(1, sizeof(reflection_object));
	intern->zo.ce = class_type;

	zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
	zend_hash_copy(intern->zo.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
	retval.handle = zend_objects_store_put(intern, NULL, reflection_free_objects_storage, NULL TSRMLS_CC);
	retval.handlers = &reflection_object_handlers;
	return retval;
}
/* }}} */

static zval * reflection_instantiate(zend_class_entry *pce, zval *object TSRMLS_DC) /* {{{ */
{
	if (!object) {
		ALLOC_ZVAL(object);
	}
	Z_TYPE_P(object) = IS_OBJECT;
	object_init_ex(object, pce);
	Z_SET_REFCOUNT_P(object, 1);
	Z_SET_ISREF_P(object);
	return object;
}
/* }}} */

static void _const_string(string *str, zstr name, zval *value, char *indent TSRMLS_DC);
static void _function_string(string *str, zend_function *fptr, zend_class_entry *scope, zval* prop_name, zval* closure, char *indent TSRMLS_DC);
static void _property_string(string *str, zend_property_info *prop, zstr prop_name, char* indent TSRMLS_DC);
static void _class_string(string *str, zend_class_entry *ce, zval *obj, char *indent TSRMLS_DC);
static void _extension_string(string *str, zend_module_entry *module, char *indent TSRMLS_DC);

/* {{{ _class_string */
static void _class_string(string *str, zend_class_entry *ce, zval *obj, char *indent TSRMLS_DC)
{
	int count, count_static_props = 0, count_static_funcs = 0, count_shadow_props = 0;
	string sub_indent;
	
	string_init(&sub_indent);
	string_printf(&sub_indent, "%s    ", indent);

	/* TBD: Repair indenting of doc comment (or is this to be done in the parser?) */
	if (ce->type == ZEND_USER_CLASS && ce->doc_comment.v) {
		string_printf(str, "%s%v", indent, ce->doc_comment.v);
		string_write(str, "\n", 1);
	}

	if (obj) {
		string_printf(str, "%sObject of class [ ", indent);
	} else {
		string_printf(str, "%s%s [ ", indent, (ce->ce_flags & ZEND_ACC_INTERFACE) ? "Interface" : "Class");
	}
	string_printf(str, (ce->type == ZEND_USER_CLASS) ? "<user" : "<internal");
	if (ce->module) {
		string_printf(str, ":%s", ce->module->name);
	}
	string_printf(str, "> ");
	if (ce->get_iterator != NULL) {
		string_printf(str, "<iterateable> ");
	}
	if (ce->ce_flags & ZEND_ACC_INTERFACE) {
		string_printf(str, "interface ");
	} else {
		if (ce->ce_flags & (ZEND_ACC_IMPLICIT_ABSTRACT_CLASS|ZEND_ACC_EXPLICIT_ABSTRACT_CLASS)) {
			string_printf(str, "abstract ");
		}
		if (ce->ce_flags & ZEND_ACC_FINAL_CLASS) {
			string_printf(str, "final ");
		} 
		string_printf(str, "class ");
	}
	string_printf(str, "%v", ce->name);
	if (ce->parent) {
		string_printf(str, " extends %v", ce->parent->name);
	}

	if (ce->num_interfaces) {
		zend_uint i;

		if (ce->ce_flags & ZEND_ACC_INTERFACE) {
			string_printf(str, " extends %v", ce->interfaces[0]->name);
		} else {
			string_printf(str, " implements %v", ce->interfaces[0]->name);
		}
		for (i = 1; i < ce->num_interfaces; ++i) {
			string_printf(str, ", %v", ce->interfaces[i]->name);
		}
	}
	string_printf(str, " ] {\n");

	/* The information where a class is declared is only available for user classes */
	if (ce->type == ZEND_USER_CLASS) {
		string_printf(str, "%s  @@ %s %d-%d\n", indent, ce->filename,
						ce->line_start, ce->line_end);
	}

	/* Constants */
	if (&ce->constants_table) {
		zend_hash_apply_with_argument(&ce->constants_table, (apply_func_arg_t) zval_update_constant, (void*)1 TSRMLS_CC);
		string_printf(str, "\n");
		count = zend_hash_num_elements(&ce->constants_table);
		string_printf(str, "%s  - Constants [%d] {\n", indent, count);
		if (count > 0) {
			HashPosition pos;
			zval **value;
			zstr key;
			uint key_len;
			ulong num_index;

			zend_hash_internal_pointer_reset_ex(&ce->constants_table, &pos);

			while (zend_hash_get_current_data_ex(&ce->constants_table, (void **) &value, &pos) == SUCCESS) {
				zend_hash_get_current_key_ex(&ce->constants_table, &key, &key_len, &num_index, 0, &pos);

				_const_string(str, key, *value, indent TSRMLS_CC);
				zend_hash_move_forward_ex(&ce->constants_table, &pos);
			}
		}
		string_printf(str, "%s  }\n", indent);
	}

	/* Static properties */
	if (&ce->properties_info) {
		/* counting static properties */		
		count = zend_hash_num_elements(&ce->properties_info);
		if (count > 0) {
			HashPosition pos;
			zend_property_info *prop;

			zend_hash_internal_pointer_reset_ex(&ce->properties_info, &pos);

			while (zend_hash_get_current_data_ex(&ce->properties_info, (void **) &prop, &pos) == SUCCESS) {
				if(prop->flags & ZEND_ACC_SHADOW) {
					count_shadow_props++;
				} else if (prop->flags & ZEND_ACC_STATIC) {
					count_static_props++;
				}
				zend_hash_move_forward_ex(&ce->properties_info, &pos);
			}
		}

		/* static properties */		
		string_printf(str, "\n%s  - Static properties [%d] {\n", indent, count_static_props);
		if (count_static_props > 0) {
			HashPosition pos;
			zend_property_info *prop;

			zend_hash_internal_pointer_reset_ex(&ce->properties_info, &pos);

			while (zend_hash_get_current_data_ex(&ce->properties_info, (void **) &prop, &pos) == SUCCESS) {
				if ((prop->flags & ZEND_ACC_STATIC) && !(prop->flags & ZEND_ACC_SHADOW)) {
					_property_string(str, prop, NULL_ZSTR, sub_indent.string TSRMLS_CC);
				}

				zend_hash_move_forward_ex(&ce->properties_info, &pos);
			}
		}
		string_printf(str, "%s  }\n", indent);
	}
	
	/* Static methods */
	if (&ce->function_table) {
		/* counting static methods */		
		count = zend_hash_num_elements(&ce->function_table);
		if (count > 0) {
			HashPosition pos;
			zend_function *mptr;

			zend_hash_internal_pointer_reset_ex(&ce->function_table, &pos);

			while (zend_hash_get_current_data_ex(&ce->function_table, (void **) &mptr, &pos) == SUCCESS) {
				if (mptr->common.fn_flags & ZEND_ACC_STATIC
					&& ((mptr->common.fn_flags & ZEND_ACC_PRIVATE) == 0 || mptr->common.scope == ce))
				{
					count_static_funcs++;
				}
				zend_hash_move_forward_ex(&ce->function_table, &pos);
			}
		}

		/* static methods */		
		string_printf(str, "\n%s  - Static methods [%d] {", indent, count_static_funcs);
		if (count_static_funcs > 0) {
			HashPosition pos;
			zend_function *mptr;

			zend_hash_internal_pointer_reset_ex(&ce->function_table, &pos);

			while (zend_hash_get_current_data_ex(&ce->function_table, (void **) &mptr, &pos) == SUCCESS) {
				if (mptr->common.fn_flags & ZEND_ACC_STATIC
					&& ((mptr->common.fn_flags & ZEND_ACC_PRIVATE) == 0 || mptr->common.scope == ce))
				{
					string_printf(str, "\n");
					_function_string(str, mptr, ce, NULL, NULL, sub_indent.string TSRMLS_CC);
				}
				zend_hash_move_forward_ex(&ce->function_table, &pos);
			}
		} else {
			string_printf(str, "\n");
		}
		string_printf(str, "%s  }\n", indent);
	}

	/* Default/Implicit properties */
	if (&ce->properties_info) {
		count = zend_hash_num_elements(&ce->properties_info) - count_static_props - count_shadow_props;
		string_printf(str, "\n%s  - Properties [%d] {\n", indent, count);
		if (count > 0) {
			HashPosition pos;
			zend_property_info *prop;

			zend_hash_internal_pointer_reset_ex(&ce->properties_info, &pos);

			while (zend_hash_get_current_data_ex(&ce->properties_info, (void **) &prop, &pos) == SUCCESS) {
				if (!(prop->flags & (ZEND_ACC_STATIC|ZEND_ACC_SHADOW))) {
					_property_string(str, prop, NULL_ZSTR, sub_indent.string TSRMLS_CC);
				}
				zend_hash_move_forward_ex(&ce->properties_info, &pos);
			}
		}
		string_printf(str, "%s  }\n", indent);
	}

	if (obj && Z_OBJ_HT_P(obj)->get_properties) {
		string       dyn;
		HashTable    *properties = Z_OBJ_HT_P(obj)->get_properties(obj TSRMLS_CC);
		HashPosition pos;
		zval         **prop;

		string_init(&dyn);
		count = 0;

		zend_hash_internal_pointer_reset_ex(properties, &pos);

		while (zend_hash_get_current_data_ex(properties, (void **) &prop, &pos) == SUCCESS) {
			zstr prop_name;
			uint  prop_name_size;
			zend_uchar prop_type;
			ulong index;

			if ((prop_type = zend_hash_get_current_key_ex(properties, &prop_name, &prop_name_size, &index, 0, &pos)) == HASH_KEY_IS_UNICODE) {
				if (prop_name_size && prop_name.u[0]) { /* skip all private and protected properties */
					if (!zend_u_hash_exists(&ce->properties_info, prop_type, prop_name, prop_name_size)) {
						count++;
						_property_string(&dyn, NULL, prop_name, sub_indent.string TSRMLS_CC);	
					}
				}
			}
			zend_hash_move_forward_ex(properties, &pos);
		}

		string_printf(str, "\n%s  - Dynamic properties [%d] {\n", indent, count);
		string_append(str, &dyn);
		string_printf(str, "%s  }\n", indent);
		string_free(&dyn);
	}

	/* Non static methods */
	if (&ce->function_table) {
		count = zend_hash_num_elements(&ce->function_table) - count_static_funcs;
		if (count > 0) {
			HashPosition pos;
			zend_function *mptr;
			string dyn;

			count = 0;
			string_init(&dyn);
			zend_hash_internal_pointer_reset_ex(&ce->function_table, &pos);

			while (zend_hash_get_current_data_ex(&ce->function_table, (void **) &mptr, &pos) == SUCCESS) {
				if ((mptr->common.fn_flags & ZEND_ACC_STATIC) == 0
					&& ((mptr->common.fn_flags & ZEND_ACC_PRIVATE) == 0 || mptr->common.scope == ce))
				{
					zstr key;
					uint key_len;
					ulong num_index;
					uint lcname_len, len = u_strlen(mptr->common.function_name.u);
					zstr lcname = zend_u_str_case_fold(IS_UNICODE, mptr->common.function_name, len, 1, &lcname_len);

					/* Do not display old-style inherited constructors */
					if ((mptr->common.fn_flags & ZEND_ACC_CTOR) == 0
						|| mptr->common.scope == ce
						|| zend_hash_get_current_key_ex(&ce->function_table, &key, &key_len, &num_index, 0, &pos) != HASH_KEY_IS_UNICODE
						|| zend_u_binary_strcmp(key.u, key_len-1, lcname.u, lcname_len) == 0)
					{
						zend_function *closure;
						/* see if this is a closure */
						if (ce == zend_ce_closure && obj && (len == sizeof(ZEND_INVOKE_FUNC_NAME)-1)
							&& ZEND_U_EQUAL(IS_UNICODE, lcname, len, ZEND_INVOKE_FUNC_NAME, sizeof(ZEND_INVOKE_FUNC_NAME)-1)
							&& (closure = zend_get_closure_invoke_method(obj TSRMLS_CC)) != NULL)
						{
							mptr = closure;
						} else {
							closure = NULL;
						}

						string_printf(&dyn, "\n");
						_function_string(&dyn, mptr, ce, NULL, NULL, sub_indent.string TSRMLS_CC);
						count++;
						_free_function(closure TSRMLS_CC);
					}
					efree(lcname.v);
				}
				zend_hash_move_forward_ex(&ce->function_table, &pos);
			}
			string_printf(str, "\n%s  - Methods [%d] {", indent, count);
			if (!count) {
				string_printf(str, "\n");
			}
			string_append(str, &dyn);
			string_free(&dyn);
		} else {
			string_printf(str, "\n%s  - Methods [0] {\n", indent);
		}
		string_printf(str, "%s  }\n", indent);
	}
	
	string_printf(str, "%s}\n", indent);
	string_free(&sub_indent);
}
/* }}} */

/* {{{ _const_string */
static void _const_string(string *str, zstr name, zval *value, char *indent TSRMLS_DC)
{
	char *type;
	zval value_copy;
	int use_copy;

	type = zend_zval_type_name(value);

	zend_make_printable_zval(value, &value_copy, &use_copy);
	if (use_copy) {
		value = &value_copy;
	}

	string_printf(str, "%s    Constant [ %s %v ] { %s }\n",
					indent, type, name, Z_STRVAL_P(value));

	if (use_copy) {
		zval_dtor(value);
	}
}
/* }}} */

/* {{{ _get_recv_opcode */
static zend_op* _get_recv_op(zend_op_array *op_array, zend_uint offset)
{
	zend_op *op = op_array->opcodes;
	zend_op *end = op + op_array->last;

	++offset;
	while (op < end) {
		if ((op->opcode == ZEND_RECV || op->opcode == ZEND_RECV_INIT)
			&& op->op1.u.constant.value.lval == offset)
		{
			return op;
		}
		++op;
	}
	return NULL;
}
/* }}} */

/* {{{ _parameter_string */
static void _parameter_string(string *str, zend_function *fptr, struct _zend_arg_info *arg_info, zend_uint offset, zend_uint required, char* indent TSRMLS_DC)
{
	string_printf(str, "Parameter #%d [ ", offset);
	if (offset >= required) {
		string_printf(str, "<optional> ");
	} else {
		string_printf(str, "<required> ");
	}
	if (arg_info->class_name.v) {
		string_printf(str, "%v ", arg_info->class_name);
		if (arg_info->allow_null) {
			string_printf(str, "or NULL ");
		}
	} else if (arg_info->array_type_hint) {
		string_printf(str, "array ");
		if (arg_info->allow_null) {
			string_printf(str, "or NULL ");
		}
	}
	if (arg_info->pass_by_reference) {
		string_write(str, "&", sizeof("&")-1);
	}
	if (arg_info->name.v) {
		string_printf(str, "$%v", arg_info->name);
	} else {
		string_printf(str, "$param%d", offset);
	}
	if (fptr->type == ZEND_USER_FUNCTION && offset >= required) {
		zend_op *precv = _get_recv_op((zend_op_array*)fptr, offset);
		if (precv && precv->opcode == ZEND_RECV_INIT && precv->op2.op_type != IS_UNUSED) {
			zval *zv, zv_copy;
			int use_copy;
			string_write(str, " = ", sizeof(" = ")-1);
			ALLOC_ZVAL(zv);
			*zv = precv->op2.u.constant;
			zval_copy_ctor(zv);
			INIT_PZVAL(zv);
			zval_update_constant_ex(&zv, (void*)1, fptr->common.scope TSRMLS_CC);
			if (Z_TYPE_P(zv) == IS_BOOL) {
				if (Z_LVAL_P(zv)) {
					string_write(str, "true", sizeof("true")-1);
				} else {
					string_write(str, "false", sizeof("false")-1);
				}
			} else if (Z_TYPE_P(zv) == IS_NULL) {
				string_write(str, "NULL", sizeof("NULL")-1);
			} else if (Z_TYPE_P(zv) == IS_STRING) {
				string_write(str, "b'", sizeof("b")-1);
				string_write(str, "'", sizeof("'")-1);
				string_write(str, Z_STRVAL_P(zv), MIN(Z_STRLEN_P(zv), 15));
				if (Z_STRLEN_P(zv) > 15) {
					string_write(str, "...", sizeof("...")-1);
				}
				string_write(str, "'", sizeof("'")-1);
			} else if (Z_TYPE_P(zv) == IS_UNICODE) {
				string_write(str, "'", sizeof("'")-1);
				zend_make_printable_zval(zv, &zv_copy, &use_copy);
				string_write(str, Z_STRVAL(zv_copy), MIN(Z_STRLEN(zv_copy), 15));
				if (Z_STRLEN(zv_copy) > 15) {
					string_write(str, "...", sizeof("...")-1);
				}
				string_write(str, "'", sizeof("'")-1);
				if (use_copy) {
					zval_dtor(&zv_copy);
				}
			} else {
				zend_make_printable_zval(zv, &zv_copy, &use_copy);
				string_write(str, Z_STRVAL(zv_copy), Z_STRLEN(zv_copy));
				if (use_copy) {
					zval_dtor(&zv_copy);
				}
			}
			zval_ptr_dtor(&zv);
		}
	}
	string_write(str, " ]", sizeof(" ]")-1);
}
/* }}} */

/* {{{ _function_parameter_string */
static void _function_parameter_string(string *str, zend_function *fptr, char* indent TSRMLS_DC)
{
	struct _zend_arg_info *arg_info = fptr->common.arg_info;
	zend_uint i, required = fptr->common.required_num_args;

	if (!arg_info) {
		return;
	}

	string_printf(str, "\n");
	string_printf(str, "%s- Parameters [%d] {\n", indent, fptr->common.num_args);
	for (i = 0; i < fptr->common.num_args; i++) {
		string_printf(str, "%s  ", indent);
		_parameter_string(str, fptr, arg_info, i, required, indent TSRMLS_CC);
		string_write(str, "\n", sizeof("\n")-1);
		arg_info++;
	}
	string_printf(str, "%s}\n", indent);
}
/* }}} */

/* {{{ _function_string */
static void _function_string(string *str, zend_function *fptr, zend_class_entry *scope, zval* prop_name, zval* closure, char* indent TSRMLS_DC)
{
	string param_indent;
	zend_function *overwrites;
	zstr lc_name;
	unsigned int lc_name_len;

	/* TBD: Repair indenting of doc comment (or is this to be done in the parser?)
	 * What's "wrong" is that any whitespace before the doc comment start is 
	 * swallowed, leading to an unaligned comment.
	 */
	if (fptr->type == ZEND_USER_FUNCTION && fptr->op_array.doc_comment.v) {
		string_printf(str, "%s%v\n", indent, fptr->op_array.doc_comment.v);
	}

	string_write(str, indent, strlen(indent));
	string_printf(str, closure ? "Closure [ " : (fptr->common.scope ? "Method [ " : "Function [ "));
	string_printf(str, (fptr->type == ZEND_USER_FUNCTION) ? "<user" : "<internal");
	if (fptr->common.fn_flags & ZEND_ACC_DEPRECATED) {
		string_printf(str, ", deprecated");
	}
	if (fptr->type == ZEND_INTERNAL_FUNCTION && ((zend_internal_function*)fptr)->module) {
		string_printf(str, ":%s", ((zend_internal_function*)fptr)->module->name);
	}
	if (scope && fptr->common.scope) {
		if (fptr->common.scope != scope) {
			string_printf(str, ", inherits %v", fptr->common.scope->name);
		} else if (fptr->common.scope->parent) {
			lc_name = zend_u_str_case_fold(IS_UNICODE, fptr->common.function_name, USTR_LEN(fptr->common.function_name), 1, &lc_name_len);
			if (zend_u_hash_find(&fptr->common.scope->parent->function_table, IS_UNICODE, lc_name, lc_name_len + 1, (void**) &overwrites) == SUCCESS) {
				if (fptr->common.scope != overwrites->common.scope) {
					string_printf(str, ", overwrites %v", overwrites->common.scope->name);
				}
			}
			efree(lc_name.v);
		}
	}
	if (fptr->common.prototype && fptr->common.prototype->common.scope) {
		string_printf(str, ", prototype %v", fptr->common.prototype->common.scope->name);
	}
	if (fptr->common.fn_flags & ZEND_ACC_CTOR) {
		string_printf(str, ", ctor");
	}
	if (fptr->common.fn_flags & ZEND_ACC_DTOR) {
		string_printf(str, ", dtor");
	}
	string_printf(str, "> ");

	if (fptr->common.fn_flags & ZEND_ACC_ABSTRACT) {
		string_printf(str, "abstract ");
	}
	if (fptr->common.fn_flags & ZEND_ACC_FINAL) {
		string_printf(str, "final ");
	}
	if (fptr->common.fn_flags & ZEND_ACC_STATIC) {
		string_printf(str, "static ");
	}

	if (fptr->common.scope) {
		/* These are mutually exclusive */
		switch (fptr->common.fn_flags & ZEND_ACC_PPP_MASK) {
			case ZEND_ACC_PUBLIC:
				string_printf(str, "public ");
				break;
			case ZEND_ACC_PRIVATE:
				string_printf(str, "private ");
				break;
			case ZEND_ACC_PROTECTED:
				string_printf(str, "protected ");
				break;
			default:
				string_printf(str, "<visibility error> ");
				break;
		}
		string_printf(str, "method ");
	} else {
		string_printf(str, "function ");
	}

	if (fptr->op_array.return_reference) {
		string_printf(str, "&");
	}
	if (closure && prop_name) {
		string_printf(str, "%R ] {\n", Z_TYPE_P(prop_name), Z_UNIVAL_P(prop_name));
	} else {
		string_printf(str, "%v ] {\n", fptr->common.function_name);
	}
	/* The information where a function is declared is only available for user classes */
	if (fptr->type == ZEND_USER_FUNCTION) {
		string_printf(str, "%s  @@ %s %d - %d\n", indent, 
						fptr->op_array.filename,
						fptr->op_array.line_start,
						fptr->op_array.line_end);
	}
	if (closure) {
		const zend_function *closure_fptr = zend_get_closure_method_def(closure TSRMLS_CC);
		zval *closure_this = zend_get_closure_this_ptr(closure TSRMLS_CC);
		HashTable *static_variables = NULL;
		int index = 0, count = closure_this ? 1 : 0;
		if (closure_fptr->type == ZEND_USER_FUNCTION && closure_fptr->op_array.static_variables) {
			static_variables = closure_fptr->op_array.static_variables;
			count += zend_hash_num_elements(static_variables);
		}
		if (count) {
			string_printf(str, "\n");
			string_printf(str, "%s  - Static Parameters [%d] {\n", indent, count);
			if (closure_this) {
				string_printf(str, "%s    Parameter #%d [ %v $this ]\n", indent, index++, Z_OBJCE_P(closure_this)->name);
			}
			if (static_variables) {
				HashPosition pos;
				uint key_len;
				zstr key;
				ulong num_index;
				zend_hash_internal_pointer_reset_ex(static_variables, &pos);
				while (index < count) {
					zend_hash_get_current_key_ex(static_variables, &key, &key_len, &num_index, 0, &pos);
					string_printf(str, "%s    Parameter #%d [ $%v ]\n", indent, index++, key);
					zend_hash_move_forward_ex(static_variables, &pos);
				}
			}
			string_printf(str, "%s  }\n", indent);
		}
	}
	string_init(&param_indent);
	string_printf(&param_indent, "%s  ", indent);
	_function_parameter_string(str, fptr, param_indent.string TSRMLS_CC);
	string_free(&param_indent);
	string_printf(str, "%s}\n", indent);
}
/* }}} */

/* {{{ _property_string */
static void _property_string(string *str, zend_property_info *prop, zstr sz_prop_name, char* indent TSRMLS_DC)
{
	zstr class_name;
	zstr prop_name;

	string_printf(str, "%sProperty [ ", indent);
	if (!prop) {
		string_printf(str, "<dynamic> public $%v", sz_prop_name);
	} else {
		if (!(prop->flags & ZEND_ACC_STATIC)) {
			if (prop->flags & ZEND_ACC_IMPLICIT_PUBLIC) {
				string_write(str, "<implicit> ", sizeof("<implicit> ") - 1);
			} else {
				string_write(str, "<default> ", sizeof("<default> ") - 1);
			}
		}
	
		/* These are mutually exclusive */
		switch (prop->flags & ZEND_ACC_PPP_MASK) {
			case ZEND_ACC_PUBLIC:
				string_printf(str, "public ");
				break;
			case ZEND_ACC_PRIVATE:
				string_printf(str, "private ");
				break;
			case ZEND_ACC_PROTECTED:
				string_printf(str, "protected ");
				break;
		}
		if(prop->flags & ZEND_ACC_STATIC) {
			string_printf(str, "static ");
		}

		zend_u_unmangle_property_name(IS_UNICODE, prop->name, prop->name_length, &class_name, &prop_name);
		string_printf(str, "$%v", prop_name);
	}

	string_printf(str, " ]\n");
}
/* }}} */

static int _extension_ini_string(zend_ini_entry *ini_entry TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key) /* {{{ */
{
	string *str = va_arg(args, string *);
	char *indent = va_arg(args, char *);
	int number = va_arg(args, int);
	char *comma = "";

	if (number == ini_entry->module_number) {
		string_printf(str, "    %sEntry [ %s <", indent, ini_entry->name);
		if (ini_entry->modifiable == ZEND_INI_ALL) {
			string_printf(str, "ALL");
		} else {
			if (ini_entry->modifiable & ZEND_INI_USER) {
				string_printf(str, "USER");
				comma = ",";
			}
			if (ini_entry->modifiable & ZEND_INI_PERDIR) {
				string_printf(str, "%sPERDIR", comma);
				comma = ",";
			}
			if (ini_entry->modifiable & ZEND_INI_SYSTEM) {
				string_printf(str, "%sSYSTEM", comma);
			}
		}

		string_printf(str, "> ]\n");
		string_printf(str, "    %s  Current = '%s'\n", indent, ini_entry->value ? ini_entry->value : "");
		if (ini_entry->modified) {
			string_printf(str, "    %s  Default = '%s'\n", indent, ini_entry->orig_value ? ini_entry->orig_value : "");
		}
		string_printf(str, "    %s}\n", indent);
	}
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

static int _extension_class_string(zend_class_entry **pce TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key) /* {{{ */
{
	string *str = va_arg(args, string *);
	char *indent = va_arg(args, char *);
	struct _zend_module_entry *module = va_arg(args, struct _zend_module_entry*);
	int *num_classes = va_arg(args, int*);

	if ((*pce)->module && !strcasecmp((*pce)->module->name, module->name)) {
		string_printf(str, "\n");
		_class_string(str, *pce, NULL, indent TSRMLS_CC);
		(*num_classes)++;
	}
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

static int _extension_const_string(zend_constant *constant TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key) /* {{{ */
{
	string *str = va_arg(args, string *);
	char *indent = va_arg(args, char *);
	struct _zend_module_entry *module = va_arg(args, struct _zend_module_entry*);
	int *num_classes = va_arg(args, int*);

	if (constant->module_number  == module->module_number) {
		_const_string(str, constant->name, &constant->value, indent TSRMLS_CC);
		(*num_classes)++;
	}
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ _extension_string */
static void _extension_string(string *str, zend_module_entry *module, char *indent TSRMLS_DC)
{
	string_printf(str, "%sExtension [ ", indent);
	if (module->type == MODULE_PERSISTENT) {
		string_printf(str, "<persistent>");
	}
	if (module->type == MODULE_TEMPORARY) {
		string_printf(str, "<temporary>" );
	}
	string_printf(str, " extension #%d %s version %s ] {\n",
					module->module_number, module->name,
					(module->version == NO_VERSION_YET) ? "<no_version>" : module->version);

	if (module->deps) {
		const zend_module_dep* dep = module->deps;

		string_printf(str, "\n  - Dependencies {\n");

		while(dep->name) {
			string_printf(str, "%s    Dependency [ %s (", indent, dep->name);
			
			switch(dep->type) {
			case MODULE_DEP_REQUIRED:
				string_write(str, "Required", sizeof("Required") - 1);
				break;
			case MODULE_DEP_CONFLICTS:
				string_write(str, "Conflicts", sizeof("Conflicts") - 1);
				break;
			case MODULE_DEP_OPTIONAL:
				string_write(str, "Optional", sizeof("Optional") - 1);
				break;
			default:
				string_write(str, "Error", sizeof("Error") - 1); /* shouldn't happen */
				break;
			}

			if (dep->rel) {	
				string_printf(str, " %s", dep->rel);
			}
			if (dep->version) {	
				string_printf(str, " %s", dep->version);
			}
			string_write(str, ") ]\n", sizeof(") ]\n") - 1);
			dep++;
		}
		string_printf(str, "%s  }\n", indent);
	}

	{
		string str_ini;
		string_init(&str_ini);
		zend_hash_apply_with_arguments(EG(ini_directives) TSRMLS_CC, (apply_func_args_t) _extension_ini_string, 3, &str_ini, indent, module->module_number);
		if (str_ini.len > 1) {
			string_printf(str, "\n  - INI {\n");
			string_append(str, &str_ini);
			string_printf(str, "%s  }\n", indent);
		}
		string_free(&str_ini);
	}

	{
		string str_constants;
		int num_constants = 0;
		
		string_init(&str_constants);
		zend_hash_apply_with_arguments(EG(zend_constants) TSRMLS_CC, (apply_func_args_t) _extension_const_string, 4, &str_constants, indent, module, &num_constants);
		if (num_constants) {
			string_printf(str, "\n  - Constants [%d] {\n", num_constants);
			string_append(str, &str_constants);
			string_printf(str, "%s  }\n", indent);
		}
		string_free(&str_constants);
	}

	if (module->functions && module->functions->fname) {
		zend_function *fptr;
		const zend_function_entry *func = module->functions;

		string_printf(str, "\n  - Functions {\n");

		/* Is there a better way of doing this? */
		while (func->fname) {
			if (zend_ascii_hash_find(EG(function_table), func->fname, strlen(func->fname) + 1, (void**) &fptr) == FAILURE) {
				zend_error(E_WARNING, "Internal error: Cannot find extension function %s in global function table", func->fname);
				func++;
				continue;
			}
			
			_function_string(str, fptr, NULL, NULL, NULL, "    " TSRMLS_CC);
			func++;
		}
		string_printf(str, "%s  }\n", indent);
	}
	
	{
		string str_classes;
		string sub_indent;
		int num_classes = 0;
		
		string_init(&sub_indent);
		string_printf(&sub_indent, "%s    ", indent);
		string_init(&str_classes);
		zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) _extension_class_string, 4, &str_classes, sub_indent.string, module, &num_classes);
		if (num_classes) {
			string_printf(str, "\n  - Classes [%d] {", num_classes);
			string_append(str, &str_classes);
			string_printf(str, "%s  }\n", indent);
		}
		string_free(&str_classes);
		string_free(&sub_indent);
	}

	string_printf(str, "%s}\n", indent);
}
/* }}} */

/* {{{ _function_check_flag */
static void _function_check_flag(INTERNAL_FUNCTION_PARAMETERS, int mask)
{
	reflection_object *intern;
	zend_function *mptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(mptr);
	RETURN_BOOL(mptr->common.fn_flags & mask);
}
/* }}} */

/* {{{ zend_reflection_class_factory */
PHPAPI void zend_reflection_class_factory(zend_class_entry *ce, zval *object TSRMLS_DC)
{
	reflection_object *intern;
	zval *name;

	MAKE_STD_ZVAL(name);
	ZVAL_UNICODEL(name, ce->name.u, ce->name_length, 1);
	reflection_instantiate(reflection_class_ptr, object TSRMLS_CC);
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	intern->ptr = ce;
	intern->ref_type = REF_TYPE_OTHER;
	intern->ce = ce;
	zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &name, sizeof(zval *), NULL);
}
/* }}} */

/* {{{ reflection_extension_factory */
static void reflection_extension_factory(zval *object, const char *name_str TSRMLS_DC)
{
	reflection_object *intern;
	zval *name;
	int name_len = strlen(name_str);
	char *lcname;
	struct _zend_module_entry *module;
	ALLOCA_FLAG(use_heap)

	lcname = do_alloca(name_len + 1, use_heap);
	zend_str_tolower_copy(lcname, name_str, name_len);
	if (zend_hash_find(&module_registry, lcname, name_len + 1, (void **)&module) == FAILURE) {
		free_alloca(lcname, use_heap);
		return;
	}
	free_alloca(lcname, use_heap);

	reflection_instantiate(reflection_extension_ptr, object TSRMLS_CC);
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	MAKE_STD_ZVAL(name);
	ZVAL_ASCII_STRINGL(name, module->name, name_len, 1);
	intern->ptr = module;
	intern->ref_type = REF_TYPE_OTHER;
	intern->ce = NULL;
	zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &name, sizeof(zval *), NULL);
}
/* }}} */

/* {{{ reflection_parameter_factory */
static void reflection_parameter_factory(zend_function *fptr, struct _zend_arg_info *arg_info, zend_uint offset, zend_uint required, zval *object TSRMLS_DC)
{
	reflection_object *intern;
	parameter_reference *reference;
	zval *name;

	MAKE_STD_ZVAL(name);
	if (arg_info->name.v) {
		ZVAL_UNICODEL(name, arg_info->name.u, arg_info->name_len, 1);
	} else {
		ZVAL_NULL(name);
	}
	reflection_instantiate(reflection_parameter_ptr, object TSRMLS_CC);
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	reference = (parameter_reference*) emalloc(sizeof(parameter_reference));
	reference->arg_info = arg_info;
	reference->offset = offset;
	reference->required = required;
	reference->fptr = fptr;
	intern->ptr = reference;
	intern->ref_type = REF_TYPE_PARAMETER;
	intern->ce = fptr->common.scope;
	zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &name, sizeof(zval *), NULL);
}
/* }}} */

/* {{{ reflection_function_factory */
static void reflection_function_factory(zend_function *function, zval *object TSRMLS_DC)
{
	reflection_object *intern;
	zval *name;

	MAKE_STD_ZVAL(name);
	ZVAL_UNICODE(name, function->common.function_name.u, 1);

	reflection_instantiate(reflection_function_ptr, object TSRMLS_CC);
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	intern->ptr = function;
	intern->ref_type = REF_TYPE_FUNCTION;
	intern->ce = NULL;
	zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &name, sizeof(zval *), NULL);
}
/* }}} */

/* {{{ reflection_method_factory */
static void reflection_method_factory(zend_class_entry *ce, zend_function *method, zval *object TSRMLS_DC)
{
	reflection_object *intern;
	zval *name;
	zval *classname;

	MAKE_STD_ZVAL(name);
	MAKE_STD_ZVAL(classname);
	ZVAL_UNICODE(name, method->common.function_name.u, 1);
	ZVAL_UNICODEL(classname, method->common.scope->name.u, method->common.scope->name_length, 1);

	reflection_instantiate(reflection_method_ptr, object TSRMLS_CC);
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	intern->ptr = method;
	intern->ref_type = REF_TYPE_FUNCTION;
	intern->ce = ce;
	zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &name, sizeof(zval *), NULL);
	zend_ascii_hash_update(Z_OBJPROP_P(object), "class", sizeof("class"), (void **) &classname, sizeof(zval *), NULL);
}
/* }}} */

/* {{{ reflection_property_factory */
static void reflection_property_factory(zend_class_entry *ce, zend_property_info *prop, zval *object TSRMLS_DC)
{
	reflection_object *intern;
	zval *name;
	zval *classname;
	property_reference *reference;
	zstr class_name, prop_name;
	zend_uchar utype = IS_UNICODE;

	zend_u_unmangle_property_name(utype, prop->name, prop->name_length, &class_name, &prop_name);
	
	if (!(prop->flags & ZEND_ACC_PRIVATE)) {
		/* we have to search the class hierarchy for this (implicit) public or protected property */
		zend_class_entry *tmp_ce = ce, *store_ce = ce;
		zend_property_info *tmp_info = NULL;
		int prop_name_len = u_strlen(prop_name.u);
		
		while (tmp_ce && zend_u_hash_find(&tmp_ce->properties_info, utype, prop_name, prop_name_len + 1, (void **) &tmp_info) != SUCCESS) {
			ce = tmp_ce;
			tmp_ce = tmp_ce->parent;
		}

		if (tmp_info && !(tmp_info->flags & ZEND_ACC_SHADOW)) { /* found something and it's not a parent's private */
			prop = tmp_info;
		} else { /* not found, use initial value */
			ce = store_ce;
		}
	}

	MAKE_STD_ZVAL(name);
	MAKE_STD_ZVAL(classname);
	ZVAL_UNICODE(name, prop_name.u, 1);
	ZVAL_UNICODEL(classname, prop->ce->name.u, prop->ce->name_length, 1);

	reflection_instantiate(reflection_property_ptr, object TSRMLS_CC);
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	reference = (property_reference*) emalloc(sizeof(property_reference));
	reference->ce = ce;
	reference->prop = *prop;
	reference->ignore_visibility = 0;
	intern->ptr = reference;
	intern->ref_type = REF_TYPE_PROPERTY;
	intern->ce = ce;
	zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &name, sizeof(zval *), NULL);
	zend_ascii_hash_update(Z_OBJPROP_P(object), "class", sizeof("class"), (void **) &classname, sizeof(zval *), NULL);
}
/* }}} */

/* {{{ _reflection_export */
static void _reflection_export(INTERNAL_FUNCTION_PARAMETERS, zend_class_entry *ce_ptr, int ctor_argc)
{
	zval *reflector_ptr;
	zval output, *output_ptr = &output;
	zval *argument_ptr, *argument2_ptr;
	zval *retval_ptr, **params[2];
	int result;
	int return_output = 0;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	zval fname;

	if (ctor_argc == 1) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &argument_ptr, &return_output) == FAILURE) {
			return;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|b", &argument_ptr, &argument2_ptr, &return_output) == FAILURE) {
			return;
		}
	}

	INIT_PZVAL(&output);

	/* Create object */
	MAKE_STD_ZVAL(reflector_ptr);
	if (object_and_properties_init(reflector_ptr, ce_ptr, NULL) == FAILURE) {
		_DO_THROW("Could not create reflector");
	}

	/* Call __construct() */
	params[0] = &argument_ptr;
	params[1] = &argument2_ptr;

	fci.size = sizeof(fci);
	fci.function_table = NULL;
	fci.function_name = NULL;
	fci.symbol_table = NULL;
	fci.object_ptr = reflector_ptr;
	fci.retval_ptr_ptr = &retval_ptr;
	fci.param_count = ctor_argc;
	fci.params = params;
	fci.no_separation = 1;

	fcc.initialized = 1;
	fcc.function_handler = ce_ptr->constructor;
	fcc.calling_scope = ce_ptr;
	fcc.called_scope = Z_OBJCE_P(reflector_ptr);
	fcc.object_ptr = reflector_ptr;

	result = zend_call_function(&fci, &fcc TSRMLS_CC);
	
	if (retval_ptr) {
		zval_ptr_dtor(&retval_ptr);
	}

	if (EG(exception)) {
		zval_ptr_dtor(&reflector_ptr);
		return;
	}
	if (result == FAILURE) {
		zval_ptr_dtor(&reflector_ptr);
		_DO_THROW("Could not create reflector");
	}

	/* Call static reflection::export */
	ZVAL_BOOL(&output, return_output);
	params[0] = &reflector_ptr;
	params[1] = &output_ptr;

	ZVAL_ASCII_STRINGL(&fname, "reflection::export", sizeof("reflection::export") - 1, 1);
	fci.function_table = &reflection_ptr->function_table;
	fci.function_name = &fname;
	fci.object_ptr = NULL;
	fci.retval_ptr_ptr = &retval_ptr;
	fci.param_count = 2;
	fci.params = params;
	fci.no_separation = 1;

	result = zend_call_function(&fci, NULL TSRMLS_CC);

	zval_dtor(&fname);

	if (result == FAILURE && EG(exception) == NULL) {
		zval_ptr_dtor(&reflector_ptr);
		zval_ptr_dtor(&retval_ptr);
		_DO_THROW("Could not execute reflection::export()");
	}

	if (return_output) {
		COPY_PZVAL_TO_ZVAL(*return_value, retval_ptr);
	} else {
		zval_ptr_dtor(&retval_ptr);
	}

	/* Destruct reflector which is no longer needed */
	zval_ptr_dtor(&reflector_ptr);
}
/* }}} */

/* {{{ Preventing __clone from being called */
ZEND_METHOD(reflection, __clone)
{
	/* Should never be executable */
	_DO_THROW("Cannot clone object using __clone()");
}
/* }}} */

/* {{{ proto public static mixed Reflection::export(Reflector r [, bool return]) U
   Exports a reflection object. Returns the output if TRUE is specified for return, printing it otherwise. */
ZEND_METHOD(reflection, export)
{
	zval *object, fname, *retval_ptr;
	int result;
	zend_bool return_output = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|b", &object, reflector_ptr, &return_output) == FAILURE) {
		return;
	}

	/* Invoke the __toString() method */
	ZVAL_ASCII_STRINGL(&fname, "__tostring", sizeof("__tostring") - 1, 1);
	result= call_user_function_ex(NULL, &object, &fname, &retval_ptr, 0, NULL, 0, NULL TSRMLS_CC);
	zval_dtor(&fname);

	if (result == FAILURE) {
		_DO_THROW("Invocation of method __toString() failed");
		/* Returns from this function */
	}

	if (!retval_ptr) {
		zend_error(E_WARNING, "%v::__toString() did not return anything", Z_OBJCE_P(object)->name);
		RETURN_FALSE;
	}

	if (return_output) {
		COPY_PZVAL_TO_ZVAL(*return_value, retval_ptr);
	} else {
		/* No need for _r variant, return of __toString should always be a string */
		zend_print_zval(retval_ptr, 0);
		zend_printf("\n");
		zval_ptr_dtor(&retval_ptr);
	}
}
/* }}} */

/* {{{ proto public static array Reflection::getModifierNames(int modifiers) U
   Returns an array of modifier names */
ZEND_METHOD(reflection, getModifierNames)
{
	long modifiers;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &modifiers) == FAILURE) {
		return;
	}

	array_init(return_value);

	if (modifiers & (ZEND_ACC_ABSTRACT | ZEND_ACC_EXPLICIT_ABSTRACT_CLASS)) {
		add_next_index_ascii_stringl(return_value, "abstract", sizeof("abstract")-1, 1);
	}
	if (modifiers & (ZEND_ACC_FINAL | ZEND_ACC_FINAL_CLASS)) {
		add_next_index_ascii_stringl(return_value, "final", sizeof("final")-1, 1);
	}
	if (modifiers & ZEND_ACC_IMPLICIT_PUBLIC) {
		add_next_index_ascii_stringl(return_value, "public", sizeof("public")-1, 1);
	}

	/* These are mutually exclusive */
	switch (modifiers & ZEND_ACC_PPP_MASK) {
		case ZEND_ACC_PUBLIC:
			add_next_index_ascii_stringl(return_value, "public", sizeof("public")-1, 1);
			break;
		case ZEND_ACC_PRIVATE:
			add_next_index_ascii_stringl(return_value, "private", sizeof("private")-1, 1);
			break;
		case ZEND_ACC_PROTECTED:
			add_next_index_ascii_stringl(return_value, "protected", sizeof("protected")-1, 1);
			break;
	}

	if (modifiers & ZEND_ACC_STATIC) {
		add_next_index_ascii_stringl(return_value, "static", sizeof("static")-1, 1);
	}
}
/* }}} */

/* {{{ proto public static mixed ReflectionFunction::export(string name [, bool return]) U
   Exports a reflection object. Returns the output if TRUE is specified for return, printing it otherwise. */
ZEND_METHOD(reflection_function, export)
{
	_reflection_export(INTERNAL_FUNCTION_PARAM_PASSTHRU, reflection_function_ptr, 1);
}
/* }}} */

/* {{{ proto public void ReflectionFunction::__construct(string name) U
   Constructor. Throws an Exception in case the given function does not exist */
ZEND_METHOD(reflection_function, __construct)
{
	zval *name;
	zval *object;
	zval *closure = NULL;
	unsigned int lcname_len;
	zstr lcname;
	reflection_object *intern;
	zend_function *fptr;
	zstr name_str;
	int name_len;
	zend_uchar type;

	object = getThis();
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	if (intern == NULL) {
		return;
	}

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "O", &closure, zend_ce_closure) == SUCCESS) {
		fptr = (zend_function*)zend_get_closure_method_def(closure TSRMLS_CC);
		Z_ADDREF_P(closure);
	} else if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t", &name_str, &name_len, &type) == SUCCESS) {
		lcname = zend_u_str_case_fold(type, name_str, name_len, 1, &lcname_len);
		if (zend_u_hash_find(EG(function_table), type, lcname, lcname_len + 1, (void **)&fptr) == FAILURE) {
			efree(lcname.v);
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
				"Function %R() does not exist", type, name_str);
			return;
		}
		efree(lcname.v);
	} else {
		return;
	}

	MAKE_STD_ZVAL(name);
	ZVAL_UNICODE(name, fptr->common.function_name.u, 1);
	zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &name, sizeof(zval *), NULL);
	intern->ptr = fptr;
	intern->ref_type = REF_TYPE_FUNCTION;
	intern->obj = closure;
	intern->ce = NULL;
}
/* }}} */

/* {{{ proto public string ReflectionFunction::__toString() U
   Returns a string representation */
ZEND_METHOD(reflection_function, __toString)
{
	reflection_object *intern;
	zend_function *fptr;
	string str;
	zval* name;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(fptr);
	_default_lookup_entry(getThis(), "name", sizeof("name"), &name TSRMLS_CC);
	string_init(&str);
	_function_string(&str, fptr, intern->ce, name, intern->obj, "" TSRMLS_CC);
	RETURN_U_STRINGL(ZEND_U_CONVERTER(UG(output_encoding_conv)), str.string, str.len - 1, ZSTR_AUTOFREE);
}
/* }}} */

/* {{{ proto public string ReflectionFunction::getName() U
   Returns this function's name */
ZEND_METHOD(reflection_function, getName)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	_default_get_entry(getThis(), "name", sizeof("name"), return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto public bool ReflectionFunction::isClosure() U
   Returns whether this is a closure */
ZEND_METHOD(reflection_function, isClosure)
{
	reflection_object *intern;
	zend_function *fptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(fptr);
	RETURN_BOOL(intern->obj);
}
/* }}} */

/* {{{ proto public bool ReflectionFunction::getClosureThis() U
   Returns this pointer bound to closure */
ZEND_METHOD(reflection_function, getClosureThis)
{
	reflection_object *intern;
	zend_function *fptr;
	zval* closure_this;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(fptr);
	if (intern->obj) {
		closure_this = zend_get_closure_this_ptr(intern->obj TSRMLS_CC);
		if (closure_this) {
			RETURN_ZVAL(closure_this, 1, 0);
		}
	}
}
/* }}} */

/* {{{ proto public bool ReflectionFunction::isInternal() U
   Returns whether this is an internal function */
ZEND_METHOD(reflection_function, isInternal)
{
	reflection_object *intern;
	zend_function *fptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(fptr);
	RETURN_BOOL(fptr->type == ZEND_INTERNAL_FUNCTION);
}
/* }}} */

/* {{{ proto public bool ReflectionFunction::isUserDefined() U
   Returns whether this is an user-defined function */
ZEND_METHOD(reflection_function, isUserDefined)
{
	reflection_object *intern;
	zend_function *fptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(fptr);
	RETURN_BOOL(fptr->type == ZEND_USER_FUNCTION);
}
/* }}} */

/* {{{ proto public bool ReflectionFunction::isDisabled() U
   Returns whether this function has been disabled or not */
ZEND_METHOD(reflection_function, isDisabled)
{
	reflection_object *intern;
	zend_function *fptr;

	METHOD_NOTSTATIC(reflection_function_ptr);
	GET_REFLECTION_OBJECT_PTR(fptr);
	RETURN_BOOL(fptr->type == ZEND_INTERNAL_FUNCTION && fptr->internal_function.handler == zif_display_disabled_function);
}
/* }}} */

/* {{{ proto public string ReflectionFunction::getFileName() U
   Returns the filename of the file this function was declared in */
ZEND_METHOD(reflection_function, getFileName)
{
	reflection_object *intern;
	zend_function *fptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(fptr);
	if (fptr->type == ZEND_USER_FUNCTION) {
		RETURN_RT_STRING(fptr->op_array.filename, 1);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto public int ReflectionFunction::getStartLine() U
   Returns the line this function's declaration starts at */
ZEND_METHOD(reflection_function, getStartLine)
{
	reflection_object *intern;
	zend_function *fptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(fptr);
	if (fptr->type == ZEND_USER_FUNCTION) {
		RETURN_LONG(fptr->op_array.line_start);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto public int ReflectionFunction::getEndLine() U
   Returns the line this function's declaration ends at */
ZEND_METHOD(reflection_function, getEndLine)
{
	reflection_object *intern;
	zend_function *fptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(fptr);
	if (fptr->type == ZEND_USER_FUNCTION) {
		RETURN_LONG(fptr->op_array.line_end);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto public string ReflectionFunction::getDocComment() U
   Returns the doc comment for this function */
ZEND_METHOD(reflection_function, getDocComment)
{
	reflection_object *intern;
	zend_function *fptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(fptr);
	if (fptr->type == ZEND_USER_FUNCTION && fptr->op_array.doc_comment.v) {
		RETURN_ZSTRL(IS_UNICODE, fptr->op_array.doc_comment, fptr->op_array.doc_comment_len, 1);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto public array ReflectionFunction::getStaticVariables() U
   Returns an associative array containing this function's static variables and their values */
ZEND_METHOD(reflection_function, getStaticVariables)
{
	zval *tmp_copy;
	reflection_object *intern;
	zend_function *fptr;
	
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(fptr);

	/* Return an empty array in case no static variables exist */
	array_init(return_value);
	if (fptr->type == ZEND_USER_FUNCTION && fptr->op_array.static_variables != NULL) {
		zend_hash_apply_with_argument(fptr->op_array.static_variables, (apply_func_arg_t) zval_update_constant, (void*)1 TSRMLS_CC);
		zend_hash_copy(Z_ARRVAL_P(return_value), fptr->op_array.static_variables, (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *));
	}
}
/* }}} */

/* {{{ proto public mixed ReflectionFunction::getClosure()
   Returns a dynamically created closure for the function */
ZEND_METHOD(reflection_function, getClosure)
{
	reflection_object *intern;
	zend_function *fptr;
	
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(fptr);

	zend_create_closure(return_value, fptr, NULL, NULL TSRMLS_CC);
}
/* }}} */

/* {{{ proto public mixed ReflectionFunction::invoke(mixed* args) U
   Invokes the function */
ZEND_METHOD(reflection_function, invoke)
{
	zval *retval_ptr;
	zval ***params = NULL;
	int result, num_args = 0;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	reflection_object *intern;
	zend_function *fptr;
	
	METHOD_NOTSTATIC(reflection_function_ptr);
	GET_REFLECTION_OBJECT_PTR(fptr);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &params, &num_args) == FAILURE) {
		return;
	}

	fci.size = sizeof(fci);
	fci.function_table = NULL;
	fci.function_name = NULL;
	fci.symbol_table = NULL;
	fci.object_ptr = NULL;
	fci.retval_ptr_ptr = &retval_ptr;
	fci.param_count = num_args;
	fci.params = params;
	fci.no_separation = 1;

	fcc.initialized = 1;
	fcc.function_handler = fptr;
	fcc.calling_scope = EG(scope);
	fcc.called_scope = NULL;
	fcc.object_ptr = NULL;

	result = zend_call_function(&fci, &fcc TSRMLS_CC);

	efree(params);

	if (result == FAILURE) {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
			"Invocation of function %v() failed", fptr->common.function_name);
		return;
	}

	if (retval_ptr) {
		COPY_PZVAL_TO_ZVAL(*return_value, retval_ptr);
	}
}
/* }}} */

static int _zval_array_to_c_array(zval **arg, zval ****params TSRMLS_DC) /* {{{ */
{
	*(*params)++ = arg;
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ proto public mixed ReflectionFunction::invokeArgs(array args) U
   Invokes the function and pass its arguments as array. */
ZEND_METHOD(reflection_function, invokeArgs)
{
	zval *retval_ptr;
	zval ***params;
	int result;
	int argc;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	reflection_object *intern;
	zend_function *fptr;
	zval *param_array;
	
	METHOD_NOTSTATIC(reflection_function_ptr);
	GET_REFLECTION_OBJECT_PTR(fptr);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &param_array) == FAILURE) {
		return;
	}

	argc = zend_hash_num_elements(Z_ARRVAL_P(param_array));
	
	params = safe_emalloc(sizeof(zval **), argc, 0);
	zend_hash_apply_with_argument(Z_ARRVAL_P(param_array), (apply_func_arg_t)_zval_array_to_c_array, &params TSRMLS_CC);	
	params -= argc;

	fci.size = sizeof(fci);
	fci.function_table = NULL;
	fci.function_name = NULL;
	fci.symbol_table = NULL;
	fci.object_ptr = NULL;
	fci.retval_ptr_ptr = &retval_ptr;
	fci.param_count = argc;
	fci.params = params;
	fci.no_separation = 1;

	fcc.initialized = 1;
	fcc.function_handler = fptr;
	fcc.calling_scope = EG(scope);
	fcc.called_scope = NULL;
	fcc.object_ptr = NULL;

	result = zend_call_function(&fci, &fcc TSRMLS_CC);

	efree(params);

	if (result == FAILURE) {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
			"Invocation of function %v() failed", fptr->common.function_name);
		return;
	}

	if (retval_ptr) {
		COPY_PZVAL_TO_ZVAL(*return_value, retval_ptr);
	}
}
/* }}} */

/* {{{ proto public bool ReflectionFunction::returnsReference() U
   Gets whether this function returns a reference */
ZEND_METHOD(reflection_function, returnsReference)
{
	reflection_object *intern;
	zend_function *fptr;

	METHOD_NOTSTATIC(reflection_function_abstract_ptr);
	GET_REFLECTION_OBJECT_PTR(fptr);

	RETURN_BOOL(fptr->op_array.return_reference);
}
/* }}} */

/* {{{ proto public bool ReflectionFunction::getNumberOfParameters() U
   Gets the number of required parameters */
ZEND_METHOD(reflection_function, getNumberOfParameters)
{
	reflection_object *intern;
	zend_function *fptr;

	METHOD_NOTSTATIC(reflection_function_abstract_ptr);
	GET_REFLECTION_OBJECT_PTR(fptr);

	RETURN_LONG(fptr->common.num_args);
}
/* }}} */

/* {{{ proto public bool ReflectionFunction::getNumberOfRequiredParameters() U
   Gets the number of required parameters */
ZEND_METHOD(reflection_function, getNumberOfRequiredParameters)
{
	reflection_object *intern;
	zend_function *fptr;

	METHOD_NOTSTATIC(reflection_function_abstract_ptr);
	GET_REFLECTION_OBJECT_PTR(fptr);

	RETURN_LONG(fptr->common.required_num_args);
}
/* }}} */

/* {{{ proto public ReflectionParameter[] ReflectionFunction::getParameters() U
   Returns an array of parameter objects for this function */
ZEND_METHOD(reflection_function, getParameters)
{
	reflection_object *intern;
	zend_function *fptr;
	zend_uint i;
	struct _zend_arg_info *arg_info;

	METHOD_NOTSTATIC(reflection_function_abstract_ptr);
	GET_REFLECTION_OBJECT_PTR(fptr);

	arg_info= fptr->common.arg_info;

	array_init(return_value);
	for (i = 0; i < fptr->common.num_args; i++) {
		zval *parameter;   

		ALLOC_ZVAL(parameter);
		reflection_parameter_factory(_copy_function(fptr TSRMLS_CC), arg_info, i, fptr->common.required_num_args, parameter TSRMLS_CC);
		add_next_index_zval(return_value, parameter);

		arg_info++;
	}
}
/* }}} */

/* {{{ proto public ReflectionExtension|NULL ReflectionFunction::getExtension() U
   Returns NULL or the extension the function belongs to */
ZEND_METHOD(reflection_function, getExtension)
{
	reflection_object *intern;
	zend_function *fptr;
	zend_internal_function *internal;

	METHOD_NOTSTATIC(reflection_function_abstract_ptr);
	GET_REFLECTION_OBJECT_PTR(fptr);

	if (fptr->type != ZEND_INTERNAL_FUNCTION) {
		RETURN_NULL();
	}

	internal = (zend_internal_function *)fptr;
	if (internal->module) {
		reflection_extension_factory(return_value, internal->module->name TSRMLS_CC);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ proto public string|false ReflectionFunction::getExtensionName() U
   Returns false or the name of the extension the function belongs to */
ZEND_METHOD(reflection_function, getExtensionName)
{
	reflection_object *intern;
	zend_function *fptr;
	zend_internal_function *internal;

	METHOD_NOTSTATIC(reflection_function_abstract_ptr);
	GET_REFLECTION_OBJECT_PTR(fptr);

	if (fptr->type != ZEND_INTERNAL_FUNCTION) {
		RETURN_FALSE;
	}

	internal = (zend_internal_function *)fptr;
	if (internal->module) {
		RETURN_ASCII_STRING(internal->module->name, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto public static mixed ReflectionParameter::export(mixed function, mixed parameter [, bool return]) throws ReflectionException U
   Exports a reflection object. Returns the output if TRUE is specified for return, printing it otherwise. */
ZEND_METHOD(reflection_parameter, export)
{
	_reflection_export(INTERNAL_FUNCTION_PARAM_PASSTHRU, reflection_parameter_ptr, 2);
}
/* }}} */

/* {{{ proto public void ReflectionParameter::__construct(mixed function, mixed parameter) U
   Constructor. Throws an Exception in case the given method does not exist */
ZEND_METHOD(reflection_parameter, __construct)
{
	parameter_reference *ref;
	zval *reference, **parameter;
	zval *object;
	zval *name;
	reflection_object *intern;
	zend_function *fptr;
	struct _zend_arg_info *arg_info;
	int position;
	zend_class_entry *ce = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zZ", &reference, &parameter) == FAILURE) {
		return;
	}

	object = getThis();
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	if (intern == NULL) {
		return;
	}
	
	/* First, find the function */
	switch (Z_TYPE_P(reference)) {
		case IS_UNICODE:
		case IS_STRING: {
				unsigned int lcname_len;
				zstr lcname;

				lcname = zend_u_str_case_fold(Z_TYPE_P(reference), Z_UNIVAL_P(reference), Z_UNILEN_P(reference), 1, &lcname_len);
				if (zend_u_hash_find(EG(function_table), Z_TYPE_P(reference), lcname, lcname_len + 1, (void**) &fptr) == FAILURE) {
					efree(lcname.v);
					zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
						"Function %R() does not exist", Z_TYPE_P(reference), Z_UNIVAL_P(reference));
					return;
				}
				efree(lcname.v);
			}
			ce = fptr->common.scope;
			break;

		case IS_ARRAY: {
				zval **classref;
				zval **method;
				zend_class_entry **pce;
				unsigned int lcname_len;
				zstr lcname;
				
				if ((zend_hash_index_find(Z_ARRVAL_P(reference), 0, (void **) &classref) == FAILURE)
					|| (zend_hash_index_find(Z_ARRVAL_P(reference), 1, (void **) &method) == FAILURE))
				{
					_DO_THROW("Expected array($object, $method) or array($classname, $method)");
					/* returns out of this function */
				}

				if (Z_TYPE_PP(classref) == IS_OBJECT) {
					ce = Z_OBJCE_PP(classref);
				} else {
					convert_to_unicode_ex(classref);
					if (zend_u_lookup_class(Z_TYPE_PP(classref), Z_UNIVAL_PP(classref), Z_UNILEN_PP(classref), &pce TSRMLS_CC) == FAILURE) {
						zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC,
								"Class %R does not exist", Z_TYPE_PP(classref), Z_UNIVAL_PP(classref));
						return;
					}
					ce = *pce;
				}
				
				convert_to_unicode_ex(method);
				lcname = zend_u_str_case_fold(Z_TYPE_PP(method), Z_UNIVAL_PP(method), Z_UNILEN_PP(method), 1, &lcname_len);
				if (ce == zend_ce_closure && Z_TYPE_PP(classref) == IS_OBJECT
					&& lcname_len == sizeof(ZEND_INVOKE_FUNC_NAME)-1
					&& ZEND_U_EQUAL(IS_UNICODE, lcname, lcname_len, ZEND_INVOKE_FUNC_NAME, sizeof(ZEND_INVOKE_FUNC_NAME)-1)
					&& (fptr = zend_get_closure_invoke_method(*classref TSRMLS_CC)) != NULL)
				{
					/* do nothing, fptr is already set */
				} else if (zend_u_hash_find(&ce->function_table, Z_TYPE_PP(method), lcname, lcname_len + 1, (void **) &fptr) == FAILURE) {
					efree(lcname.v);
					zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
						"Method %v::%R() does not exist", ce->name, Z_TYPE_PP(method), Z_UNIVAL_PP(method));
					return;
				}
				efree(lcname.v);
			}
			break;

		case IS_OBJECT: {
				ce = Z_OBJCE_P(reference);
				
				if (instanceof_function(ce, zend_ce_closure TSRMLS_CC)) {
					fptr = zend_get_closure_invoke_method(reference TSRMLS_CC);
				} else if (zend_u_hash_find(&ce->function_table, IS_STRING, ZSTR(ZEND_INVOKE_FUNC_NAME), sizeof(ZEND_INVOKE_FUNC_NAME), (void **)&fptr) == FAILURE) {
					zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC,
						"Method %v::%s() does not exist", ce->name, ZEND_INVOKE_FUNC_NAME);
					return;
				}
			}
			break;

		default:
			_DO_THROW("The parameter class is expected to be either a string, an array(class, method) or a callable object");
			/* returns out of this function */
	}
	
	/* Now, search for the parameter */
	arg_info = fptr->common.arg_info;
	if (Z_TYPE_PP(parameter) == IS_LONG) {
		position= Z_LVAL_PP(parameter);
		if (position < 0 || (zend_uint)position >= fptr->common.num_args) {
			if (fptr->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) {
				if (fptr->type != ZEND_OVERLOADED_FUNCTION) {
					efree(fptr->common.function_name.v);
				}
				efree(fptr);
			}
			_DO_THROW("The parameter specified by its offset could not be found");
			/* returns out of this function */
		}
	} else {
		zend_uint i;

		position= -1;
		convert_to_unicode_ex(parameter);
		for (i = 0; i < fptr->common.num_args; i++) {
			if (Z_TYPE_PP(parameter) == IS_STRING && arg_info[i].name.s && strcmp(arg_info[i].name.s, Z_STRVAL_PP(parameter)) == 0) {
				position= i;
				break;
			} else if (Z_TYPE_PP(parameter) == IS_UNICODE && arg_info[i].name.u && u_strcmp(arg_info[i].name.u, Z_USTRVAL_PP(parameter)) == 0) {
				position= i;
				break;
			}
		}
		if (position == -1) {
			if (fptr->common.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) {
				if (fptr->type != ZEND_OVERLOADED_FUNCTION) {
					efree(fptr->common.function_name.v);
				}
				efree(fptr);
			}
			_DO_THROW("The parameter specified by its name could not be found");
			/* returns out of this function */
		}
	}
	
	MAKE_STD_ZVAL(name);
	if (arg_info[position].name.v) {
		ZVAL_UNICODEL(name, arg_info[position].name.u, arg_info[position].name_len, 1);
	} else {
		ZVAL_NULL(name);
	}
	zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &name, sizeof(zval *), NULL);

	ref = (parameter_reference*) emalloc(sizeof(parameter_reference));
	ref->arg_info = &arg_info[position];
	ref->offset = (zend_uint)position;
	ref->required = fptr->common.required_num_args;
	ref->fptr = fptr;
	intern->ptr = ref;
	intern->ref_type = REF_TYPE_PARAMETER;
	intern->ce = ce;
}
/* }}} */

/* {{{ proto public string ReflectionParameter::__toString() U
   Returns a string representation */
ZEND_METHOD(reflection_parameter, __toString)
{
	reflection_object *intern;
	parameter_reference *param;
	string str;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(param);
	string_init(&str);
	_parameter_string(&str, param->fptr, param->arg_info, param->offset, param->required, "" TSRMLS_CC);
	RETURN_U_STRINGL(ZEND_U_CONVERTER(UG(output_encoding_conv)), str.string, str.len - 1, ZSTR_AUTOFREE);
}
/* }}} */

/* {{{ proto public string ReflectionParameter::getName() U
   Returns this parameters's name */
ZEND_METHOD(reflection_parameter, getName)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	_default_get_entry(getThis(), "name", sizeof("name"), return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto public ReflectionFunction ReflectionParameter::getDeclaringFunction() U
   Returns the ReflectionFunction for the function of this parameter */
ZEND_METHOD(reflection_parameter, getDeclaringFunction)
{
	reflection_object *intern;
	parameter_reference *param;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(param);

	if (!param->fptr->common.scope) {
		reflection_function_factory(_copy_function(param->fptr TSRMLS_CC), return_value TSRMLS_CC);
	} else {
		reflection_method_factory(param->fptr->common.scope, _copy_function(param->fptr TSRMLS_CC), return_value TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto public ReflectionClass|NULL ReflectionParameter::getDeclaringClass() U
   Returns in which class this parameter is defined (not the typehint of the parameter) */
ZEND_METHOD(reflection_parameter, getDeclaringClass)
{
	reflection_object *intern;
	parameter_reference *param;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(param);

	if (param->fptr->common.scope) {
		zend_reflection_class_factory(param->fptr->common.scope, return_value TSRMLS_CC);
	}	
}
/* }}} */

/* {{{ proto public ReflectionClass|NULL ReflectionParameter::getClass() U
   Returns this parameters's class hint or NULL if there is none */
ZEND_METHOD(reflection_parameter, getClass)
{
	reflection_object *intern;
	parameter_reference *param;
	zend_class_entry **pce, *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(param);

	if (param->arg_info->class_name.v) {

		/* Class name is stored as a string, we might also get "self" or "parent"
		 * - For "self", simply use the function scope. If scope is NULL then
		 *   the function is global and thus self does not make any sense
		 *
		 * - For "parent", use the function scope's parent. If scope is NULL then
		 *   the function is global and thus parent does not make any sense.
		 *   If the parent is NULL then the class does not extend anything and
		 *   thus parent does not make any sense, either.
		 *
		 * TODO: Think about moving these checks to the compiler or some sort of
		 * lint-mode.
		 */
		if (ZEND_U_EQUAL(IS_UNICODE, param->arg_info->class_name, param->arg_info->class_name_len, "self", sizeof("self")- 1)) {
			ce = param->fptr->common.scope;
			if (!ce) {
				zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
					"Parameter uses 'self' as type hint but function is not a class member!");
				return;
			}
			pce= &ce;
		} else if (ZEND_U_EQUAL(IS_UNICODE, param->arg_info->class_name,  param->arg_info->class_name_len, "parent", sizeof("parent")- 1)) {
			ce = param->fptr->common.scope;
			if (!ce) {
				zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
					"Parameter uses 'parent' as type hint but function is not a class member!");
				return;
			}
			if (!ce->parent) {
				zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
					"Parameter uses 'parent' as type hint although class does not have a parent!");
				return;
			}
			pce= &ce->parent;
		} else if (zend_u_lookup_class(IS_UNICODE, param->arg_info->class_name, param->arg_info->class_name_len, &pce TSRMLS_CC) == FAILURE) {
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
				"Class %v does not exist", param->arg_info->class_name);
			return;
		}
		zend_reflection_class_factory(*pce, return_value TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto public bool ReflectionParameter::isArray() U
   Returns whether parameter MUST be an array */
ZEND_METHOD(reflection_parameter, isArray)
{
	reflection_object *intern;
	parameter_reference *param;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(param);

	RETVAL_BOOL(param->arg_info->array_type_hint);
}
/* }}} */

/* {{{ proto public bool ReflectionParameter::allowsNull() U
   Returns whether NULL is allowed as this parameters's value */
ZEND_METHOD(reflection_parameter, allowsNull)
{
	reflection_object *intern;
	parameter_reference *param;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(param);

	RETVAL_BOOL(param->arg_info->allow_null);
}
/* }}} */

/* {{{ proto public bool ReflectionParameter::isPassedByReference() U
   Returns whether this parameters is passed to by reference */
ZEND_METHOD(reflection_parameter, isPassedByReference)
{
	reflection_object *intern;
	parameter_reference *param;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(param);

	RETVAL_BOOL(param->arg_info->pass_by_reference);
}
/* }}} */

/* {{{ proto public bool ReflectionParameter::getPosition() U
   Returns whether this parameter is an optional parameter */
ZEND_METHOD(reflection_parameter, getPosition)
{
	reflection_object *intern;
	parameter_reference *param;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(param);

	RETVAL_LONG(param->offset);
}
/* }}} */

/* {{{ proto public bool ReflectionParameter::isOptional() U
   Returns whether this parameter is an optional parameter */
ZEND_METHOD(reflection_parameter, isOptional)
{
	reflection_object *intern;
	parameter_reference *param;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(param);

	RETVAL_BOOL(param->offset >= param->required);
}
/* }}} */

/* {{{ proto public bool ReflectionParameter::isDefaultValueAvailable() U
   Returns whether the default value of this parameter is available */
ZEND_METHOD(reflection_parameter, isDefaultValueAvailable)
{
	reflection_object *intern;
	parameter_reference *param;
	zend_op *precv;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(param);

	if (param->fptr->type != ZEND_USER_FUNCTION)
	{
		RETURN_FALSE;
	}
	if (param->offset < param->required) {
		RETURN_FALSE;
	}
	precv = _get_recv_op((zend_op_array*)param->fptr, param->offset);
	if (!precv || precv->opcode != ZEND_RECV_INIT || precv->op2.op_type == IS_UNUSED) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public bool ReflectionParameter::getDefaultValue() U
   Returns the default value of this parameter or throws an exception */
ZEND_METHOD(reflection_parameter, getDefaultValue)
{
	reflection_object *intern;
	parameter_reference *param;
	zend_op *precv;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(param);

	if (param->fptr->type != ZEND_USER_FUNCTION)
	{
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, "Cannot determine default value for internal functions");
		return;
	}
	if (param->offset < param->required) {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, "Parameter is not optional"); 
		return;
	}
	precv = _get_recv_op((zend_op_array*)param->fptr, param->offset);
	if (!precv || precv->opcode != ZEND_RECV_INIT || precv->op2.op_type == IS_UNUSED) {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, "Internal error"); 
		return;
	}

	*return_value = precv->op2.u.constant;
	INIT_PZVAL(return_value);
	if (Z_TYPE_P(return_value) != IS_CONSTANT) {
		zval_copy_ctor(return_value);
	}
	zval_update_constant_ex(&return_value, (void*)0, param->fptr->common.scope TSRMLS_CC);
}
/* }}} */

/* {{{ proto public static mixed ReflectionMethod::export(mixed class, string name [, bool return]) throws ReflectionException U
   Exports a reflection object. Returns the output if TRUE is specified for return, printing it otherwise. */
ZEND_METHOD(reflection_method, export)
{
	_reflection_export(INTERNAL_FUNCTION_PARAM_PASSTHRU, reflection_method_ptr, 2);
}
/* }}} */

/* {{{ proto public void ReflectionMethod::__construct(mixed class_or_method [, string name]) U
   Constructor. Throws an Exception in case the given method does not exist */
ZEND_METHOD(reflection_method, __construct)
{
	zval *name, *classname;
	zval *object, *orig_obj;
	reflection_object *intern;
	unsigned int lcname_len;
	zstr lcname;
	zend_class_entry **pce;
	zend_class_entry *ce;
	zend_function *mptr;
	zstr name_str;
	zstr tmp;
	int name_len, tmp_len, free_name_str = 0;
	zval ztmp;
	zend_uchar type;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "o", &classname) == SUCCESS) {
		name_str.u = USTR_MAKE(ZEND_INVOKE_FUNC_NAME);
		free_name_str = 1;
		type = IS_UNICODE;
		name_len = sizeof(ZEND_INVOKE_FUNC_NAME)-1;
		orig_obj = classname;
	} else if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "zt", &classname, &name_str, &name_len, &type) == FAILURE) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t", &name_str, &name_len, &type) == FAILURE) {
			return;
		}
		if ((type == IS_UNICODE && (tmp.u = u_strstr(name_str.u, u_doublecolon)) == NULL)
			|| (type == IS_STRING  && (tmp.s =   strstr(name_str.s, "::")) == NULL))
		{
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, "Invalid method name %R", type, name_str);
			return;
		}
		classname = &ztmp;
		if (type == IS_UNICODE) {
			tmp_len = tmp.u - name_str.u;
		} else {
			tmp_len = tmp.s - name_str.s;
		}
		ZVAL_ZSTRL(classname, type, name_str, tmp_len, 1);
		name_len = name_len - (tmp_len + 2);
		if (type == IS_UNICODE) {
			name_str.u = tmp.u + 2;
		} else {
			name_str.s = tmp.s + 2;
		}
		orig_obj = NULL;
	} else if (Z_TYPE_P(classname) == IS_OBJECT) {
		orig_obj = classname;
	} else {
		orig_obj = NULL;
	}

	object = getThis();
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	if (intern == NULL) {
		if (free_name_str) {
			efree(name_str.v);
		}
		return;
	}
	
	/* Find the class entry */
	switch (Z_TYPE_P(classname)) {
		case IS_STRING:
		case IS_UNICODE:
			if (zend_u_lookup_class(Z_TYPE_P(classname), Z_UNIVAL_P(classname), Z_UNILEN_P(classname), &pce TSRMLS_CC) == FAILURE) {
				zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC,
						"Class %R does not exist", Z_TYPE_P(classname), Z_UNIVAL_P(classname)); 
				if (classname == &ztmp) {
					zval_dtor(&ztmp);
				}
				if (free_name_str) {
					efree(name_str.v);
				}
				return;
			}
			ce = *pce;
			break;

		case IS_OBJECT:
			ce = Z_OBJCE_P(classname);
			break;

		default:
			if (classname == &ztmp) {
				zval_dtor(&ztmp);
			}
			_DO_THROW("The parameter class is expected to be either a string or an object");
			/* returns out of this function */
	}

	if (classname == &ztmp) {
		zval_dtor(&ztmp);
	}

	lcname = zend_u_str_case_fold(type, name_str, name_len, 1, &lcname_len);

	if (ce == zend_ce_closure && orig_obj && (lcname_len == sizeof(ZEND_INVOKE_FUNC_NAME)-1)
		&& ZEND_U_EQUAL(IS_UNICODE, lcname, lcname_len, ZEND_INVOKE_FUNC_NAME, sizeof(ZEND_INVOKE_FUNC_NAME)-1)
		&& (mptr = zend_get_closure_invoke_method(orig_obj TSRMLS_CC)) != NULL)
	{
		/* do nothing, mptr already set */
	} else if (zend_u_hash_find(&ce->function_table, type, lcname, lcname_len + 1, (void **) &mptr) == FAILURE) {
		/* Check if this is a property storing a closure */
		mptr = NULL;  /* Set by closure detection again */
		if (orig_obj) {
			zval **callable, member;
			zend_property_info *property_info;
			zend_object *zobj = zend_objects_get_address(orig_obj TSRMLS_CC);

			ZVAL_ZSTRL(&member, type, name_str, name_len, 0);
			property_info = zend_get_property_info(ce, &member, 1 TSRMLS_CC);
	
			if (property_info && zend_u_hash_quick_find(zobj->properties, type, property_info->name, property_info->name_length+1, property_info->h, (void **) &callable) == SUCCESS) {
				zval *callable_obj;
				zend_class_entry *ce_ptr;
				zend_function *fbc;
	
				if (Z_TYPE_PP(callable) == IS_OBJECT
				&& Z_OBJ_HANDLER_PP(callable, get_closure)
				&& Z_OBJ_HANDLER_PP(callable, get_closure)(*callable, &ce_ptr, &fbc, &callable_obj TSRMLS_CC) == SUCCESS) {
					mptr = fbc;
					Z_ADDREF_PP(callable);
					intern->obj = *callable;
				}
			}
		}
		if (!mptr) {
			efree(lcname.v);
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
				"Method %v::%R() does not exist", ce->name, type, name_str);
			if (free_name_str) {
				efree(name_str.v);
			}
			return;
		}
	}
	
	MAKE_STD_ZVAL(classname);
	MAKE_STD_ZVAL(name);

	if (intern->obj) {
		ZVAL_UNICODEL(classname, ce->name.u, ce->name_length, 1);
		ZVAL_ZSTRL(name, type, name_str, name_len, 1);		
	} else {
		ZVAL_UNICODEL(classname, mptr->common.scope->name.u, mptr->common.scope->name_length, 1);
		ZVAL_UNICODE(name, mptr->common.function_name.u, 1);
	}
	efree(lcname.v);
	if (free_name_str) {
		efree(name_str.v);
	}
	
	zend_ascii_hash_update(Z_OBJPROP_P(object), "class", sizeof("class"), (void **) &classname, sizeof(zval *), NULL);
	zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &name, sizeof(zval *), NULL);
	
	intern->ptr = mptr;
	intern->ref_type = REF_TYPE_FUNCTION;
	intern->ce = ce;
}
/* }}} */

/* {{{ proto public string ReflectionMethod::__toString() U
   Returns a string representation */
ZEND_METHOD(reflection_method, __toString)
{
	reflection_object *intern;
	zend_function *mptr;
	string str;
	zval* name;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(mptr);
	_default_lookup_entry(getThis(), "name", sizeof("name"), &name TSRMLS_CC);
	string_init(&str);
	_function_string(&str, mptr, intern->ce, name, intern->obj, "" TSRMLS_CC);
	RETURN_U_STRINGL(ZEND_U_CONVERTER(UG(output_encoding_conv)), str.string, str.len - 1, ZSTR_AUTOFREE);
}
/* }}} */

/* {{{ proto public mixed ReflectionMethod::getClosure([mixed object])
   Invokes the function */
ZEND_METHOD(reflection_method, getClosure)
{
	reflection_object *intern;
	zval *obj;
	zend_function *mptr;
	
	METHOD_NOTSTATIC(reflection_method_ptr);
	GET_REFLECTION_OBJECT_PTR(mptr);

	if (mptr->common.fn_flags & ZEND_ACC_STATIC)  {
		zend_create_closure(return_value, mptr, mptr->common.scope, NULL TSRMLS_CC);
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &obj) == FAILURE) {
			return;
		}

		if (!instanceof_function(Z_OBJCE_P(obj), mptr->common.scope TSRMLS_CC)) {
			_DO_THROW("Given object is not an instance of the class this method was declared in");
			/* Returns from this function */
		}

		/* This is an original closure object and __invoke is to be called. */
		if (Z_OBJCE_P(obj) == zend_ce_closure && mptr->type == ZEND_INTERNAL_FUNCTION &&
			(mptr->internal_function.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) != 0)
		{
			RETURN_ZVAL(obj, 1, 0);
		} else {
			zend_create_closure(return_value, mptr, mptr->common.scope, obj TSRMLS_CC);
		}
	}
}
/* }}} */

/* {{{ proto public mixed ReflectionMethod::invoke(mixed object, mixed* args) U
   Invokes the method. */
ZEND_METHOD(reflection_method, invoke)
{
	zval *retval_ptr;
	zval ***params = NULL;
	zval *object_ptr;
	reflection_object *intern;
	zend_function *mptr;
	int result, num_args = 0;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	zend_class_entry *obj_ce;
	
	METHOD_NOTSTATIC(reflection_method_ptr);
	
	GET_REFLECTION_OBJECT_PTR(mptr);

	if (!(mptr->common.fn_flags & ZEND_ACC_PUBLIC)
		|| (mptr->common.fn_flags & ZEND_ACC_ABSTRACT))
	{
		if (mptr->common.fn_flags & ZEND_ACC_ABSTRACT) {
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
				"Trying to invoke abstract method %v::%v()", 
				mptr->common.scope->name, mptr->common.function_name);
		} else {
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC,
				"Trying to invoke %s method %v::%v() from scope %v", 
				mptr->common.fn_flags & ZEND_ACC_PROTECTED ? "protected" : "private",
				mptr->common.scope->name, mptr->common.function_name,
				Z_OBJCE_P(getThis())->name);
		}
		return;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &params, &num_args) == FAILURE) {
		return;
	}
	
	/* In case this is a static method, we should'nt pass an object_ptr
	 * (which is used as calling context aka $this). We can thus ignore the
	 * first parameter.
	 *
	 * Else, we verify that the given object is an instance of the class.
	 */
	if (mptr->common.fn_flags & ZEND_ACC_STATIC) {
		object_ptr = NULL;
		obj_ce = mptr->common.scope;
	} else {
		if ((Z_TYPE_PP(params[0]) != IS_OBJECT)) {
			efree(params);
			_DO_THROW("Non-object passed to Invoke()");
			/* Returns from this function */
		}
		obj_ce = Z_OBJCE_PP(params[0]);

		if (!instanceof_function(obj_ce, mptr->common.scope TSRMLS_CC)) {
			efree(params);
			_DO_THROW("Given object is not an instance of the class this method was declared in");
			/* Returns from this function */
		}
	
		object_ptr = *params[0];
	}
	
	fci.size = sizeof(fci);
	fci.function_table = NULL;
	fci.function_name = NULL;
	fci.symbol_table = NULL;
	fci.object_ptr = object_ptr;
	fci.retval_ptr_ptr = &retval_ptr;
	fci.param_count = num_args-1;
	fci.params = params+1;
	fci.no_separation = 1;

	fcc.initialized = 1;
	fcc.function_handler = mptr;
	fcc.calling_scope = obj_ce;
	fcc.called_scope = obj_ce;
	fcc.object_ptr = object_ptr;

	result = zend_call_function(&fci, &fcc TSRMLS_CC);
	
	efree(params);

	if (result == FAILURE) {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
			"Invocation of method %v::%v() failed", mptr->common.scope->name, mptr->common.function_name);
		return;
	}

	if (retval_ptr) {
		COPY_PZVAL_TO_ZVAL(*return_value, retval_ptr);
	}
}
/* }}} */

/* {{{ proto public mixed ReflectionMethod::invokeArgs(mixed object, array args) U
   Invokes the function and pass its arguments as array. */
ZEND_METHOD(reflection_method, invokeArgs)
{
	zval *retval_ptr;
	zval ***params;
	zval *object;
	reflection_object *intern;
	zend_function *mptr;
	int argc;
	int result;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	zend_class_entry *obj_ce;
	zval *param_array;
	
	METHOD_NOTSTATIC(reflection_method_ptr);

	GET_REFLECTION_OBJECT_PTR(mptr);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o!a", &object, &param_array) == FAILURE) {
		return;
	}

	if (!(mptr->common.fn_flags & ZEND_ACC_PUBLIC)
		|| (mptr->common.fn_flags & ZEND_ACC_ABSTRACT))
	{
		if (mptr->common.fn_flags & ZEND_ACC_ABSTRACT) {
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
				"Trying to invoke abstract method %v::%v", 
				mptr->common.scope->name, mptr->common.function_name);
		} else {
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC,
				"Trying to invoke %s method %v::%v from scope %v", 
				mptr->common.fn_flags & ZEND_ACC_PROTECTED ? "protected" : "private",
				mptr->common.scope->name, mptr->common.function_name,
				Z_OBJCE_P(getThis())->name);
		}
		return;
	}

	argc = zend_hash_num_elements(Z_ARRVAL_P(param_array));
	
	params = safe_emalloc(sizeof(zval **), argc, 0);
	zend_hash_apply_with_argument(Z_ARRVAL_P(param_array), (apply_func_arg_t)_zval_array_to_c_array, &params TSRMLS_CC);	
	params -= argc;
	
	/* In case this is a static method, we should'nt pass an object_ptr
	 * (which is used as calling context aka $this). We can thus ignore the
	 * first parameter.
	 *
	 * Else, we verify that the given object is an instance of the class.
	 */
	if (mptr->common.fn_flags & ZEND_ACC_STATIC) {
		object = NULL;
		obj_ce = mptr->common.scope;
	} else {
		if (!object) {
			efree(params);
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC,
				"Trying to invoke non static method %v::%v without an object", 
				mptr->common.scope->name, mptr->common.function_name);
			return;
		}
		
		obj_ce = Z_OBJCE_P(object);

		if (!instanceof_function(obj_ce, mptr->common.scope TSRMLS_CC)) {
			efree(params);
			_DO_THROW("Given object is not an instance of the class this method was declared in");
			/* Returns from this function */
		}
	}
	
	fci.size = sizeof(fci);
	fci.function_table = NULL;
	fci.function_name = NULL;
	fci.symbol_table = NULL;
	fci.object_ptr = object;
	fci.retval_ptr_ptr = &retval_ptr;
	fci.param_count = argc;
	fci.params = params;
	fci.no_separation = 1;

	fcc.initialized = 1;
	fcc.function_handler = mptr;
	fcc.calling_scope = obj_ce;
	fcc.called_scope = obj_ce;
	fcc.object_ptr = object;

	result = zend_call_function(&fci, &fcc TSRMLS_CC);
	
	efree(params);

	if (result == FAILURE) {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
			"Invocation of method %v::%v() failed", mptr->common.scope->name, mptr->common.function_name);
		return;
	}

	if (retval_ptr) {
		COPY_PZVAL_TO_ZVAL(*return_value, retval_ptr);
	}
}
/* }}} */

/* {{{ proto public bool ReflectionMethod::isFinal() U
   Returns whether this method is final */
ZEND_METHOD(reflection_method, isFinal)
{
	_function_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_FINAL);
}
/* }}} */

/* {{{ proto public bool ReflectionMethod::isAbstract() U
   Returns whether this method is abstract */
ZEND_METHOD(reflection_method, isAbstract)
{
	_function_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_ABSTRACT);
}
/* }}} */

/* {{{ proto public bool ReflectionMethod::isPublic() U
   Returns whether this method is public */
ZEND_METHOD(reflection_method, isPublic)
{
	_function_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_PUBLIC);
}
/* }}} */

/* {{{ proto public bool ReflectionMethod::isPrivate() U
   Returns whether this method is private */
ZEND_METHOD(reflection_method, isPrivate)
{
	_function_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_PRIVATE);
}
/* }}} */

/* {{{ proto public bool ReflectionMethod::isProtected() U
   Returns whether this method is protected */
ZEND_METHOD(reflection_method, isProtected)
{
	_function_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_PROTECTED);
}
/* }}} */

/* {{{ proto public bool ReflectionMethod::isStatic() U
   Returns whether this method is static */
ZEND_METHOD(reflection_method, isStatic)
{
	_function_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_STATIC);
}
/* }}} */

/* {{{ proto public bool ReflectionFunction::isDeprecated() U
   Returns whether this function is deprecated */
ZEND_METHOD(reflection_function, isDeprecated)
{
	_function_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_DEPRECATED);
}
/* }}} */

/* {{{ proto public bool ReflectionFunction::inNamespace()
   Returns whether this function is defined in namespace */
ZEND_METHOD(reflection_function, inNamespace)
{
	zval **name;
	zstr colon;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	if (zend_hash_find(Z_OBJPROP_P(getThis()), "name", sizeof("name"), (void **) &name) == FAILURE) {
		RETURN_FALSE;
	}
	if (Z_TYPE_PP(name) == IS_STRING
		&& (colon.s = zend_memrchr(Z_STRVAL_PP(name), '\\', Z_STRLEN_PP(name)))
		&& colon.s > Z_STRVAL_PP(name))
	{
		RETURN_TRUE;
	} else if (Z_TYPE_PP(name) == IS_UNICODE
		&& (colon.u = u_memrchr(Z_USTRVAL_PP(name), '\\', Z_USTRLEN_PP(name)))
		&& colon.u > Z_USTRVAL_PP(name))
	{
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto public string ReflectionFunction::getNamespaceName() U
   Returns the name of namespace where this function is defined */
ZEND_METHOD(reflection_function, getNamespaceName)
{
	zval **name;
	zstr backslash;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	if (zend_hash_find(Z_OBJPROP_P(getThis()), "name", sizeof("name"), (void **) &name) == FAILURE) {
		RETURN_FALSE;
	}
	if (Z_TYPE_PP(name) == IS_STRING
		&& (backslash.s = zend_memrchr(Z_STRVAL_PP(name), '\\', Z_STRLEN_PP(name)))
		&& backslash.s > Z_STRVAL_PP(name))
	{
		RETURN_STRINGL(Z_STRVAL_PP(name), backslash.s - Z_STRVAL_PP(name), 1);
	} else if (Z_TYPE_PP(name) == IS_UNICODE
		&& (backslash.u = u_memrchr(Z_USTRVAL_PP(name), '\\', Z_USTRLEN_PP(name)))
		&& backslash.u > Z_USTRVAL_PP(name))
	{
		RETURN_UNICODEL(Z_USTRVAL_PP(name), backslash.u - Z_USTRVAL_PP(name), 1);
	}
	RETURN_EMPTY_UNICODE();
}
/* }}} */

/* {{{ proto public string ReflectionFunction::getShortName() U
   Returns the short name of the function (without namespace part) */
ZEND_METHOD(reflection_function, getShortName)
{
	zval **name;
	zstr backslash;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	if (zend_hash_find(Z_OBJPROP_P(getThis()), "name", sizeof("name"), (void **) &name) == FAILURE) {
		RETURN_FALSE;
	}
	if (Z_TYPE_PP(name) == IS_STRING
		&& (backslash.s = zend_memrchr(Z_STRVAL_PP(name), '\\', Z_STRLEN_PP(name)))
		&& backslash.s > Z_STRVAL_PP(name))
	{
		RETURN_STRINGL(backslash.s + 1, Z_STRLEN_PP(name) - (backslash.s - Z_STRVAL_PP(name) + 1), 1);
	} else if (Z_TYPE_PP(name) == IS_UNICODE
		&& (backslash.u = u_memrchr(Z_USTRVAL_PP(name), '\\', Z_USTRLEN_PP(name)))
		&& backslash.u > Z_USTRVAL_PP(name))
	{
		RETURN_UNICODEL(backslash.u + 1, Z_USTRLEN_PP(name) - (backslash.u - Z_USTRVAL_PP(name) + 1), 1);
	}
	RETURN_ZVAL(*name, 1, 0);
}
/* }}} */

/* {{{ proto public bool ReflectionMethod::isConstructor() U
   Returns whether this method is the constructor */
ZEND_METHOD(reflection_method, isConstructor)
{
	reflection_object *intern;
	zend_function *mptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(mptr);
	/* we need to check if the ctor is the ctor of the class level we we 
	 * looking at since we might be looking at an inherited old style ctor
	 * defined in base class. */
	RETURN_BOOL(mptr->common.fn_flags & ZEND_ACC_CTOR && intern->ce->constructor && intern->ce->constructor->common.scope == mptr->common.scope);
}
/* }}} */

/* {{{ proto public bool ReflectionMethod::isDestructor() U
   Returns whether this method is static */
ZEND_METHOD(reflection_method, isDestructor)
{
	reflection_object *intern;
	zend_function *mptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(mptr);
	RETURN_BOOL(mptr->common.fn_flags & ZEND_ACC_DTOR);
}
/* }}} */

/* {{{ proto public int ReflectionMethod::getModifiers() U
   Returns a bitfield of the access modifiers for this method */
ZEND_METHOD(reflection_method, getModifiers)
{
	reflection_object *intern;
	zend_function *mptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(mptr);

	RETURN_LONG(mptr->common.fn_flags);
}
/* }}} */

/* {{{ proto public ReflectionClass ReflectionMethod::getDeclaringClass() U
   Get the declaring class */
ZEND_METHOD(reflection_method, getDeclaringClass)
{
	reflection_object *intern;
	zend_function *mptr;

	METHOD_NOTSTATIC(reflection_method_ptr);
	GET_REFLECTION_OBJECT_PTR(mptr);

	zend_reflection_class_factory(mptr->common.scope, return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto public ReflectionClass ReflectionMethod::getPrototype() U
   Get the prototype */
ZEND_METHOD(reflection_method, getPrototype)
{
	reflection_object *intern;
	zend_function *mptr;

	METHOD_NOTSTATIC(reflection_method_ptr);
	GET_REFLECTION_OBJECT_PTR(mptr);
	
	if (!mptr->common.prototype) {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
			"Method %v::%v does not have a prototype", intern->ce->name, mptr->common.function_name);
		return;
	}

	reflection_method_factory(mptr->common.prototype->common.scope, mptr->common.prototype, return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto public static mixed ReflectionClass::export(mixed argument [, bool return]) throws ReflectionException U
   Exports a reflection object. Returns the output if TRUE is specified for return, printing it otherwise. */
ZEND_METHOD(reflection_class, export)
{
	_reflection_export(INTERNAL_FUNCTION_PARAM_PASSTHRU, reflection_class_ptr, 1);
}
/* }}} */

/* {{{ reflection_class_object_ctor */
static void reflection_class_object_ctor(INTERNAL_FUNCTION_PARAMETERS, int is_object)
{
	zval *argument;
	zval *object;
	zval *classname;
	reflection_object *intern;
	zend_class_entry **ce;
	zstr name;
	int name_len;
	zend_uchar name_type;

	if (is_object) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &argument) == FAILURE) {
			return;
		}
	} else {
		if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "o", &argument) == FAILURE) {
			argument = NULL;
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t", &name, &name_len, &name_type) == FAILURE) {
				return;
			}
		}
	}

	object = getThis();
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	if (intern == NULL) {
		return;
	}
	
	if (argument) {
		MAKE_STD_ZVAL(classname);
		ZVAL_UNICODEL(classname, Z_OBJCE_P(argument)->name.u, Z_OBJCE_P(argument)->name_length, 1);
		zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &classname, sizeof(zval *), NULL);
		intern->ptr = Z_OBJCE_P(argument);
		if (is_object) {
			intern->obj = argument;
			zval_add_ref(&argument);
		}
	} else { 
		if (zend_u_lookup_class(name_type, name, name_len, &ce TSRMLS_CC) == FAILURE) {
			if (!EG(exception)) {
				zend_throw_exception_ex(reflection_exception_ptr, -1 TSRMLS_CC, "Class %R does not exist", name_type, name);
			}
			return;
		}

		MAKE_STD_ZVAL(classname);
		ZVAL_UNICODEL(classname, (*ce)->name.u, (*ce)->name_length, 1);
		zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &classname, sizeof(zval *), NULL);

		intern->ptr = *ce;
	}
	intern->ref_type = REF_TYPE_OTHER;
}
/* }}} */

/* {{{ proto public void ReflectionClass::__construct(mixed argument) throws ReflectionException U
   Constructor. Takes a string or an instance as an argument */
ZEND_METHOD(reflection_class, __construct)
{
	reflection_class_object_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto public array ReflectionClass::getStaticProperties() U
   Returns an associative array containing all static property values of the class */
ZEND_METHOD(reflection_class, getStaticProperties)
{
	reflection_object *intern;
	zend_class_entry *ce;
	HashPosition pos;
	zval **value;
	
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);

	zend_update_class_constants(ce TSRMLS_CC);

	array_init(return_value);
	zend_hash_internal_pointer_reset_ex(CE_STATIC_MEMBERS(ce), &pos);

	while (zend_hash_get_current_data_ex(CE_STATIC_MEMBERS(ce), (void **) &value, &pos) == SUCCESS) {
		uint key_len;
		zstr key;
		ulong num_index;

		if (zend_hash_get_current_key_ex(CE_STATIC_MEMBERS(ce), &key, &key_len, &num_index, 0, &pos) != FAILURE) {
			zstr prop_name, class_name;
			int prop_name_len;

			zend_u_unmangle_property_name(IS_UNICODE, key, key_len-1, &class_name, &prop_name);
			prop_name_len = u_strlen(prop_name.u);

			zval_add_ref(value);

			zend_u_hash_update(Z_ARRVAL_P(return_value), IS_UNICODE, prop_name, prop_name_len+1, value, sizeof(zval *), NULL);
		}
		zend_hash_move_forward_ex(CE_STATIC_MEMBERS(ce), &pos);
	}
}
/* }}} */

/* {{{ proto public mixed ReflectionClass::getStaticPropertyValue(string name [, mixed default]) U
   Returns the value of a static property */
ZEND_METHOD(reflection_class, getStaticPropertyValue)
{
	reflection_object *intern;
	zend_class_entry *ce;
	zstr name;
	int name_len;
	zval **prop, *def_value = NULL;
	zend_uchar name_type;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t|z", &name, &name_len, &name_type, &def_value) == FAILURE) {
		return;
	}

	GET_REFLECTION_OBJECT_PTR(ce);

	zend_update_class_constants(ce TSRMLS_CC);
	prop = zend_std_get_static_property(ce, name_type, name, name_len, 1 TSRMLS_CC);
	if (!prop) {
		if (def_value) {
			RETURN_ZVAL(def_value, 1, 0);
		} else {
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
				"Class %v does not have a property named %R", ce->name, name_type, name);
		}
		return;
	} else {
		RETURN_ZVAL(*prop, 1, 0);
	}
}
/* }}} */

/* {{{ proto public void ReflectionClass::setStaticPropertyValue($name, $value) U
   Sets the value of a static property */
ZEND_METHOD(reflection_class, setStaticPropertyValue)
{
	reflection_object *intern;
	zend_class_entry *ce;
	zstr name;
	int name_len;
	zval **variable_ptr, *value;
	int refcount;
	zend_uchar is_ref;
	zend_uchar name_type;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "tz", &name, &name_len, &name_type, &value) == FAILURE) {
		return;
	}

	GET_REFLECTION_OBJECT_PTR(ce);

	zend_update_class_constants(ce TSRMLS_CC);
	variable_ptr = zend_std_get_static_property(ce, name_type, name, name_len, 1 TSRMLS_CC);
	if (!variable_ptr) {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
				"Class %v does not have a property named %R", ce->name, name_type, name);
		return;
	}	
	refcount = Z_REFCOUNT_PP(variable_ptr);
	is_ref = Z_ISREF_PP(variable_ptr);
	zval_dtor(*variable_ptr);
	**variable_ptr = *value;
	zval_copy_ctor(*variable_ptr);
	Z_SET_REFCOUNT_PP(variable_ptr, refcount);
	Z_SET_ISREF_TO_PP(variable_ptr, is_ref);

}
/* }}} */

/* {{{ proto public array ReflectionClass::getDefaultProperties() U
   Returns an associative array containing copies of all default property values of the class */
ZEND_METHOD(reflection_class, getDefaultProperties)
{
	reflection_object *intern;
	zend_class_entry *ce;
	int count, i;
	HashTable *ht_list[3];
	
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);
	array_init(return_value);

	zend_update_class_constants(ce TSRMLS_CC);

	ht_list[0] = CE_STATIC_MEMBERS(ce);
	ht_list[1] = &ce->default_properties;
	ht_list[2] = NULL; /* this should be always the last element */

	for (i = 0; ht_list[i] != NULL; i++) {
		count = zend_hash_num_elements(ht_list[i]);
		if (count > 0) {
			HashPosition pos;
			zval **prop;

			zend_hash_internal_pointer_reset_ex(ht_list[i], &pos);
			while (zend_hash_get_current_data_ex(ht_list[i], (void **) &prop, &pos) == SUCCESS) {
				zstr key, class_name, prop_name;
				uint key_len;
				ulong num_index;
				zval *prop_copy;

				zend_hash_get_current_key_ex(ht_list[i], &key, &key_len, &num_index, 0, &pos);
				zend_hash_move_forward_ex(ht_list[i], &pos);
				zend_u_unmangle_property_name(IS_UNICODE, key, key_len-1, &class_name, &prop_name);
				if (class_name.s && class_name.s[0] != '*'
					&& u_strcmp(class_name.u, ce->name.u))
				{
					/* filter privates from base classes */
					continue;
				}

				/* copy: enforce read only access */
				ALLOC_ZVAL(prop_copy);
				*prop_copy = **prop;
				zval_copy_ctor(prop_copy);
				INIT_PZVAL(prop_copy);

				add_u_assoc_zval(return_value, IS_UNICODE, prop_name, prop_copy);
			}
		}
	}
}
/* }}} */

/* {{{ proto public string ReflectionClass::__toString() U
   Returns a string representation */
ZEND_METHOD(reflection_class, __toString)
{
	reflection_object *intern;
	zend_class_entry *ce;
	string str;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);
	string_init(&str);
	_class_string(&str, ce, intern->obj, "" TSRMLS_CC);
	RETURN_U_STRINGL(ZEND_U_CONVERTER(UG(output_encoding_conv)), str.string, str.len - 1, ZSTR_AUTOFREE);
}
/* }}} */

/* {{{ proto public string ReflectionClass::getName() U
   Returns the class' name */
ZEND_METHOD(reflection_class, getName)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	_default_get_entry(getThis(), "name", sizeof("name"), return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto public bool ReflectionClass::isInternal() U
   Returns whether this class is an internal class */
ZEND_METHOD(reflection_class, isInternal)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);
	RETURN_BOOL(ce->type == ZEND_INTERNAL_CLASS);
}
/* }}} */

/* {{{ proto public bool ReflectionClass::isUserDefined() U
   Returns whether this class is user-defined */
ZEND_METHOD(reflection_class, isUserDefined)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);
	RETURN_BOOL(ce->type == ZEND_USER_CLASS);
}
/* }}} */

/* {{{ proto public string ReflectionClass::getFileName() U
   Returns the filename of the file this class was declared in */
ZEND_METHOD(reflection_class, getFileName)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);
	if (ce->type == ZEND_USER_CLASS) {
		RETURN_RT_STRING(ce->filename, 1);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto public int ReflectionClass::getStartLine() U
   Returns the line this class' declaration starts at */
ZEND_METHOD(reflection_class, getStartLine)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);
	if (ce->type == ZEND_USER_FUNCTION) {
		RETURN_LONG(ce->line_start);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto public int ReflectionClass::getEndLine() U
   Returns the line this class' declaration ends at */
ZEND_METHOD(reflection_class, getEndLine)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);
	if (ce->type == ZEND_USER_CLASS) {
		RETURN_LONG(ce->line_end);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto public string ReflectionClass::getDocComment() U
   Returns the doc comment for this class */
ZEND_METHOD(reflection_class, getDocComment)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);
	if (ce->type == ZEND_USER_CLASS && ce->doc_comment.v) {
		RETURN_ZSTRL(IS_UNICODE, ce->doc_comment, ce->doc_comment_len, 1);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto public ReflectionMethod ReflectionClass::getConstructor() U
   Returns the class' constructor if there is one, NULL otherwise */
ZEND_METHOD(reflection_class, getConstructor)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);

	if (ce->constructor) {
		reflection_method_factory(ce, ce->constructor, return_value TSRMLS_CC);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ proto public bool ReflectionClass::hasMethod(string name) U
   Returns whether a method exists or not */
ZEND_METHOD(reflection_class, hasMethod)
{
	reflection_object *intern;
	zend_class_entry *ce;
	unsigned int lc_name_len;
	zstr name, lc_name; 
	int name_len;
	zend_uchar type;

	METHOD_NOTSTATIC(reflection_class_ptr);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t", &name, &name_len, &type) == FAILURE) {
		return;
	}

	GET_REFLECTION_OBJECT_PTR(ce);
	lc_name = zend_u_str_case_fold(type, name, name_len, 1, &lc_name_len);
	if (zend_u_hash_exists(&ce->function_table, type, lc_name, lc_name_len + 1)) {
		efree(lc_name.v);
		RETURN_TRUE;
	} else {
		efree(lc_name.v);
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto public ReflectionMethod ReflectionClass::getMethod(string name) throws ReflectionException U
   Returns the class' method specified by its name */
ZEND_METHOD(reflection_class, getMethod)
{
	reflection_object *intern;
	zend_class_entry *ce;
	zend_function *mptr;
	unsigned int lc_name_len;
	zstr name, lc_name; 
	int name_len;
	zend_uchar type;

	METHOD_NOTSTATIC(reflection_class_ptr);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t", &name, &name_len, &type) == FAILURE) {
		return;
	}

	GET_REFLECTION_OBJECT_PTR(ce);
	lc_name = zend_u_str_case_fold(type, name, name_len, 1, &lc_name_len);
	if (ce == zend_ce_closure && intern->obj && (lc_name_len == sizeof(ZEND_INVOKE_FUNC_NAME)-1)
		&& ZEND_U_EQUAL(IS_UNICODE, lc_name, lc_name_len, ZEND_INVOKE_FUNC_NAME, sizeof(ZEND_INVOKE_FUNC_NAME)-1)
		&& (mptr = zend_get_closure_invoke_method(intern->obj TSRMLS_CC)) != NULL)
	{
		reflection_method_factory(ce, mptr, return_value TSRMLS_CC);
		efree(lc_name.v);
	} else if (zend_u_hash_find(&ce->function_table, type, lc_name, lc_name_len + 1, (void**) &mptr) == SUCCESS) {
		reflection_method_factory(ce, mptr, return_value TSRMLS_CC);
		efree(lc_name.v);
	} else {
		efree(lc_name.v);
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
				"Method %R does not exist", type, name);
		return;
	}
}
/* }}} */

/* {{{ _addmethod */
static void _addmethod(zend_function *mptr, zend_class_entry *ce, zval *retval, long filter, zval *obj TSRMLS_DC)
{
	zval *method;
	uint len = u_strlen(mptr->common.function_name.u);
	zend_function *closure;

	if (mptr->common.fn_flags & filter) {
		unsigned int lc_name_len;
		zstr lc_name = zend_u_str_case_fold(IS_UNICODE, mptr->common.function_name, len, 1, &lc_name_len);
		ALLOC_ZVAL(method);
		if (ce == zend_ce_closure && obj && (lc_name_len == sizeof(ZEND_INVOKE_FUNC_NAME)-1)
			&& ZEND_U_EQUAL(IS_UNICODE, lc_name, lc_name_len, ZEND_INVOKE_FUNC_NAME, sizeof(ZEND_INVOKE_FUNC_NAME)-1)
			&& (closure = zend_get_closure_invoke_method(obj TSRMLS_CC)) != NULL)
		{
			mptr = closure;
		}
		reflection_method_factory(ce, mptr, method TSRMLS_CC);
		add_next_index_zval(retval, method);
		efree(lc_name.v);
	}
}
/* }}} */

/* {{{ _addmethod */
static int _addmethod_va(zend_function *mptr TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	zend_class_entry *ce = *va_arg(args, zend_class_entry**);
	zval *retval = va_arg(args, zval*);
	long filter = va_arg(args, long);
	zval *obj = va_arg(args, zval *);
	
	_addmethod(mptr, ce, retval, filter, obj TSRMLS_CC);
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ proto public ReflectionMethod[] ReflectionClass::getMethods([long $filter]) U
   Returns an array of this class' methods */
ZEND_METHOD(reflection_class, getMethods)
{
	reflection_object *intern;
	zend_class_entry *ce;
	long filter = 0;
	int argc = ZEND_NUM_ARGS();

	METHOD_NOTSTATIC(reflection_class_ptr);
	if (argc) {
		if (zend_parse_parameters(argc TSRMLS_CC, "|l", &filter) == FAILURE) {
			return;
		}
	} else {
		/* No parameters given, default to "return all" */
		filter = ZEND_ACC_PPP_MASK | ZEND_ACC_ABSTRACT | ZEND_ACC_FINAL | ZEND_ACC_STATIC;
	}

	GET_REFLECTION_OBJECT_PTR(ce);

	array_init(return_value);
	zend_hash_apply_with_arguments(&ce->function_table TSRMLS_CC, (apply_func_args_t) _addmethod_va, 4, &ce, return_value, filter, intern->obj);
	if (intern->obj && instanceof_function(ce, zend_ce_closure TSRMLS_CC)) {
		zend_function *closure = zend_get_closure_invoke_method(intern->obj TSRMLS_CC);
		if (closure) {
			_addmethod(closure, ce, return_value, filter, intern->obj TSRMLS_CC);
			_free_function(closure TSRMLS_CC);
		}
	}
}
/* }}} */

/* {{{ proto public bool ReflectionClass::hasProperty(string name) U
   Returns whether a property exists or not */
ZEND_METHOD(reflection_class, hasProperty)
{
	reflection_object *intern;
	zend_class_entry *ce;
	zstr name; 
	int name_len;
	zend_uchar name_type;
	zval *property;

	METHOD_NOTSTATIC(reflection_class_ptr);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t", &name, &name_len, &name_type) == FAILURE) {
		return;
	}

	GET_REFLECTION_OBJECT_PTR(ce);
	if (zend_u_hash_exists(&ce->properties_info, name_type, name, name_len + 1)) {
		RETURN_TRUE;
	} else {
		if (intern->obj && Z_OBJ_HANDLER_P(intern->obj, has_property))
		{
			MAKE_STD_ZVAL(property);
			if (name_type == IS_STRING) {
				ZVAL_STRINGL(property, name.s, name_len, 1);
			} else {
				ZVAL_UNICODEL(property, name.u, name_len, 1);
			}
			if (Z_OBJ_HANDLER_P(intern->obj, has_property)(intern->obj, property, 0 TSRMLS_CC)) {
				zval_ptr_dtor(&property);
				RETURN_TRUE;
			}
			zval_ptr_dtor(&property);
		}
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto public ReflectionProperty ReflectionClass::getProperty(string name) throws ReflectionException U
   Returns the class' property specified by its name */
ZEND_METHOD(reflection_class, getProperty)
{
	reflection_object *intern;
	zend_class_entry *ce, **pce;
	zend_property_info *property_info;
	zstr name, classname;
	zstr tmp; 
	int name_len, classname_len;
	unsigned int tmp_len;
	zend_uchar name_type;

	METHOD_NOTSTATIC(reflection_class_ptr);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t", &name, &name_len, &name_type) == FAILURE) {
		return;
	}

	GET_REFLECTION_OBJECT_PTR(ce);
	if (zend_u_hash_find(&ce->properties_info, name_type, name, name_len + 1, (void**) &property_info) == SUCCESS) {
		if ((property_info->flags & ZEND_ACC_SHADOW) == 0) {
			reflection_property_factory(ce, property_info, return_value TSRMLS_CC);
			return;
		}
	} else if (intern->obj) {
		/* Check for dynamic properties */
		if (zend_u_hash_exists(Z_OBJ_HT_P(intern->obj)->get_properties(intern->obj TSRMLS_CC), name_type, name, name_len+1)) {
			zend_property_info property_info_tmp;
			property_info_tmp.flags = ZEND_ACC_IMPLICIT_PUBLIC;
			property_info_tmp.name = name;
			property_info_tmp.name_length = name_len;
			property_info_tmp.h = zend_u_get_hash_value(name_type, name, name_len+1);
			property_info_tmp.doc_comment = NULL_ZSTR;
			property_info_tmp.ce = ce;

			reflection_property_factory(ce, &property_info_tmp, return_value TSRMLS_CC);
			return;
		}
	}
	if ((name_type == IS_UNICODE && (tmp.u = u_strstr(name.u, u_doublecolon)) != NULL)
		|| (name_type == IS_STRING  && (tmp.s =   strstr(name.s, "::")) != NULL))
	{
		if (name_type == IS_UNICODE) {
			classname_len = tmp.u - name.u;
		} else {
			classname_len = tmp.s - name.s;
		}
		classname = zend_u_str_case_fold(name_type, name, classname_len, 1, &tmp_len);
		classname_len = tmp_len;
		name_len = name_len - (classname_len + 2);
		if (name_type == IS_UNICODE) {
			name.u = tmp.u + 2;
		} else {
			name.s = tmp.s + 2;
		}

		if (zend_u_lookup_class(name_type, classname, classname_len, &pce TSRMLS_CC) == FAILURE) {
			if (!EG(exception)) {
				zend_throw_exception_ex(reflection_exception_ptr, -1 TSRMLS_CC, "Class %R does not exist", name_type, classname);
			}
			efree(classname.v);
			return;
		}
		efree(classname.v);

		if (!instanceof_function(ce, *pce TSRMLS_CC)) {
			zend_throw_exception_ex(reflection_exception_ptr, -1 TSRMLS_CC, "Fully qualified property name %v::%R does not specify a base class of %v", (*pce)->name, name_type, name, ce->name);
			return;
		}
		ce = *pce;

		if (zend_u_hash_find(&ce->properties_info, name_type, name, name_len + 1, (void**) &property_info) == SUCCESS && (property_info->flags & ZEND_ACC_SHADOW) == 0) {
			reflection_property_factory(ce, property_info, return_value TSRMLS_CC);
			return;
		}		
	}
	zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
			"Property %R does not exist", name_type, name);
}
/* }}} */

/* {{{ _addproperty */
static int _addproperty(zend_property_info *pptr TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	zval *property;
	zend_class_entry *ce = *va_arg(args, zend_class_entry**);
	zval *retval = va_arg(args, zval*);
	long filter = va_arg(args, long);

	if (pptr->flags	& ZEND_ACC_SHADOW) {
		return 0;
	}
	
	if (pptr->flags	& filter) {
		ALLOC_ZVAL(property);
		reflection_property_factory(ce, pptr, property TSRMLS_CC);
		add_next_index_zval(retval, property);
	}
	return 0;
}
/* }}} */

/* {{{ _adddynproperty */
static int _adddynproperty(zval **pptr TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	zval *property;
	zend_class_entry *ce = *va_arg(args, zend_class_entry**);
	zval *retval = va_arg(args, zval*), member;

	if (hash_key->type == IS_UNICODE) {
		if (hash_key->arKey.u[0] == 0) {
			return 0; /* non public cannot be dynamic */
		}
		ZVAL_UNICODEL(&member, hash_key->arKey.u, hash_key->nKeyLength-1, 0);
	} else {
		if (hash_key->arKey.s[0] == '\0') {
			return 0; /* non public cannot be dynamic */
		}
		ZVAL_STRINGL(&member, hash_key->arKey.s, hash_key->nKeyLength-1, 0);
	}
	if (zend_get_property_info(ce, &member, 1 TSRMLS_CC) == &EG(std_property_info)) {
		MAKE_STD_ZVAL(property);
		reflection_property_factory(ce, &EG(std_property_info), property TSRMLS_CC);
		add_next_index_zval(retval, property);
	}
	return 0;
}
/* }}} */

/* {{{ proto public ReflectionProperty[] ReflectionClass::getProperties([long $filter]) U
   Returns an array of this class' properties */
ZEND_METHOD(reflection_class, getProperties)
{
	reflection_object *intern;
	zend_class_entry *ce;
	long filter = 0;
	int argc = ZEND_NUM_ARGS();

	METHOD_NOTSTATIC(reflection_class_ptr);
	if (argc) {
		if (zend_parse_parameters(argc TSRMLS_CC, "|l", &filter) == FAILURE) {
			return;
		}
	} else {
		/* No parameters given, default to "return all" */
		filter = ZEND_ACC_PPP_MASK | ZEND_ACC_STATIC;
	}

	GET_REFLECTION_OBJECT_PTR(ce);

	array_init(return_value);
	zend_hash_apply_with_arguments(&ce->properties_info TSRMLS_CC, (apply_func_args_t) _addproperty, 3, &ce, return_value, filter);

	if (intern->obj && (filter & ZEND_ACC_PUBLIC) != 0 && Z_OBJ_HT_P(intern->obj)->get_properties) {
		HashTable *properties = Z_OBJ_HT_P(intern->obj)->get_properties(intern->obj TSRMLS_CC);
		zend_hash_apply_with_arguments(properties TSRMLS_CC, (apply_func_args_t) _adddynproperty, 2, &ce, return_value);
	}
}
/* }}} */

/* {{{ proto public bool ReflectionClass::hasConstant(string name) U
   Returns whether a constant exists or not */
ZEND_METHOD(reflection_class, hasConstant)
{
	reflection_object *intern;
	zend_class_entry *ce;
	zstr name;
	int name_len;
	zend_uchar name_type;

	METHOD_NOTSTATIC(reflection_class_ptr);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t", &name, &name_len, &name_type) == FAILURE) {
		return;
	}

	GET_REFLECTION_OBJECT_PTR(ce);
	if (zend_u_hash_exists(&ce->constants_table, name_type, name, name_len + 1)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto public array ReflectionClass::getConstants() U
   Returns an associative array containing this class' constants and their values */
ZEND_METHOD(reflection_class, getConstants)
{
	zval *tmp_copy;
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}	
	GET_REFLECTION_OBJECT_PTR(ce);
	array_init(return_value);
	zend_hash_apply_with_argument(&ce->constants_table, (apply_func_arg_t) zval_update_constant, (void*)1 TSRMLS_CC);
	zend_hash_copy(Z_ARRVAL_P(return_value), &ce->constants_table, (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *));
}
/* }}} */

/* {{{ proto public mixed ReflectionClass::getConstant(string name) U
   Returns the class' constant specified by its name */
ZEND_METHOD(reflection_class, getConstant)
{
	reflection_object *intern;
	zend_class_entry *ce;
	zval **value;
	zstr name; 
	int name_len;
	zend_uchar name_type;

	METHOD_NOTSTATIC(reflection_class_ptr);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t", &name, &name_len, &name_type) == FAILURE) {
		return;
	}

	GET_REFLECTION_OBJECT_PTR(ce);
	zend_hash_apply_with_argument(&ce->constants_table, (apply_func_arg_t) zval_update_constant, (void*)1 TSRMLS_CC);
	if (zend_u_hash_find(&ce->constants_table, name_type, name, name_len + 1, (void **) &value) == FAILURE) {
		RETURN_FALSE;
	}
	*return_value = **value;
	zval_copy_ctor(return_value);
	INIT_PZVAL(return_value);
}
/* }}} */

/* {{{ _class_check_flag */
static void _class_check_flag(INTERNAL_FUNCTION_PARAMETERS, int mask)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);
	RETVAL_BOOL(ce->ce_flags & mask);
}
/* }}} */

/* {{{ proto public bool ReflectionClass::isInstantiable() U
   Returns whether this class is instantiable */
ZEND_METHOD(reflection_class, isInstantiable)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);
	if (ce->ce_flags & (ZEND_ACC_INTERFACE | ZEND_ACC_EXPLICIT_ABSTRACT_CLASS | ZEND_ACC_IMPLICIT_ABSTRACT_CLASS)) {
		RETURN_FALSE;
	}

	/* Basically, the class is instantiable. Though, if there is a constructor
	 * and it is not publicly accessible, it isn't! */
	if (!ce->constructor) {
		RETURN_TRUE;
	}

	RETURN_BOOL(ce->constructor->common.fn_flags & ZEND_ACC_PUBLIC);
}
/* }}} */

/* {{{ proto public bool ReflectionClass::isInterface() U
   Returns whether this is an interface or a class */
ZEND_METHOD(reflection_class, isInterface)
{
	_class_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_INTERFACE);
}
/* }}} */

/* {{{ proto public bool ReflectionClass::isFinal() U
   Returns whether this class is final */
ZEND_METHOD(reflection_class, isFinal)
{
	_class_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_FINAL_CLASS);
}
/* }}} */

/* {{{ proto public bool ReflectionClass::isAbstract() U
   Returns whether this class is abstract */
ZEND_METHOD(reflection_class, isAbstract)
{
	_class_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_IMPLICIT_ABSTRACT_CLASS|ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);
}
/* }}} */

/* {{{ proto public int ReflectionClass::getModifiers() U
   Returns a bitfield of the access modifiers for this class */
ZEND_METHOD(reflection_class, getModifiers)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);

	RETURN_LONG(ce->ce_flags);
}
/* }}} */

/* {{{ proto public bool ReflectionClass::isInstance(stdclass object) U
   Returns whether the given object is an instance of this class */
ZEND_METHOD(reflection_class, isInstance)
{
	reflection_object *intern;
	zend_class_entry *ce;
	zval *object;

	METHOD_NOTSTATIC(reflection_class_ptr);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &object) == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);	
	RETURN_BOOL(HAS_CLASS_ENTRY(*object) && instanceof_function(Z_OBJCE_P(object), ce TSRMLS_CC));
}
/* }}} */

/* {{{ proto public stdclass ReflectionClass::newInstance(mixed* args, ...) U
   Returns an instance of this class */
ZEND_METHOD(reflection_class, newInstance)
{
	zval *retval_ptr = NULL;
	reflection_object *intern;
	zend_class_entry *ce;
	
	METHOD_NOTSTATIC(reflection_class_ptr);
	GET_REFLECTION_OBJECT_PTR(ce);

	/* Run the constructor if there is one */
	if (ce->constructor) {
		zval ***params = NULL;
		int num_args = 0;
		zend_fcall_info fci;
		zend_fcall_info_cache fcc;

		if (!(ce->constructor->common.fn_flags & ZEND_ACC_PUBLIC)) {
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, "Access to non-public constructor of class %v", ce->name);
			return;
		}

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "*", &params, &num_args) == FAILURE) {
			if (params) {
				efree(params);
			}
			RETURN_FALSE;
		}

		object_init_ex(return_value, ce);

		fci.size = sizeof(fci);
		fci.function_table = EG(function_table);
		fci.function_name = NULL;
		fci.symbol_table = NULL;
		fci.object_ptr = return_value;
		fci.retval_ptr_ptr = &retval_ptr;
		fci.param_count = num_args;
		fci.params = params;
		fci.no_separation = 1;

		fcc.initialized = 1;
		fcc.function_handler = ce->constructor;
		fcc.calling_scope = EG(scope);
		fcc.called_scope = Z_OBJCE_P(return_value);
		fcc.object_ptr = return_value;

		if (zend_call_function(&fci, &fcc TSRMLS_CC) == FAILURE) {
			efree(params);
			if (retval_ptr) {
				zval_ptr_dtor(&retval_ptr);
			}
			zend_error(E_WARNING, "Invocation of %v's constructor failed", ce->name);
			RETURN_NULL();
		}
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}
		
		if (params) {
			efree(params);
		}
	} else if (!ZEND_NUM_ARGS()) {
		object_init_ex(return_value, ce);
	} else {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, "Class %v does not have a constructor, so you cannot pass any constructor arguments", ce->name);
	}
}
/* }}} */

/* {{{ proto public stdclass ReflectionClass::newInstanceArgs([array args]) U
   Returns an instance of this class */
ZEND_METHOD(reflection_class, newInstanceArgs)
{
	zval *retval_ptr = NULL;
	reflection_object *intern;
	zend_class_entry *ce;
	int argc = 0;
	HashTable *args;
	
	
	METHOD_NOTSTATIC(reflection_class_ptr);
	GET_REFLECTION_OBJECT_PTR(ce);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|h", &args) == FAILURE) {
		return;
	}
	if (ZEND_NUM_ARGS() > 0) {
		argc = args->nNumOfElements;
	}
	
	/* Run the constructor if there is one */
	if (ce->constructor) {
		zval ***params = NULL;
		zend_fcall_info fci;
		zend_fcall_info_cache fcc;

		if (!(ce->constructor->common.fn_flags & ZEND_ACC_PUBLIC)) {
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, "Access to non-public constructor of class %v", ce->name);
			return;
		}
		
		if (argc) {
			params = safe_emalloc(sizeof(zval **), argc, 0);
			zend_hash_apply_with_argument(args, (apply_func_arg_t)_zval_array_to_c_array, &params TSRMLS_CC);	
			params -= argc;
		}

		object_init_ex(return_value, ce);

		fci.size = sizeof(fci);
		fci.function_table = EG(function_table);
		fci.function_name = NULL;
		fci.symbol_table = NULL;
		fci.object_ptr = return_value;
		fci.retval_ptr_ptr = &retval_ptr;
		fci.param_count = argc;
		fci.params = params;
		fci.no_separation = 1;

		fcc.initialized = 1;
		fcc.function_handler = ce->constructor;
		fcc.calling_scope = EG(scope);
		fcc.called_scope = Z_OBJCE_P(return_value);
		fcc.object_ptr = return_value;

		if (zend_call_function(&fci, &fcc TSRMLS_CC) == FAILURE) {
			if (params) {
				efree(params);
			}
			if (retval_ptr) {
				zval_ptr_dtor(&retval_ptr);
			}
			zend_error(E_WARNING, "Invocation of %v's constructor failed", ce->name);
			RETURN_NULL();
		}
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}
		if (params) {
			efree(params);
		}
	} else if (!ZEND_NUM_ARGS()) {
		object_init_ex(return_value, ce);
	} else {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, "Class %v does not have a constructor, so you cannot pass any constructor arguments", ce->name);
	}
}
/* }}} */

/* {{{ proto public ReflectionClass[] ReflectionClass::getInterfaces() U
   Returns an array of interfaces this class implements */
ZEND_METHOD(reflection_class, getInterfaces)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);

	/* Return an empty array if this class implements no interfaces */
	array_init(return_value);

	if (ce->num_interfaces) {
		zend_uint i;

		for (i=0; i < ce->num_interfaces; i++) {
			zval *interface;
			ALLOC_ZVAL(interface);
			zend_reflection_class_factory(ce->interfaces[i], interface TSRMLS_CC);
			add_u_assoc_zval_ex(return_value, IS_UNICODE, ce->interfaces[i]->name, ce->interfaces[i]->name_length + 1, interface);
		}
	}
}
/* }}} */

/* {{{ proto public String[] ReflectionClass::getInterfaceNames() U
   Returns an array of names of interfaces this class implements */
ZEND_METHOD(reflection_class, getInterfaceNames)
{
	reflection_object *intern;
	zend_class_entry *ce;
	zend_uint i;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);

	/* Return an empty array if this class implements no interfaces */
	array_init(return_value);

	for (i=0; i < ce->num_interfaces; i++) {
		add_next_index_unicodel(return_value, ce->interfaces[i]->name.u, ce->interfaces[i]->name_length, 1);
	}
}
/* }}} */

/* {{{ proto public ReflectionClass ReflectionClass::getParentClass() U
   Returns the class' parent class, or, if none exists, FALSE */
ZEND_METHOD(reflection_class, getParentClass)
{
	reflection_object *intern;
	zend_class_entry *ce;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ce);
	
	if (ce->parent) {
		zend_reflection_class_factory(ce->parent, return_value TSRMLS_CC);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto public bool ReflectionClass::isSubclassOf(string|ReflectionClass class) U
   Returns whether this class is a subclass of another class */
ZEND_METHOD(reflection_class, isSubclassOf)
{
	reflection_object *intern, *argument;
	zend_class_entry *ce, **pce, *class_ce;
	zval *class_name;

	METHOD_NOTSTATIC(reflection_class_ptr);
	GET_REFLECTION_OBJECT_PTR(ce);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &class_name) == FAILURE) {
		return;
	}
	
	switch(class_name->type) {
		case IS_STRING:
		case IS_UNICODE:
			if (zend_u_lookup_class(Z_TYPE_P(class_name), Z_UNIVAL_P(class_name), Z_UNILEN_P(class_name), &pce TSRMLS_CC) == FAILURE) {
				zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
						"Class %R does not exist", Z_TYPE_P(class_name), Z_UNIVAL_P(class_name));
				return;
			}
			class_ce = *pce;
			break;			
		case IS_OBJECT:
			if (instanceof_function(Z_OBJCE_P(class_name), reflection_class_ptr TSRMLS_CC)) {
				argument = (reflection_object *) zend_object_store_get_object(class_name TSRMLS_CC);
				if (argument == NULL || argument->ptr == NULL) {
					zend_error(E_ERROR, "Internal error: Failed to retrieve the argument's reflection object");
					/* Bails out */
				}
				class_ce = argument->ptr;
				break;
			}
			/* no break */
		default:
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
					"Parameter one must either be a string or a ReflectionClass object");
			return;
	}

	RETURN_BOOL((ce != class_ce && instanceof_function(ce, class_ce TSRMLS_CC)));
}
/* }}} */

/* {{{ proto public bool ReflectionClass::implementsInterface(string|ReflectionClass interface_name) U
   Returns whether this class is a subclass of another class */
ZEND_METHOD(reflection_class, implementsInterface)
{
	reflection_object *intern, *argument;
	zend_class_entry *ce, *interface_ce, **pce;
	zval *interface;

	METHOD_NOTSTATIC(reflection_class_ptr);
	GET_REFLECTION_OBJECT_PTR(ce);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &interface) == FAILURE) {
		return;
	}
	
	switch(interface->type) {
		case IS_STRING:
		case IS_UNICODE:
			if (zend_u_lookup_class(Z_TYPE_P(interface), Z_UNIVAL_P(interface), Z_UNILEN_P(interface), &pce TSRMLS_CC) == FAILURE) {
				zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
						"Interface %R does not exist", Z_TYPE_P(interface), Z_UNIVAL_P(interface));
				return;
			}
			interface_ce = *pce;
			break;			
		case IS_OBJECT:
			if (instanceof_function(Z_OBJCE_P(interface), reflection_class_ptr TSRMLS_CC)) {
				argument = (reflection_object *) zend_object_store_get_object(interface TSRMLS_CC);
				if (argument == NULL || argument->ptr == NULL) {
					zend_error(E_ERROR, "Internal error: Failed to retrieve the argument's reflection object");
					/* Bails out */
				}
				interface_ce = argument->ptr;
				break;
			}
			/* no break */
		default:
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
					"Parameter one must either be a string or a ReflectionClass object");
			return;
	}

	if (!(interface_ce->ce_flags & ZEND_ACC_INTERFACE)) {
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
				"Interface %v is a Class", interface_ce->name);
		return;
	}
	RETURN_BOOL(instanceof_function(ce, interface_ce TSRMLS_CC));
}
/* }}} */

/* {{{ proto public bool ReflectionClass::isIterateable() U
   Returns whether this class is iterateable (can be used inside foreach) */
ZEND_METHOD(reflection_class, isIterateable)
{
	reflection_object *intern;
	zend_class_entry *ce;

	METHOD_NOTSTATIC(reflection_class_ptr);
	GET_REFLECTION_OBJECT_PTR(ce);

	RETURN_BOOL(ce->get_iterator != NULL);
}
/* }}} */

/* {{{ proto public ReflectionExtension|NULL ReflectionClass::getExtension() U
   Returns NULL or the extension the class belongs to */
ZEND_METHOD(reflection_class, getExtension)
{
	reflection_object *intern;
	zend_class_entry *ce;

	METHOD_NOTSTATIC(reflection_class_ptr);
	GET_REFLECTION_OBJECT_PTR(ce);

	if (ce->module) {
		reflection_extension_factory(return_value, ce->module->name TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto public string|false ReflectionClass::getExtensionName() U
   Returns false or the name of the extension the class belongs to */
ZEND_METHOD(reflection_class, getExtensionName)
{
	reflection_object *intern;
	zend_class_entry *ce;

	METHOD_NOTSTATIC(reflection_class_ptr);
	GET_REFLECTION_OBJECT_PTR(ce);

	if (ce->module) {
		RETURN_ASCII_STRING(ce->module->name, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto public bool ReflectionClass::inNamespace()
   Returns whether this class is defined in namespace */
ZEND_METHOD(reflection_class, inNamespace)
{
	zval **name;
	zstr colon;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	if (zend_hash_find(Z_OBJPROP_P(getThis()), "name", sizeof("name"), (void **) &name) == FAILURE) {
		RETURN_FALSE;
	}
	if (Z_TYPE_PP(name) == IS_STRING
		&& (colon.s = zend_memrchr(Z_STRVAL_PP(name), '\\', Z_STRLEN_PP(name)))
		&& colon.s > Z_STRVAL_PP(name))
	{
		RETURN_TRUE;
	} else if (Z_TYPE_PP(name) == IS_UNICODE
		&& (colon.u = u_memrchr(Z_USTRVAL_PP(name), '\\', Z_USTRLEN_PP(name)))
		&& colon.u > Z_USTRVAL_PP(name))
	{
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto public string ReflectionClass::getNamespaceName()
   Returns the name of namespace where this class is defined */
ZEND_METHOD(reflection_class, getNamespaceName)
{
	zval **name;
	zstr colon;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	if (zend_hash_find(Z_OBJPROP_P(getThis()), "name", sizeof("name"), (void **) &name) == FAILURE) {
		RETURN_FALSE;
	}
	if (Z_TYPE_PP(name) == IS_STRING
		&& (colon.s = zend_memrchr(Z_STRVAL_PP(name), '\\', Z_STRLEN_PP(name)))
		&& colon.s > Z_STRVAL_PP(name))
	{
		RETURN_STRINGL(Z_STRVAL_PP(name), colon.s - Z_STRVAL_PP(name) - 1, 1);
	} else if (Z_TYPE_PP(name) == IS_UNICODE
		&& (colon.u = u_memrchr(Z_USTRVAL_PP(name), '\\', Z_USTRLEN_PP(name)))
		&& colon.u > Z_USTRVAL_PP(name))
	{
		RETURN_UNICODEL(Z_USTRVAL_PP(name), colon.u - Z_USTRVAL_PP(name), 1);
	}
	RETURN_EMPTY_UNICODE();
}
/* }}} */

/* {{{ proto public string ReflectionClass::getShortName()
   Returns the short name of the class (without namespace part) */
ZEND_METHOD(reflection_class, getShortName)
{
	zval **name;
	zstr colon;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	if (zend_hash_find(Z_OBJPROP_P(getThis()), "name", sizeof("name"), (void **) &name) == FAILURE) {
		RETURN_FALSE;
	}
	if (Z_TYPE_PP(name) == IS_STRING
		&& (colon.s = zend_memrchr(Z_STRVAL_PP(name), '\\', Z_STRLEN_PP(name)))
		&& colon.s > Z_STRVAL_PP(name))
	{
		RETURN_STRINGL(colon.s + 1, Z_STRLEN_PP(name) - (colon.s - Z_STRVAL_PP(name) + 1), 1);
	} else if (Z_TYPE_PP(name) == IS_UNICODE
		&& (colon.u = u_memrchr(Z_USTRVAL_PP(name), '\\', Z_USTRLEN_PP(name)))
		&& colon.u > Z_USTRVAL_PP(name))
	{
		RETURN_UNICODEL(colon.u + 1, Z_USTRLEN_PP(name) - (colon.u - Z_USTRVAL_PP(name) + 1), 1);
	}
	RETURN_ZVAL(*name, 1, 0);
}
/* }}} */

/* {{{ proto public static mixed ReflectionObject::export(mixed argument [, bool return]) throws ReflectionException U
   Exports a reflection object. Returns the output if TRUE is specified for return, printing it otherwise. */
ZEND_METHOD(reflection_object, export)
{
	_reflection_export(INTERNAL_FUNCTION_PARAM_PASSTHRU, reflection_object_ptr, 1);
}
/* }}} */

/* {{{ proto public void ReflectionObject::__construct(mixed argument) throws ReflectionException U
   Constructor. Takes an instance as an argument */
ZEND_METHOD(reflection_object, __construct)
{
	reflection_class_object_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto public static mixed ReflectionProperty::export(mixed class, string name [, bool return]) throws ReflectionException U
   Exports a reflection object. Returns the output if TRUE is specified for return, printing it otherwise. */
ZEND_METHOD(reflection_property, export)
{
	_reflection_export(INTERNAL_FUNCTION_PARAM_PASSTHRU, reflection_property_ptr, 2);
}
/* }}} */

/* {{{ proto public void ReflectionProperty::__construct(mixed class, string name) U
   Constructor. Throws an Exception in case the given property does not exist */
ZEND_METHOD(reflection_property, __construct)
{
	zval *propname, *classname;
	zstr name_str, class_name, prop_name;
	int name_len, dynam_prop = 0;
	zval *object;
	reflection_object *intern;
	zend_class_entry **pce;
	zend_class_entry *ce;
	zend_property_info *property_info = NULL;
	property_reference *reference;
	zend_uchar name_type;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zt", &classname, &name_str, &name_len, &name_type) == FAILURE) {
		return;
	}

	object = getThis();
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	if (intern == NULL) {
		return;
	}
	
	/* Find the class entry */
	switch (Z_TYPE_P(classname)) {
		case IS_STRING:
		case IS_UNICODE:
			if (zend_u_lookup_class(Z_TYPE_P(classname), Z_UNIVAL_P(classname), Z_UNILEN_P(classname), &pce TSRMLS_CC) == FAILURE) {
				zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC,
						"Class %R does not exist", Z_TYPE_P(classname), Z_UNIVAL_P(classname)); 
				return;
			}
			ce = *pce;
			break;

		case IS_OBJECT:
			ce = Z_OBJCE_P(classname);
			break;
			
		default:
			_DO_THROW("The parameter class is expected to be either a string or an object");
			/* returns out of this function */
	}

	if (zend_u_hash_find(&ce->properties_info, name_type, name_str, name_len + 1, (void **) &property_info) == FAILURE || (property_info->flags & ZEND_ACC_SHADOW)) {
		/* Check for dynamic properties */
		if (property_info == NULL && Z_TYPE_P(classname) == IS_OBJECT && Z_OBJ_HT_P(classname)->get_properties) {
			if (zend_u_hash_exists(Z_OBJ_HT_P(classname)->get_properties(classname TSRMLS_CC), name_type, name_str, name_len+1)) {
				dynam_prop = 1;
			}
		}
		if (dynam_prop == 0) {
			zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, "Property %v::$%v does not exist", ce->name, name_str);
			return;
		}
	}
	
	if (dynam_prop == 0 && !(property_info->flags & ZEND_ACC_PRIVATE)) {
		/* we have to search the class hierarchy for this (implicit) public or protected property */
		zend_class_entry *tmp_ce = ce;
		zend_property_info *tmp_info;
		
		while (tmp_ce && zend_u_hash_find(&tmp_ce->properties_info, name_type, name_str, name_len + 1, (void **) &tmp_info) != SUCCESS) {
			ce = tmp_ce;
			property_info = tmp_info;
			tmp_ce = tmp_ce->parent;
		}
	}

	MAKE_STD_ZVAL(classname);
	MAKE_STD_ZVAL(propname);

	if (dynam_prop == 0) {
		zend_u_unmangle_property_name(IS_UNICODE, property_info->name, property_info->name_length, &class_name, &prop_name);
		ZVAL_UNICODEL(classname, property_info->ce->name.u, property_info->ce->name_length, 1);
		ZVAL_UNICODE(propname, prop_name.u, 1);
	} else {
		ZVAL_UNICODEL(classname, ce->name.u, ce->name_length, 1);
		ZVAL_UNICODEL(propname, name_str.u, name_len, 1);
	}
	zend_ascii_hash_update(Z_OBJPROP_P(object), "class", sizeof("class"), (void **) &classname, sizeof(zval *), NULL);
	zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &propname, sizeof(zval *), NULL);

	reference = (property_reference*) emalloc(sizeof(property_reference));
	if (dynam_prop) {
		reference->prop.flags = ZEND_ACC_IMPLICIT_PUBLIC;
		reference->prop.name = Z_UNIVAL_P(propname);
		reference->prop.name_length = Z_UNILEN_P(propname);
		reference->prop.h = zend_u_get_hash_value(name_type, name_str, name_len+1);
		reference->prop.doc_comment = NULL_ZSTR;
		reference->prop.ce = ce;
	} else {
		reference->prop = *property_info;
	}
	reference->ce = ce;
	reference->ignore_visibility = 0;
	intern->ptr = reference;
	intern->ref_type = REF_TYPE_PROPERTY;
	intern->ce = ce;
}
/* }}} */

/* {{{ proto public string ReflectionProperty::__toString() U
   Returns a string representation */
ZEND_METHOD(reflection_property, __toString)
{
	reflection_object *intern;
	property_reference *ref;
	string str;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ref);
	string_init(&str);
	_property_string(&str, &ref->prop, NULL_ZSTR, "" TSRMLS_CC);
	RETURN_U_STRINGL(ZEND_U_CONVERTER(UG(output_encoding_conv)), str.string, str.len - 1, ZSTR_AUTOFREE);
}
/* }}} */

/* {{{ proto public string ReflectionProperty::getName() U
   Returns the class' name */
ZEND_METHOD(reflection_property, getName)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	_default_get_entry(getThis(), "name", sizeof("name"), return_value TSRMLS_CC);
}
/* }}} */

static void _property_check_flag(INTERNAL_FUNCTION_PARAMETERS, int mask) /* {{{ */
{
	reflection_object *intern;
	property_reference *ref;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ref);
	RETURN_BOOL(ref->prop.flags & mask);
}
/* }}} */

/* {{{ proto public bool ReflectionProperty::isPublic() U
   Returns whether this property is public */
ZEND_METHOD(reflection_property, isPublic)
{
	_property_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_PUBLIC | ZEND_ACC_IMPLICIT_PUBLIC);
}
/* }}} */

/* {{{ proto public bool ReflectionProperty::isPrivate() U
   Returns whether this property is private */
ZEND_METHOD(reflection_property, isPrivate)
{
	_property_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_PRIVATE);
}
/* }}} */

/* {{{ proto public bool ReflectionProperty::isProtected() U
   Returns whether this property is protected */
ZEND_METHOD(reflection_property, isProtected)
{
	_property_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_PROTECTED);
}
/* }}} */

/* {{{ proto public bool ReflectionProperty::isStatic() U
   Returns whether this property is static */
ZEND_METHOD(reflection_property, isStatic)
{
	_property_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ZEND_ACC_STATIC);
}
/* }}} */

/* {{{ proto public bool ReflectionProperty::isDefault() U
   Returns whether this property is default (declared at compilation time). */
ZEND_METHOD(reflection_property, isDefault)
{
	_property_check_flag(INTERNAL_FUNCTION_PARAM_PASSTHRU, ~ZEND_ACC_IMPLICIT_PUBLIC);
}
/* }}} */

/* {{{ proto public mixed ReflectionProperty::getDefaultValue() U
   Returns the default value of the property. */
ZEND_METHOD(reflection_property, getDefaultValue)
{
	reflection_object *intern;
	property_reference *ref;
	HashTable *prop_defaults;
	zval **zdef, *zv;
	zend_uchar utype = IS_UNICODE;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ref);

	if (ref->prop.flags & ZEND_ACC_STATIC) {
		prop_defaults = &ref->ce->default_static_members;
	} else {
		prop_defaults = &ref->ce->default_properties;
	}

	if (zend_u_hash_quick_find(prop_defaults, utype, ref->prop.name, ref->prop.name_length+1, ref->prop.h, (void**)&zdef) == SUCCESS) {
		ALLOC_ZVAL(zv);
		*zv = **zdef;
		zval_copy_ctor(zv);
		INIT_PZVAL(zv);
		zval_update_constant_ex(&zv, (void*)1, ref->ce TSRMLS_CC);
		RETURN_ZVAL(zv, 1, 1);
	}
}
/* }}} */

/* {{{ proto public int ReflectionProperty::getModifiers() U
   Returns a bitfield of the access modifiers for this property */
ZEND_METHOD(reflection_property, getModifiers)
{
	reflection_object *intern;
	property_reference *ref;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ref);

	RETURN_LONG(ref->prop.flags);
}
/* }}} */

/* {{{ proto public mixed ReflectionProperty::getValue([stdclass object]) U
   Returns this property's value */
ZEND_METHOD(reflection_property, getValue)
{
	reflection_object *intern;
	property_reference *ref;
	zval *object, name;
	zval **member= NULL, *member_p;
	zend_uchar utype = IS_UNICODE;

	METHOD_NOTSTATIC(reflection_property_ptr);
	GET_REFLECTION_OBJECT_PTR(ref);

	if (!(ref->prop.flags & (ZEND_ACC_PUBLIC | ZEND_ACC_IMPLICIT_PUBLIC)) && ref->ignore_visibility == 0) {
		_default_get_entry(getThis(), "name", sizeof("name"), &name TSRMLS_CC);
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
			"Cannot access non-public member %v::%v", intern->ce->name, Z_UNIVAL(name));
		zval_dtor(&name);
		return;
	}

	if ((ref->prop.flags & ZEND_ACC_STATIC)) {
		zend_update_class_constants(intern->ce TSRMLS_CC);
		if (zend_u_hash_quick_find(CE_STATIC_MEMBERS(intern->ce), utype, ref->prop.name, ref->prop.name_length + 1, ref->prop.h, (void **) &member) == FAILURE) {
			zend_error(E_ERROR, "Internal error: Could not find the property %v::%v", intern->ce->name, ref->prop.name);
			/* Bails out */
		}
		*return_value= **member;
		zval_copy_ctor(return_value);
		INIT_PZVAL(return_value);
	} else {
		zstr class_name, prop_name;
		int prop_name_len;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &object) == FAILURE) {
			return;
		}
		zend_u_unmangle_property_name(IS_UNICODE, ref->prop.name, ref->prop.name_length, &class_name, &prop_name);
		prop_name_len = u_strlen(prop_name.u);
		member_p = zend_u_read_property(ref->ce, object, IS_UNICODE, prop_name, prop_name_len, 1 TSRMLS_CC);
		*return_value= *member_p;
		zval_copy_ctor(return_value);
		INIT_PZVAL(return_value);
		if (member_p != EG(uninitialized_zval_ptr)) {
			zval_add_ref(&member_p);
			zval_ptr_dtor(&member_p);
		}
	}
}
/* }}} */

/* {{{ proto public void ReflectionProperty::setValue([stdclass object,] mixed value) U
   Sets this property's value */
ZEND_METHOD(reflection_property, setValue)
{
	reflection_object *intern;
	property_reference *ref;
	zval **variable_ptr;
	zval *object, name;
	zval *value;
	int setter_done = 0;
	zval *tmp;
	HashTable *prop_table;
	zend_uchar utype = IS_UNICODE;

	METHOD_NOTSTATIC(reflection_property_ptr);
	GET_REFLECTION_OBJECT_PTR(ref);

	if (!(ref->prop.flags & ZEND_ACC_PUBLIC) && ref->ignore_visibility == 0) {
		_default_get_entry(getThis(), "name", sizeof("name"), &name TSRMLS_CC);
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
			"Cannot access non-public member %v::%v", intern->ce->name, Z_UNIVAL(name));
		zval_dtor(&name);
		return;
	}

	if ((ref->prop.flags & ZEND_ACC_STATIC)) {
		if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &tmp, &value) == FAILURE) {
				return;
			}
		}
		zend_update_class_constants(intern->ce TSRMLS_CC);
		prop_table = CE_STATIC_MEMBERS(intern->ce);

		if (zend_u_hash_quick_find(prop_table, utype, ref->prop.name, ref->prop.name_length + 1, ref->prop.h, (void **) &variable_ptr) == FAILURE) {
			zend_error(E_ERROR, "Internal error: Could not find the property %v::%v", intern->ce->name, ref->prop.name);
			/* Bails out */
		}
		if (*variable_ptr == value) {
			setter_done = 1;
		} else {
			if (PZVAL_IS_REF(*variable_ptr)) {
				zval_dtor(*variable_ptr);
				(*variable_ptr)->type = value->type;
				(*variable_ptr)->value = value->value;
				if (Z_REFCOUNT_P(value) > 0) {
					zval_copy_ctor(*variable_ptr);
				}
				setter_done = 1;
			}
		}
		if (!setter_done) {
			zval **foo;

			Z_ADDREF_P(value);
			if (PZVAL_IS_REF(value)) {
				SEPARATE_ZVAL(&value);
			}
			zend_u_hash_quick_update(prop_table, utype, ref->prop.name, ref->prop.name_length+1, ref->prop.h, &value, sizeof(zval *), (void **) &foo);
		}
	} else {
		zstr class_name, prop_name;
		int prop_name_len;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "oz", &object, &value) == FAILURE) {
			return;
		}
		zend_u_unmangle_property_name(IS_UNICODE, ref->prop.name, ref->prop.name_length, &class_name, &prop_name);
		prop_name_len = u_strlen(prop_name.u);
		zend_u_update_property(ref->ce, object, IS_UNICODE, prop_name, prop_name_len, value TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto public ReflectionClass ReflectionProperty::getDeclaringClass() U
   Get the declaring class */
ZEND_METHOD(reflection_property, getDeclaringClass)
{
	reflection_object *intern;
	property_reference *ref;
	zend_class_entry *tmp_ce, *ce;
	zend_property_info *tmp_info;
	zstr prop_name, class_name;
	int prop_name_len;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ref);

	if (zend_u_unmangle_property_name(IS_UNICODE, ref->prop.name, ref->prop.name_length, &class_name, &prop_name) != SUCCESS) {
		RETURN_FALSE;
	}

	prop_name_len = USTR_LEN(prop_name);
	ce = tmp_ce = ref->ce;
	while (tmp_ce && zend_u_hash_find(&tmp_ce->properties_info, IS_UNICODE, prop_name, prop_name_len + 1, (void **) &tmp_info) == SUCCESS) {
		if (tmp_info->flags & ZEND_ACC_PRIVATE || tmp_info->flags & ZEND_ACC_SHADOW) {
			/* it's a private property, so it can't be inherited */
			break;
		}
		ce = tmp_ce;
		if (tmp_ce == tmp_info->ce) {
			/* declared in this class, done */
			break;
		}
		tmp_ce = tmp_ce->parent;
	}

	zend_reflection_class_factory(ce, return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto public string ReflectionProperty::getDocComment() U
   Returns the doc comment for this property */
ZEND_METHOD(reflection_property, getDocComment)
{
	reflection_object *intern;
	property_reference *ref;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ref);
	if (ref->prop.doc_comment.v) {
		RETURN_ZSTRL(IS_UNICODE, ref->prop.doc_comment, ref->prop.doc_comment_len, 1);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto public int ReflectionProperty::setAccessible() U
   Sets whether non-public properties can be requested */
ZEND_METHOD(reflection_property, setAccessible)
{
	reflection_object *intern;
	property_reference *ref;
	zend_bool visible;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &visible) == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(ref);
	ref->ignore_visibility = visible;
}
/* }}} */

/* {{{ proto public static mixed ReflectionExtension::export(string name [, bool return]) throws ReflectionException U
   Exports a reflection object. Returns the output if TRUE is specified for return, printing it otherwise. */
ZEND_METHOD(reflection_extension, export)
{
	_reflection_export(INTERNAL_FUNCTION_PARAM_PASSTHRU, reflection_extension_ptr, 1);
}
/* }}} */

/* {{{ proto public void ReflectionExtension::__construct(string name) U
   Constructor. Throws an Exception in case the given extension does not exist */
ZEND_METHOD(reflection_extension, __construct)
{
	zval *name;
	zval *object;
	char *lcname;
	reflection_object *intern;
	zend_module_entry *module;
	char *name_str;
	int name_len;
	ALLOCA_FLAG(use_heap)

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name_str, &name_len) == FAILURE) {
		return;
	}

	object = getThis();
	intern = (reflection_object *) zend_object_store_get_object(object TSRMLS_CC);
	if (intern == NULL) {
		return;
	}
	lcname = do_alloca(name_len + 1, use_heap);
	zend_str_tolower_copy(lcname, name_str, name_len);
	if (zend_hash_find(&module_registry, lcname, name_len + 1, (void **)&module) == FAILURE) {
		free_alloca(lcname, use_heap);
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
			"Extension %s does not exist", name_str);
		return;
	}
	free_alloca(lcname, use_heap);
	MAKE_STD_ZVAL(name);
	ZVAL_ASCII_STRING(name, module->name, 1);
	zend_ascii_hash_update(Z_OBJPROP_P(object), "name", sizeof("name"), (void **) &name, sizeof(zval *), NULL);
	intern->ptr = module;
	intern->ref_type = REF_TYPE_OTHER;
	intern->ce = NULL;
}
/* }}} */

/* {{{ proto public string ReflectionExtension::__toString() U
   Returns a string representation */
ZEND_METHOD(reflection_extension, __toString)
{
	reflection_object *intern;
	zend_module_entry *module;
	string str;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(module);
	string_init(&str);
	_extension_string(&str, module, "" TSRMLS_CC);
	RETURN_U_STRINGL(ZEND_U_CONVERTER(UG(output_encoding_conv)), str.string, str.len - 1, ZSTR_AUTOFREE);
}
/* }}} */

/* {{{ proto public string ReflectionExtension::getName() U
   Returns this extension's name */
ZEND_METHOD(reflection_extension, getName)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	_default_get_entry(getThis(), "name", sizeof("name"), return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto public string ReflectionExtension::getVersion() U
   Returns this extension's version */
ZEND_METHOD(reflection_extension, getVersion)
{
	reflection_object *intern;
	zend_module_entry *module;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(module);

	/* An extension does not necessarily have a version number */
	if (module->version == NO_VERSION_YET) {
		RETURN_NULL();
	} else {
		RETURN_ASCII_STRING(module->version, 1);
	}
}
/* }}} */

/* {{{ proto public ReflectionFunction[] ReflectionExtension::getFunctions() U
   Returns an array of this extension's fuctions */
ZEND_METHOD(reflection_extension, getFunctions)
{
	reflection_object *intern;
	zend_module_entry *module;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(module);

	array_init(return_value);
	if (module->functions) {
		zval *function;
		zend_function *fptr;
		const zend_function_entry *func = module->functions;

		/* Is there a better way of doing this? */
		while (func->fname) {
			if (zend_ascii_hash_find(EG(function_table), func->fname, strlen(func->fname) + 1, (void**) &fptr) == FAILURE) {
				zend_error(E_WARNING, "Internal error: Cannot find extension function %s in global function table", func->fname);
				func++;
				continue;
			}
			
			ALLOC_ZVAL(function);
			reflection_function_factory(fptr, function TSRMLS_CC);
			add_ascii_assoc_zval_ex(return_value, func->fname, strlen(func->fname)+1, function);
			func++;
		}
	}
}
/* }}} */

static int _addconstant(zend_constant *constant TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key) /* {{{ */
{
	zval *const_val;
	zval *retval = va_arg(args, zval*);
	int number = va_arg(args, int);

	if (number == constant->module_number) {
		ALLOC_ZVAL(const_val);
		*const_val = constant->value;
		zval_copy_ctor(const_val);
		INIT_PZVAL(const_val);
		add_u_assoc_zval_ex(retval, IS_UNICODE, constant->name, constant->name_len, const_val);
	}
	return 0;
}
/* }}} */

/* {{{ proto public array ReflectionExtension::getConstants() U
   Returns an associative array containing this extension's constants and their values */
ZEND_METHOD(reflection_extension, getConstants)
{
	reflection_object *intern;
	zend_module_entry *module;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(module);

	array_init(return_value);
	zend_hash_apply_with_arguments(EG(zend_constants) TSRMLS_CC, (apply_func_args_t) _addconstant, 2, return_value, module->module_number);
}
/* }}} */

/* {{{ _addinientry */
static int _addinientry(zend_ini_entry *ini_entry TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	zval *retval = va_arg(args, zval*);
	int number = va_arg(args, int);

	if (number == ini_entry->module_number) {
		if (ini_entry->value) {
			add_ascii_assoc_rt_stringl(retval, ini_entry->name, ini_entry->value, ini_entry->value_length, 1);
		} else {
			add_ascii_assoc_null(retval, ini_entry->name);
		}
	}
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ proto public array ReflectionExtension::getINIEntries() U
   Returns an associative array containing this extension's INI entries and their values */
ZEND_METHOD(reflection_extension, getINIEntries)
{
	reflection_object *intern;
	zend_module_entry *module;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(module);

	array_init(return_value);
	zend_hash_apply_with_arguments(EG(ini_directives) TSRMLS_CC, (apply_func_args_t) _addinientry, 2, return_value, module->module_number);
}
/* }}} */

/* {{{ add_extension_class */
static int add_extension_class(zend_class_entry **pce TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	zval *class_array = va_arg(args, zval*), *zclass;
	struct _zend_module_entry *module = va_arg(args, struct _zend_module_entry*);
	int add_reflection_class = va_arg(args, int);

	if ((*pce)->module && !strcasecmp((*pce)->module->name, module->name)) {
		if (add_reflection_class) {
			ALLOC_ZVAL(zclass);
			zend_reflection_class_factory(*pce, zclass TSRMLS_CC);
			add_u_assoc_zval_ex(class_array, IS_UNICODE, (*pce)->name, (*pce)->name_length + 1, zclass);
		} else {
			add_next_index_unicodel(class_array, (*pce)->name.u, (*pce)->name_length, 1);
		}
	}
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ proto public ReflectionClass[] ReflectionExtension::getClasses() U
   Returns an array containing ReflectionClass objects for all classes of this extension */
ZEND_METHOD(reflection_extension, getClasses)
{
	reflection_object *intern;
	zend_module_entry *module;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(module);

	array_init(return_value);
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) add_extension_class, 3, return_value, module, 1);
}
/* }}} */

/* {{{ proto public array ReflectionExtension::getClassNames() U
   Returns an array containing all names of all classes of this extension */
ZEND_METHOD(reflection_extension, getClassNames)
{
	reflection_object *intern;
	zend_module_entry *module;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(module);

	array_init(return_value);
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) add_extension_class, 3, return_value, module, 0);
}
/* }}} */

/* {{{ proto public array ReflectionExtension::getDependencies() U
   Returns an array containing all names of all extensions this extension depends on */
ZEND_METHOD(reflection_extension, getDependencies)
{
	reflection_object *intern;
	zend_module_entry *module;
	const zend_module_dep *dep;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(module);
	
	array_init(return_value);

	dep = module->deps;
	
	if (!dep)
	{
		return;
	}

	while(dep->name) {
		char *relation;
		char *rel_type;
		int len;
		
		switch(dep->type) {
		case MODULE_DEP_REQUIRED:
			rel_type = "Required";
			break;
		case MODULE_DEP_CONFLICTS:
			rel_type = "Conflicts";
			break;
		case MODULE_DEP_OPTIONAL:
			rel_type = "Optional";
			break;
		default:
			rel_type = "Error"; /* shouldn't happen */
			break;
		}

		len = spprintf(&relation, 0, "%s%s%s%s%s", 
						rel_type,
						dep->rel ? " " : "",
						dep->rel ? dep->rel : "", 
						dep->version ? " " : "",
						dep->version ? dep->version : "");
		add_ascii_assoc_ascii_stringl(return_value, dep->name, relation, len, ZSTR_AUTOFREE);
		dep++;
	}
}
/* }}} */

/* {{{ proto public void ReflectionExtension::info() U
       Prints phpinfo block for the extension */
ZEND_METHOD(reflection_extension, info)
{
	reflection_object *intern;
	zend_module_entry *module;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	GET_REFLECTION_OBJECT_PTR(module);

	php_info_print_module(module TSRMLS_CC);
}
/* }}} */

/* {{{ method tables */
static const zend_function_entry reflection_exception_functions[] = {
	{NULL, NULL, NULL}
};


ZEND_BEGIN_ARG_INFO(arginfo_reflection_getModifierNames, 0)
	ZEND_ARG_INFO(0, modifiers)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_export, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, reflector, Reflector, 0)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

static const zend_function_entry reflection_functions[] = {
	ZEND_ME(reflection, getModifierNames, arginfo_reflection_getModifierNames, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(reflection, export, arginfo_reflection_export, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

static const zend_function_entry reflector_functions[] = {
	ZEND_FENTRY(export, NULL, NULL, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	ZEND_ABSTRACT_ME(reflector, __toString, NULL)
	{NULL, NULL, NULL}
};

ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_function_export, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_function___construct, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_function_invoke, 0)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_function_invokeArgs, 0)
	ZEND_ARG_ARRAY_INFO(0, args, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry reflection_function_abstract_functions[] = {
	ZEND_ME(reflection, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_FINAL)
	PHP_ABSTRACT_ME(reflection_function, __toString, NULL)
	ZEND_ME(reflection_function, inNamespace, NULL, 0)
	ZEND_ME(reflection_function, isClosure, NULL, 0)
	ZEND_ME(reflection_function, isDeprecated, NULL, 0)
	ZEND_ME(reflection_function, isInternal, NULL, 0)
	ZEND_ME(reflection_function, isUserDefined, NULL, 0)
	ZEND_ME(reflection_function, getClosureThis, NULL, 0)
	ZEND_ME(reflection_function, getDocComment, NULL, 0)
	ZEND_ME(reflection_function, getEndLine, NULL, 0)
	ZEND_ME(reflection_function, getExtension, NULL, 0)
	ZEND_ME(reflection_function, getExtensionName, NULL, 0)
	ZEND_ME(reflection_function, getFileName, NULL, 0)
	ZEND_ME(reflection_function, getName, NULL, 0)
	ZEND_ME(reflection_function, getNamespaceName, NULL, 0)
	ZEND_ME(reflection_function, getNumberOfParameters, NULL, 0)
	ZEND_ME(reflection_function, getNumberOfRequiredParameters, NULL, 0)
	ZEND_ME(reflection_function, getParameters, NULL, 0)
	ZEND_ME(reflection_function, getShortName, NULL, 0)
	ZEND_ME(reflection_function, getStartLine, NULL, 0)
	ZEND_ME(reflection_function, getStaticVariables, NULL, 0)
	ZEND_ME(reflection_function, returnsReference, NULL, 0)
	{NULL, NULL, NULL}
};

static const zend_function_entry reflection_function_functions[] = {
	ZEND_ME(reflection_function, __construct, arginfo_reflection_function___construct, 0)
	ZEND_ME(reflection_function, __toString, NULL, 0)
	ZEND_ME(reflection_function, export, arginfo_reflection_function_export, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	ZEND_ME(reflection_function, isDisabled, NULL, 0)
	ZEND_ME(reflection_function, getClosure, NULL, 0)
	ZEND_ME(reflection_function, invoke, arginfo_reflection_function_invoke, 0)
	ZEND_ME(reflection_function, invokeArgs, arginfo_reflection_function_invokeArgs, 0)
	{NULL, NULL, NULL}
};

ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_method_export, 0, 0, 2)
	ZEND_ARG_INFO(0, class)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_method___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, class_or_method)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_method_invoke, 0)
	ZEND_ARG_INFO(0, object)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_method_invokeArgs, 0)
	ZEND_ARG_INFO(0, object)
	ZEND_ARG_ARRAY_INFO(0, args, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry reflection_method_functions[] = {
	ZEND_ME(reflection_method, export, arginfo_reflection_method_export, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	ZEND_ME(reflection_method, __construct, arginfo_reflection_method___construct, 0)
	ZEND_ME(reflection_method, __toString, NULL, 0)
	ZEND_ME(reflection_method, isPublic, NULL, 0)
	ZEND_ME(reflection_method, isPrivate, NULL, 0)
	ZEND_ME(reflection_method, isProtected, NULL, 0)
	ZEND_ME(reflection_method, isAbstract, NULL, 0)
	ZEND_ME(reflection_method, isFinal, NULL, 0)
	ZEND_ME(reflection_method, isStatic, NULL, 0)
	ZEND_ME(reflection_method, isConstructor, NULL, 0)
	ZEND_ME(reflection_method, isDestructor, NULL, 0)
	ZEND_ME(reflection_method, getModifiers, NULL, 0)
	ZEND_ME(reflection_method, getClosure, NULL, 0)
	ZEND_ME(reflection_method, invoke, arginfo_reflection_method_invoke, 0)
	ZEND_ME(reflection_method, invokeArgs, arginfo_reflection_method_invokeArgs, 0)
	ZEND_ME(reflection_method, getDeclaringClass, NULL, 0)
	ZEND_ME(reflection_method, getPrototype, NULL, 0)
	{NULL, NULL, NULL}
};


ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_class_export, 0, 0, 1)
	ZEND_ARG_INFO(0, argument)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class___construct, 0)
	ZEND_ARG_INFO(0, argument)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_class_getStaticPropertyValue, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, default)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class_setStaticPropertyValue, 0)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class_hasMethod, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class_getMethod, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_class_getMethods, 0, 0, 0)
	ZEND_ARG_INFO(0, filter)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class_hasProperty, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class_getProperty, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_class_getProperties, 0, 0, 0)
	ZEND_ARG_INFO(0, filter)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class_hasConstant, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class_getConstant, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class_isInstance, 0)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class_newInstance, 0)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_class_newInstanceArgs, 0, 0, 0)
	ZEND_ARG_ARRAY_INFO(0, args, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class_isSubclassOf, 0)
	ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_class_implementsInterface, 0)
	ZEND_ARG_INFO(0, interface)
ZEND_END_ARG_INFO()

static const zend_function_entry reflection_class_functions[] = {
	ZEND_ME(reflection, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_FINAL)
	ZEND_ME(reflection_class, export, arginfo_reflection_class_export, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	ZEND_ME(reflection_class, __construct, arginfo_reflection_class___construct, 0)
	ZEND_ME(reflection_class, __toString, NULL, 0)
	ZEND_ME(reflection_class, getName, NULL, 0)
	ZEND_ME(reflection_class, isInternal, NULL, 0)
	ZEND_ME(reflection_class, isUserDefined, NULL, 0)
	ZEND_ME(reflection_class, isInstantiable, NULL, 0)
	ZEND_ME(reflection_class, getFileName, NULL, 0)
	ZEND_ME(reflection_class, getStartLine, NULL, 0)
	ZEND_ME(reflection_class, getEndLine, NULL, 0)
	ZEND_ME(reflection_class, getDocComment, NULL, 0)
	ZEND_ME(reflection_class, getConstructor, NULL, 0)
	ZEND_ME(reflection_class, hasMethod, arginfo_reflection_class_hasMethod, 0)
	ZEND_ME(reflection_class, getMethod, arginfo_reflection_class_getMethod, 0)
	ZEND_ME(reflection_class, getMethods, arginfo_reflection_class_getMethods, 0)
	ZEND_ME(reflection_class, hasProperty, arginfo_reflection_class_hasProperty, 0)
	ZEND_ME(reflection_class, getProperty, arginfo_reflection_class_getProperty, 0)
	ZEND_ME(reflection_class, getProperties, arginfo_reflection_class_getProperties, 0)
	ZEND_ME(reflection_class, hasConstant, arginfo_reflection_class_hasConstant, 0)
	ZEND_ME(reflection_class, getConstants, NULL, 0)
	ZEND_ME(reflection_class, getConstant, arginfo_reflection_class_getConstant, 0)
	ZEND_ME(reflection_class, getInterfaces, NULL, 0)
	ZEND_ME(reflection_class, getInterfaceNames, NULL, 0)
	ZEND_ME(reflection_class, isInterface, NULL, 0)
	ZEND_ME(reflection_class, isAbstract, NULL, 0)
	ZEND_ME(reflection_class, isFinal, NULL, 0)
	ZEND_ME(reflection_class, getModifiers, NULL, 0)
	ZEND_ME(reflection_class, isInstance, arginfo_reflection_class_isInstance, 0)
	ZEND_ME(reflection_class, newInstance, arginfo_reflection_class_newInstance, 0)
	ZEND_ME(reflection_class, newInstanceArgs, arginfo_reflection_class_newInstanceArgs, 0)
	ZEND_ME(reflection_class, getParentClass, NULL, 0)
	ZEND_ME(reflection_class, isSubclassOf, arginfo_reflection_class_isSubclassOf, 0)
	ZEND_ME(reflection_class, getStaticProperties, NULL, 0)
	ZEND_ME(reflection_class, getStaticPropertyValue, arginfo_reflection_class_getStaticPropertyValue, 0)
	ZEND_ME(reflection_class, setStaticPropertyValue, arginfo_reflection_class_setStaticPropertyValue, 0)
	ZEND_ME(reflection_class, getDefaultProperties, NULL, 0)
	ZEND_ME(reflection_class, isIterateable, NULL, 0)
	ZEND_ME(reflection_class, implementsInterface, arginfo_reflection_class_implementsInterface, 0)
	ZEND_ME(reflection_class, getExtension, NULL, 0)
	ZEND_ME(reflection_class, getExtensionName, NULL, 0)
	ZEND_ME(reflection_class, inNamespace, NULL, 0)
	ZEND_ME(reflection_class, getNamespaceName, NULL, 0)
	ZEND_ME(reflection_class, getShortName, NULL, 0)
	{NULL, NULL, NULL}
};


ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_object_export, 0, 0, 1)
	ZEND_ARG_INFO(0, argument)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_object___construct, 0)
	ZEND_ARG_INFO(0, argument)
ZEND_END_ARG_INFO()

static const zend_function_entry reflection_object_functions[] = {
	ZEND_ME(reflection_object, export, arginfo_reflection_object_export, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	ZEND_ME(reflection_object, __construct, arginfo_reflection_object___construct, 0)
	{NULL, NULL, NULL}
};


ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_property_export, 0, 0, 1)
	ZEND_ARG_INFO(0, argument)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_property___construct, 0)
	ZEND_ARG_INFO(0, argument)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_property_getValue, 0, 0, 0)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_property_setValue, 0)
	ZEND_ARG_INFO(0, object)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_property_setAccessible, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static const zend_function_entry reflection_property_functions[] = {
	ZEND_ME(reflection, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_FINAL)
	ZEND_ME(reflection_property, export, arginfo_reflection_property_export, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	ZEND_ME(reflection_property, __construct, arginfo_reflection_property___construct, 0)
	ZEND_ME(reflection_property, __toString, NULL, 0)
	ZEND_ME(reflection_property, getName, NULL, 0)
	ZEND_ME(reflection_property, getValue, arginfo_reflection_property_getValue, 0)
	ZEND_ME(reflection_property, setValue, arginfo_reflection_property_setValue, 0)
	ZEND_ME(reflection_property, isPublic, NULL, 0)
	ZEND_ME(reflection_property, isPrivate, NULL, 0)
	ZEND_ME(reflection_property, isProtected, NULL, 0)
	ZEND_ME(reflection_property, isStatic, NULL, 0)
	ZEND_ME(reflection_property, isDefault, NULL, 0)
	ZEND_ME(reflection_property, getModifiers, NULL, 0)
	ZEND_ME(reflection_property, getDefaultValue, NULL, 0)
	ZEND_ME(reflection_property, getDeclaringClass, NULL, 0)
	ZEND_ME(reflection_property, getDocComment, NULL, 0)
	ZEND_ME(reflection_property, setAccessible, arginfo_reflection_property_setAccessible, 0)
	{NULL, NULL, NULL}
};

ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_parameter_export, 0, 0, 2)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, parameter)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_parameter___construct, 0)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, parameter)
ZEND_END_ARG_INFO()

static const zend_function_entry reflection_parameter_functions[] = {
	ZEND_ME(reflection, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_FINAL)
	ZEND_ME(reflection_parameter, export, arginfo_reflection_parameter_export, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	ZEND_ME(reflection_parameter, __construct, arginfo_reflection_parameter___construct, 0)
	ZEND_ME(reflection_parameter, __toString, NULL, 0)
	ZEND_ME(reflection_parameter, getName, NULL, 0)
	ZEND_ME(reflection_parameter, isPassedByReference, NULL, 0)
	ZEND_ME(reflection_parameter, getDeclaringFunction, NULL, 0)
	ZEND_ME(reflection_parameter, getDeclaringClass, NULL, 0)
	ZEND_ME(reflection_parameter, getClass, NULL, 0)
	ZEND_ME(reflection_parameter, isArray, NULL, 0)
	ZEND_ME(reflection_parameter, allowsNull, NULL, 0)
	ZEND_ME(reflection_parameter, getPosition, NULL, 0)
	ZEND_ME(reflection_parameter, isOptional, NULL, 0)
	ZEND_ME(reflection_parameter, isDefaultValueAvailable, NULL, 0)
	ZEND_ME(reflection_parameter, getDefaultValue, NULL, 0)
	{NULL, NULL, NULL}
};

ZEND_BEGIN_ARG_INFO_EX(arginfo_reflection_extension_export, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, return)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_reflection_extension___construct, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static const zend_function_entry reflection_extension_functions[] = {
	ZEND_ME(reflection, __clone, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_FINAL)
	ZEND_ME(reflection_extension, export, arginfo_reflection_extension_export, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
	ZEND_ME(reflection_extension, __construct, arginfo_reflection_extension___construct, 0)
	ZEND_ME(reflection_extension, __toString, NULL, 0)
	ZEND_ME(reflection_extension, getName, NULL, 0)
	ZEND_ME(reflection_extension, getVersion, NULL, 0)
	ZEND_ME(reflection_extension, getFunctions, NULL, 0)
	ZEND_ME(reflection_extension, getConstants, NULL, 0)
	ZEND_ME(reflection_extension, getINIEntries, NULL, 0)
	ZEND_ME(reflection_extension, getClasses, NULL, 0)
	ZEND_ME(reflection_extension, getClassNames, NULL, 0)
	ZEND_ME(reflection_extension, getDependencies, NULL, 0)
	ZEND_ME(reflection_extension, info, NULL, 0)
	{NULL, NULL, NULL}
};
/* }}} */

const zend_function_entry reflection_ext_functions[] = { /* {{{ */
	{NULL, NULL, NULL}
}; /* }}} */

static zend_object_handlers *zend_std_obj_handlers;

/* {{{ _reflection_write_property */
static void _reflection_write_property(zval *object, zval *member, zval *value TSRMLS_DC)
{
	if ((Z_TYPE_P(member) == IS_STRING || Z_TYPE_P(member) == IS_UNICODE)
		&& zend_u_hash_exists(&Z_OBJCE_P(object)->default_properties, Z_TYPE_P(member), Z_UNIVAL_P(member), Z_UNILEN_P(member)+1)
		&& (ZEND_U_EQUAL(Z_TYPE_P(member), Z_UNIVAL_P(member), Z_UNILEN_P(member), "name", sizeof("name")-1)
			|| ZEND_U_EQUAL(Z_TYPE_P(member), Z_UNIVAL_P(member), Z_UNILEN_P(member), "class", sizeof("class")-1)))
	{
		zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
			"Cannot set read-only property %v::$%R", Z_OBJCE_P(object)->name, Z_TYPE_P(member), Z_UNIVAL_P(member));
	}
	else
	{
		zend_std_obj_handlers->write_property(object, member, value TSRMLS_CC);		
	}
}
/* }}} */

PHP_MINIT_FUNCTION(reflection) /* {{{ */
{
	zend_class_entry _reflection_entry;

	zend_std_obj_handlers = zend_get_std_object_handlers();
	memcpy(&reflection_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	reflection_object_handlers.clone_obj = NULL;
	reflection_object_handlers.write_property = _reflection_write_property;

	INIT_CLASS_ENTRY(_reflection_entry, "ReflectionException", reflection_exception_functions);
	reflection_exception_ptr = zend_register_internal_class_ex(&_reflection_entry, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);

	INIT_CLASS_ENTRY(_reflection_entry, "Reflection", reflection_functions);
	reflection_ptr = zend_register_internal_class(&_reflection_entry TSRMLS_CC);

	INIT_CLASS_ENTRY(_reflection_entry, "Reflector", reflector_functions);
	reflector_ptr = zend_register_internal_interface(&_reflection_entry TSRMLS_CC);

	INIT_CLASS_ENTRY(_reflection_entry, "ReflectionFunctionAbstract", reflection_function_abstract_functions);
	_reflection_entry.create_object = reflection_objects_new;
	reflection_function_abstract_ptr = zend_register_internal_class(&_reflection_entry TSRMLS_CC);
	reflection_register_implement(reflection_function_abstract_ptr, reflector_ptr TSRMLS_CC);
	zend_declare_property_string(reflection_function_abstract_ptr, "name", sizeof("name")-1, "", ZEND_ACC_ABSTRACT TSRMLS_CC);

	INIT_CLASS_ENTRY(_reflection_entry, "ReflectionFunction", reflection_function_functions);
	_reflection_entry.create_object = reflection_objects_new;
	reflection_function_ptr = zend_register_internal_class_ex(&_reflection_entry, reflection_function_abstract_ptr, NULL TSRMLS_CC);
	zend_declare_property_string(reflection_function_ptr, "name", sizeof("name")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);

	REGISTER_REFLECTION_CLASS_CONST_LONG(function, "IS_DEPRECATED", ZEND_ACC_DEPRECATED);

	INIT_CLASS_ENTRY(_reflection_entry, "ReflectionParameter", reflection_parameter_functions);
	_reflection_entry.create_object = reflection_objects_new;
	reflection_parameter_ptr = zend_register_internal_class(&_reflection_entry TSRMLS_CC);
	reflection_register_implement(reflection_parameter_ptr, reflector_ptr TSRMLS_CC);
	zend_declare_property_string(reflection_parameter_ptr, "name", sizeof("name")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);

	INIT_CLASS_ENTRY(_reflection_entry, "ReflectionMethod", reflection_method_functions);
	_reflection_entry.create_object = reflection_objects_new;
	reflection_method_ptr = zend_register_internal_class_ex(&_reflection_entry, reflection_function_abstract_ptr, NULL TSRMLS_CC);
	zend_declare_property_string(reflection_method_ptr, "name", sizeof("name")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(reflection_method_ptr, "class", sizeof("class")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);

	REGISTER_REFLECTION_CLASS_CONST_LONG(method, "IS_STATIC", ZEND_ACC_STATIC);
	REGISTER_REFLECTION_CLASS_CONST_LONG(method, "IS_PUBLIC", ZEND_ACC_PUBLIC);
	REGISTER_REFLECTION_CLASS_CONST_LONG(method, "IS_PROTECTED", ZEND_ACC_PROTECTED);
	REGISTER_REFLECTION_CLASS_CONST_LONG(method, "IS_PRIVATE", ZEND_ACC_PRIVATE);
	REGISTER_REFLECTION_CLASS_CONST_LONG(method, "IS_ABSTRACT", ZEND_ACC_ABSTRACT);
	REGISTER_REFLECTION_CLASS_CONST_LONG(method, "IS_FINAL", ZEND_ACC_FINAL);

	INIT_CLASS_ENTRY(_reflection_entry, "ReflectionClass", reflection_class_functions);
	_reflection_entry.create_object = reflection_objects_new;
	reflection_class_ptr = zend_register_internal_class(&_reflection_entry TSRMLS_CC);
	reflection_register_implement(reflection_class_ptr, reflector_ptr TSRMLS_CC);
	zend_declare_property_string(reflection_class_ptr, "name", sizeof("name")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);

	REGISTER_REFLECTION_CLASS_CONST_LONG(class, "IS_IMPLICIT_ABSTRACT", ZEND_ACC_IMPLICIT_ABSTRACT_CLASS);
	REGISTER_REFLECTION_CLASS_CONST_LONG(class, "IS_EXPLICIT_ABSTRACT", ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);
	REGISTER_REFLECTION_CLASS_CONST_LONG(class, "IS_FINAL", ZEND_ACC_FINAL_CLASS);

	INIT_CLASS_ENTRY(_reflection_entry, "ReflectionObject", reflection_object_functions);
	_reflection_entry.create_object = reflection_objects_new;
	reflection_object_ptr = zend_register_internal_class_ex(&_reflection_entry, reflection_class_ptr, NULL TSRMLS_CC);

	INIT_CLASS_ENTRY(_reflection_entry, "ReflectionProperty", reflection_property_functions);
	_reflection_entry.create_object = reflection_objects_new;
	reflection_property_ptr = zend_register_internal_class(&_reflection_entry TSRMLS_CC);
	reflection_register_implement(reflection_property_ptr, reflector_ptr TSRMLS_CC);
	zend_declare_property_string(reflection_property_ptr, "name", sizeof("name")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(reflection_property_ptr, "class", sizeof("class")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);

	REGISTER_REFLECTION_CLASS_CONST_LONG(property, "IS_STATIC", ZEND_ACC_STATIC);
	REGISTER_REFLECTION_CLASS_CONST_LONG(property, "IS_PUBLIC", ZEND_ACC_PUBLIC);
	REGISTER_REFLECTION_CLASS_CONST_LONG(property, "IS_PROTECTED", ZEND_ACC_PROTECTED);
	REGISTER_REFLECTION_CLASS_CONST_LONG(property, "IS_PRIVATE", ZEND_ACC_PRIVATE);

	INIT_CLASS_ENTRY(_reflection_entry, "ReflectionExtension", reflection_extension_functions);
	_reflection_entry.create_object = reflection_objects_new;
	reflection_extension_ptr = zend_register_internal_class(&_reflection_entry TSRMLS_CC);
	reflection_register_implement(reflection_extension_ptr, reflector_ptr TSRMLS_CC);
	zend_declare_property_string(reflection_extension_ptr, "name", sizeof("name")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);

	return SUCCESS;
} /* }}} */

PHP_MINFO_FUNCTION(reflection) /* {{{ */
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Reflection", "enabled");

	php_info_print_table_row(2, "Version", "$Revision: 1.345 $");

	php_info_print_table_end();
} /* }}} */

zend_module_entry reflection_module_entry = { /* {{{ */
	STANDARD_MODULE_HEADER,
	"Reflection",
	reflection_ext_functions,
	PHP_MINIT(reflection),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(reflection),
	"$Revision: 1.345 $",
	STANDARD_MODULE_PROPERTIES
}; /* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 */
