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
   | Author: Wez Furlong <wez@thebrainroom.com>                           |
   +----------------------------------------------------------------------+
 */

/* $Id: php_stream_plain_wrapper.h,v 1.13 2009/03/10 23:40:01 helly Exp $ */

/* definitions for the plain files wrapper */

/* operations for a plain file; use the php_stream_fopen_XXX funcs below */
PHPAPI extern php_stream_ops php_stream_stdio_ops;
PHPAPI extern php_stream_wrapper php_plain_files_wrapper;

BEGIN_EXTERN_C()

/* like fopen, but returns a stream */
PHPAPI php_stream *_php_stream_fopen(const char *filename, const char *mode, char **opened_path, int options STREAMS_DC TSRMLS_DC);
#define php_stream_fopen(filename, mode, opened)	_php_stream_fopen((filename), (mode), (opened), 0 STREAMS_CC TSRMLS_CC)

PHPAPI php_stream *_php_stream_fopen_with_path(char *filename, char *mode, char *path, char **opened_path, int options STREAMS_DC TSRMLS_DC);
#define php_stream_fopen_with_path(filename, mode, path, opened)	_php_stream_fopen_with_path((filename), (mode), (path), (opened) STREAMS_CC TSRMLS_CC)

PHPAPI php_stream *_php_stream_fopen_from_file(FILE *file, const char *mode STREAMS_DC TSRMLS_DC);
#define php_stream_fopen_from_file(file, mode)	_php_stream_fopen_from_file((file), (mode) STREAMS_CC TSRMLS_CC)

PHPAPI php_stream *_php_stream_fopen_from_fd(int fd, const char *mode, const char *persistent_id STREAMS_DC TSRMLS_DC);
#define php_stream_fopen_from_fd(fd, mode, persistent_id)	_php_stream_fopen_from_fd((fd), (mode), (persistent_id) STREAMS_CC TSRMLS_CC)

PHPAPI php_stream *_php_stream_fopen_from_pipe(FILE *file, const char *mode STREAMS_DC TSRMLS_DC);
#define php_stream_fopen_from_pipe(file, mode)	_php_stream_fopen_from_pipe((file), (mode) STREAMS_CC TSRMLS_CC)

PHPAPI php_stream *_php_stream_fopen_tmpfile(int dummy STREAMS_DC TSRMLS_DC);
#define php_stream_fopen_tmpfile()	_php_stream_fopen_tmpfile(0 STREAMS_CC TSRMLS_CC)

PHPAPI php_stream *_php_stream_fopen_temporary_file(const char *dir, const char *pfx, char **opened_path STREAMS_DC TSRMLS_DC);
#define php_stream_fopen_temporary_file(dir, pfx, opened_path)	_php_stream_fopen_temporary_file((dir), (pfx), (opened_path) STREAMS_CC TSRMLS_CC)

/* This is a utility API for extensions that are opening a stream, converting it
 * to a FILE* and then closing it again.  Be warned that fileno() on the result
 * will most likely fail on systems with fopencookie. */
PHPAPI FILE * _php_stream_open_wrapper_as_file(char * path, char * mode, int options, char **opened_path STREAMS_DC TSRMLS_DC);
#define php_stream_open_wrapper_as_file(path, mode, options, opened_path) _php_stream_open_wrapper_as_file((path), (mode), (options), (opened_path) STREAMS_CC TSRMLS_CC)

END_EXTERN_C()

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
