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
   | Authors: Zeev Suraski <zeev@zend.com>                                |
   |          Zak Greant <zak@mysql.com>                                  |
   |          Georg Richter <georg@php.net>                               |
   +----------------------------------------------------------------------+
*/
 
/* $Id: php_mysql.c,v 1.278 2009/05/26 14:03:07 andrey Exp $ */

/* TODO:
 *
 * ? Safe mode implementation
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_globals.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/standard/basic_functions.h"
#include "zend_exceptions.h"

#if HAVE_MYSQL

#ifdef PHP_WIN32
# include <winsock2.h>
# define signal(a, b) NULL
#elif defined(NETWARE)
# include <sys/socket.h>
# define signal(a, b) NULL
#else
# if HAVE_SIGNAL_H
#  include <signal.h>
# endif
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# include <netdb.h>
# include <netinet/in.h>
# if HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
#endif

#include "php_ini.h"
#include "php_mysql_structs.h"

/* True globals, no need for thread safety */
static int le_result, le_link, le_plink;

#ifdef HAVE_MYSQL_REAL_CONNECT
# ifdef HAVE_ERRMSG_H
#  include <errmsg.h>
# endif
#endif

#define SAFE_STRING(s) ((s)?(s):"")

#if MYSQL_VERSION_ID > 32199 || defined(MYSQL_USE_MYSQLND)
# define mysql_row_length_type unsigned long
# define HAVE_MYSQL_ERRNO
#else
# define mysql_row_length_type unsigned int
# ifdef mysql_errno
#  define HAVE_MYSQL_ERRNO
# endif
#endif

#if MYSQL_VERSION_ID >= 32032 || defined(MYSQL_USE_MYSQLND)
#define HAVE_GETINFO_FUNCS
#endif

#if MYSQL_VERSION_ID > 32133 || defined(FIELD_TYPE_TINY)
#define MYSQL_HAS_TINY
#endif

#if MYSQL_VERSION_ID >= 32200
#define MYSQL_HAS_YEAR
#endif

#define MYSQL_ASSOC		1<<0
#define MYSQL_NUM		1<<1
#define MYSQL_BOTH		(MYSQL_ASSOC|MYSQL_NUM)

#define MYSQL_USE_RESULT	0
#define MYSQL_STORE_RESULT	1

#if MYSQL_VERSION_ID < 32224
#define PHP_MYSQL_VALID_RESULT(mysql)		\
	(mysql_num_fields(mysql)>0)
#else
#define PHP_MYSQL_VALID_RESULT(mysql)		\
	(mysql_field_count(mysql)>0)
#endif

ZEND_DECLARE_MODULE_GLOBALS(mysql)
static PHP_GINIT_FUNCTION(mysql);

typedef struct _php_mysql_conn {
	MYSQL *conn;
	int active_result_id;
	int multi_query;
} php_mysql_conn;

#ifdef MYSQL_USE_MYSQLND
static MYSQLND_ZVAL_PCACHE *mysql_mysqlnd_zval_cache;
static MYSQLND_QCACHE		*mysql_mysqlnd_qcache;
#endif


#ifdef CLIENT_MULTI_STATEMENTS
#define MYSQL_DISABLE_MQ if (mysql->multi_query) { \
	mysql_set_server_option(mysql->conn, MYSQL_OPTION_MULTI_STATEMENTS_OFF); \
	mysql->multi_query = 0; \
}
#else
#define MYSQL_DISABLE_MQ
#endif


/* {{{ mysql_functions[]
 */
static const zend_function_entry mysql_functions[] = {
	PHP_FE(mysql_connect,								NULL)
	PHP_FE(mysql_pconnect,								NULL)
	PHP_FE(mysql_close,									NULL)
	PHP_FE(mysql_select_db,								NULL)
#ifndef NETWARE		/* The below two functions not supported on NetWare */
#if MYSQL_VERSION_ID < 40000
	PHP_DEP_FE(mysql_create_db,							NULL)
	PHP_DEP_FE(mysql_drop_db,							NULL)
#endif
#endif	/* NETWARE */
	PHP_FE(mysql_query,									NULL)
	PHP_FE(mysql_unbuffered_query,						NULL)
	PHP_FE(mysql_db_query,								NULL)
	PHP_FE(mysql_list_dbs,								NULL)
	PHP_DEP_FE(mysql_list_tables,						NULL)
	PHP_FE(mysql_list_fields,							NULL)
	PHP_FE(mysql_list_processes,						NULL)
	PHP_FE(mysql_error,									NULL)
#ifdef HAVE_MYSQL_ERRNO
	PHP_FE(mysql_errno,									NULL)
#endif
	PHP_FE(mysql_affected_rows,							NULL)
	PHP_FE(mysql_insert_id,								NULL)
	PHP_FE(mysql_result,								NULL)
	PHP_FE(mysql_num_rows,								NULL)
	PHP_FE(mysql_num_fields,							NULL)
	PHP_FE(mysql_fetch_row,								NULL)
	PHP_FE(mysql_fetch_array,							NULL)
	PHP_FE(mysql_fetch_assoc,							NULL)
	PHP_FE(mysql_fetch_object,							NULL)
	PHP_FE(mysql_data_seek,								NULL)
	PHP_FE(mysql_fetch_lengths,							NULL)
	PHP_FE(mysql_fetch_field,							NULL)
	PHP_FE(mysql_field_seek,							NULL)
	PHP_FE(mysql_free_result,							NULL)
	PHP_FE(mysql_field_name,							NULL)
	PHP_FE(mysql_field_table,							NULL)
	PHP_FE(mysql_field_len,								NULL)
	PHP_FE(mysql_field_type,							NULL)
	PHP_FE(mysql_field_flags,							NULL)
	PHP_FE(mysql_escape_string,							NULL)
	PHP_FE(mysql_real_escape_string,					NULL)
	PHP_FE(mysql_stat,									NULL)
	PHP_FE(mysql_thread_id,								NULL)
	PHP_FE(mysql_client_encoding,					NULL)
	PHP_FE(mysql_ping,									NULL)
#ifdef HAVE_GETINFO_FUNCS
	PHP_FE(mysql_get_client_info,						NULL)
	PHP_FE(mysql_get_host_info,							NULL)
	PHP_FE(mysql_get_proto_info,						NULL)
	PHP_FE(mysql_get_server_info,						NULL)
#endif

	PHP_FE(mysql_info,									NULL)
#ifdef MYSQL_HAS_SET_CHARSET
	PHP_FE(mysql_set_charset,							NULL)
#endif
	/* for downwards compatability */
	PHP_FALIAS(mysql,				mysql_db_query,		NULL)
	PHP_FALIAS(mysql_fieldname,		mysql_field_name,	NULL)
	PHP_FALIAS(mysql_fieldtable,	mysql_field_table,	NULL)
	PHP_FALIAS(mysql_fieldlen,		mysql_field_len,	NULL)
	PHP_FALIAS(mysql_fieldtype,		mysql_field_type,	NULL)
	PHP_FALIAS(mysql_fieldflags,	mysql_field_flags,	NULL)
	PHP_FALIAS(mysql_selectdb,		mysql_select_db,	NULL)
#ifndef NETWARE		/* The below two functions not supported on NetWare */
#if MYSQL_VERSION_ID < 40000
	PHP_DEP_FALIAS(mysql_createdb,	mysql_create_db,	NULL)
	PHP_DEP_FALIAS(mysql_dropdb,	mysql_drop_db,		NULL)
#endif
#endif	/* NETWARE */
	PHP_FALIAS(mysql_freeresult,	mysql_free_result,	NULL)
	PHP_FALIAS(mysql_numfields,		mysql_num_fields,	NULL)
	PHP_FALIAS(mysql_numrows,		mysql_num_rows,		NULL)
	PHP_FALIAS(mysql_listdbs,		mysql_list_dbs,		NULL)
	PHP_DEP_FALIAS(mysql_listtables,mysql_list_tables,	NULL)
	PHP_FALIAS(mysql_listfields,	mysql_list_fields,	NULL)
	PHP_FALIAS(mysql_db_name,		mysql_result,		NULL)
	PHP_FALIAS(mysql_dbname,		mysql_result,		NULL)
	PHP_FALIAS(mysql_tablename,		mysql_result,		NULL)
	PHP_FALIAS(mysql_table_name,	mysql_result,		NULL)
	{NULL, NULL, NULL}
};
/* }}} */

/* Dependancies */
static const zend_module_dep mysql_deps[] = {
#if defined(MYSQL_USE_MYSQLND)
	ZEND_MOD_REQUIRED("mysqlnd")
#endif
	{NULL, NULL, NULL}
};

/* {{{ mysql_module_entry
 */
zend_module_entry mysql_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
	STANDARD_MODULE_HEADER_EX, NULL,
	mysql_deps,
#elif ZEND_MODULE_API_NO >= 20010901
 	STANDARD_MODULE_HEADER,
#endif
	"mysql",
	mysql_functions,
	ZEND_MODULE_STARTUP_N(mysql),
	PHP_MSHUTDOWN(mysql),
	PHP_RINIT(mysql),
	PHP_RSHUTDOWN(mysql),
	PHP_MINFO(mysql),
	"1.0",
	PHP_MODULE_GLOBALS(mysql),
	PHP_GINIT(mysql),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_MYSQL
ZEND_GET_MODULE(mysql)
#endif

void timeout(int sig);

#define CHECK_LINK(link) { if (link==-1) { php_error_docref(NULL TSRMLS_CC, E_WARNING, "A link to the server could not be established"); RETURN_FALSE; } }

