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
   | Author:  Jim Winstead <jimw@php.net>                                 |
   +----------------------------------------------------------------------+
*/

/* $Id: php_filestat.h,v 1.36 2009/03/10 23:39:40 helly Exp $ */

#ifndef PHP_FILESTAT_H
#define PHP_FILESTAT_H

PHP_RINIT_FUNCTION(filestat);
PHP_RSHUTDOWN_FUNCTION(filestat);

PHP_FUNCTION(clearstatcache);
PHP_FUNCTION(fileatime);
PHP_FUNCTION(filectime);
PHP_FUNCTION(filegroup);
PHP_FUNCTION(fileinode);
PHP_FUNCTION(filemtime);
PHP_FUNCTION(fileowner);
PHP_FUNCTION(fileperms);
PHP_FUNCTION(filesize);
PHP_FUNCTION(filetype);
PHP_FUNCTION(is_writable);
PHP_FUNCTION(is_readable);
PHP_FUNCTION(is_executable);
PHP_FUNCTION(is_file);
PHP_FUNCTION(is_dir);
PHP_FUNCTION(is_link);
PHP_FUNCTION(file_exists);
PHP_NAMED_FUNCTION(php_if_stat);
PHP_NAMED_FUNCTION(php_if_lstat);
PHP_FUNCTION(disk_total_space);
PHP_FUNCTION(disk_free_space);
PHP_FUNCTION(chown);
PHP_FUNCTION(chgrp);
#if HAVE_LCHOWN
PHP_FUNCTION(lchown);
#endif
#if HAVE_LCHOWN
PHP_FUNCTION(lchgrp);
#endif
PHP_FUNCTION(chmod);
#if HAVE_UTIME
PHP_FUNCTION(touch);
#endif
PHP_FUNCTION(clearstatcache);

#define MAKE_LONG_ZVAL_INCREF(name, val)\
	MAKE_STD_ZVAL(name); \
	ZVAL_LONG(name, val); \
	Z_ADDREF_P(name); 

#ifdef PHP_WIN32
#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#define S_IXUSR S_IEXEC
#define S_IRGRP S_IREAD
#define S_IWGRP S_IWRITE
#define S_IXGRP S_IEXEC
#define S_IROTH S_IREAD
#define S_IWOTH S_IWRITE
#define S_IXOTH S_IEXEC

#undef getgid
#define getgroups(a, b) 0
#define getgid() 1
#define getuid() 1
#endif

#ifdef PHP_WIN32
typedef unsigned int php_stat_len;
#else
typedef int php_stat_len;
#endif

PHPAPI void php_clear_stat_cache(zend_bool clear_realpath_cache, const char *filename, int filename_len TSRMLS_DC);
PHPAPI void php_stat(const char *filename, php_stat_len filename_length, int type, zval *return_value TSRMLS_DC);
PHPAPI void php_u_stat(zend_uchar filename_type, const zstr filename, php_stat_len filename_length, int type, php_stream_context *context, zval *return_value TSRMLS_DC);

/* Switches for various filestat functions: */
#define FS_PERMS    0
#define FS_INODE    1
#define FS_SIZE     2
#define FS_OWNER    3
#define FS_GROUP    4
#define FS_ATIME    5
#define FS_MTIME    6
#define FS_CTIME    7
#define FS_TYPE     8
#define FS_IS_W     9
#define FS_IS_R    10
#define FS_IS_X    11
#define FS_IS_FILE 12
#define FS_IS_DIR  13
#define FS_IS_LINK 14
#define FS_EXISTS  15
#define FS_LSTAT   16
#define FS_STAT    17

#endif /* PHP_FILESTAT_H */
