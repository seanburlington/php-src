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
   | Authors: Marcus Boerger <helly@php.net>                              |
   +----------------------------------------------------------------------+
 */

/* $Id: spl_array.c,v 1.166 2009/05/25 14:32:14 felipe Exp $ */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"
#include "zend_interfaces.h"
#include "zend_API.h"
#include "zend_exceptions.h"

#include "php_spl.h"
#include "spl_functions.h"
#include "spl_engine.h"
#include "spl_iterators.h"
#include "spl_array.h"
#include "spl_exceptions.h"

zend_object_handlers spl_handler_ArrayObject;
PHPAPI zend_class_entry  *spl_ce_ArrayObject;

zend_object_handlers spl_handler_ArrayIterator;
PHPAPI zend_class_entry  *spl_ce_ArrayIterator;
PHPAPI zend_class_entry  *spl_ce_RecursiveArrayIterator;

#define SPL_ARRAY_STD_PROP_LIST      0x00000001
#define SPL_ARRAY_ARRAY_AS_PROPS     0x00000002
#define SPL_ARRAY_CHILD_ARRAYS_ONLY  0x00000004
#define SPL_ARRAY_OVERLOADED_REWIND  0x00010000
#define SPL_ARRAY_OVERLOADED_VALID   0x00020000
#define SPL_ARRAY_OVERLOADED_KEY     0x00040000
#define SPL_ARRAY_OVERLOADED_CURRENT 0x00080000
#define SPL_ARRAY_OVERLOADED_NEXT    0x00100000
#define SPL_ARRAY_IS_REF             0x01000000
#define SPL_ARRAY_IS_SELF            0x02000000
#define SPL_ARRAY_USE_OTHER          0x04000000
#define SPL_ARRAY_INT_MASK           0xFFFF0000
#define SPL_ARRAY_CLONE_MASK         0x0300FFFF

typedef struct _spl_array_object {
	zend_object            std;
	zval                   *array;
	zval                   *retval;
	HashPosition           pos;
	ulong                  pos_h;
	int                    ar_flags;
	int                    is_self;
	zend_function          *fptr_offset_get;
	zend_function          *fptr_offset_set;
	zend_function          *fptr_offset_has;
	zend_function          *fptr_offset_del;
	zend_function          *fptr_count;
	zend_function          *fptr_serialize;
	zend_function          *fptr_unserialize;
	zend_class_entry       *ce_get_iterator;
	php_serialize_data_t   *serialize_data;
	php_unserialize_data_t *unserialize_data;
} spl_array_object;

static inline HashTable *spl_array_get_hash_table(spl_array_object* intern, int check_std_props TSRMLS_DC) { /* {{{ */
	if ((intern->ar_flags & SPL_ARRAY_IS_SELF) != 0) {
		return intern->std.properties;
	} else if ((intern->ar_flags & SPL_ARRAY_USE_OTHER) && (check_std_props == 0 || (intern->ar_flags & SPL_ARRAY_STD_PROP_LIST) == 0) && Z_TYPE_P(intern->array) == IS_OBJECT) {
		spl_array_object *other  = (spl_array_object*)zend_object_store_get_object(intern->array TSRMLS_CC);
		return spl_array_get_hash_table(other, check_std_props TSRMLS_CC);
	} else if ((intern->ar_flags & ((check_std_props ? SPL_ARRAY_STD_PROP_LIST : 0) | SPL_ARRAY_IS_SELF)) != 0) {
		return intern->std.properties;
	} else {
		return HASH_OF(intern->array);
	}
} /* }}} */

static void spl_array_rewind(spl_array_object *intern TSRMLS_DC);

static void spl_array_update_pos(spl_array_object* intern) /* {{{ */
{
	Bucket *pos = intern->pos;
	if (pos != NULL) {
		intern->pos_h = pos->h;
	}
} /* }}} */

static void spl_array_set_pos(spl_array_object* intern, HashPosition pos) /* {{{ */
{
	intern->pos = pos;
	spl_array_update_pos(intern);
} /* }}} */

SPL_API int spl_hash_verify_pos_ex(spl_array_object * intern, HashTable * ht TSRMLS_DC) /* {{{ */
{
	Bucket *p;

/*	IS_CONSISTENT(ht);*/

/*	HASH_PROTECT_RECURSION(ht);*/
	p = ht->arBuckets[intern->pos_h & ht->nTableMask];
	while (p != NULL) {
		if (p == intern->pos) {
			return SUCCESS;
		}
		p = p->pNext;
	}
/*	HASH_UNPROTECT_RECURSION(ht); */
	spl_array_rewind(intern TSRMLS_CC);
	return FAILURE;

} /* }}} */

SPL_API int spl_hash_verify_pos(spl_array_object * intern TSRMLS_DC) /* {{{ */
{
	HashTable *ht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);
	return spl_hash_verify_pos_ex(intern, ht TSRMLS_CC);
}
/* }}} */

/* {{{ spl_array_object_free_storage */
static void spl_array_object_free_storage(void *object TSRMLS_DC)
{
	spl_array_object *intern = (spl_array_object *)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	zval_ptr_dtor(&intern->array);
	zval_ptr_dtor(&intern->retval);

	efree(object);
}
/* }}} */

zend_object_iterator *spl_array_get_iterator(zend_class_entry *ce, zval *object, int by_ref TSRMLS_DC);
int spl_array_serialize(zval *object, int *type, zstr *buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC);
int spl_array_unserialize(zval **object, zend_class_entry *ce, int type, const zstr buf, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC);

/* {{{ spl_array_object_new_ex */
static zend_object_value spl_array_object_new_ex(zend_class_entry *class_type, spl_array_object **obj, zval *orig, int clone_orig TSRMLS_DC)
{
	zend_object_value retval;
	spl_array_object *intern;
	zval *tmp;
	zend_class_entry * parent = class_type;
	int inherited = 0;

	intern = emalloc(sizeof(spl_array_object));
	memset(intern, 0, sizeof(spl_array_object));
	*obj = intern;
	ALLOC_INIT_ZVAL(intern->retval);

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	intern->ar_flags = 0;
	intern->serialize_data   = NULL;
	intern->unserialize_data = NULL;
	intern->ce_get_iterator = spl_ce_ArrayIterator;
	if (orig) {
		spl_array_object *other = (spl_array_object*)zend_object_store_get_object(orig TSRMLS_CC);

		intern->ar_flags &= ~ SPL_ARRAY_CLONE_MASK;
		intern->ar_flags |= (other->ar_flags & SPL_ARRAY_CLONE_MASK);
		intern->ce_get_iterator = other->ce_get_iterator;
		if (clone_orig) {
			intern->array = other->array;
			if (Z_OBJ_HT_P(orig) == &spl_handler_ArrayObject) {
				MAKE_STD_ZVAL(intern->array);
				array_init(intern->array);
				zend_hash_copy(HASH_OF(intern->array), HASH_OF(other->array), (copy_ctor_func_t) zval_add_ref, &tmp, sizeof(zval*));
			}
			if (Z_OBJ_HT_P(orig) == &spl_handler_ArrayIterator) {
				Z_ADDREF_P(other->array);
			}
		} else {
			intern->array = orig;
			Z_ADDREF_P(intern->array);
			intern->ar_flags |= SPL_ARRAY_IS_REF | SPL_ARRAY_USE_OTHER;
		}
	} else {
		MAKE_STD_ZVAL(intern->array);
		array_init(intern->array);
		intern->ar_flags &= ~SPL_ARRAY_IS_REF;
	}

	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) spl_array_object_free_storage, NULL TSRMLS_CC);
	while (parent) {
		if (parent == spl_ce_ArrayIterator || parent == spl_ce_RecursiveArrayIterator) {
			retval.handlers = &spl_handler_ArrayIterator;
			class_type->get_iterator = spl_array_get_iterator;
			break;
		} else if (parent == spl_ce_ArrayObject) {
			retval.handlers = &spl_handler_ArrayObject;
			break;
		}
		parent = parent->parent;
		inherited = 1;
	}
	if (!parent) { /* this must never happen */
		php_error_docref(NULL TSRMLS_CC, E_COMPILE_ERROR, "Internal compiler error, Class is not child of ArrayObject or ArrayIterator");
	}
	if (inherited) {
		zend_hash_find(&class_type->function_table, "offsetget",    sizeof("offsetget"),    (void **) &intern->fptr_offset_get);
		if (intern->fptr_offset_get->common.scope == parent) {
			intern->fptr_offset_get = NULL;
		}
		zend_hash_find(&class_type->function_table, "offsetset",    sizeof("offsetset"),    (void **) &intern->fptr_offset_set);
		if (intern->fptr_offset_set->common.scope == parent) {
			intern->fptr_offset_set = NULL;
		}
		zend_hash_find(&class_type->function_table, "offsetexists", sizeof("offsetexists"), (void **) &intern->fptr_offset_has);
		if (intern->fptr_offset_has->common.scope == parent) {
			intern->fptr_offset_has = NULL;
		}
		zend_hash_find(&class_type->function_table, "offsetunset",  sizeof("offsetunset"),  (void **) &intern->fptr_offset_del);
		if (intern->fptr_offset_del->common.scope == parent) {
			intern->fptr_offset_del = NULL;
		}
		zend_hash_find(&class_type->function_table, "count",        sizeof("count"),        (void **) &intern->fptr_count);
		if (intern->fptr_count->common.scope == parent) {
			intern->fptr_count = NULL;
		}
		zend_hash_find(&class_type->function_table, "serialize",    sizeof("serialize"),     (void **) &intern->fptr_serialize);
		if (intern->fptr_serialize->common.scope == parent) {
			intern->fptr_serialize = NULL;
		}
		zend_hash_find(&class_type->function_table, "unserialize",  sizeof("unserialize"),   (void **) &intern->fptr_unserialize);
		if (intern->fptr_unserialize->common.scope == parent) {
			intern->fptr_unserialize = NULL;
		}
	}
	/* Cache iterator functions if ArrayIterator or derived. Check current's */
	/* cache since only current is always required */
	if (retval.handlers == &spl_handler_ArrayIterator) {
		if (!class_type->iterator_funcs.zf_current) {
			zend_hash_find(&class_type->function_table, "rewind",  sizeof("rewind"),  (void **) &class_type->iterator_funcs.zf_rewind);
			zend_hash_find(&class_type->function_table, "valid",   sizeof("valid"),   (void **) &class_type->iterator_funcs.zf_valid);
			zend_hash_find(&class_type->function_table, "key",     sizeof("key"),     (void **) &class_type->iterator_funcs.zf_key);
			zend_hash_find(&class_type->function_table, "current", sizeof("current"), (void **) &class_type->iterator_funcs.zf_current);
			zend_hash_find(&class_type->function_table, "next",    sizeof("next"),    (void **) &class_type->iterator_funcs.zf_next);
		}
		if (inherited) {
			if (class_type->iterator_funcs.zf_rewind->common.scope  != parent) intern->ar_flags |= SPL_ARRAY_OVERLOADED_REWIND;
			if (class_type->iterator_funcs.zf_valid->common.scope   != parent) intern->ar_flags |= SPL_ARRAY_OVERLOADED_VALID;
			if (class_type->iterator_funcs.zf_key->common.scope     != parent) intern->ar_flags |= SPL_ARRAY_OVERLOADED_KEY;
			if (class_type->iterator_funcs.zf_current->common.scope != parent) intern->ar_flags |= SPL_ARRAY_OVERLOADED_CURRENT;
			if (class_type->iterator_funcs.zf_next->common.scope    != parent) intern->ar_flags |= SPL_ARRAY_OVERLOADED_NEXT;
		}
	}

	spl_array_rewind(intern TSRMLS_CC);
	return retval;
}
/* }}} */