#if defined(MYSQL_USE_MYSQLND)
#define PHPMY_UNBUFFERED_QUERY_CHECK() \
{\
	if (mysql->active_result_id) { \
		do {					\
			int type;			\
			MYSQL_RES *mysql_result;	\
							\
			mysql_result = (MYSQL_RES *) zend_list_find(mysql->active_result_id, &type);	\
			if (mysql_result && type==le_result) {						\
				if (mysql_result_is_unbuffered(mysql_result) && !mysql_eof(mysql_result)) { \
					php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Function called without first fetching all rows from a previous unbuffered query");	\
				}						\
				zend_list_delete(mysql->active_result_id);	\
				mysql->active_result_id = 0;			\
			} \
		} while(0); \
	}\
}
#else
#define PHPMY_UNBUFFERED_QUERY_CHECK()			\
{							\
	if (mysql->active_result_id) {			\
		do {					\
			int type;			\
			MYSQL_RES *mysql_result;	\
							\
			mysql_result = (MYSQL_RES *) zend_list_find(mysql->active_result_id, &type);	\
			if (mysql_result && type==le_result) {						\
				if (!mysql_eof(mysql_result)) {						\
					php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Function called without first fetching all rows from a previous unbuffered query");	\
					while (mysql_fetch_row(mysql_result));	\
				}						\
				zend_list_delete(mysql->active_result_id);	\
				mysql->active_result_id = 0;			\
			}							\
		} while(0);							\
	}									\
}
#endif

/* {{{ _free_mysql_result
 * This wrapper is required since mysql_free_result() returns an integer, and
 * thus, cannot be used directly
 */
static void _free_mysql_result(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	MYSQL_RES *mysql_result = (MYSQL_RES *)rsrc->ptr;

	mysql_free_result(mysql_result);
	MySG(result_allocated)--;
}
/* }}} */

/* {{{ php_mysql_set_default_link
 */
static void php_mysql_set_default_link(int id TSRMLS_DC)
{
	if (MySG(default_link) != -1) {
		zend_list_delete(MySG(default_link));
	}
	MySG(default_link) = id;
	zend_list_addref(id);
}
/* }}} */

/* {{{ php_mysql_select_db
*/
static int php_mysql_select_db(php_mysql_conn *mysql, char *db TSRMLS_DC)
{
	PHPMY_UNBUFFERED_QUERY_CHECK();

	if (mysql_select_db(mysql->conn, db) != 0) {
		return 0;
	} else {
		return 1;
	}
}
/* }}} */

/* {{{ _close_mysql_link
 */
static void _close_mysql_link(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_mysql_conn *link = (php_mysql_conn *)rsrc->ptr;
	void (*handler) (int); 

	handler = signal(SIGPIPE, SIG_IGN);
	mysql_close(link->conn);
	signal(SIGPIPE, handler);
	efree(link);
	MySG(num_links)--;
}
/* }}} */

/* {{{ _close_mysql_plink
 */
static void _close_mysql_plink(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_mysql_conn *link = (php_mysql_conn *)rsrc->ptr;
	void (*handler) (int);

	handler = signal(SIGPIPE, SIG_IGN);
	mysql_close(link->conn);
	signal(SIGPIPE, handler);

	free(link);
	MySG(num_persistent)--;
	MySG(num_links)--;
}
/* }}} */

/* {{{ PHP_INI_MH
 */
static PHP_INI_MH(OnMySQLPort)
{
	if (new_value != NULL) { /* default port */
		MySG(default_port) = atoi(new_value);
	} else {
		MySG(default_port) = -1;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_INI */
PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("mysql.allow_persistent",	"1",	PHP_INI_SYSTEM,		OnUpdateLong,		allow_persistent,	zend_mysql_globals,		mysql_globals)
	STD_PHP_INI_ENTRY_EX("mysql.max_persistent",	"-1",	PHP_INI_SYSTEM,		OnUpdateLong,		max_persistent,		zend_mysql_globals,		mysql_globals,	display_link_numbers)
	STD_PHP_INI_ENTRY_EX("mysql.max_links",			"-1",	PHP_INI_SYSTEM,		OnUpdateLong,		max_links,			zend_mysql_globals,		mysql_globals,	display_link_numbers)
	STD_PHP_INI_ENTRY("mysql.default_host",			NULL,	PHP_INI_ALL,		OnUpdateString,		default_host,		zend_mysql_globals,		mysql_globals)
	STD_PHP_INI_ENTRY("mysql.default_user",			NULL,	PHP_INI_ALL,		OnUpdateString,		default_user,		zend_mysql_globals,		mysql_globals)
	STD_PHP_INI_ENTRY("mysql.default_password",		NULL,	PHP_INI_ALL,		OnUpdateString,		default_password,	zend_mysql_globals,		mysql_globals)
	PHP_INI_ENTRY("mysql.default_port",				NULL,	PHP_INI_ALL,		OnMySQLPort)
#ifdef MYSQL_UNIX_ADDR
	STD_PHP_INI_ENTRY("mysql.default_socket",		MYSQL_UNIX_ADDR,PHP_INI_ALL,OnUpdateStringUnempty,	default_socket,	zend_mysql_globals,		mysql_globals)
#else
	STD_PHP_INI_ENTRY("mysql.default_socket",		NULL,	PHP_INI_ALL,		OnUpdateStringUnempty,	default_socket,	zend_mysql_globals,		mysql_globals)
#endif
	STD_PHP_INI_ENTRY("mysql.connect_timeout",		"60",	PHP_INI_ALL,		OnUpdateLong,		connect_timeout, 	zend_mysql_globals,		mysql_globals)
	STD_PHP_INI_BOOLEAN("mysql.trace_mode",			"0",	PHP_INI_ALL,		OnUpdateLong,		trace_mode, 		zend_mysql_globals,		mysql_globals)
	STD_PHP_INI_BOOLEAN("mysql.allow_local_infile",	"1",	PHP_INI_SYSTEM,		OnUpdateLong,		allow_local_infile, zend_mysql_globals,		mysql_globals)
#ifdef MYSQL_USE_MYSQLND
	STD_PHP_INI_ENTRY("mysql.cache_size",			"2000",	PHP_INI_SYSTEM,		OnUpdateLong,		cache_size,			zend_mysql_globals,		mysql_globals)
#endif
PHP_INI_END()
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
 */
static PHP_GINIT_FUNCTION(mysql)
{
	mysql_globals->num_persistent = 0;
	mysql_globals->default_socket = NULL;
	mysql_globals->default_host = NULL;
	mysql_globals->default_user = NULL;
	mysql_globals->default_password = NULL;
	mysql_globals->connect_errno = 0;
	mysql_globals->connect_error = NULL;
	mysql_globals->connect_timeout = 0;
	mysql_globals->trace_mode = 0;
	mysql_globals->allow_local_infile = 1;
	mysql_globals->result_allocated = 0;
#ifdef MYSQL_USE_MYSQLND
	mysql_globals->cache_size = 0;
	mysql_globals->mysqlnd_thd_zval_cache = NULL;
#endif
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
ZEND_MODULE_STARTUP_D(mysql)
{
	REGISTER_INI_ENTRIES();
	le_result = zend_register_list_destructors_ex(_free_mysql_result, NULL, "mysql result", module_number);
	le_link = zend_register_list_destructors_ex(_close_mysql_link, NULL, "mysql link", module_number);
	le_plink = zend_register_list_destructors_ex(NULL, _close_mysql_plink, "mysql link persistent", module_number);
	Z_TYPE(mysql_module_entry) = type;

	REGISTER_LONG_CONSTANT("MYSQL_ASSOC", MYSQL_ASSOC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQL_NUM", MYSQL_NUM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQL_BOTH", MYSQL_BOTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQL_CLIENT_COMPRESS", CLIENT_COMPRESS, CONST_CS | CONST_PERSISTENT);
#if MYSQL_VERSION_ID >= 40000	
	REGISTER_LONG_CONSTANT("MYSQL_CLIENT_SSL", CLIENT_SSL, CONST_CS | CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("MYSQL_CLIENT_INTERACTIVE", CLIENT_INTERACTIVE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQL_CLIENT_IGNORE_SPACE", CLIENT_IGNORE_SPACE, CONST_CS | CONST_PERSISTENT); 

#ifndef MYSQL_USE_MYSQLND
#if MYSQL_VERSION_ID >= 40000
	if (mysql_server_init(0, NULL, NULL)) {
		return FAILURE;
	}
#endif
#else
	mysql_mysqlnd_zval_cache = mysqlnd_palloc_init_cache(MySG(cache_size));
	mysql_mysqlnd_qcache = mysqlnd_qcache_init_cache();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(mysql)
{
#ifndef MYSQL_USE_MYSQLND
#if MYSQL_VERSION_ID >= 40000
#ifdef PHP_WIN32
	unsigned long client_ver = mysql_get_client_version();
	/*
	  Can't call mysql_server_end() multiple times prior to 5.0.46 on Windows.
	  PHP bug#41350 MySQL bug#25621
	*/
	if ((client_ver >= 50046 && client_ver < 50100) || client_ver > 50122) {
		mysql_server_end();
	}
#else
	mysql_server_end();
#endif
#endif
#else
	mysqlnd_palloc_free_cache(mysql_mysqlnd_zval_cache);
	mysqlnd_qcache_free_cache_reference(&mysql_mysqlnd_qcache);
#endif

	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(mysql)
{
#if !defined(MYSQL_USE_MYSQLND) && defined(ZTS) && MYSQL_VERSION_ID >= 40000
	if (mysql_thread_init()) {
		return FAILURE;
	}
#endif
	MySG(default_link)=-1;
	MySG(num_links) = MySG(num_persistent);
	/* Reset connect error/errno on every request */
	MySG(connect_error) = NULL;
	MySG(connect_errno) =0;
	MySG(result_allocated) = 0;

#ifdef MYSQL_USE_MYSQLND
	MySG(mysqlnd_thd_zval_cache) = mysqlnd_palloc_rinit(mysql_mysqlnd_zval_cache);
#endif

	return SUCCESS;
}
/* }}} */


#ifdef MYSQL_USE_MYSQLND
static int php_mysql_persistent_helper(zend_rsrc_list_entry *le TSRMLS_DC)
{
	if (le->type == le_plink) {
		mysqlnd_end_psession(((php_mysql_conn *) le->ptr)->conn);
	}
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */
#endif


/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(mysql)
{
#if !defined(MYSQL_USE_MYSQLND) && defined(ZTS) && MYSQL_VERSION_ID >= 40000
	mysql_thread_end();
#endif

	if (MySG(trace_mode)) {
		if (MySG(result_allocated)){
			php_error_docref("function.mysql-free-result" TSRMLS_CC, E_WARNING, "%lu result set(s) not freed. Use mysql_free_result to free result sets which were requested using mysql_query()", MySG(result_allocated));
		}
	}

	if (MySG(connect_error)!=NULL) {
		efree(MySG(connect_error));
	}

#ifdef MYSQL_USE_MYSQLND
	zend_hash_apply(&EG(persistent_list), (apply_func_t) php_mysql_persistent_helper TSRMLS_CC);
	mysqlnd_palloc_rshutdown(MySG(mysqlnd_thd_zval_cache));
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mysql)
{
	char buf[32];

	php_info_print_table_start();
	php_info_print_table_header(2, "MySQL Support", "enabled");
	snprintf(buf, sizeof(buf), "%ld", MySG(num_persistent));
	php_info_print_table_row(2, "Active Persistent Links", buf);
	snprintf(buf, sizeof(buf), "%ld", MySG(num_links));
	php_info_print_table_row(2, "Active Links", buf);
	php_info_print_table_row(2, "Client API version", mysql_get_client_info());
#if !defined (PHP_WIN32) && !defined (NETWARE) && !defined(MYSQL_USE_MYSQLND)
	php_info_print_table_row(2, "MYSQL_MODULE_TYPE", PHP_MYSQL_TYPE);
	php_info_print_table_row(2, "MYSQL_SOCKET", MYSQL_UNIX_ADDR);
	php_info_print_table_row(2, "MYSQL_INCLUDE", PHP_MYSQL_INCLUDE);
	php_info_print_table_row(2, "MYSQL_LIBS", PHP_MYSQL_LIBS);
#endif
#if defined(MYSQL_USE_MYSQLND)
	{
		zval values;

		php_info_print_table_header(2, "Persistent cache", mysql_mysqlnd_zval_cache? "enabled":"disabled");
		
		if (mysql_mysqlnd_zval_cache) {
			/* Now report cache status */
			mysqlnd_palloc_stats(mysql_mysqlnd_zval_cache, &values);
			mysqlnd_minfo_print_hash(&values);
			zval_dtor(&values);
		}
	}
#endif

	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();

}
/* }}} */

/* {{{ php_mysql_do_connect
 */
#define MYSQL_DO_CONNECT_CLEANUP()	\
	if (free_host) {				\
		efree(host);				\
	}

#define MYSQL_DO_CONNECT_RETURN_FALSE()		\
	MYSQL_DO_CONNECT_CLEANUP();				\
	RETURN_FALSE;

#ifdef MYSQL_USE_MYSQLND
#define MYSQL_PORT 0
#endif

static void php_mysql_do_connect(INTERNAL_FUNCTION_PARAMETERS, int persistent)
{
	char *user=NULL, *passwd=NULL, *host_and_port=NULL, *socket=NULL, *tmp=NULL, *host=NULL;
	int  user_len, passwd_len, host_len;
	char *hashed_details=NULL;
	int hashed_details_length, port = MYSQL_PORT;
	long client_flags = 0;
	php_mysql_conn *mysql=NULL;
#if !defined(MYSQL_USE_MYSQLND) && !defined(MYSQL_HASH_SET_CHARSET)
	char *encoding;
#endif
#if MYSQL_VERSION_ID <= 32230
	void (*handler) (int);
#endif
	zend_bool free_host=0, new_link=0;
	long connect_timeout;

#if !defined(MYSQL_USE_MYSQLND)
	if ((MYSQL_VERSION_ID / 100) != (mysql_get_client_version() / 100)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Headers and client library minor version mismatch. Headers:%d Library:%ld",
						MYSQL_VERSION_ID, mysql_get_client_version());
	}
#endif

	connect_timeout = MySG(connect_timeout);

	socket = MySG(default_socket);

	if (MySG(default_port) < 0) {
#if !defined(PHP_WIN32) && !defined(NETWARE)
		struct servent *serv_ptr;
		char *env;
		
		MySG(default_port) = MYSQL_PORT;
		if ((serv_ptr = getservbyname("mysql", "tcp"))) {
			MySG(default_port) = (uint) ntohs((ushort) serv_ptr->s_port);
		}
		if ((env = getenv("MYSQL_TCP_PORT"))) {
			MySG(default_port) = (uint) atoi(env);
		}
#else
		MySG(default_port) = MYSQL_PORT;
#endif
	}
	
	if (PG(sql_safe_mode)) {
		if (ZEND_NUM_ARGS()>0) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "SQL safe mode in effect - ignoring host/user/password information");
		}
		host_and_port=passwd=NULL;
		user=php_get_current_user();
		hashed_details_length = spprintf(&hashed_details, 0, "mysql__%s_", user);
		client_flags = CLIENT_INTERACTIVE;
	} else {
		host_and_port = MySG(default_host);
		user = MySG(default_user);
		passwd = MySG(default_password);

		/* mysql_pconnect does not support new_link parameter */
		if (persistent) {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s&s&s&l", &host_and_port, &host_len, UG(utf8_conv),
									&user, &user_len, UG(utf8_conv), &passwd, &passwd_len, UG(utf8_conv),
									&client_flags)==FAILURE) {
				return;
			}
		} else {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s&s&s&bl", &host_and_port, &host_len, UG(utf8_conv),
									&user, &user_len, UG(utf8_conv), &passwd, &passwd_len, UG(utf8_conv),
									&new_link, &client_flags)==FAILURE) {
				return;
			}
		}

		/* disable local infile option for open_basedir */
		if (((PG(open_basedir) && PG(open_basedir)[0] != '\0')) && (client_flags & CLIENT_LOCAL_FILES)) {
			client_flags ^= CLIENT_LOCAL_FILES;
		}

#ifdef CLIENT_MULTI_STATEMENTS
		client_flags &= ~CLIENT_MULTI_STATEMENTS;   /* don't allow multi_queries via connect parameter */
#endif
		hashed_details_length = spprintf(&hashed_details, 0, "mysql_%s_%s_%s_%ld", SAFE_STRING(host_and_port), SAFE_STRING(user), SAFE_STRING(passwd), client_flags);
	}

	/* We cannot use mysql_port anymore in windows, need to use
	 * mysql_real_connect() to set the port.
	 */
	if (host_and_port && (tmp=strchr(host_and_port, ':'))) {
		host = estrndup(host_and_port, tmp-host_and_port);
		free_host = 1;
		tmp++;
		if (tmp[0] != '/') {
			port = atoi(tmp);
			if ((tmp=strchr(tmp, ':'))) {
				tmp++;
				socket=tmp;
			}
		} else {
			socket = tmp;
		}
	} else {
		host = host_and_port;
		port = MySG(default_port);
	}

#if MYSQL_VERSION_ID < 32200
	mysql_port = port;
#endif

	if (!MySG(allow_persistent)) {
		persistent=0;
	}
	if (persistent) {
		zend_rsrc_list_entry *le;

		/* try to find if we already have this link in our persistent list */
		if (zend_hash_find(&EG(persistent_list), hashed_details, hashed_details_length+1, (void **) &le)==FAILURE) {  /* we don't */
			zend_rsrc_list_entry new_le;

			if (MySG(max_links) != -1 && MySG(num_links) >= MySG(max_links)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Too many open links (%ld)", MySG(num_links));
				efree(hashed_details);
				MYSQL_DO_CONNECT_RETURN_FALSE();
			}
			if (MySG(max_persistent) != -1 && MySG(num_persistent) >= MySG(max_persistent)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Too many open persistent links (%ld)", MySG(num_persistent));
				efree(hashed_details);
				MYSQL_DO_CONNECT_RETURN_FALSE();
			}
			/* create the link */
			mysql = (php_mysql_conn *) malloc(sizeof(php_mysql_conn));
			mysql->active_result_id = 0;
#ifdef CLIENT_MULTI_STATEMENTS
			mysql->multi_query = client_flags & CLIENT_MULTI_STATEMENTS? 1:0;
#else
			mysql->multi_query = 0;
#endif
#ifndef MYSQL_USE_MYSQLND
			mysql->conn = mysql_init(NULL);
#else
			mysql->conn = mysql_init(persistent);
#endif
			mysql_options(mysql->conn, MYSQL_SET_CHARSET_NAME, "utf8");

			if (connect_timeout != -1) {
				mysql_options(mysql->conn, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&connect_timeout);
			}
#ifndef MYSQL_USE_MYSQLND
			if (mysql_real_connect(mysql->conn, host, user, passwd, NULL, port, socket, client_flags)==NULL)
#else
			if (mysqlnd_connect(mysql->conn, host, user, passwd, 0, NULL, 0, 
								port, socket, client_flags, MySG(mysqlnd_thd_zval_cache) TSRMLS_CC) == NULL)
#endif
			{
				/* Populate connect error globals so that the error functions can read them */
				if (MySG(connect_error) != NULL) {
					efree(MySG(connect_error));
				}
				MySG(connect_error) = estrdup(mysql_error(mysql->conn));
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", MySG(connect_error));
#if defined(HAVE_MYSQL_ERRNO)
				MySG(connect_errno) = mysql_errno(mysql->conn);
#endif
				free(mysql);
				efree(hashed_details);
				MYSQL_DO_CONNECT_RETURN_FALSE();
			}
			mysql_options(mysql->conn, MYSQL_OPT_LOCAL_INFILE, (char *)&MySG(allow_local_infile));

#if !defined(MYSQL_USE_MYSQLND)
#ifdef MYSQL_HAS_SET_CHARSET
			mysql_set_character_set(mysql->conn, "utf8");
#else
			encoding = mysql_character_set_name(mysql->conn);
			if (strcasecmp((char*)encoding, "utf8")) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't connect in Unicode mode. Client library was compiled with default charset %s", encoding);
				MYSQL_DO_CONNECT_RETURN_FALSE();
			}	
#endif
#endif

			/* hash it up */
			Z_TYPE(new_le) = le_plink;
			new_le.ptr = mysql;
			if (zend_hash_update(&EG(persistent_list), hashed_details, hashed_details_length+1, (void *) &new_le, sizeof(zend_rsrc_list_entry), NULL)==FAILURE) {
				free(mysql);
				efree(hashed_details);
				MYSQL_DO_CONNECT_RETURN_FALSE();
			}
			MySG(num_persistent)++;
			MySG(num_links)++;
		} else {  /* The link is in our list of persistent connections */
			if (Z_TYPE_P(le) != le_plink) {
				MYSQL_DO_CONNECT_RETURN_FALSE();
			}
			mysql = (php_mysql_conn *) le->ptr;
			mysql->active_result_id = 0;
#ifdef CLIENT_MULTI_STATEMENTS
			mysql->multi_query = client_flags & CLIENT_MULTI_STATEMENTS? 1:0;
#else
			mysql->multi_query = 0;
#endif
			/* ensure that the link did not die */
#if defined(MYSQL_USE_MYSQLND)
			mysqlnd_end_psession(mysql->conn);
#endif
			if (mysql_ping(mysql->conn)) {
				if (mysql_errno(mysql->conn) == 2006) {
					mysql_options(mysql->conn, MYSQL_SET_CHARSET_NAME, "utf8");
#ifndef MYSQL_USE_MYSQLND
					if (mysql_real_connect(mysql->conn, host, user, passwd, NULL, port, socket, client_flags)==NULL)
#else
					if (mysqlnd_connect(mysql->conn, host, user, passwd, 0, NULL, 0, 
										port, socket, client_flags, MySG(mysqlnd_thd_zval_cache) TSRMLS_CC) == NULL)
#endif
					{
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Link to server lost, unable to reconnect");
						zend_hash_del(&EG(persistent_list), hashed_details, hashed_details_length+1);
						efree(hashed_details);
						MYSQL_DO_CONNECT_RETURN_FALSE();
					}
#if !defined(MYSQL_USE_MYSQLND)
#ifdef MYSQL_HAS_SET_CHARSET
					mysql_set_character_set(mysql->conn, "utf8");
#else
					if (strcasecmp((char*)encoding, "utf8")) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't connect in Unicode mode. Client library was compiled with default charset %s", encoding);
						MYSQL_DO_CONNECT_RETURN_FALSE();
					}
#endif
#endif
					mysql_options(mysql->conn, MYSQL_OPT_LOCAL_INFILE, (char *)&MySG(allow_local_infile));
				}
			} else {
#ifdef MYSQL_USE_MYSQLND
				mysqlnd_restart_psession(mysql->conn, MySG(mysqlnd_thd_zval_cache));
#endif
			}
		}
		ZEND_REGISTER_RESOURCE(return_value, mysql, le_plink);
	} else { /* non persistent */
		zend_rsrc_list_entry *index_ptr, new_index_ptr;
		
		/* first we check the hash for the hashed_details key.  if it exists,
		 * it should point us to the right offset where the actual mysql link sits.
		 * if it doesn't, open a new mysql link, add it to the resource list,
		 * and add a pointer to it with hashed_details as the key.
		 */
		if (!new_link && zend_hash_find(&EG(regular_list), hashed_details, hashed_details_length+1,(void **) &index_ptr)==SUCCESS) {
			int type;
			long link;
			void *ptr;

			if (Z_TYPE_P(index_ptr) != le_index_ptr) {
				MYSQL_DO_CONNECT_RETURN_FALSE();
			}
			link = (long) index_ptr->ptr;
			ptr = zend_list_find(link,&type);   /* check if the link is still there */
			if (ptr && (type==le_link || type==le_plink)) {
				zend_list_addref(link);
				Z_LVAL_P(return_value) = link;
				php_mysql_set_default_link(link TSRMLS_CC);
				Z_TYPE_P(return_value) = IS_RESOURCE;
				efree(hashed_details);
				MYSQL_DO_CONNECT_CLEANUP();
				return;
			} else {
				zend_hash_del(&EG(regular_list), hashed_details, hashed_details_length+1);
			}
		}
		if (MySG(max_links) != -1 && MySG(num_links) >= MySG(max_links)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Too many open links (%ld)", MySG(num_links));
			efree(hashed_details);
			MYSQL_DO_CONNECT_RETURN_FALSE();
		}

		mysql = (php_mysql_conn *) emalloc(sizeof(php_mysql_conn));
		mysql->active_result_id = 0;
#ifdef CLIENT_MULTI_STATEMENTS
		mysql->multi_query = 1;
#else
		mysql->multi_query = 0;
#endif
#ifndef MYSQL_USE_MYSQLND
		mysql->conn = mysql_init(NULL);
#else
		mysql->conn = mysql_init(persistent);
#endif

		mysql_options(mysql->conn, MYSQL_SET_CHARSET_NAME, "utf8");

		if (connect_timeout != -1) {
			mysql_options(mysql->conn, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&connect_timeout);
		}

#ifndef MYSQL_USE_MYSQLND
		if (mysql_real_connect(mysql->conn, host, user, passwd, NULL, port, socket, client_flags)==NULL) 
#else
		if (mysqlnd_connect(mysql->conn, host, user, passwd, 0, NULL, 0, 
							port, socket, client_flags, MySG(mysqlnd_thd_zval_cache) TSRMLS_CC) == NULL)
#endif
		{
			/* Populate connect error globals so that the error functions can read them */
			if (MySG(connect_error) != NULL) {
				efree(MySG(connect_error));
			}
			MySG(connect_error) = estrdup(mysql_error(mysql->conn));
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", MySG(connect_error));
#if defined(HAVE_MYSQL_ERRNO)
			MySG(connect_errno) = mysql_errno(mysql->conn);
#endif
			efree(hashed_details);
			/* free mysql structure */
#ifdef MYSQL_USE_MYSQLND
			mysqlnd_close(mysql->conn, MYSQLND_CLOSE_DISCONNECTED);
#endif
			efree(mysql);
			MYSQL_DO_CONNECT_RETURN_FALSE();
		}

#if !defined(MYSQL_USE_MYSQLND)
#ifdef MYSQL_HAS_SET_CHARSET
		mysql_set_character_set(mysql->conn, "utf8");
#else
		if (strcasecmp((char*)encoding, "utf8")) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't connect in Unicode mode. Client library was compiled with default charset %s", encoding);
			MYSQL_DO_CONNECT_RETURN_FALSE();
		}
#endif
#endif
		mysql_options(mysql->conn, MYSQL_OPT_LOCAL_INFILE, (char *)&MySG(allow_local_infile));

		/* add it to the list */
		ZEND_REGISTER_RESOURCE(return_value, mysql, le_link);

		/* add it to the hash */
		new_index_ptr.ptr = (void *) Z_LVAL_P(return_value);
		Z_TYPE(new_index_ptr) = le_index_ptr;
		if (zend_hash_update(&EG(regular_list), hashed_details, hashed_details_length+1,(void *) &new_index_ptr, sizeof(zend_rsrc_list_entry), NULL)==FAILURE) {
			efree(hashed_details);
			MYSQL_DO_CONNECT_RETURN_FALSE();
		}
		MySG(num_links)++;
	}

	efree(hashed_details);
	php_mysql_set_default_link(Z_LVAL_P(return_value) TSRMLS_CC);
	MYSQL_DO_CONNECT_CLEANUP();
}
/* }}} */

/* {{{ php_mysql_get_default_link
 */
static int php_mysql_get_default_link(INTERNAL_FUNCTION_PARAMETERS)
{
	if (MySG(default_link)==-1) { /* no link opened yet, implicitly open one */
		ht = 0;
		php_mysql_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
	}
	return MySG(default_link);
}
/* }}} */

/* {{{ proto resource mysql_connect([string hostname[:port][:/path/to/socket] [, string username [, string password [, bool new [, int flags]]]]]) U
   Opens a connection to a MySQL Server */
PHP_FUNCTION(mysql_connect)
{
	php_mysql_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto resource mysql_pconnect([string hostname[:port][:/path/to/socket] [, string username [, string password [, int flags]]]]) U
   Opens a persistent connection to a MySQL Server */
PHP_FUNCTION(mysql_pconnect)
{
	php_mysql_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto bool mysql_close([int link_identifier]) U
   Close a MySQL connection */
PHP_FUNCTION(mysql_close)
{
	zval *mysql_link=NULL;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (mysql_link) {
		ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, -1, "MySQL-Link", le_link, le_plink);
	} else {
		ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, NULL, MySG(default_link), "MySQL-Link", le_link, le_plink);
	}

	if (mysql_link) { /* explicit resource number */
		PHPMY_UNBUFFERED_QUERY_CHECK();
		zend_list_delete(Z_RESVAL_P(mysql_link));
	}

	if (!mysql_link 
		|| (mysql_link && Z_RESVAL_P(mysql_link)==MySG(default_link))) {
		PHPMY_UNBUFFERED_QUERY_CHECK();
		zend_list_delete(MySG(default_link));
		MySG(default_link) = -1;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool mysql_select_db(string database_name [, int link_identifier]) U
   Selects a MySQL database */
PHP_FUNCTION(mysql_select_db)
{
	zval *mysql_link;
	char *db;
	int id=-1, db_len;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s&|r", &db, &db_len, UG(utf8_conv), &mysql_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 1) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);
	
	if (php_mysql_select_db(mysql, db TSRMLS_CC)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

#ifdef HAVE_GETINFO_FUNCS

/* {{{ proto string mysql_get_client_info(void) U
   Returns a string that represents the client library version */
PHP_FUNCTION(mysql_get_client_info)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_UTF8_STRING((char *)mysql_get_client_info(), ZSTR_DUPLICATE);
}
/* }}} */

/* {{{ proto string mysql_get_host_info([int link_identifier]) U
   Returns a string describing the type of connection in use, including the server host name */
PHP_FUNCTION(mysql_get_host_info)
{
	zval *mysql_link = NULL;
	int id = -1;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (!mysql_link) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	RETURN_UTF8_STRING((char *)mysql_get_host_info(mysql->conn), ZSTR_DUPLICATE);
}
/* }}} */

/* {{{ proto int mysql_get_proto_info([int link_identifier]) U
   Returns the protocol version used by current connection */
PHP_FUNCTION(mysql_get_proto_info)
{
	zval *mysql_link = NULL;
	int id = -1;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (!mysql_link) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	RETURN_LONG(mysql_get_proto_info(mysql->conn));
}
/* }}} */

/* {{{ proto string mysql_get_server_info([int link_identifier]) U
   Returns a string that represents the server version number */
PHP_FUNCTION(mysql_get_server_info)
{
	zval *mysql_link = NULL;
	int id = -1;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (!mysql_link) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	RETURN_UTF8_STRING((char *)mysql_get_server_info(mysql->conn), ZSTR_DUPLICATE);
}
/* }}} */

/* {{{ proto string mysql_info([int link_identifier]) U
   Returns a string containing information about the most recent query */
PHP_FUNCTION(mysql_info)
{
	zval *mysql_link = NULL;
	int id = -1;
	char *str;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 0) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	if ((str = (char *)mysql_info(mysql->conn))) {
		RETURN_UTF8_STRING(str,ZSTR_DUPLICATE);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int mysql_thread_id([int link_identifier]) U
	Returns the thread id of current connection */
PHP_FUNCTION(mysql_thread_id)
{
	zval *mysql_link = NULL;
	int  id = -1;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 0) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}
	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	RETURN_LONG((long) mysql_thread_id(mysql->conn));
}
/* }}} */

/* {{{ proto string mysql_stat([int link_identifier]) U
	Returns a string containing status information */
PHP_FUNCTION(mysql_stat)
{
	zval *mysql_link = NULL;
	int id = -1;
	php_mysql_conn *mysql;
	char *stat;
#ifdef MYSQL_USE_MYSQLND
	uint stat_len;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 0) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}
	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	PHPMY_UNBUFFERED_QUERY_CHECK();
#ifndef MYSQL_USE_MYSQLND
	if ((stat = (char *)mysql_stat(mysql->conn))) {
		RETURN_UTF8_STRING(stat, ZSTR_DUPLICATE);
#else
	if (mysqlnd_stat(mysql->conn, &stat, &stat_len) == PASS) {
		RETURN_UTF8_STRINGL(stat, stat_len, ZSTR_AUTOFREE);
#endif
	} else {
		RETURN_FALSE;
	}

}
/* }}} */

/* {{{ proto string mysql_client_encoding([int link_identifier]) U
	Returns the default character set for the current connection */
PHP_FUNCTION(mysql_client_encoding)
{
	zval *mysql_link = NULL;
	int id = -1;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 0) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	RETURN_UTF8_STRING((char *)mysql_character_set_name(mysql->conn), ZSTR_DUPLICATE);
}
/* }}} */
#endif

#ifdef MYSQL_HAS_SET_CHARSET
/* {{{ proto bool mysql_set_charset(string csname [, int link_identifier]) U
   sets client character set */
PHP_FUNCTION(mysql_set_charset)
{
	zval *mysql_link = NULL;
	char *csname;
	int id = -1, csname_len;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s&|r", &csname, &csname_len, UG(utf8_conv), &mysql_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 1) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	/* Only allow the use of this function with unicode.semantics=On */
	if (csname_len != 4  || strncasecmp(csname, "utf8", 4)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Character set %s is not supported when running PHP with unicode.semantics=On.", csname);
		RETURN_FALSE;
	}

	if (!mysql_set_character_set(mysql->conn, csname)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */
#endif

#ifndef NETWARE		/* The below two functions not supported on NetWare */
#if MYSQL_VERSION_ID < 40000
/* {{{ proto bool mysql_create_db(string database_name [, int link_identifier]) U
   Create a MySQL database */
PHP_FUNCTION(mysql_create_db)
{
	zval *mysql_link;
	int id = -1, db_len;
	char *db;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s&|r", &db, &db_len, UG(utf8_conv), &mysql_link) == FAILURE) {
		RETURN_FALSE;
	}

	if (ZEND_NUM_ARGS() == 1) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	PHPMY_UNBUFFERED_QUERY_CHECK();

	if (mysql_create_db(mysql->conn, db)==0) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool mysql_drop_db(string database_name [, int link_identifier]) U
   Drops (delete) a MySQL database */
PHP_FUNCTION(mysql_drop_db)
{
	zval *mysql_link;
	char *db;
	int id = -1, db_len;
	php_mysql_conn *mysql;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s&|r", &db, &db_len, UG(utf8_conv), &mysql_link) == FAILURE) {
		RETURN_FALSE;
	}

	if (ZEND_NUM_ARGS() == 1) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);
	
	if (mysql_drop_db(mysql->conn, db)==0) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */
#endif
#endif	/* NETWARE */

/* {{{ php_mysql_do_query_general
 */
static void php_mysql_do_query_general(char *query, zval **mysql_link, int link_id, char *db, int use_store, zval *return_value TSRMLS_DC)
{
	php_mysql_conn *mysql;
	MYSQL_RES *mysql_result;
	
	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, mysql_link, link_id, "MySQL-Link", le_link, le_plink);
	
	if (db) {
		if (!php_mysql_select_db(mysql, db TSRMLS_CC)) {
			RETURN_FALSE;
		}
	}

	PHPMY_UNBUFFERED_QUERY_CHECK();

	MYSQL_DISABLE_MQ;
#ifndef MYSQL_USE_MYSQLND
	/* check explain */
	if (MySG(trace_mode)) {
		if (!strncasecmp("select", query, 6)){
			MYSQL_ROW 	row;
			
			char *newquery;
			spprintf(&newquery, 0, "EXPLAIN %s", query);
			mysql_real_query(mysql->conn, newquery, strlen(newquery));
			efree (newquery);
			if (mysql_errno(mysql->conn)) {
				php_error_docref("http://www.mysql.com/doc" TSRMLS_CC, E_WARNING, "%s", mysql_error(mysql->conn));
				RETURN_FALSE;
			}
			else {
				mysql_result = mysql_use_result(mysql->conn);
				while ((row = mysql_fetch_row(mysql_result))) {
					if (!strcmp("ALL", row[1])) {
						php_error_docref("http://www.mysql.com/doc" TSRMLS_CC, E_WARNING, "Your query requires a full tablescan (table %s, %s rows affected). Use EXPLAIN to optimize your query", row[0], row[6]);
					} else if (!strcmp("INDEX", row[1])) {
						php_error_docref("http://www.mysql.com/doc" TSRMLS_CC, E_WARNING, "Your query requires a full indexscan (table %s, %s rows affected). Use EXPLAIN to optimize your query", row[0], row[6]);
					}
				}
				mysql_free_result(mysql_result);
			}
		}	
	} /* end explain */
#endif

	/* mysql_query is binary unsafe, use mysql_real_query */
#if MYSQL_VERSION_ID > 32199
	if (mysql_real_query(mysql->conn, query, strlen(query))!=0) {
		/* check possible error */
		if (MySG(trace_mode)){
			if (mysql_errno(mysql->conn)){
				php_error_docref("http://www.mysql.com/doc" TSRMLS_CC, E_WARNING, "%s", mysql_error(mysql->conn));
			}
		}
		RETURN_FALSE;
	}
#else
	if (mysql_query(mysql->conn, query)!=0) {
		/* check possible error */
		if (MySG(trace_mode)){
			if (mysql_errno(mysql->conn)){
				php_error_docref("http://www.mysql.com/doc" TSRMLS_CC, E_WARNING, "%s", mysql_error(mysql->conn));
			}
		}
		RETURN_FALSE;
	}
#endif
	if(use_store == MYSQL_USE_RESULT) {
		mysql_result=mysql_use_result(mysql->conn);
	} else {
		mysql_result=mysql_store_result(mysql->conn);
	}
	if (!mysql_result) {
		if (PHP_MYSQL_VALID_RESULT(mysql->conn)) { /* query should have returned rows */
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to save result set");
			RETURN_FALSE;
		} else {
			RETURN_TRUE;
		}
	}
	MySG(result_allocated)++;
	ZEND_REGISTER_RESOURCE(return_value, mysql_result, le_result);
	if (use_store == MYSQL_USE_RESULT) {
		mysql->active_result_id = Z_LVAL_P(return_value);
	}
}
/* }}} */

/* {{{ php_mysql_do_query
 */
static void php_mysql_do_query(INTERNAL_FUNCTION_PARAMETERS, int use_store)
{
	zval	*mysql_link;
	char	*query;
	int		id=-1, query_len;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s&|r", &query, &query_len, UG(utf8_conv), &mysql_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 1) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}
	php_mysql_do_query_general(query, &mysql_link, id, NULL, use_store, return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto resource mysql_query(string query [, int link_identifier]) U
   Sends an SQL query to MySQL */
PHP_FUNCTION(mysql_query)
{
	php_mysql_do_query(INTERNAL_FUNCTION_PARAM_PASSTHRU, MYSQL_STORE_RESULT);
}
/* }}} */


/* {{{ proto resource mysql_unbuffered_query(string query [, int link_identifier]) U
   Sends an SQL query to MySQL, without fetching and buffering the result rows */
PHP_FUNCTION(mysql_unbuffered_query)
{
	php_mysql_do_query(INTERNAL_FUNCTION_PARAM_PASSTHRU, MYSQL_USE_RESULT);
}
/* }}} */


/* {{{ proto resource mysql_db_query(string database_name, string query [, int link_identifier]) U
   Sends an SQL query to MySQL */
PHP_FUNCTION(mysql_db_query)
{
	zval *mysql_link;
	char *db, *query;
	int id = -1, db_len, query_len;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s&s&|r", &db, &db_len, UG(utf8_conv), &query, &query_len, UG(utf8_conv), &mysql_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 2) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	php_error_docref(NULL TSRMLS_CC, E_DEPRECATED, "use mysql_query() instead");
	
	php_mysql_do_query_general(query, &mysql_link, id, db, MYSQL_STORE_RESULT, return_value TSRMLS_CC);
}
/* }}} */


/* {{{ proto resource mysql_list_dbs([int link_identifier]) U
   List databases available on a MySQL server */
PHP_FUNCTION(mysql_list_dbs)
{
	zval *mysql_link = NULL;
	int id = -1;
	php_mysql_conn *mysql;
	MYSQL_RES *mysql_result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (!mysql_link) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	PHPMY_UNBUFFERED_QUERY_CHECK();

	if ((mysql_result=mysql_list_dbs(mysql->conn, NULL))==NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to save MySQL query result");
		RETURN_FALSE;
	}
	ZEND_REGISTER_RESOURCE(return_value, mysql_result, le_result);
}
/* }}} */


/* {{{ proto resource mysql_list_tables(string database_name [, int link_identifier]) U
   List tables in a MySQL database */
PHP_FUNCTION(mysql_list_tables)
{
	zval *mysql_link = NULL;
	char *db;
	int id = -1, db_len;
	php_mysql_conn *mysql;
	MYSQL_RES *mysql_result;


	switch(ZEND_NUM_ARGS()) {
		case 1:
			if (zend_parse_parameters(1 TSRMLS_CC, "s&", &db, &db_len, UG(utf8_conv)) == FAILURE) {
				RETURN_FALSE;
			}
			id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
			CHECK_LINK(id);
			break;
		case 2:
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s&r", &db, &db_len, UG(utf8_conv), &mysql_link) == FAILURE) {
				RETURN_FALSE;
			}
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	if (!php_mysql_select_db(mysql, db TSRMLS_CC)) {
		RETURN_FALSE;
	}

	PHPMY_UNBUFFERED_QUERY_CHECK();

	if ((mysql_result=mysql_list_tables(mysql->conn, NULL))==NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to save MySQL query result");
		RETURN_FALSE;
	}
	ZEND_REGISTER_RESOURCE(return_value, mysql_result, le_result);
}
/* }}} */


/* {{{ proto resource mysql_list_fields(string database_name, string table_name [, int link_identifier]) U
   List MySQL result fields */
PHP_FUNCTION(mysql_list_fields)
{
	zval *mysql_link;
	char *db, *table;
	int id = -1, db_len, table_len;
	php_mysql_conn *mysql;
	MYSQL_RES *mysql_result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s&s&|r", &db, &db_len, UG(utf8_conv), &table, &table_len, UG(utf8_conv), &mysql_link) == FAILURE) {
		RETURN_FALSE;
	}

	if (ZEND_NUM_ARGS() < 3) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	if (!php_mysql_select_db(mysql, db TSRMLS_CC)) {
		RETURN_FALSE;
	}

	PHPMY_UNBUFFERED_QUERY_CHECK();

	if ((mysql_result=mysql_list_fields(mysql->conn, table, NULL))==NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to save MySQL query result");
		RETURN_FALSE;
	}
	ZEND_REGISTER_RESOURCE(return_value, mysql_result, le_result);
}
/* }}} */

/* {{{ proto resource mysql_list_processes([int link_identifier]) U
	Returns a result set describing the current server threads */
PHP_FUNCTION(mysql_list_processes)
{
	zval *mysql_link = NULL;
	int id = -1;
	php_mysql_conn *mysql;
	MYSQL_RES *mysql_result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 0) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	PHPMY_UNBUFFERED_QUERY_CHECK();

	mysql_result = mysql_list_processes(mysql->conn);
	if (mysql_result == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to save MySQL query result");
		RETURN_FALSE;
	}

	ZEND_REGISTER_RESOURCE(return_value, mysql_result, le_result);
}
/* }}} */


/* {{{ proto string mysql_error([int link_identifier]) U
   Returns the text of the error message from previous MySQL operation */
PHP_FUNCTION(mysql_error)
{
	zval *mysql_link = NULL;
	int id = -1;
	php_mysql_conn *mysql;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (!mysql_link) {
		id = MySG(default_link);
		if (id==-1) {
			if (MySG(connect_error)!=NULL){
				RETURN_STRING(MySG(connect_error),1);
			} else {
				RETURN_FALSE;
			}
		}
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);
	
	RETURN_UTF8_STRING((char *)mysql_error(mysql->conn), ZSTR_DUPLICATE);
}
/* }}} */


/* {{{ proto int mysql_errno([int link_identifier])
   Returns the number of the error message from previous MySQL operation */
#ifdef HAVE_MYSQL_ERRNO
PHP_FUNCTION(mysql_errno)
{
	zval *mysql_link = NULL;
	int id = -1;
	php_mysql_conn *mysql;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}
	
	if (!mysql_link) {
		id = MySG(default_link);
		if (id==-1) {
			if (MySG(connect_errno)!=0){
				RETURN_LONG(MySG(connect_errno));
			} else {
				RETURN_FALSE;
			}
		}
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	RETURN_LONG(mysql_errno(mysql->conn));
}
#endif
/* }}} */


/* {{{ proto int mysql_affected_rows([int link_identifier])
   Gets number of affected rows in previous MySQL operation */
PHP_FUNCTION(mysql_affected_rows)
{
	zval *mysql_link = NULL;
	int id = -1;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (!mysql_link) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	/* conversion from int64 to long happing here */
	Z_LVAL_P(return_value) = (long) mysql_affected_rows(mysql->conn);
	Z_TYPE_P(return_value) = IS_LONG;
}
/* }}} */


/* {{{ proto string mysql_escape_string(string to_be_escaped) U
   Escape string for mysql query */
PHP_FUNCTION(mysql_escape_string)
{
	char *str, *new_str;
	int  str_len, new_str_len;


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s&", &str, &str_len, UG(utf8_conv)) == FAILURE) {
		return;
	}

	new_str = (char *) safe_emalloc(str_len, 2, 1);
	new_str_len = mysql_escape_string(new_str, str, str_len);
	/* Now we kind of realloc() by returning a zval pointing to a buffer with enough size */
	RETVAL_UTF8_STRINGL(new_str, new_str_len, ZSTR_DUPLICATE);
	efree(new_str);
	if (MySG(trace_mode)){
		php_error_docref("function.mysql-real-escape-string" TSRMLS_CC, E_DEPRECATED, "use mysql_real_escape_string() instead.");
	}
}
/* }}} */

/* {{{ proto string mysql_real_escape_string(string to_be_escaped [, int link_identifier]) U
	Escape special characters in a string for use in a SQL statement, taking into account the current charset of the connection */
PHP_FUNCTION(mysql_real_escape_string)
{
	zval *mysql_link = NULL;
	char *str;
	char *new_str;
	int id = -1, str_len, new_str_len;
	php_mysql_conn *mysql;


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s&|r", &str, &str_len, UG(utf8_conv), &mysql_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 1) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	new_str = safe_emalloc(str_len, 2, 1);
	new_str_len = mysql_real_escape_string(mysql->conn, new_str, str, str_len);
	/* Now we kind of realloc() by returning a zval pointing to a buffer with enough size */
	RETVAL_UTF8_STRINGL(new_str, new_str_len, ZSTR_DUPLICATE);
	efree(new_str);
}
/* }}} */

/* {{{ proto int mysql_insert_id([int link_identifier]) U
   Gets the ID generated from the previous INSERT operation */
PHP_FUNCTION(mysql_insert_id)
{
	zval *mysql_link = NULL;
	int id = -1;
	php_mysql_conn *mysql;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &mysql_link) == FAILURE) {
		return;
	}

	if (!mysql_link) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	/* conversion from int64 to long happing here */
	Z_LVAL_P(return_value) = (long) mysql_insert_id(mysql->conn);
	Z_TYPE_P(return_value) = IS_LONG;
}
/* }}} */


/* {{{ proto mixed mysql_result(resource result, int row [, mixed field]) U
   Gets result data */
PHP_FUNCTION(mysql_result)
{
	zval *result, *field=NULL;
	long row;
	MYSQL_RES *mysql_result;
#ifndef MYSQL_USE_MYSQLND
	MYSQL_ROW sql_row;
	mysql_row_length_type *sql_row_lengths;
#endif
	int field_offset=0;


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl|z", &result, &row, &field) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(mysql_result, MYSQL_RES *, &result, -1, "MySQL result", le_result);

	if (row<0 || row>=(long)mysql_num_rows(mysql_result)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to jump to row %ld on MySQL result index %ld", row, Z_LVAL_P(result));
		RETURN_FALSE;
	}
	mysql_data_seek(mysql_result, row);

	if (field) {
		/* XXX: What about unicode type ??? Please test :) */
		switch(Z_TYPE_P(field)) {
			case IS_STRING: {
					int i=0;
					const MYSQL_FIELD *tmp_field;
					char *table_name, *field_name, *tmp;

					if ((tmp=strchr(Z_STRVAL_P(field), '.'))) {
						table_name = estrndup(Z_STRVAL_P(field), tmp-Z_STRVAL_P(field));
						field_name = estrdup(tmp+1);
					} else {
						table_name = NULL;
						field_name = estrndup(Z_STRVAL_P(field),Z_STRLEN_P(field));
					}
					mysql_field_seek(mysql_result, 0);
					while ((tmp_field=mysql_fetch_field(mysql_result))) {
						if ((!table_name || !strcasecmp(tmp_field->table, table_name)) && !strcasecmp(tmp_field->name, field_name)) {
							field_offset = i;
							break;
						}
						i++;
					}
					if (!tmp_field) { /* no match found */
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s%s%s not found in MySQL result index %ld",
									(table_name?table_name:""), (table_name?".":""), field_name, Z_LVAL_P(result));
						efree(field_name);
						if (table_name) {
							efree(table_name);
						}
						RETURN_FALSE;
					}
					efree(field_name);
					if (table_name) {
						efree(table_name);
					}
				}
				break;
			default:
				convert_to_long_ex(&field);
				field_offset = Z_LVAL_P(field);
				if (field_offset<0 || field_offset>=(int)mysql_num_fields(mysql_result)) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Bad column offset specified");
					RETURN_FALSE;
				}
				break;
		}
	}

#ifndef MYSQL_USE_MYSQLND
	if ((sql_row=mysql_fetch_row(mysql_result))==NULL 
		|| (sql_row_lengths=mysql_fetch_lengths(mysql_result))==NULL) { /* shouldn't happen? */
		RETURN_FALSE;
	}

	if (sql_row[field_offset]) {
		RETURN_UTF8_STRINGL(sql_row[field_offset], sql_row_lengths[field_offset], ZSTR_DUPLICATE);
	} else {
		Z_TYPE_P(return_value) = IS_NULL;
	}
#else
	mysqlnd_result_fetch_field_data(mysql_result, field_offset, return_value);
#endif
}
/* }}} */


/* {{{ proto int mysql_num_rows(resource result) U
   Gets number of rows in a result */
PHP_FUNCTION(mysql_num_rows) 
{
	zval *result;
	MYSQL_RES *mysql_result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &result) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(mysql_result, MYSQL_RES *, &result, -1, "MySQL result", le_result);

	/* conversion from int64 to long happing here */
	Z_LVAL_P(return_value) = (long) mysql_num_rows(mysql_result);
	Z_TYPE_P(return_value) = IS_LONG;
}
/* }}} */

/* {{{ proto int mysql_num_fields(resource result) U
   Gets number of fields in a result */
PHP_FUNCTION(mysql_num_fields)
{
	zval *result;
	MYSQL_RES *mysql_result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &result) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(mysql_result, MYSQL_RES *, &result, -1, "MySQL result", le_result);

	Z_LVAL_P(return_value) = mysql_num_fields(mysql_result);
	Z_TYPE_P(return_value) = IS_LONG;
}
/* }}} */

#define MYSQL_BINARY_CHARSET_NR 63

#if MYSQL_VERSION_ID > 50002 || defined(MYSQL_USE_MYSQLND)
/* we have BIT */
#define IS_BINARY_DATA(f) (((f)->type == MYSQL_TYPE_TINY_BLOB || (f)->type == MYSQL_TYPE_BLOB || \
	(f)->type == MYSQL_TYPE_MEDIUM_BLOB || (f)->type == MYSQL_TYPE_LONG_BLOB || \
	(f)->type == MYSQL_TYPE_BIT || \
	(f)->type == MYSQL_TYPE_VAR_STRING || (f)->type == MYSQL_TYPE_VARCHAR || \
	(f)->type == MYSQL_TYPE_STRING)&& (f)->charsetnr == MYSQL_BINARY_CHARSET_NR)
