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
   | Author: Sascha Schumann <sascha@schumann.cx>                         |
   +----------------------------------------------------------------------+
 */

/* $Id: apache_config.c,v 1.19 2009/03/10 23:40:01 helly Exp $ */

#define ZEND_INCLUDE_FULL_WINDOWS_HEADERS

#include "php.h"
#include "php_ini.h"
#include "php_apache.h"

#include "apr_strings.h"
#include "ap_config.h"
#include "util_filter.h"
#include "httpd.h"
#include "http_config.h"
#include "http_request.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_log.h"
#include "http_main.h"
#include "util_script.h"
#include "http_core.h"

#ifdef PHP_AP_DEBUG
#define phpapdebug(a) fprintf a
#else
#define phpapdebug(a)
#endif

typedef struct {
	HashTable config;
} php_conf_rec;

typedef struct {
	char *value;
	size_t value_len;
	char status;
	char htaccess;
} php_dir_entry;

static const char *real_value_hnd(cmd_parms *cmd, void *dummy, const char *name, const char *value, int status)
{
	php_conf_rec *d = dummy;
	php_dir_entry e;

	phpapdebug((stderr, "Getting %s=%s for %p (%d)\n", name, value, dummy, zend_hash_num_elements(&d->config)));
	
	if (!strncasecmp(value, "none", sizeof("none"))) {
		value = "";
	}
	
	e.value = apr_pstrdup(cmd->pool, value);
	e.value_len = strlen(value);
	e.status = status;
	e.htaccess = ((cmd->override & (RSRC_CONF|ACCESS_CONF)) == 0);

	zend_hash_update(&d->config, (char *) name, strlen(name) + 1, &e, sizeof(e), NULL);
	return NULL;
}

static const char *php_apache_value_handler(cmd_parms *cmd, void *dummy, const char *name, const char *value)
{
	return real_value_hnd(cmd, dummy, name, value, PHP_INI_PERDIR);
}

static const char *php_apache_admin_value_handler(cmd_parms *cmd, void *dummy, const char *name, const char *value)
{
	return real_value_hnd(cmd, dummy, name, value, PHP_INI_SYSTEM);
}

static const char *real_flag_hnd(cmd_parms *cmd, void *dummy, const char *arg1, const char *arg2, int status)
{
	char bool_val[2];

	if (!strcasecmp(arg2, "On") || (arg2[0] == '1' && arg2[1] == '\0')) {
		bool_val[0] = '1';
	} else {
		bool_val[0] = '0';
	}
	bool_val[1] = 0;

	return real_value_hnd(cmd, dummy, arg1, bool_val, status);
}

static const char *php_apache_flag_handler(cmd_parms *cmd, void *dummy, const char *name, const char *value)
{
	return real_flag_hnd(cmd, dummy, name, value, PHP_INI_PERDIR);
}

static const char *php_apache_admin_flag_handler(cmd_parms *cmd, void *dummy, const char *name, const char *value)
{
	return real_flag_hnd(cmd, dummy, name, value, PHP_INI_SYSTEM);
}

static const char *php_apache_phpini_set(cmd_parms *cmd, void *mconfig, const char *arg)
{
	if (apache2_php_ini_path_override) {
		return "Only first PHPINIDir directive honored per configuration tree - subsequent ones ignored";
	}
	apache2_php_ini_path_override = ap_server_root_relative(cmd->pool, arg);
	return NULL;
}

static zend_bool should_overwrite_per_dir_entry(HashTable *target_ht, php_dir_entry *new_per_dir_entry, zend_hash_key *hash_key, void *pData)
{
	php_dir_entry *orig_per_dir_entry;

	if (zend_hash_find(target_ht, hash_key->arKey.s, hash_key->nKeyLength, (void **) &orig_per_dir_entry)==FAILURE) {
		return 1; /* does not exist in dest, copy from source */
	}

	if (new_per_dir_entry->status >= orig_per_dir_entry->status) {
		/* use new entry */
		phpapdebug((stderr, "ADDING/OVERWRITING %s (%d vs. %d)\n", hash_key->arKey.s, new_per_dir_entry->status, orig_per_dir_entry->status));
		return 1;
	} else {
		return 0;
	}
}