/* {{{ spl_array_object_new */
static zend_object_value spl_array_object_new(zend_class_entry *class_type TSRMLS_DC)
{
	spl_array_object *tmp;
	return spl_array_object_new_ex(class_type, &tmp, NULL, 0 TSRMLS_CC);
}
/* }}} */

/* {{{ spl_array_object_clone */
static zend_object_value spl_array_object_clone(zval *zobject TSRMLS_DC)
{
	zend_object_value new_obj_val;
	zend_object *old_object;
	zend_object *new_object;
	zend_object_handle handle = Z_OBJ_HANDLE_P(zobject);
	spl_array_object *intern;

	old_object = zend_objects_get_address(zobject TSRMLS_CC);
	new_obj_val = spl_array_object_new_ex(old_object->ce, &intern, zobject, 1 TSRMLS_CC);
	new_object = &intern->std;

	zend_objects_clone_members(new_object, new_obj_val, old_object, handle TSRMLS_CC);

	return new_obj_val;
}
/* }}} */

static zval **spl_array_get_dimension_ptr_ptr(int check_inherited, zval *object, zval *offset, int type TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	zval **retval;
	long index;
	HashTable *ht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

/*  We cannot get the pointer pointer so we don't allow it here for now
	if (check_inherited && intern->fptr_offset_get) {
		return zend_call_method_with_1_params(&object, intern->std.ce, &intern->fptr_offset_get, "offsetGet", NULL, offset);
	}*/

	if (!offset) {
		return &EG(uninitialized_zval_ptr);
	}
	
	switch(Z_TYPE_P(offset)) {
	case IS_STRING:
	case IS_UNICODE:
		if (zend_u_symtable_find(ht, Z_TYPE_P(offset), Z_UNIVAL_P(offset), Z_UNILEN_P(offset)+1, (void **) &retval) == FAILURE) {
			if (type == BP_VAR_W || type == BP_VAR_RW) {
				zval *value;
				ALLOC_INIT_ZVAL(value);
				zend_u_symtable_update(ht, Z_TYPE_P(offset), Z_UNIVAL_P(offset), Z_UNILEN_P(offset)+1, (void**)&value, sizeof(void*), NULL);
				zend_u_symtable_find(ht, Z_TYPE_P(offset), Z_UNIVAL_P(offset), Z_UNILEN_P(offset)+1, (void **) &retval);
				return retval;
			} else {
				zend_error(E_NOTICE, "Undefined index:  %R", Z_TYPE_P(offset), Z_STRVAL_P(offset));
				return &EG(uninitialized_zval_ptr);
			}
		} else {
			return retval;
		}
	case IS_DOUBLE:
	case IS_RESOURCE:
	case IS_BOOL: 
	case IS_LONG: 
		if (offset->type == IS_DOUBLE) {
			index = (long)Z_DVAL_P(offset);
		} else {
			index = Z_LVAL_P(offset);
		}
		if (zend_hash_index_find(ht, index, (void **) &retval) == FAILURE) {
			if (type == BP_VAR_W || type == BP_VAR_RW) {
				zval *value;
				ALLOC_INIT_ZVAL(value);
				zend_hash_index_update(ht, index, (void**)&value, sizeof(void*), NULL);
				zend_hash_index_find(ht, index, (void **) &retval);
				return retval;
			} else {
				zend_error(E_NOTICE, "Undefined offset:  %ld", index);
				return &EG(uninitialized_zval_ptr);
			}
		} else {
			return retval;
		}
		break;
	default:
		zend_error(E_WARNING, "Illegal offset type");
		return &EG(uninitialized_zval_ptr);
	}
} /* }}} */

static zval *spl_array_read_dimension_ex(int check_inherited, zval *object, zval *offset, int type TSRMLS_DC) /* {{{ */
{
	zval **ret;

	if (check_inherited) {
		spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
		if (intern->fptr_offset_get) {
			zval *rv;
			SEPARATE_ARG_IF_REF(offset);
			zend_call_method_with_1_params(&object, intern->std.ce, &intern->fptr_offset_get, "offsetGet", &rv, offset);	
			zval_ptr_dtor(&offset);
			if (rv) {
				zval_ptr_dtor(&intern->retval);
				MAKE_STD_ZVAL(intern->retval);
				ZVAL_ZVAL(intern->retval, rv, 1, 1);
				return intern->retval;
			}
			return EG(uninitialized_zval_ptr);
		}
	}
	ret = spl_array_get_dimension_ptr_ptr(check_inherited, object, offset, type TSRMLS_CC);

	/* When in a write context,
	 * ZE has to be fooled into thinking this is in a reference set
	 * by separating (if necessary) and returning as an is_ref=1 zval (even if refcount == 1) */
	if ((type == BP_VAR_W || type == BP_VAR_RW) && !Z_ISREF_PP(ret)) {
		if (Z_REFCOUNT_PP(ret) > 1) {
			zval *newval;

			/* Separate */
			MAKE_STD_ZVAL(newval);
			*newval = **ret;
			zval_copy_ctor(newval);
			Z_SET_REFCOUNT_P(newval, 1);

			/* Replace */
			Z_DELREF_PP(ret);
			*ret = newval;
		}

		Z_SET_ISREF_PP(ret);
	}

	return *ret;
} /* }}} */

static zval *spl_array_read_dimension(zval *object, zval *offset, int type TSRMLS_DC) /* {{{ */
{
	return spl_array_read_dimension_ex(1, object, offset, type TSRMLS_CC);
} /* }}} */