#else
#define IS_BINARY_DATA(f) (((f)->type == MYSQL_TYPE_TINY_BLOB || (f)->type == MYSQL_TYPE_BLOB || \
	(f)->type == MYSQL_TYPE_MEDIUM_BLOB || (f)->type == MYSQL_TYPE_LONG_BLOB || \
	(f)->type == MYSQL_TYPE_VAR_STRING || (f)->type == MYSQL_TYPE_VARCHAR || \
	(f)->type == MYSQL_TYPE_STRING)&& (f)->charsetnr == MYSQL_BINARY_CHARSET_NR)

#endif

/* {{{ php_mysql_fetch_hash
 */
static void php_mysql_fetch_hash(INTERNAL_FUNCTION_PARAMETERS, int result_type, int expected_args, int into_object)
{
	MYSQL_RES	*mysql_result;
	zval		*res, *ctor_params = NULL;
	zend_class_entry *ce = NULL;
#ifndef MYSQL_USE_MYSQLND
	int i;
	MYSQL_FIELD *mysql_field;
	MYSQL_ROW mysql_row;
	mysql_row_length_type *mysql_row_lengths;
#endif

	if (into_object) {
		char *class_name = NULL;
		int class_name_len = 0;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|s&z", &res, &class_name, &class_name_len, UG(utf8_conv), &ctor_params) == FAILURE) {
			return;
		}

		if (ZEND_NUM_ARGS() < 2) {
			ce = zend_standard_class_def;
		} else {
			ce = zend_fetch_class(class_name, class_name_len, ZEND_FETCH_CLASS_AUTO TSRMLS_CC);
		}
		if (!ce) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not find class '%s'", class_name);
			return;
		}
		result_type = MYSQL_ASSOC;
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &res, &result_type) == FAILURE) {
			return;
		}
		if (!result_type) {
			/* result_type might have been set outside, so only overwrite when not set */
			 result_type = MYSQL_BOTH;
		}
	}

	if ((result_type & MYSQL_BOTH) == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The result type should be either MYSQL_NUM, MYSQL_ASSOC or MYSQL_BOTH");
		result_type = MYSQL_BOTH;
	}

	ZEND_FETCH_RESOURCE(mysql_result, MYSQL_RES *, &res, -1, "MySQL result", le_result);