void *merge_php_config(apr_pool_t *p, void *base_conf, void *new_conf)
{
	php_conf_rec *d = base_conf, *e = new_conf, *n = NULL;
#if STAS_0
	php_dir_entry *pe;
	php_dir_entry *data;
	zstr str;
	uint str_len;
	ulong num_index;
#endif

	n = create_php_config(p, "merge_php_config");
	/* copy old config */
	zend_hash_copy(&n->config, &d->config, NULL, NULL, sizeof(php_dir_entry));
	/* merge new config */
	phpapdebug((stderr, "Merge dir (%p)+(%p)=(%p)\n", base_conf, new_conf, n));
	zend_hash_merge_ex(&n->config, &e->config, NULL, sizeof(php_dir_entry), (merge_checker_func_t) should_overwrite_per_dir_entry, NULL);
#if STAS_0
	for (zend_hash_internal_pointer_reset(&d->config);
			zend_hash_get_current_key_ex(&d->config, &str, &str_len, 
				&num_index, 0, NULL) == HASH_KEY_IS_STRING;
			zend_hash_move_forward(&d->config)) {
		pe = NULL;
		zend_hash_get_current_data(&d->config, (void **) &data);
		if (zend_hash_find(&n->config, str.s, str_len, (void **) &pe) == SUCCESS) {
			if (pe->status >= data->status) continue;
		}
		zend_hash_update(&n->config, str.s, str_len, data, sizeof(*data), NULL);
		phpapdebug((stderr, "ADDING/OVERWRITING %s (%d vs. %d)\n", str.s, data->status, pe?pe->status:-1));
	}
#endif
	return n;
}

char *get_php_config(void *conf, char *name, size_t name_len)
{
	php_conf_rec *d = conf;
	php_dir_entry *pe;
	
	if (zend_hash_find(&d->config, name, name_len, (void **) &pe) == SUCCESS) {
		return pe->value;
	}

	return "";
}

void apply_config(void *dummy)
{
	php_conf_rec *d = dummy;
	zstr str;
	uint str_len;
	php_dir_entry *data;
	
	for (zend_hash_internal_pointer_reset(&d->config);
			zend_hash_get_current_key_ex(&d->config, &str, &str_len, NULL, 0, 
				NULL) == HASH_KEY_IS_STRING;
			zend_hash_move_forward(&d->config)) {
		zend_hash_get_current_data(&d->config, (void **) &data);
		phpapdebug((stderr, "APPLYING (%s)(%s)\n", str.s, data->value));
		if (zend_alter_ini_entry(str.s, str_len, data->value, data->value_len, data->status, data->htaccess?PHP_INI_STAGE_HTACCESS:PHP_INI_STAGE_ACTIVATE) == FAILURE) {
			phpapdebug((stderr, "..FAILED\n"));
		}	
	}
}

const command_rec php_dir_cmds[] =
{
	AP_INIT_TAKE2("php_value", php_apache_value_handler, NULL, OR_OPTIONS, "PHP Value Modifier"),
	AP_INIT_TAKE2("php_flag", php_apache_flag_handler, NULL, OR_OPTIONS, "PHP Flag Modifier"),
	AP_INIT_TAKE2("php_admin_value", php_apache_admin_value_handler, NULL, ACCESS_CONF|RSRC_CONF, "PHP Value Modifier (Admin)"),
	AP_INIT_TAKE2("php_admin_flag", php_apache_admin_flag_handler, NULL, ACCESS_CONF|RSRC_CONF, "PHP Flag Modifier (Admin)"),
	AP_INIT_TAKE1("PHPINIDir", php_apache_phpini_set, NULL, RSRC_CONF, "Directory containing the php.ini file"),
	{NULL}
};

static apr_status_t destroy_php_config(void *data)
{
	php_conf_rec *d = data;

	phpapdebug((stderr, "Destroying config %p\n", data));	
	zend_hash_destroy(&d->config);

	return APR_SUCCESS;
}

void *create_php_config(apr_pool_t *p, char *dummy)
{
	php_conf_rec *newx = (php_conf_rec *) apr_pcalloc(p, sizeof(*newx));

	phpapdebug((stderr, "Creating new config (%p) for %s\n", newx, dummy));
	zend_hash_init(&newx->config, 0, NULL, NULL, 1);
	apr_pool_cleanup_register(p, newx, destroy_php_config, apr_pool_cleanup_null);
	return (void *) newx;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