static void spl_array_write_dimension_ex(int check_inherited, zval *object, zval *offset, zval *value TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	long index;

	if (check_inherited && intern->fptr_offset_set) {
		if (!offset) {
			ALLOC_INIT_ZVAL(offset);
		} else {
			SEPARATE_ARG_IF_REF(offset);
		}
		zend_call_method_with_2_params(&object, intern->std.ce, &intern->fptr_offset_set, "offsetSet", NULL, offset, value);
		zval_ptr_dtor(&offset);
		return;
	}
	
	if (!offset) {
		Z_ADDREF_P(value);
		zend_hash_next_index_insert(spl_array_get_hash_table(intern, 0 TSRMLS_CC), (void**)&value, sizeof(void*), NULL);
		return;
	}
	switch(Z_TYPE_P(offset)) {
	case IS_STRING:
	case IS_UNICODE:
		Z_ADDREF_P(value);
		zend_u_symtable_update(spl_array_get_hash_table(intern, 0 TSRMLS_CC), Z_TYPE_P(offset), Z_UNIVAL_P(offset), Z_UNILEN_P(offset)+1, (void**)&value, sizeof(void*), NULL);
		return;
	case IS_DOUBLE:
	case IS_RESOURCE:
	case IS_BOOL: 
	case IS_LONG: 
		if (offset->type == IS_DOUBLE) {
			index = (long)Z_DVAL_P(offset);
		} else {
			index = Z_LVAL_P(offset);
		}
		Z_ADDREF_P(value);
		zend_hash_index_update(spl_array_get_hash_table(intern, 0 TSRMLS_CC), index, (void**)&value, sizeof(void*), NULL);
		return;
	case IS_NULL:
		Z_ADDREF_P(value);
		zend_hash_next_index_insert(spl_array_get_hash_table(intern, 0 TSRMLS_CC), (void**)&value, sizeof(void*), NULL);
		return;
	default:
		zend_error(E_WARNING, "Illegal offset type");
		return;
	}
} /* }}} */

static void spl_array_write_dimension(zval *object, zval *offset, zval *value TSRMLS_DC) /* {{{ */
{
	spl_array_write_dimension_ex(1, object, offset, value TSRMLS_CC);
} /* }}} */

static void spl_array_unset_dimension_ex(int check_inherited, zval *object, zval *offset TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	long index;

	if (check_inherited && intern->fptr_offset_del) {
		SEPARATE_ARG_IF_REF(offset);
		zend_call_method_with_1_params(&object, intern->std.ce, &intern->fptr_offset_del, "offsetUnset", NULL, offset);
		zval_ptr_dtor(&offset);
		return;
	}

	switch(Z_TYPE_P(offset)) {
	case IS_STRING:
	case IS_UNICODE:
		if (spl_array_get_hash_table(intern, 0 TSRMLS_CC) == &EG(symbol_table)) {
			if (zend_u_delete_global_variable(Z_TYPE_P(offset), Z_UNIVAL_P(offset), Z_UNILEN_P(offset) TSRMLS_CC)) {
				zend_error(E_NOTICE,"Undefined index:  %s", Z_STRVAL_P(offset));
			}
		} else {
			if (zend_u_symtable_del(spl_array_get_hash_table(intern, 0 TSRMLS_CC), Z_TYPE_P(offset), Z_UNIVAL_P(offset), Z_UNILEN_P(offset)+1) == FAILURE) {
				zend_error(E_NOTICE,"Undefined index:  %R", Z_TYPE_P(offset), Z_UNIVAL_P(offset));
			}
		}
		break;
	case IS_DOUBLE:
	case IS_RESOURCE:
	case IS_BOOL: 
	case IS_LONG: 
		if (offset->type == IS_DOUBLE) {
			index = (long)Z_DVAL_P(offset);
		} else {
			index = Z_LVAL_P(offset);
		}
		if (zend_hash_index_del(spl_array_get_hash_table(intern, 0 TSRMLS_CC), index) == FAILURE) {
			zend_error(E_NOTICE,"Undefined offset:  %ld", Z_LVAL_P(offset));
		}
		break;
	default:
		zend_error(E_WARNING, "Illegal offset type");
		return;
	}
	spl_hash_verify_pos(intern TSRMLS_CC); /* call rewind on FAILURE */
} /* }}} */

static void spl_array_unset_dimension(zval *object, zval *offset TSRMLS_DC) /* {{{ */
{
	spl_array_unset_dimension_ex(1, object, offset TSRMLS_CC);
} /* }}} */

static int spl_array_has_dimension_ex(int check_inherited, zval *object, zval *offset, int check_empty TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	long index;
	zval *rv, **tmp;

	if (check_inherited && intern->fptr_offset_has) {
		SEPARATE_ARG_IF_REF(offset);
		zend_call_method_with_1_params(&object, intern->std.ce, &intern->fptr_offset_has, "offsetExists", &rv, offset);
		zval_ptr_dtor(&offset);
		if (rv && zend_is_true(rv)) {
			zval_ptr_dtor(&rv);
			return 1;
		}
		if (rv) {
			zval_ptr_dtor(&rv);
		}
		return 0;
	}
	
	switch(Z_TYPE_P(offset)) {
	case IS_STRING:
	case IS_UNICODE:
		if (check_empty) {
			if (zend_u_symtable_find(spl_array_get_hash_table(intern, 0 TSRMLS_CC), Z_TYPE_P(offset), Z_UNIVAL_P(offset), Z_UNILEN_P(offset)+1, (void **) &tmp) != FAILURE && zend_is_true(*tmp)) {
				return 1;
			}
			return 0;
		} else {
			return zend_u_symtable_exists(spl_array_get_hash_table(intern, 0 TSRMLS_CC), Z_TYPE_P(offset), Z_UNIVAL_P(offset), Z_UNILEN_P(offset)+1);
		}
	case IS_DOUBLE:
	case IS_RESOURCE:
	case IS_BOOL: 
	case IS_LONG: 
		if (offset->type == IS_DOUBLE) {
			index = (long)Z_DVAL_P(offset);
		} else {
			index = Z_LVAL_P(offset);
		}
		if (check_empty) {
			HashTable *ht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);
			if (zend_hash_index_find(ht, index, (void **)&tmp) != FAILURE && zend_is_true(*tmp)) {
				return 1;
			}
			return 0;
		} else {
			return zend_hash_index_exists(spl_array_get_hash_table(intern, 0 TSRMLS_CC), index);
		}
	default:
		zend_error(E_WARNING, "Illegal offset type");
	}
	return 0;
} /* }}} */

static int spl_array_has_dimension(zval *object, zval *offset, int check_empty TSRMLS_DC) /* {{{ */
{
	return spl_array_has_dimension_ex(1, object, offset, check_empty TSRMLS_CC);
} /* }}} */

/* {{{ proto bool ArrayObject::offsetExists(mixed $index) U
       proto bool ArrayIterator::offsetExists(mixed $index) U
   Returns whether the requested $index exists. */
SPL_METHOD(Array, offsetExists)
{
	zval *index;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &index) == FAILURE) {
		return;
	}
	RETURN_BOOL(spl_array_has_dimension_ex(0, getThis(), index, 0 TSRMLS_CC));
} /* }}} */

/* {{{ proto mixed ArrayObject::offsetGet(mixed $index) U
       proto mixed ArrayIterator::offsetGet(mixed $index) U
   Returns the value at the specified $index. */
SPL_METHOD(Array, offsetGet)
{
	zval *index, *value;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &index) == FAILURE) {
		return;
	}
	value = spl_array_read_dimension_ex(0, getThis(), index, BP_VAR_R TSRMLS_CC);
	RETURN_ZVAL(value, 1, 0);
} /* }}} */

/* {{{ proto void ArrayObject::offsetSet(mixed $index, mixed $newval) U
       proto void ArrayIterator::offsetSet(mixed $index, mixed $newval) U
   Sets the value at the specified $index to $newval. */
SPL_METHOD(Array, offsetSet)
{
	zval *index, *value;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &index, &value) == FAILURE) {
		return;
	}
	spl_array_write_dimension_ex(0, getThis(), index, value TSRMLS_CC);
} /* }}} */

void spl_array_iterator_append(zval *object, zval *append_value TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}
	
	if (Z_TYPE_P(intern->array) == IS_OBJECT) {
		php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR, "Cannot append properties to objects, use %v::offsetSet() instead", intern->std.ce->name);
		return;
	}

	spl_array_write_dimension(object, NULL, append_value TSRMLS_CC);
	if (!intern->pos) {
		spl_array_set_pos(intern, aht->pListTail);
	}
} /* }}} */

/* {{{ proto void ArrayObject::append(mixed $newval) U
       proto void ArrayIterator::append(mixed $newval) U
   Appends the value (cannot be called for objects). */
SPL_METHOD(Array, append)
{
	zval *value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
		return;
	}
	spl_array_iterator_append(getThis(), value TSRMLS_CC);
} /* }}} */

/* {{{ proto void ArrayObject::offsetUnset(mixed $index) U
       proto void ArrayIterator::offsetUnset(mixed $index) U
   Unsets the value at the specified $index. */