#ifndef MYSQL_USE_MYSQLND
	if ((mysql_row = mysql_fetch_row(mysql_result)) == NULL  ||
		(mysql_row_lengths = mysql_fetch_lengths(mysql_result)) == NULL) {
		RETURN_FALSE;
	}

	array_init(return_value);

	mysql_field_seek(mysql_result, 0);
	for (mysql_field = mysql_fetch_field(mysql_result), i = 0;
		 mysql_field;
		 mysql_field = mysql_fetch_field(mysql_result), i++)
	{
		if (mysql_row[i]) {
			zval *data;

			MAKE_STD_ZVAL(data);

			if (!IS_BINARY_DATA(mysql_field)) {
				UChar *ustr;
				int ulen;

				zend_string_to_unicode(UG(utf8_conv), &ustr, &ulen, mysql_row[i], mysql_row_lengths[i] TSRMLS_CC);
				ZVAL_UNICODEL(data, ustr, ulen, 0);
			} else {
				ZVAL_STRINGL(data, mysql_row[i], mysql_row_lengths[i], 1);
			}

			if (result_type & MYSQL_NUM) {
				add_index_zval(return_value, i, data);
			}
			if (result_type & MYSQL_ASSOC) {
				UChar *ustr;
				int ulen;

				if (result_type & MYSQL_NUM) {
					Z_ADDREF_P(data);
				}

				zend_string_to_unicode(UG(utf8_conv), &ustr, &ulen, mysql_field->name, strlen(mysql_field->name) TSRMLS_CC);
				add_u_assoc_zval_ex(return_value, IS_UNICODE, ZSTR(ustr), ulen + 1, data);
				efree(ustr);
			}
		} else {
			/* NULL value. */
			if (result_type & MYSQL_NUM) {
				add_index_null(return_value, i);
			}

			if (result_type & MYSQL_ASSOC) {
				UChar *ustr;
				int ulen;

				zend_string_to_unicode(UG(utf8_conv), &ustr, &ulen, mysql_field->name, strlen(mysql_field->name) TSRMLS_CC);
				add_u_assoc_null_ex(return_value, IS_UNICODE, ZSTR(ustr), ulen + 1);
				efree(ustr);
			}
		}
	}
