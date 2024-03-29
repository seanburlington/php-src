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
  | Author: Wez Furlong <wez@php.net>                                    |
  +----------------------------------------------------------------------+
*/

/* $Id: php_pdo_sqlite_int.h,v 1.9 2009/03/10 23:39:31 helly Exp $ */

#ifndef PHP_PDO_SQLITE_INT_H
#define PHP_PDO_SQLITE_INT_H

#include <sqlite3.h>

typedef struct {
	const char *file;
	int line;
	unsigned int errcode;
	char *errmsg;
} pdo_sqlite_error_info;

struct pdo_sqlite_fci {
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
};

struct pdo_sqlite_func {
	struct pdo_sqlite_func *next;

	zval *func, *step, *fini;
	int argc;
	const char *funcname;
	
	/* accelerated callback references */
	struct pdo_sqlite_fci afunc, astep, afini;
};

typedef struct {
	sqlite3 *db;
	pdo_sqlite_error_info einfo;
	struct pdo_sqlite_func *funcs;
} pdo_sqlite_db_handle;

typedef struct {
	pdo_sqlite_db_handle 	*H;
	sqlite3_stmt *stmt;
	unsigned pre_fetched:1;
	unsigned done:1;
} pdo_sqlite_stmt;

extern pdo_driver_t pdo_sqlite_driver;

extern int _pdo_sqlite_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file, int line TSRMLS_DC);
extern int _pdo_sqlite_error_msg(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *sqlstate, const char *msg, 
					const char *file, int line TSRMLS_DC);
#define pdo_sqlite_error(dbh) _pdo_sqlite_error(dbh, NULL, __FILE__, __LINE__ TSRMLS_CC)
#define pdo_sqlite_errmsg(dbh, st, msg) _pdo_sqlite_error_msg(dbh, NULL, st, msg, __FILE__, __LINE__ TSRMLS_CC)
#define pdo_sqlite_error_stmt(stmt) _pdo_sqlite_error(stmt->dbh, stmt, __FILE__, __LINE__ TSRMLS_CC)
#define pdo_sqlite_errmsg_stmt(stmt, st, msg) _pdo_sqlite_error_msg(stmt->dbh, stmt, st, msg, __FILE__, __LINE__ TSRMLS_CC)


extern struct pdo_stmt_methods sqlite_stmt_methods;
#endif