SPL_METHOD(Array, offsetUnset)
{
	zval *index;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &index) == FAILURE) {
		return;
	}
	spl_array_unset_dimension_ex(0, getThis(), index TSRMLS_CC);
} /* }}} */

/* {{{ proto array ArrayObject::getArrayCopy() U
      proto array ArrayIterator::getArrayCopy() U
   Return a copy of the contained array */
SPL_METHOD(Array, getArrayCopy)
{
	zval *object = getThis(), *tmp;
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

    array_init(return_value);
	zend_hash_copy(HASH_OF(return_value), spl_array_get_hash_table(intern, 0 TSRMLS_CC), (copy_ctor_func_t) zval_add_ref, &tmp, sizeof(zval*));
} /* }}} */

static HashTable *spl_array_get_properties(zval *object TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	return spl_array_get_hash_table(intern, 1 TSRMLS_CC);
} /* }}} */

static HashTable* spl_array_get_debug_info(zval *obj, int *is_temp TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(obj TSRMLS_CC);
	HashTable *rv;
	zval *tmp, *storage;
	int name_len;
	zstr zname;
	zend_class_entry *base;

	if (HASH_OF(intern->array) == intern->std.properties) {
		*is_temp = 0;
		return intern->std.properties;
	} else {
		*is_temp = 1;
	
		ALLOC_HASHTABLE(rv);
		ZEND_INIT_SYMTABLE_EX(rv, zend_hash_num_elements(intern->std.properties) + 1, 0);
	
		zend_hash_copy(rv, intern->std.properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
	
		storage = intern->array;
		zval_add_ref(&storage);
	
		base = (Z_OBJ_HT_P(obj) == &spl_handler_ArrayIterator) ? spl_ce_ArrayIterator : spl_ce_ArrayObject;
		zname = spl_gen_private_prop_name(base, "storage", sizeof("storage")-1, &name_len TSRMLS_CC);
		zend_u_symtable_update(rv, IS_UNICODE, zname, name_len+1, &storage, sizeof(zval *), NULL);
		efree(zname.v);
	
		return rv;
	}
}
/* }}} */

static zval *spl_array_read_property(zval *object, zval *member, int type TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if ((intern->ar_flags & SPL_ARRAY_ARRAY_AS_PROPS) != 0
	&& !std_object_handlers.has_property(object, member, 2 TSRMLS_CC)) {
		return spl_array_read_dimension(object, member, type TSRMLS_CC);
	}
	return std_object_handlers.read_property(object, member, type TSRMLS_CC);
} /* }}} */

static void spl_array_write_property(zval *object, zval *member, zval *value TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if ((intern->ar_flags & SPL_ARRAY_ARRAY_AS_PROPS) != 0
	&& !std_object_handlers.has_property(object, member, 2 TSRMLS_CC)) {
		spl_array_write_dimension(object, member, value TSRMLS_CC);
		return;
	}
	std_object_handlers.write_property(object, member, value TSRMLS_CC);
} /* }}} */

static zval **spl_array_get_property_ptr_ptr(zval *object, zval *member TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if ((intern->ar_flags & SPL_ARRAY_ARRAY_AS_PROPS) != 0
	&& !std_object_handlers.has_property(object, member, 2 TSRMLS_CC)) {
		return spl_array_get_dimension_ptr_ptr(1, object, member, 0 TSRMLS_CC);		
	}
	return std_object_handlers.get_property_ptr_ptr(object, member TSRMLS_CC);
} /* }}} */

static int spl_array_has_property(zval *object, zval *member, int has_set_exists TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if ((intern->ar_flags & SPL_ARRAY_ARRAY_AS_PROPS) != 0
	&& !std_object_handlers.has_property(object, member, 2 TSRMLS_CC)) {
		return spl_array_has_dimension(object, member, has_set_exists TSRMLS_CC);
	}
	return std_object_handlers.has_property(object, member, has_set_exists TSRMLS_CC);

} /* }}} */

static void spl_array_unset_property(zval *object, zval *member TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if ((intern->ar_flags & SPL_ARRAY_ARRAY_AS_PROPS) != 0
	&& !std_object_handlers.has_property(object, member, 2 TSRMLS_CC)) {
		spl_array_unset_dimension(object, member TSRMLS_CC);
		spl_array_rewind(intern TSRMLS_CC); /* because deletion might invalidate position */
		return;
	}
	std_object_handlers.unset_property(object, member TSRMLS_CC);
} /* }}} */

static int spl_array_skip_protected(spl_array_object *intern, HashTable *aht TSRMLS_DC) /* {{{ */
{
	zstr string_key;
	uint string_length;
	ulong num_key;

	if (Z_TYPE_P(intern->array) == IS_OBJECT) {
		do {
			if (zend_hash_get_current_key_ex(aht, &string_key, &string_length, &num_key, 0, &intern->pos) == HASH_KEY_IS_UNICODE) {
				if (!string_length || string_key.u[0]) {
					return SUCCESS;
				}
			} else {
				return SUCCESS;
			}
			if (zend_hash_has_more_elements_ex(aht, &intern->pos) != SUCCESS) {
				return FAILURE;
			}
			zend_hash_move_forward_ex(aht, &intern->pos);
			spl_array_update_pos(intern);
		} while (1);
	}
	return FAILURE;
} /* }}} */

static int spl_array_next_no_verify(spl_array_object *intern, HashTable *aht TSRMLS_DC) /* {{{ */
{
	zend_hash_move_forward_ex(aht, &intern->pos);
	spl_array_update_pos(intern);
	if (Z_TYPE_P(intern->array) == IS_OBJECT) {
		return spl_array_skip_protected(intern, aht TSRMLS_CC);
	} else {
		return zend_hash_has_more_elements_ex(aht, &intern->pos);
	}
} /* }}} */

static int spl_array_next_ex(spl_array_object *intern, HashTable *aht TSRMLS_DC) /* {{{ */
{
	if ((intern->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(intern, aht TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and internal position is no longer valid");
		return FAILURE;
	}

	return spl_array_next_no_verify(intern, aht TSRMLS_CC);
} /* }}} */

static int spl_array_next(spl_array_object *intern TSRMLS_DC) /* {{{ */
{
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	return spl_array_next_ex(intern, aht TSRMLS_CC);

} /* }}} */

/* {{{ define an overloaded iterator structure */
typedef struct {
	zend_user_iterator    intern;
	spl_array_object      *object;
} spl_array_it; /* }}} */

static void spl_array_it_dtor(zend_object_iterator *iter TSRMLS_DC) /* {{{ */
{
	spl_array_it *iterator = (spl_array_it *)iter;

	zend_user_it_invalidate_current(iter TSRMLS_CC);
	zval_ptr_dtor((zval**)&iterator->intern.it.data);

	efree(iterator);
}
/* }}} */

static int spl_array_it_valid(zend_object_iterator *iter TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator = (spl_array_it *)iter;
	spl_array_object   *object   = iterator->object;
	HashTable          *aht      = spl_array_get_hash_table(object, 0 TSRMLS_CC);

	if (object->ar_flags & SPL_ARRAY_OVERLOADED_VALID) {
		return zend_user_it_valid(iter TSRMLS_CC);
	} else {
		if (!aht) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "ArrayIterator::valid(): Array was modified outside object and is no longer an array");
			return FAILURE;
		}
	
		if (object->pos && (object->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(object, aht TSRMLS_CC) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "ArrayIterator::valid(): Array was modified outside object and internal position is no longer valid");
			return FAILURE;
		} else {
			return zend_hash_has_more_elements_ex(aht, &object->pos);
		}
	}
}
/* }}} */

static void spl_array_it_get_current_data(zend_object_iterator *iter, zval ***data TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator = (spl_array_it *)iter;
	spl_array_object   *object   = iterator->object;
	HashTable          *aht      = spl_array_get_hash_table(object, 0 TSRMLS_CC);

	if (object->ar_flags & SPL_ARRAY_OVERLOADED_CURRENT) {
		zend_user_it_get_current_data(iter, data TSRMLS_CC);
	} else {
		if (zend_hash_get_current_data_ex(aht, (void**)data, &object->pos) == FAILURE) {
			*data = NULL;
		}
	}
}
/* }}} */

static int spl_array_it_get_current_key(zend_object_iterator *iter, zstr *str_key, uint *str_key_len, ulong *int_key TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator = (spl_array_it *)iter;
	spl_array_object   *object   = iterator->object;
	HashTable          *aht      = spl_array_get_hash_table(object, 0 TSRMLS_CC);

	if (object->ar_flags & SPL_ARRAY_OVERLOADED_KEY) {
		return zend_user_it_get_current_key(iter, str_key, str_key_len, int_key TSRMLS_CC);
	} else {
		if (!aht) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "ArrayIterator::current(): Array was modified outside object and is no longer an array");
			return HASH_KEY_NON_EXISTANT;
		}
	
		if ((object->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(object, aht TSRMLS_CC) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "ArrayIterator::current(): Array was modified outside object and internal position is no longer valid");
			return HASH_KEY_NON_EXISTANT;
		}
	
		return zend_hash_get_current_key_ex(aht, str_key, str_key_len, int_key, 1, &object->pos);
	}
}
/* }}} */