#else
	mysqlnd_fetch_into(mysql_result, MYSQLND_FETCH_ASSOC, return_value, MYSQLND_MYSQL);
#endif

	/* mysqlnd might return FALSE if no more rows */
	if (into_object && Z_TYPE_P(return_value) != IS_BOOL) {
		zval dataset = *return_value;
		zend_fcall_info fci;
		zend_fcall_info_cache fcc;
		zval *retval_ptr; 

		object_and_properties_init(return_value, ce, NULL);
		zend_merge_properties(return_value, Z_ARRVAL(dataset), 1 TSRMLS_CC);

		if (ce->constructor) {
			fci.size = sizeof(fci);
			fci.function_table = &ce->function_table;
			fci.function_name = NULL;
			fci.symbol_table = NULL;
			fci.object_ptr = return_value;
			fci.retval_ptr_ptr = &retval_ptr;
			if (ctor_params && Z_TYPE_P(ctor_params) != IS_NULL) {
				if (Z_TYPE_P(ctor_params) == IS_ARRAY) {
					HashTable *ht = Z_ARRVAL_P(ctor_params);
					Bucket *p;

					fci.param_count = 0;
					fci.params = safe_emalloc(sizeof(zval*), ht->nNumOfElements, 0);
					p = ht->pListHead;
					while (p != NULL) {
						fci.params[fci.param_count++] = (zval**)p->pData;
						p = p->pListNext;
					}
				} else {
					/* Two problems why we throw exceptions here: PHP is typeless
					 * and hence passing one argument that's not an array could be
					 * by mistake and the other way round is possible, too. The 
					 * single value is an array. Also we'd have to make that one
					 * argument passed by reference.
					 */
					zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Parameter ctor_params must be an array", 0 TSRMLS_CC);
					return;
				}
			} else {
				fci.param_count = 0;
				fci.params = NULL;
			}
			fci.no_separation = 1;

			fcc.initialized = 1;
			fcc.function_handler = ce->constructor;
			fcc.calling_scope = EG(scope);
			fcc.called_scope = Z_OBJCE_P(return_value);
			fcc.object_ptr = return_value;

			if (zend_call_function(&fci, &fcc TSRMLS_CC) == FAILURE) {
				zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC, "Could not execute %v::%v()", ce->name, ce->constructor->common.function_name);
			} else {
				if (retval_ptr) {
					zval_ptr_dtor(&retval_ptr);
				}
			}
			if (fci.params) {
				efree(fci.params);
			}
		} else if (ctor_params) {
			zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC, "Class %v does not have a constructor hence you cannot use ctor_params", ce->name);
		}
	}
}
/* }}} */