static void spl_array_it_move_forward(zend_object_iterator *iter TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator = (spl_array_it *)iter;
	spl_array_object   *object   = iterator->object;
	HashTable          *aht      = spl_array_get_hash_table(object, 0 TSRMLS_CC);

	if (object->ar_flags & SPL_ARRAY_OVERLOADED_NEXT) {
		zend_user_it_move_forward(iter TSRMLS_CC);
	} else {
		zend_user_it_invalidate_current(iter TSRMLS_CC);
		if (!aht) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "ArrayIterator::current(): Array was modified outside object and is no longer an array");
			return;
		}
	
		if ((object->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(object, aht TSRMLS_CC) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "ArrayIterator::next(): Array was modified outside object and internal position is no longer valid");
		} else {
			spl_array_next_no_verify(object, aht TSRMLS_CC);
		}
	}
}
/* }}} */

static void spl_array_rewind_ex(spl_array_object *intern, HashTable *aht TSRMLS_DC) /* {{{ */
{

	zend_hash_internal_pointer_reset_ex(aht, &intern->pos);
	spl_array_update_pos(intern);
	spl_array_skip_protected(intern, aht TSRMLS_CC);

} /* }}} */

static void spl_array_rewind(spl_array_object *intern TSRMLS_DC) /* {{{ */
{
	HashTable          *aht      = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "ArrayIterator::rewind(): Array was modified outside object and is no longer an array");
		return;
	}

	spl_array_rewind_ex(intern, aht TSRMLS_CC);
}
/* }}} */

static void spl_array_it_rewind(zend_object_iterator *iter TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator = (spl_array_it *)iter;
	spl_array_object   *object   = iterator->object;

	if (object->ar_flags & SPL_ARRAY_OVERLOADED_REWIND) {
		zend_user_it_rewind(iter TSRMLS_CC);
	} else {
		zend_user_it_invalidate_current(iter TSRMLS_CC);
		spl_array_rewind(object TSRMLS_CC);
	}
}
/* }}} */

/* {{{ spl_array_set_array */
static void spl_array_set_array(zval *object, spl_array_object *intern, zval **array, long ar_flags, int just_array TSRMLS_DC) {

	if (Z_TYPE_PP(array) == IS_ARRAY) {
		SEPARATE_ZVAL_IF_NOT_REF(array);
	}

	if (Z_TYPE_PP(array) == IS_OBJECT && (Z_OBJ_HT_PP(array) == &spl_handler_ArrayObject || Z_OBJ_HT_PP(array) == &spl_handler_ArrayIterator)) {
		zval_ptr_dtor(&intern->array);
		if (just_array)	{
			spl_array_object *other = (spl_array_object*)zend_object_store_get_object(*array TSRMLS_CC);
			ar_flags = other->ar_flags & ~SPL_ARRAY_INT_MASK;
		}		
		ar_flags |= SPL_ARRAY_USE_OTHER;
		intern->array = *array;
	} else {
		if (Z_TYPE_PP(array) != IS_OBJECT && Z_TYPE_PP(array) != IS_ARRAY) {
			zend_throw_exception(spl_ce_InvalidArgumentException, "Passed variable is not an array or object, using empty array instead", 0 TSRMLS_CC);
			return;
		}
		zval_ptr_dtor(&intern->array);
		intern->array = *array;
	}
	if (object == *array) {
		intern->ar_flags |= SPL_ARRAY_IS_SELF;
		intern->ar_flags &= ~SPL_ARRAY_USE_OTHER;
	} else {
		intern->ar_flags &= ~SPL_ARRAY_IS_SELF;
	}
	intern->ar_flags |= ar_flags;
	Z_ADDREF_P(intern->array);
	if (Z_TYPE_PP(array) == IS_OBJECT) {
		zend_object_get_properties_t handler = Z_OBJ_HANDLER_PP(array, get_properties);
		if ((handler != std_object_handlers.get_properties && handler != spl_array_get_properties)
		|| !spl_array_get_hash_table(intern, 0 TSRMLS_CC)) {
			zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0 TSRMLS_CC, "Overloaded object of type %v is not compatible with %v", Z_OBJCE_PP(array)->name, intern->std.ce->name);
		}
	}

	spl_array_rewind(intern TSRMLS_CC);
}
/* }}} */

/* {{{ iterator handler table */
zend_object_iterator_funcs spl_array_it_funcs = {
	spl_array_it_dtor,
	spl_array_it_valid,
	spl_array_it_get_current_data,
	spl_array_it_get_current_key,
	spl_array_it_move_forward,
	spl_array_it_rewind
}; /* }}} */

zend_object_iterator *spl_array_get_iterator(zend_class_entry *ce, zval *object, int by_ref TSRMLS_DC) /* {{{ */
{
	spl_array_it       *iterator;
	spl_array_object   *array_object = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if (by_ref && (array_object->ar_flags & SPL_ARRAY_OVERLOADED_CURRENT)) {
		zend_error(E_ERROR, "An iterator cannot be used with foreach by reference");
	}

	iterator     = emalloc(sizeof(spl_array_it));

	Z_ADDREF_P(object);
	iterator->intern.it.data = (void*)object;
	iterator->intern.it.funcs = &spl_array_it_funcs;
	iterator->intern.ce = ce;
	iterator->intern.value = NULL;
	iterator->object = array_object;
	
	return (zend_object_iterator*)iterator;
}
/* }}} */

/* {{{ proto void ArrayObject::__construct(array|object ar = array() [, int flags = 0 [, string iterator_class = "ArrayIterator"]]) U
       proto void ArrayIterator::__construct(array|object ar = array() [, int flags = 0]) U
   Constructs a new array iterator from a path. */
SPL_METHOD(Array, __construct)
{
	zval *object = getThis();
	spl_array_object *intern;
	zval **array;
	long ar_flags = 0;
	zend_class_entry *ce_get_iterator = spl_ce_Iterator;
	zend_error_handling error_handling;

	if (ZEND_NUM_ARGS() == 0) {
		return; /* nothing to do */
	}

	zend_replace_error_handling(EH_THROW, spl_ce_InvalidArgumentException, &error_handling TSRMLS_CC);

	intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z|lC", &array, &ar_flags, &ce_get_iterator) == FAILURE) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (ZEND_NUM_ARGS() > 2) {
		intern->ce_get_iterator = ce_get_iterator;
	}

	ar_flags &= ~SPL_ARRAY_INT_MASK;

	spl_array_set_array(object, intern, array, ar_flags, ZEND_NUM_ARGS() == 1 TSRMLS_CC);
	zend_restore_error_handling(&error_handling TSRMLS_CC);
}
/* }}} */

/* {{{ proto void ArrayObject::setIteratorClass(string iterator_class) U
   Set the class used in getIterator. */
SPL_METHOD(Array, setIteratorClass)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	zend_class_entry *ce_get_iterator = spl_ce_Iterator;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "C", &ce_get_iterator) == FAILURE) {
		return;
	}

	intern->ce_get_iterator = ce_get_iterator;
}
/* }}} */

/* {{{ proto string ArrayObject::getIteratorClass() U
   Get the class used in getIterator. */
SPL_METHOD(Array, getIteratorClass)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	RETURN_UNICODEL(intern->ce_get_iterator->name.u, intern->ce_get_iterator->name_length, 1);
}
/* }}} */

/* {{{ proto int ArrayObject::getFlags() U
   Get flags */
SPL_METHOD(Array, getFlags)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	
	RETURN_LONG(intern->ar_flags & ~SPL_ARRAY_INT_MASK);
}
/* }}} */

/* {{{ proto void ArrayObject::setFlags(int flags) U
   Set flags */
SPL_METHOD(Array, setFlags)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	long ar_flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &ar_flags) == FAILURE) {
		return;
	}
	
	intern->ar_flags = (intern->ar_flags & SPL_ARRAY_INT_MASK) | (ar_flags & ~SPL_ARRAY_INT_MASK);
}
/* }}} */

/* {{{ proto Array|Object ArrayObject::exchangeArray(Array|Object ar = array()) U
   Replace the referenced array or object with a new one and return the old one (right now copy - to be changed) */
SPL_METHOD(Array, exchangeArray)
{
	zval *object = getThis(), *tmp, **array;
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	array_init(return_value);
	zend_hash_copy(HASH_OF(return_value), spl_array_get_hash_table(intern, 0 TSRMLS_CC), (copy_ctor_func_t) zval_add_ref, &tmp, sizeof(zval*));
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z", &array) == FAILURE) {
		return;
	}

	spl_array_set_array(object, intern, array, 0L, 1 TSRMLS_CC);

}
/* }}} */

/* {{{ proto ArrayIterator ArrayObject::getIterator() U
   Create a new iterator from a ArrayObject instance */
SPL_METHOD(Array, getIterator)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	spl_array_object *iterator;
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}

	return_value->type = IS_OBJECT;
	return_value->value.obj = spl_array_object_new_ex(intern->ce_get_iterator, &iterator, object, 0 TSRMLS_CC);
	Z_SET_REFCOUNT_P(return_value, 1);
	Z_SET_ISREF_P(return_value);
}
/* }}} */

/* {{{ proto void ArrayIterator::rewind() U
   Rewind array back to the start */
SPL_METHOD(Array, rewind)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	spl_array_rewind(intern TSRMLS_CC);
}
/* }}} */

/* {{{ proto void ArrayIterator::seek(int $position) U
   Seek to position. */
SPL_METHOD(Array, seek)
{
	long opos, position;
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);
	int result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &position) == FAILURE) {
		return;
	}

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}

	opos = position;

	if (position >= 0) { /* negative values are not supported */
		spl_array_rewind(intern TSRMLS_CC);
		result = SUCCESS;
		
		while (position-- > 0 && (result = spl_array_next(intern TSRMLS_CC)) == SUCCESS);
	
		if (result == SUCCESS && zend_hash_has_more_elements_ex(aht, &intern->pos) == SUCCESS) {
			return; /* ok */
		}
	}
	zend_throw_exception_ex(spl_ce_OutOfBoundsException, 0 TSRMLS_CC, "Seek position %ld is out of range", opos);
} /* }}} */

int static spl_array_object_count_elements_helper(spl_array_object *intern, long *count TSRMLS_DC) /* {{{ */
{
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);
	HashPosition pos;

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		*count = 0;
		return FAILURE;
	}

	if (Z_TYPE_P(intern->array) == IS_OBJECT) {
		/* We need to store the 'pos' since we'll modify it in the functions 
		 * we're going to call and which do not support 'pos' as parameter. */
		pos = intern->pos;
		*count = 0;
		spl_array_rewind(intern TSRMLS_CC);
		while(intern->pos && spl_array_next(intern TSRMLS_CC) == SUCCESS) {
			(*count)++;
		}
		spl_array_set_pos(intern, pos);
		return SUCCESS;
	} else {
		*count = zend_hash_num_elements(aht);
		return SUCCESS;
	}
} /* }}} */

int spl_array_object_count_elements(zval *object, long *count TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if (intern->fptr_count) {
		zval *rv;
		zend_call_method_with_0_params(&object, intern->std.ce, &intern->fptr_count, "count", &rv);
		if (rv) {
			zval_ptr_dtor(&intern->retval);
			MAKE_STD_ZVAL(intern->retval);
			ZVAL_ZVAL(intern->retval, rv, 1, 1);
			convert_to_long(intern->retval);
			*count = (long) Z_LVAL_P(intern->retval);
			return SUCCESS;
		}
		*count = 0;
		return FAILURE;
	}
	return spl_array_object_count_elements_helper(intern, count TSRMLS_CC);
} /* }}} */

/* {{{ proto int ArrayObject::count() U
       proto int ArrayIterator::count() U
   Return the number of elements in the Iterator. */
SPL_METHOD(Array, count)
{
	long count;
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);

	spl_array_object_count_elements_helper(intern, &count TSRMLS_CC);
	RETURN_LONG(count);
} /* }}} */

/* {{{ static void spl_array_method
*/
static void spl_array_method(INTERNAL_FUNCTION_PARAMETERS, char *fname, int fname_len, int use_arg)
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);
	zval *tmp, *arg;
	zval *retval_ptr = NULL;
	
	MAKE_STD_ZVAL(tmp);
	Z_TYPE_P(tmp) = IS_ARRAY;
	Z_ARRVAL_P(tmp) = aht;
	
	if (use_arg) {
		if (ZEND_NUM_ARGS() != 1 || zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "z", &arg) == FAILURE) {
			Z_TYPE_P(tmp) = IS_NULL;
			zval_ptr_dtor(&tmp);
			zend_throw_exception(spl_ce_BadMethodCallException, "Function expects exactly one argument", 0 TSRMLS_CC);
			return;
		}
		zend_call_method(NULL, NULL, NULL, fname, fname_len, &retval_ptr, 2, tmp, arg TSRMLS_CC);
	} else {
		zend_call_method(NULL, NULL, NULL, fname, fname_len, &retval_ptr, 1, tmp, NULL TSRMLS_CC);
	}
	Z_TYPE_P(tmp) = IS_NULL; /* we want to destroy the zval, not the hashtable */
	zval_ptr_dtor(&tmp);
	if (retval_ptr) {
		COPY_PZVAL_TO_ZVAL(*return_value, retval_ptr);
	}
} /* }}} */

/* {{{ SPL_ARRAY_METHOD */
#define SPL_ARRAY_METHOD(cname, fname, use_arg) \
SPL_METHOD(cname, fname) \
{ \
	spl_array_method(INTERNAL_FUNCTION_PARAM_PASSTHRU, #fname, sizeof(#fname)-1, use_arg); \
} /* }}} */

/* {{{ proto int ArrayObject::asort() U
       proto int ArrayIterator::asort() U
   Sort the entries by values. */
SPL_ARRAY_METHOD(Array, asort, 0) /* }}} */

/* {{{ proto int ArrayObject::ksort() U
       proto int ArrayIterator::ksort() U
   Sort the entries by key. */
SPL_ARRAY_METHOD(Array, ksort, 0) /* }}} */

/* {{{ proto int ArrayObject::uasort(callback cmp_function) U
       proto int ArrayIterator::uasort(callback cmp_function) U
   Sort the entries by values user defined function. */
SPL_ARRAY_METHOD(Array, uasort, 1) /* }}} */

/* {{{ proto int ArrayObject::uksort(callback cmp_function) U
       proto int ArrayIterator::uksort(callback cmp_function) U
   Sort the entries by key using user defined function. */
SPL_ARRAY_METHOD(Array, uksort, 1) /* }}} */

/* {{{ proto int ArrayObject::natsort() U
       proto int ArrayIterator::natsort() U
   Sort the entries by values using "natural order" algorithm. */
SPL_ARRAY_METHOD(Array, natsort, 0) /* }}} */

/* {{{ proto int ArrayObject::natcasesort() U
       proto int ArrayIterator::natcasesort() U
   Sort the entries by key using case insensitive "natural order" algorithm. */
SPL_ARRAY_METHOD(Array, natcasesort, 0) /* }}} */

/* {{{ proto mixed|NULL ArrayIterator::current() U
   Return current array entry */
SPL_METHOD(Array, current)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	zval **entry;
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}

	if ((intern->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(intern, aht TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and internal position is no longer valid");
		return;
	}

	if (zend_hash_get_current_data_ex(aht, (void **) &entry, &intern->pos) == FAILURE) {
		return;
	}
	RETVAL_ZVAL(*entry, 1, 0);
}
/* }}} */

void spl_array_iterator_key(zval *object, zval *return_value TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	zstr string_key;
	uint string_length;
	ulong num_key;
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}

	if ((intern->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(intern, aht TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and internal position is no longer valid");
		return;
	}

	switch (zend_hash_get_current_key_ex(aht, &string_key, &string_length, &num_key, 1, &intern->pos)) {
		case HASH_KEY_IS_STRING:
			RETVAL_STRINGL(string_key.s, string_length - 1, 0);
			break;
		case HASH_KEY_IS_UNICODE:
			RETVAL_UNICODEL(string_key.u, string_length - 1, 0);
			break;
		case HASH_KEY_IS_LONG:
			RETVAL_LONG(num_key);
			break;
		case HASH_KEY_NON_EXISTANT:
			return;
	}
}
/* }}} */

/* {{{  proto mixed|NULL ArrayIterator::key() U
   Return current array key */
SPL_METHOD(Array, key)
{
	spl_array_iterator_key(getThis(), return_value TSRMLS_CC);	
}
/* }}} */

/* {{{ proto void ArrayIterator::next() U
   Move to next entry */
SPL_METHOD(Array, next)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}

	spl_array_next_ex(intern, aht TSRMLS_CC);
}
/* }}} */

/* {{{ proto bool ArrayIterator::valid() U
   Check whether array contains more entries */