/* {{{ proto array mysql_fetch_row(resource result) U
   Gets a result row as an enumerated array */
PHP_FUNCTION(mysql_fetch_row)
{
#ifdef MYSQL_USE_MYSQLND
	MYSQL_RES		*result;
	zval			*mysql_result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mysql_result) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(result, MYSQL_RES *, &mysql_result, -1, "MySQL result", le_result);

	mysqlnd_fetch_into(result, MYSQLND_FETCH_NUM, return_value, MYSQLND_MYSQL); 
#else
	php_mysql_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, MYSQL_NUM, 1, 0);
#endif
}
/* }}} */


/* {{{ proto object mysql_fetch_object(resource result [, string class_name [, NULL|array ctor_params]])
   Fetch a result row as an object */
PHP_FUNCTION(mysql_fetch_object)
{
	php_mysql_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, MYSQL_ASSOC, 2, 1);

	if (Z_TYPE_P(return_value) == IS_ARRAY) {
		object_and_properties_init(return_value, ZEND_STANDARD_CLASS_DEF_PTR, Z_ARRVAL_P(return_value));
	}
}
/* }}} */


/* {{{ proto array mysql_fetch_array(resource result [, int result_type]) U
   Fetch a result row as an array (associative, numeric or both) */
PHP_FUNCTION(mysql_fetch_array)
{
#ifndef MYSQL_USE_MYSQLND
	php_mysql_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 2, 0);
#else
	MYSQL_RES	*result;
	zval		*mysql_result;
	long		mode = MYSQLND_FETCH_BOTH;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &mysql_result, &mode) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(result, MYSQL_RES *, &mysql_result, -1, "MySQL result", le_result);

	mysqlnd_fetch_into(result, mode, return_value, MYSQLND_MYSQL);
#endif
}
/* }}} */


/* {{{ proto array mysql_fetch_assoc(resource result) U
   Fetch a result row as an associative array */
PHP_FUNCTION(mysql_fetch_assoc)
{
#ifndef MYSQL_USE_MYSQLND
	php_mysql_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, MYSQL_ASSOC, 1, 0);
#else
	MYSQL_RES	*result;
	zval		*mysql_result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mysql_result) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(result, MYSQL_RES *, &mysql_result, -1, "MySQL result", le_result);

	mysqlnd_fetch_into(result, MYSQLND_FETCH_ASSOC, return_value, MYSQLND_MYSQL);
#endif
}
/* }}} */

/* {{{ proto bool mysql_data_seek(resource result, int row_number) U
   Move internal result pointer */
PHP_FUNCTION(mysql_data_seek)
{
	zval *result;
	long offset;
	MYSQL_RES *mysql_result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &result, &offset)) {
		return;
	}

	ZEND_FETCH_RESOURCE(mysql_result, MYSQL_RES *, &result, -1, "MySQL result", le_result);

	if (offset<0 || offset>=(long)mysql_num_rows(mysql_result)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Offset %ld is invalid for MySQL result index %ld (or the query data is unbuffered)", offset, Z_LVAL_P(result));
		RETURN_FALSE;
	}
	mysql_data_seek(mysql_result, offset);
	RETURN_TRUE;
}
/* }}} */


/* {{{ proto array mysql_fetch_lengths(resource result) U
   Gets max data size of each column in a result */
PHP_FUNCTION(mysql_fetch_lengths)
{
	zval *result;
	MYSQL_RES *mysql_result;
	mysql_row_length_type *lengths;
	int num_fields;
	unsigned int i;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &result) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(mysql_result, MYSQL_RES *, &result, -1, "MySQL result", le_result);

	if ((lengths=mysql_fetch_lengths(mysql_result))==NULL) {
		RETURN_FALSE;
	}
	array_init(return_value);
	num_fields = mysql_num_fields(mysql_result);

	for (i=0; i<num_fields; i++) {
		add_index_long(return_value, i, lengths[i]);
	}
}
/* }}} */

/* {{{ php_mysql_get_field_name
 */
static char *php_mysql_get_field_name(int field_type)
{
	switch(field_type) {
		case FIELD_TYPE_STRING:
		case FIELD_TYPE_VAR_STRING:
			return "string";
			break;
#if MYSQL_VERSION_ID > 50002 || defined(MYSQL_USE_MYSQLND)
		case MYSQL_TYPE_BIT:
#endif
#ifdef MYSQL_HAS_TINY
		case FIELD_TYPE_TINY:
#endif
		case FIELD_TYPE_SHORT:
		case FIELD_TYPE_LONG:
		case FIELD_TYPE_LONGLONG:
		case FIELD_TYPE_INT24:
			return "int";
			break;
		case FIELD_TYPE_FLOAT:
		case FIELD_TYPE_DOUBLE:
		case FIELD_TYPE_DECIMAL:
#ifdef FIELD_TYPE_NEWDECIMAL
		case FIELD_TYPE_NEWDECIMAL:
#endif
			return "real";
			break;
		case FIELD_TYPE_TIMESTAMP:
			return "timestamp";
			break;
#ifdef MYSQL_HAS_YEAR
		case FIELD_TYPE_YEAR:
			return "year";
			break;
#endif
		case FIELD_TYPE_DATE:
#ifdef FIELD_TYPE_NEWDATE
		case FIELD_TYPE_NEWDATE:
#endif
			return "date";
			break;
		case FIELD_TYPE_TIME:
			return "time";
			break;
		case FIELD_TYPE_SET:
			return "set";
			break;
		case FIELD_TYPE_ENUM:
			return "enum";
			break;
#ifdef FIELD_TYPE_GEOMETRY
		case FIELD_TYPE_GEOMETRY:
			return "geometry";
			break;
#endif
		case FIELD_TYPE_DATETIME:
			return "datetime";
			break;
		case FIELD_TYPE_TINY_BLOB:
		case FIELD_TYPE_MEDIUM_BLOB:
		case FIELD_TYPE_LONG_BLOB:
		case FIELD_TYPE_BLOB:
			return "blob";
			break;
		case FIELD_TYPE_NULL:
			return "null";
			break;
		default:
			return "unknown";
			break;
	}
}
/* }}} */