SPL_METHOD(Array, valid)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}

	if (intern->pos && (intern->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(intern, aht TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and internal position is no longer valid");
		RETURN_FALSE;
	} else {
		RETURN_BOOL(zend_hash_has_more_elements_ex(aht, &intern->pos) == SUCCESS);
	}
}
/* }}} */

/* {{{ proto bool RecursiveArrayIterator::hasChildren() U
   Check whether current element has children (e.g. is an array) */
SPL_METHOD(Array, hasChildren)
{
	zval *object = getThis(), **entry;
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);
	
	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		RETURN_FALSE;
	}

	if ((intern->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(intern, aht TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and internal position is no longer valid");
		RETURN_FALSE;
	}

	if (zend_hash_get_current_data_ex(aht, (void **) &entry, &intern->pos) == FAILURE) {
		RETURN_FALSE;
	}

	RETURN_BOOL(Z_TYPE_PP(entry) == IS_ARRAY || (Z_TYPE_PP(entry) == IS_OBJECT && (intern->ar_flags & SPL_ARRAY_CHILD_ARRAYS_ONLY) == 0));
}
/* }}} */

/* {{{ proto object RecursiveArrayIterator::getChildren() U
   Create a sub iterator for the current element (same class as $this) */
SPL_METHOD(Array, getChildren)
{
	zval *object = getThis(), **entry, *flags;
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return;
	}

	if ((intern->ar_flags & SPL_ARRAY_IS_REF) && spl_hash_verify_pos_ex(intern, aht TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and internal position is no longer valid");
		return;
	}

	if (zend_hash_get_current_data_ex(aht, (void **) &entry, &intern->pos) == FAILURE) {
		return;
	}

	if (Z_TYPE_PP(entry) == IS_OBJECT) {
		if ((intern->ar_flags & SPL_ARRAY_CHILD_ARRAYS_ONLY) != 0) {
			return;
		}
		if (instanceof_function(Z_OBJCE_PP(entry), Z_OBJCE_P(getThis()) TSRMLS_CC)) {
			RETURN_ZVAL(*entry, 0, 0);
		}
	}

	MAKE_STD_ZVAL(flags);
	ZVAL_LONG(flags, SPL_ARRAY_USE_OTHER | intern->ar_flags);

	spl_instantiate_arg_ex2(intern->std.ce, &return_value, 0, *entry, flags TSRMLS_CC);
	zval_ptr_dtor(&flags);
}
/* }}} */

smart_str spl_array_serialize_helper(spl_array_object *intern, php_serialize_data_t *var_hash_p TSRMLS_DC) /* {{{ */
{
	HashTable *aht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);
	zval members, *pmembers;
	smart_str buf = {0};
	zval *flags;

	if (!aht) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Array was modified outside object and is no longer an array");
		return buf;
	}

	MAKE_STD_ZVAL(flags);
	ZVAL_LONG(flags, (intern->ar_flags & SPL_ARRAY_CLONE_MASK));

	/* storage */
	smart_str_appendl(&buf, "x:", 2);
	php_var_serialize(&buf, &flags, var_hash_p TSRMLS_CC);
	zval_ptr_dtor(&flags);

	if (!(intern->ar_flags & SPL_ARRAY_IS_SELF)) {
		php_var_serialize(&buf, &intern->array, var_hash_p TSRMLS_CC);
		smart_str_appendc(&buf, ';');
	}

	/* members */
	smart_str_appendl(&buf, "m:", 2);
	INIT_PZVAL(&members);
	Z_ARRVAL(members) = intern->std.properties;
	Z_TYPE(members) = IS_ARRAY;
	pmembers = &members;
	php_var_serialize(&buf, &pmembers, var_hash_p TSRMLS_CC); /* finishes the string */

	return buf;
}
/* }}} */

/* {{{ proto string ArrayObject::serialize()
   Serialize the object */
SPL_METHOD(Array, serialize)
{
	zval *object = getThis();
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	int was_in_serialize = intern->serialize_data != NULL;
	smart_str buf;

	if (!was_in_serialize) {
		intern->serialize_data = emalloc(sizeof(php_serialize_data_t));
		PHP_VAR_SERIALIZE_INIT(*intern->serialize_data);
	}

	buf = spl_array_serialize_helper(intern, intern->serialize_data TSRMLS_CC);

	if (!was_in_serialize) {
		PHP_VAR_SERIALIZE_DESTROY(*intern->serialize_data);
		efree(intern->serialize_data);
		intern->serialize_data = NULL;
	}

	if (buf.c) {
		RETURN_STRINGL(buf.c, buf.len, 0);
	} 

	RETURN_NULL();
} /* }}} */

int spl_array_serialize(zval *object, int *type, zstr *buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC) /* {{{ */
{
	spl_array_object     *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);

	if (intern->fptr_serialize) {
		int retval;
		php_serialize_data_t *before;

		before = intern->serialize_data;
		intern->serialize_data = (php_serialize_data_t *)data;

		retval = zend_user_serialize(object, type, buffer, buf_len, data TSRMLS_CC);

		intern->serialize_data = before;

		return retval;
	} else {
		smart_str buf;

		buf = spl_array_serialize_helper(intern, (php_serialize_data_t *)data TSRMLS_CC);

		if (buf.c) {
			buffer->s = estrndup(buf.c, buf.len);
			*buf_len  = buf.len;
			*type     = IS_STRING;
			efree(buf.c);
			return SUCCESS;
		} else {
			return FAILURE;
		}
	}
}
/* }}} */

void spl_array_unserialize_helper(spl_array_object *intern, const unsigned char *buf, zend_uint buf_len, php_unserialize_data_t *var_hash_p TSRMLS_DC) /* {{{ */
{
	const unsigned char *p, *s;
	zval *pmembers, *pflags = NULL;
	long flags;
	
	s = p = buf;

	if (*p!= 'x' || *++p != ':') {
		goto outexcept;
	}
	++p;

	ALLOC_INIT_ZVAL(pflags);
	if (!php_var_unserialize(&pflags, &p, s + buf_len, var_hash_p TSRMLS_CC) || Z_TYPE_P(pflags) != IS_LONG) {
		zval_ptr_dtor(&pflags);
		goto outexcept;
	}

	--p; /* for ';' */
	flags = Z_LVAL_P(pflags);
	zval_ptr_dtor(&pflags);
	/* flags needs to be verified and we also need to verify whether the next
	 * thing we get is ';'. After that we require an 'm' or somethign else
	 * where 'm' stands for members and anything else should be an array. If
	 * neither 'a' or 'm' follows we have an error. */

	if (*p != ';') {
		goto outexcept;
	}
	++p;

	if (*p!='m') {
		if (*p!='a' && *p!='O' && *p!='C') {
			goto outexcept;
		}
		intern->ar_flags &= ~SPL_ARRAY_CLONE_MASK;
		intern->ar_flags |= flags & SPL_ARRAY_CLONE_MASK;
		zval_ptr_dtor(&intern->array);
		ALLOC_INIT_ZVAL(intern->array);
		if (!php_var_unserialize(&intern->array, &p, s + buf_len, var_hash_p TSRMLS_CC)) {
			goto outexcept;
		}
	}
	if (*p != ';') {
		goto outexcept;
	}
	++p;

	/* members */
	if (*p!= 'm' || *++p != ':') {
		goto outexcept;
	}
	++p;

	ALLOC_INIT_ZVAL(pmembers);
	if (!php_var_unserialize(&pmembers, &p, s + buf_len, var_hash_p TSRMLS_CC)) {
		zval_ptr_dtor(&pmembers);
		goto outexcept;
	}

	/* copy members */
	zend_hash_copy(intern->std.properties, Z_ARRVAL_P(pmembers), (copy_ctor_func_t) zval_add_ref, (void *) NULL, sizeof(zval *));
	zval_ptr_dtor(&pmembers);

	/* done reading $serialized */

	return;

outexcept:
	zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Error at offset %ld of %d bytes", (long)((char*)p - (char *)buf), buf_len);
	return;
}
/* }}} */

/* {{{ proto void ArrayObject::unserialize(string serialized)
   Unserialize the object */
SPL_METHOD(Array, unserialize)
{
	char *buf;
	int buf_len;
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
	int was_in_unserialize = intern->unserialize_data != NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &buf, &buf_len) == FAILURE) {
		return;
	}

	if (buf_len == 0) {
		zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Serialized string cannot be empty");
		return;
	}

	if (!was_in_unserialize) {
		intern->unserialize_data = emalloc(sizeof(php_unserialize_data_t));
		PHP_VAR_UNSERIALIZE_INIT(*intern->unserialize_data);
	}

	spl_array_unserialize_helper(intern, (const unsigned char *)buf, buf_len, intern->unserialize_data TSRMLS_CC);

	if (!was_in_unserialize) {
		PHP_VAR_UNSERIALIZE_DESTROY(*intern->unserialize_data);
		efree(intern->unserialize_data);
		intern->unserialize_data = NULL;
	}
}
/* }}} */