/* {{{ proto object mysql_fetch_field(resource result [, int field_offset]) U
   Gets column information from a result and return as an object */
PHP_FUNCTION(mysql_fetch_field)
{
	zval *result;
	long field=0;
	MYSQL_RES *mysql_result;
	const MYSQL_FIELD *mysql_field;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &result, &field) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(mysql_result, MYSQL_RES *, &result, -1, "MySQL result", le_result);

	if (ZEND_NUM_ARGS() > 1) {
		if (field<0 || field>=(int)mysql_num_fields(mysql_result)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Bad field offset");
			RETURN_FALSE;
		}
		mysql_field_seek(mysql_result, field);
	}
	if ((mysql_field=mysql_fetch_field(mysql_result))==NULL) {
		RETURN_FALSE;
	}
	object_init(return_value);

	add_property_utf8_string(return_value, "name",(mysql_field->name?mysql_field->name:""), ZSTR_DUPLICATE);
	add_property_utf8_string(return_value, "table",(mysql_field->table?mysql_field->table:""), ZSTR_DUPLICATE);
	add_property_utf8_string(return_value, "def",(mysql_field->def?mysql_field->def:""), ZSTR_DUPLICATE);
	add_property_long(return_value, "max_length", mysql_field->max_length);
	add_property_long(return_value, "not_null", IS_NOT_NULL(mysql_field->flags)?1:0);
	add_property_long(return_value, "primary_key", IS_PRI_KEY(mysql_field->flags)?1:0);
	add_property_long(return_value, "multiple_key",(mysql_field->flags&MULTIPLE_KEY_FLAG?1:0));
	add_property_long(return_value, "unique_key",(mysql_field->flags&UNIQUE_KEY_FLAG?1:0));
	add_property_long(return_value, "numeric", IS_NUM(Z_TYPE_P(mysql_field))?1:0);
	add_property_long(return_value, "blob", IS_BLOB(mysql_field->flags)?1:0);
	add_property_utf8_string(return_value, "type", php_mysql_get_field_name(Z_TYPE_P(mysql_field)), ZSTR_DUPLICATE);
	add_property_long(return_value, "unsigned",(mysql_field->flags&UNSIGNED_FLAG?1:0));
	add_property_long(return_value, "zerofill",(mysql_field->flags&ZEROFILL_FLAG?1:0));
}
/* }}} */


/* {{{ proto bool mysql_field_seek(resource result, int field_offset) U
   Sets result pointer to a specific field offset */
PHP_FUNCTION(mysql_field_seek)
{
	zval *result;
	long offset;
	MYSQL_RES *mysql_result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &result, &offset) == FAILURE) {
		return;
	}	
	ZEND_FETCH_RESOURCE(mysql_result, MYSQL_RES *, &result, -1, "MySQL result", le_result);

	if (offset<0 || offset>=(int)mysql_num_fields(mysql_result)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Field %ld is invalid for MySQL result index %ld", offset, Z_LVAL_P(result));
		RETURN_FALSE;
	}
	mysql_field_seek(mysql_result, offset);
	RETURN_TRUE;
}
/* }}} */


#define PHP_MYSQL_FIELD_NAME 1
#define PHP_MYSQL_FIELD_TABLE 2
#define PHP_MYSQL_FIELD_LEN 3
#define PHP_MYSQL_FIELD_TYPE 4
#define PHP_MYSQL_FIELD_FLAGS 5
 
/* {{{ php_mysql_field_info
 */
static void php_mysql_field_info(INTERNAL_FUNCTION_PARAMETERS, int entry_type)
{
	zval *result;
	long field;
	MYSQL_RES *mysql_result;
	const MYSQL_FIELD *mysql_field = {0};
	char buf[512];
	int  len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &result, &field) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(mysql_result, MYSQL_RES *, &result, -1, "MySQL result", le_result);

	if (field<0 || field>=(int)mysql_num_fields(mysql_result)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Field %ld is invalid for MySQL result index %ld", field, Z_LVAL_P(result));
		RETURN_FALSE;
	}
	mysql_field_seek(mysql_result, field);
	if ((mysql_field=mysql_fetch_field(mysql_result))==NULL) {
		RETURN_FALSE;
	}

	switch (entry_type) {
		case PHP_MYSQL_FIELD_NAME:
			RETVAL_UTF8_STRING(mysql_field->name, ZSTR_DUPLICATE);
			break;
		case PHP_MYSQL_FIELD_TABLE:
			RETVAL_UTF8_STRING(mysql_field->table, ZSTR_DUPLICATE);
			break;
		case PHP_MYSQL_FIELD_LEN:
			Z_LVAL_P(return_value) = mysql_field->length;
			Z_TYPE_P(return_value) = IS_LONG;
			break;
		case PHP_MYSQL_FIELD_TYPE:
			RETVAL_UTF8_STRING(php_mysql_get_field_name(Z_TYPE_P(mysql_field)), ZSTR_DUPLICATE);
			break;
		case PHP_MYSQL_FIELD_FLAGS:
			memcpy(buf, "", sizeof(""));
#ifdef IS_NOT_NULL
			if (IS_NOT_NULL(mysql_field->flags)) {
				strcat(buf, "not_null ");
			}
#endif
#ifdef IS_PRI_KEY
			if (IS_PRI_KEY(mysql_field->flags)) {
				strcat(buf, "primary_key ");
			}
#endif
#ifdef UNIQUE_KEY_FLAG
			if (mysql_field->flags&UNIQUE_KEY_FLAG) {
				strcat(buf, "unique_key ");
			}
#endif
#ifdef MULTIPLE_KEY_FLAG
			if (mysql_field->flags&MULTIPLE_KEY_FLAG) {
				strcat(buf, "multiple_key ");
			}
#endif
#ifdef IS_BLOB
			if (IS_BLOB(mysql_field->flags)) {
				strcat(buf, "blob ");
			}
#endif
#ifdef UNSIGNED_FLAG
			if (mysql_field->flags&UNSIGNED_FLAG) {
				strcat(buf, "unsigned ");
			}
#endif
#ifdef ZEROFILL_FLAG
			if (mysql_field->flags&ZEROFILL_FLAG) {
				strcat(buf, "zerofill ");
			}
#endif
#ifdef BINARY_FLAG
			if (mysql_field->flags&BINARY_FLAG) {
				strcat(buf, "binary ");
			}
#endif
#ifdef ENUM_FLAG
			if (mysql_field->flags&ENUM_FLAG) {
				strcat(buf, "enum ");
			}
#endif
#ifdef SET_FLAG
			if (mysql_field->flags&SET_FLAG) {
				strcat(buf, "set ");
			}
#endif
#ifdef AUTO_INCREMENT_FLAG
			if (mysql_field->flags&AUTO_INCREMENT_FLAG) {
				strcat(buf, "auto_increment ");
			}
#endif
#ifdef TIMESTAMP_FLAG
			if (mysql_field->flags&TIMESTAMP_FLAG) {
				strcat(buf, "timestamp ");
			}
#endif
			len = strlen(buf);
			/* remove trailing space, if present */
			if (len && buf[len-1] == ' ') {
				buf[len-1] = 0;
				len--;
			}

			RETVAL_UTF8_STRING(buf, ZSTR_DUPLICATE);
			break;

		default:
			RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string mysql_field_name(resource result, int field_index)
   Gets the name of the specified field in a result */
PHP_FUNCTION(mysql_field_name)
{
	php_mysql_field_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_MYSQL_FIELD_NAME);
}
/* }}} */


/* {{{ proto string mysql_field_table(resource result, int field_offset) U
   Gets name of the table the specified field is in */
PHP_FUNCTION(mysql_field_table)
{
	php_mysql_field_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_MYSQL_FIELD_TABLE);
}
/* }}} */


/* {{{ proto int mysql_field_len(resource result, int field_offset) U
   Returns the length of the specified field */
PHP_FUNCTION(mysql_field_len)
{
	php_mysql_field_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_MYSQL_FIELD_LEN);
}
/* }}} */


/* {{{ proto string mysql_field_type(resource result, int field_offset) U
   Gets the type of the specified field in a result */
PHP_FUNCTION(mysql_field_type)
{
	php_mysql_field_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_MYSQL_FIELD_TYPE);
}
/* }}} */


/* {{{ proto string mysql_field_flags(resource result, int field_offset) U
   Gets the flags associated with the specified field in a result */
PHP_FUNCTION(mysql_field_flags)
{
	php_mysql_field_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_MYSQL_FIELD_FLAGS);
}
/* }}} */


/* {{{ proto bool mysql_free_result(resource result) U
   Free result memory */
PHP_FUNCTION(mysql_free_result)
{
	zval *result;
	MYSQL_RES *mysql_result;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &result) == FAILURE) {
		return;
	}

	if (Z_LVAL_P(result)==0) {
		RETURN_FALSE;
	}

	ZEND_FETCH_RESOURCE(mysql_result, MYSQL_RES *, &result, -1, "MySQL result", le_result);

	zend_list_delete(Z_LVAL_P(result));
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool mysql_ping([int link_identifier]) U
   Ping a server connection. If no connection then reconnect. */
PHP_FUNCTION(mysql_ping)
{
	zval	*mysql_link = NULL;
	int		id = -1;
	php_mysql_conn *mysql;

	if (0 == ZEND_NUM_ARGS()) {
		id = php_mysql_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	} else if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mysql_link)==FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE2(mysql, php_mysql_conn *, &mysql_link, id, "MySQL-Link", le_link, le_plink);

	PHPMY_UNBUFFERED_QUERY_CHECK();

	RETURN_BOOL(! mysql_ping(mysql->conn));
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