int spl_array_unserialize(zval **object, zend_class_entry *ce, int type, const zstr buf, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC) /* {{{ */
{
	spl_array_object *intern;

	object_init_ex(*object, ce);
	intern = (spl_array_object*)zend_object_store_get_object(*object TSRMLS_CC);

	if (intern->fptr_unserialize) {
		zval *zdata;
		php_unserialize_data_t *before;
		MAKE_STD_ZVAL(zdata);
		ZVAL_ZSTRL(zdata, type, buf, buf_len, 1);

		before = intern->unserialize_data;
		intern->unserialize_data = (php_unserialize_data_t *)data;

		zend_call_method_with_1_params(object, ce, &ce->unserialize_func, "unserialize", NULL, zdata);

		intern->unserialize_data = before;

		zval_ptr_dtor(&zdata);
	} else {
		if (type == IS_STRING) {
			spl_array_unserialize_helper(intern, (unsigned char *)buf.s, buf_len, (php_unserialize_data_t *)data TSRMLS_CC);
		} else {
			unsigned char *bufc = (unsigned char*)zend_unicode_to_ascii(buf.u, buf_len TSRMLS_CC);
			if (!bufc) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Binary or ASCII-Unicode string expected, non-ASCII-Unicode string received");
				return FAILURE;
			}

			spl_array_unserialize_helper(intern, bufc, buf_len, (php_unserialize_data_t *)data TSRMLS_CC);

			efree(bufc);
		}
	}

	if (EG(exception)) {
		return FAILURE;
	} else {
		return SUCCESS;
	}
}
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_array___construct, 0)
	ZEND_ARG_INFO(0, array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_offsetGet, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_offsetSet, 0, 0, 2)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, newval)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_append, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_seek, 0)
	ZEND_ARG_INFO(0, position)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_exchangeArray, 0)
	ZEND_ARG_INFO(0, array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_setFlags, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_setIteratorClass, 0)
	ZEND_ARG_INFO(0, iteratorClass)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_array_uXsort, 0)
	ZEND_ARG_INFO(0, cmp_function)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_array_unserialize, 0)
	ZEND_ARG_INFO(0, serialized)
ZEND_END_ARG_INFO();
/* }}} */

static const zend_function_entry spl_funcs_ArrayObject[] = { /* {{{ */
	SPL_ME(Array, __construct,      arginfo_array___construct,      ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetExists,     arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetGet,        arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetSet,        arginfo_array_offsetSet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetUnset,      arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, append,           arginfo_array_append,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, getArrayCopy,     NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, count,            NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, getFlags,         NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, setFlags,         arginfo_array_setFlags,         ZEND_ACC_PUBLIC)
	SPL_ME(Array, asort,            NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, ksort,            NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, uasort,           arginfo_array_uXsort,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, uksort,           arginfo_array_uXsort,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, natsort,          NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, natcasesort,      NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, unserialize,      arginfo_array_unserialize,      ZEND_ACC_PUBLIC)
	SPL_ME(Array, serialize,        NULL,                           ZEND_ACC_PUBLIC)
	/* ArrayObject specific */
	SPL_ME(Array, getIterator,      NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, exchangeArray,    arginfo_array_exchangeArray,    ZEND_ACC_PUBLIC)
	SPL_ME(Array, setIteratorClass, arginfo_array_setIteratorClass, ZEND_ACC_PUBLIC)
	SPL_ME(Array, getIteratorClass, NULL,                           ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
}; /* }}} */

static const zend_function_entry spl_funcs_ArrayIterator[] = { /* {{{ */
	SPL_ME(Array, __construct,      arginfo_array___construct,      ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetExists,     arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetGet,        arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetSet,        arginfo_array_offsetSet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, offsetUnset,      arginfo_array_offsetGet,        ZEND_ACC_PUBLIC)
	SPL_ME(Array, append,           arginfo_array_append,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, getArrayCopy,     NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, count,            NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, getFlags,         NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, setFlags,         arginfo_array_setFlags,         ZEND_ACC_PUBLIC)
	SPL_ME(Array, asort,            NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, ksort,            NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, uasort,           arginfo_array_uXsort,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, uksort,           arginfo_array_uXsort,           ZEND_ACC_PUBLIC)
	SPL_ME(Array, natsort,          NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, natcasesort,      NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, unserialize,     arginfo_array_unserialize,       ZEND_ACC_PUBLIC)
	SPL_ME(Array, serialize,        NULL,                           ZEND_ACC_PUBLIC)
	/* ArrayIterator specific */
	SPL_ME(Array, rewind,           NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, current,          NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, key,              NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, next,             NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, valid,            NULL,                           ZEND_ACC_PUBLIC)
	SPL_ME(Array, seek,             arginfo_array_seek,             ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
}; /* }}} */

static const zend_function_entry spl_funcs_RecursiveArrayIterator[] = { /* {{{ */
	SPL_ME(Array, hasChildren,   NULL, ZEND_ACC_PUBLIC)
	SPL_ME(Array, getChildren,   NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
}; /* }}} */

/* {{{ PHP_MINIT_FUNCTION(spl_array) */
PHP_MINIT_FUNCTION(spl_array)
{
	REGISTER_SPL_STD_CLASS_EX(ArrayObject, spl_array_object_new, spl_funcs_ArrayObject);
	REGISTER_SPL_IMPLEMENTS(ArrayObject, Aggregate);
	REGISTER_SPL_IMPLEMENTS(ArrayObject, ArrayAccess);
	REGISTER_SPL_IMPLEMENTS(ArrayObject, Serializable);
	spl_ce_ArrayObject->serialize   = spl_array_serialize;
	spl_ce_ArrayObject->unserialize = spl_array_unserialize;
	memcpy(&spl_handler_ArrayObject, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	spl_handler_ArrayObject.clone_obj = spl_array_object_clone;
	spl_handler_ArrayObject.read_dimension = spl_array_read_dimension;
	spl_handler_ArrayObject.write_dimension = spl_array_write_dimension;
	spl_handler_ArrayObject.unset_dimension = spl_array_unset_dimension;
	spl_handler_ArrayObject.has_dimension = spl_array_has_dimension;
	spl_handler_ArrayObject.count_elements = spl_array_object_count_elements;

	spl_handler_ArrayObject.get_properties = spl_array_get_properties;
	spl_handler_ArrayObject.get_debug_info = spl_array_get_debug_info;
	spl_handler_ArrayObject.read_property = spl_array_read_property;
	spl_handler_ArrayObject.write_property = spl_array_write_property;
	spl_handler_ArrayObject.get_property_ptr_ptr = spl_array_get_property_ptr_ptr;
	spl_handler_ArrayObject.has_property = spl_array_has_property;
	spl_handler_ArrayObject.unset_property = spl_array_unset_property;

	REGISTER_SPL_STD_CLASS_EX(ArrayIterator, spl_array_object_new, spl_funcs_ArrayIterator);
	REGISTER_SPL_IMPLEMENTS(ArrayIterator, Iterator);
	REGISTER_SPL_IMPLEMENTS(ArrayIterator, ArrayAccess);
	REGISTER_SPL_IMPLEMENTS(ArrayIterator, SeekableIterator);
	REGISTER_SPL_IMPLEMENTS(ArrayIterator, Serializable);
	spl_ce_ArrayIterator->serialize   = spl_array_serialize;
	spl_ce_ArrayIterator->unserialize = spl_array_unserialize;
	memcpy(&spl_handler_ArrayIterator, &spl_handler_ArrayObject, sizeof(zend_object_handlers));
	spl_ce_ArrayIterator->get_iterator = spl_array_get_iterator;
	
	REGISTER_SPL_SUB_CLASS_EX(RecursiveArrayIterator, ArrayIterator, spl_array_object_new, spl_funcs_RecursiveArrayIterator);
	REGISTER_SPL_IMPLEMENTS(RecursiveArrayIterator, RecursiveIterator);
	spl_ce_RecursiveArrayIterator->get_iterator = spl_array_get_iterator;

	REGISTER_SPL_IMPLEMENTS(ArrayObject, Countable);
	REGISTER_SPL_IMPLEMENTS(ArrayIterator, Countable);

	REGISTER_SPL_CLASS_CONST_LONG(ArrayObject,   "STD_PROP_LIST",    SPL_ARRAY_STD_PROP_LIST);
	REGISTER_SPL_CLASS_CONST_LONG(ArrayObject,   "ARRAY_AS_PROPS",   SPL_ARRAY_ARRAY_AS_PROPS);

	REGISTER_SPL_CLASS_CONST_LONG(ArrayIterator, "STD_PROP_LIST",    SPL_ARRAY_STD_PROP_LIST);
	REGISTER_SPL_CLASS_CONST_LONG(ArrayIterator, "ARRAY_AS_PROPS",   SPL_ARRAY_ARRAY_AS_PROPS);

	REGISTER_SPL_CLASS_CONST_LONG(RecursiveArrayIterator, "CHILD_ARRAYS_ONLY", SPL_ARRAY_CHILD_ARRAYS_ONLY);

	return SUCCESS;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
