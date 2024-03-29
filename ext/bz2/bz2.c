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
  | Author: Sterling Hughes <sterling@php.net>                           |
  +----------------------------------------------------------------------+
*/
 
/* $Id: bz2.c,v 1.43 2009/05/15 17:27:56 kalle Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_bz2.h"

#if HAVE_BZ2

/* PHP Includes */
#include "ext/standard/file.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

/* for fileno() */
#include <stdio.h>

/* Internal error constants */
#define PHP_BZ_ERRNO   0
#define PHP_BZ_ERRSTR  1
#define PHP_BZ_ERRBOTH 2

static PHP_MINIT_FUNCTION(bz2);
static PHP_MSHUTDOWN_FUNCTION(bz2);
static PHP_MINFO_FUNCTION(bz2);
static PHP_FUNCTION(bzopen);
static PHP_FUNCTION(bzread);
static PHP_FUNCTION(bzerrno);
static PHP_FUNCTION(bzerrstr);
static PHP_FUNCTION(bzerror);
static PHP_FUNCTION(bzcompress);
static PHP_FUNCTION(bzdecompress);

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_bzread, 0, 0, 1)
	ZEND_ARG_INFO(0, bz)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_bzopen, 0)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_bzerrno, 0)
	ZEND_ARG_INFO(0, bz)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_bzerrstr, 0)
	ZEND_ARG_INFO(0, bz)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_bzerror, 0)
	ZEND_ARG_INFO(0, bz)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_bzcompress, 0, 0, 2)
	ZEND_ARG_INFO(0, source)
	ZEND_ARG_INFO(0, blocksize)
	ZEND_ARG_INFO(0, workfactor)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_bzdecompress, 0, 0, 1)
	ZEND_ARG_INFO(0, source)
	ZEND_ARG_INFO(0, small)
ZEND_END_ARG_INFO()

/* }}} */

static const zend_function_entry bz2_functions[] = {
	PHP_FE(bzopen,       arginfo_bzopen)
	PHP_FE(bzread,       arginfo_bzread)
	PHP_FALIAS(bzwrite,   fwrite,		NULL)
	PHP_FALIAS(bzflush,   fflush,		NULL)
	PHP_FALIAS(bzclose,   fclose,		NULL)
	PHP_FE(bzerrno,      arginfo_bzerrno)
	PHP_FE(bzerrstr,     arginfo_bzerrstr)
	PHP_FE(bzerror,      arginfo_bzerror)
	PHP_FE(bzcompress,   arginfo_bzcompress)
	PHP_FE(bzdecompress, arginfo_bzdecompress)
	{NULL, NULL, NULL}
};

zend_module_entry bz2_module_entry = {
	STANDARD_MODULE_HEADER,
	"bz2",
	bz2_functions,
	PHP_MINIT(bz2),
	PHP_MSHUTDOWN(bz2),
	NULL,
	NULL,
	PHP_MINFO(bz2),
	NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_BZ2
ZEND_GET_MODULE(bz2)
#endif

struct php_bz2_stream_data_t {
	BZFILE *bz_file;
	php_stream *stream;
};

/* {{{ BZip2 stream implementation */

static size_t php_bz2iop_read(php_stream *stream, char *buf, size_t count TSRMLS_DC)
{
	struct php_bz2_stream_data_t *self = (struct php_bz2_stream_data_t *) stream->abstract;
	size_t ret;
	
	ret = BZ2_bzread(self->bz_file, buf, count);

	if (ret == 0) {
		stream->eof = 1;
	}

	return ret;
}

static size_t php_bz2iop_write(php_stream *stream, const char *buf, size_t count TSRMLS_DC)
{
	struct php_bz2_stream_data_t *self = (struct php_bz2_stream_data_t *) stream->abstract;

	return BZ2_bzwrite(self->bz_file, (char*)buf, count); 
}

static int php_bz2iop_close(php_stream *stream, int close_handle TSRMLS_DC)
{
	struct php_bz2_stream_data_t *self = (struct php_bz2_stream_data_t *)stream->abstract;
	int ret = EOF;
	
	if (close_handle) {
		BZ2_bzclose(self->bz_file);
	}

	if (self->stream) {
		php_stream_free(self->stream, PHP_STREAM_FREE_CLOSE | (close_handle == 0 ? PHP_STREAM_FREE_PRESERVE_HANDLE : 0));
	}

	efree(self);

	return ret;
}

static int php_bz2iop_flush(php_stream *stream TSRMLS_DC)
{
	struct php_bz2_stream_data_t *self = (struct php_bz2_stream_data_t *)stream->abstract;
	return BZ2_bzflush(self->bz_file);
}
/* }}} */

php_stream_ops php_stream_bz2io_ops = {
	php_bz2iop_write, php_bz2iop_read,
	php_bz2iop_close, php_bz2iop_flush,
	"BZip2",
	NULL, /* seek */
	NULL, /* cast */
	NULL, /* stat */
	NULL  /* set_option */
};

/* {{{ Bzip2 stream openers */
PHP_BZ2_API php_stream *_php_stream_bz2open_from_BZFILE(BZFILE *bz, 
														char *mode, php_stream *innerstream STREAMS_DC TSRMLS_DC)
{
	struct php_bz2_stream_data_t *self;
	
	self = emalloc(sizeof(*self));

	self->stream = innerstream;
	self->bz_file = bz;

	return php_stream_alloc_rel(&php_stream_bz2io_ops, self, 0, mode);
}

PHP_BZ2_API php_stream *_php_stream_bz2open(php_stream_wrapper *wrapper,
											char *path,
											char *mode,
											int options,
											char **opened_path,
											php_stream_context *context STREAMS_DC TSRMLS_DC)
{
	php_stream *retstream = NULL, *stream = NULL;
	char *path_copy = NULL;
	BZFILE *bz_file = NULL;

	if (strncasecmp("compress.bzip2://", path, 17) == 0) {
		path += 17;
	}
	if (mode[0] == '\0' || (mode[0] != 'w' && mode[0] != 'r' && mode[1] != '\0')) {
		return NULL;
	}

#ifdef VIRTUAL_DIR
	virtual_filepath_ex(path, &path_copy, NULL TSRMLS_CC);
#else
	path_copy = path;
#endif  

	if (php_check_open_basedir(path_copy TSRMLS_CC)) {
		return NULL;
	}
	
	/* try and open it directly first */
	bz_file = BZ2_bzopen(path_copy, mode);

	if (opened_path && bz_file) {
		*opened_path = estrdup(path_copy);
	}
	path_copy = NULL;
	
	if (bz_file == NULL) {
		/* that didn't work, so try and get something from the network/wrapper */
		stream = php_stream_open_wrapper(path, mode, options | STREAM_WILL_CAST | ENFORCE_SAFE_MODE, opened_path);
	
		if (stream) {
			int fd;
			if (SUCCESS == php_stream_cast(stream, PHP_STREAM_AS_FD, (void **) &fd, REPORT_ERRORS)) {
				bz_file = BZ2_bzdopen(fd, mode);
			}
		}

		/* remove the file created by php_stream_open_wrapper(), it is not needed since BZ2 functions
		 * failed.
		 */
		if (opened_path && !bz_file && mode[0] == 'w') {
			VCWD_UNLINK(*opened_path);
		}
	}
	
	if (bz_file) {
		retstream = _php_stream_bz2open_from_BZFILE(bz_file, mode, stream STREAMS_REL_CC TSRMLS_CC);
		if (retstream) {
			return retstream;
		}

		BZ2_bzclose(bz_file);
	}

	if (stream) {
		php_stream_close(stream);
	}

	return NULL;
}

/* }}} */

static php_stream_wrapper_ops bzip2_stream_wops = {
	_php_stream_bz2open,
	NULL, /* close */
	NULL, /* fstat */
	NULL, /* stat */
	NULL, /* opendir */
	"BZip2",
	NULL, /* unlink */
	NULL, /* rename */
	NULL, /* mkdir */
	NULL  /* rmdir */
};

static php_stream_wrapper php_stream_bzip2_wrapper = {
	&bzip2_stream_wops,
	NULL,
	0 /* is_url */
};

static void php_bz2_error(INTERNAL_FUNCTION_PARAMETERS, int);

static PHP_MINIT_FUNCTION(bz2)
{
	php_register_url_stream_wrapper("compress.bzip2", &php_stream_bzip2_wrapper TSRMLS_CC);
	php_stream_filter_register_factory("bzip2.*", &php_bz2_filter_factory TSRMLS_CC);
	return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(bz2)
{
	php_unregister_url_stream_wrapper("compress.bzip2" TSRMLS_CC);
	php_stream_filter_unregister_factory("bzip2.*" TSRMLS_CC);

	return SUCCESS;
}

static PHP_MINFO_FUNCTION(bz2)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "BZip2 Support", "Enabled");
	php_info_print_table_row(2, "Stream Wrapper support", "compress.bz2://");
	php_info_print_table_row(2, "Stream Filter support", "bzip2.decompress, bzip2.compress");
	php_info_print_table_row(2, "BZip2 Version", (char *) BZ2_bzlibVersion());
	php_info_print_table_end();
}

/* {{{ proto string bzread(resource bz[, int length]) U
   Reads up to length bytes from a BZip2 stream, or 1024 bytes if length is not specified */
static PHP_FUNCTION(bzread)
{
	zval *bz;
	long len = 1024;
	php_stream *stream;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &bz, &len)) {
		RETURN_FALSE;
	}
	
	php_stream_from_zval(stream, &bz);

	if ((len + 1) < 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "length may not be negative");
		RETURN_FALSE;
	}

	if (stream->readbuf_type == IS_UNICODE) {
		int buflen = len;
		UChar *buf = php_stream_read_unicode_chars(stream, &buflen);

		if (!buf) {
			/* For BC */
			RETURN_EMPTY_UNICODE();
		}

		RETURN_UNICODEL(buf, buflen, 0);
	} else { /* IS_STRING */
		char *buf = emalloc(len + 1);
		int buflen = php_stream_read(stream, buf, len);

		if (!buflen) {
			efree(buf);
			RETURN_EMPTY_STRING();
		}
		buf[buflen] = 0;
		RETURN_STRINGL(buf, buflen, 0);
	}
}
/* }}} */

/* {{{ proto resource bzopen(string|int file|fp, string mode) U
   Opens a new BZip2 stream */
static PHP_FUNCTION(bzopen)
{
	zval	**file;   /* The file to open */
	char	*mode;    /* The mode to open the stream with */
	int	mode_len;
	BZFILE   *bz;     /* The compressed file stream */
	php_stream *stream = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Zs", &file, &mode, &mode_len) == FAILURE) {
		return;
	}

	if (mode_len != 1 || (mode[0] != 'r' && mode[0] != 'w')) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid mode for bzopen(). Only 'w' and 'r' are supported", mode);
		RETURN_FALSE;
	}

	/* If it's not a resource its a string containing the filename to open */
	if (Z_TYPE_PP(file) != IS_RESOURCE) {
		char *filename;
		int filename_len;

		if (FAILURE == php_stream_path_param_encode(file, &filename, &filename_len, REPORT_ERRORS, FG(default_context))) {
			RETURN_FALSE;
		}

		if (filename_len == 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "filename cannot be empty");
			RETURN_FALSE;
		}

		stream = php_stream_bz2open(NULL, filename, mode, REPORT_ERRORS, NULL);
	} else {
		/* If it is a resource, than its a stream resource */
		int fd;
		int stream_mode_len;

		php_stream_from_zval(stream, file);
		stream_mode_len = strlen(stream->mode);
		
		if (stream_mode_len != 1 && !(stream_mode_len == 2 && memchr(stream->mode, 'b', 2))) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "cannot use stream opened in mode '%s'", stream->mode);
			RETURN_FALSE;
		} else if (stream_mode_len == 1 && stream->mode[0] != 'r' && stream->mode[0] != 'w' && stream->mode[0] != 'a' && stream->mode[0] != 'x') {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "cannot use stream opened in mode '%s'", stream->mode);
			RETURN_FALSE;
		}

		switch(mode[0]) {
			case 'r':
				/* only "r" and "rb" are supported */
				if (stream->mode[0] != mode[0] && !(stream_mode_len == 2 && stream->mode[1] != mode[0])) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "cannot read from a stream opened in write only mode");
					RETURN_FALSE;
				}
				break;
			case 'w':
				/* support only "w"(b), "a"(b), "x"(b) */
				if (stream->mode[0] != mode[0] && !(stream_mode_len == 2 && stream->mode[1] != mode[0])
					&& stream->mode[0] != 'a' && !(stream_mode_len == 2 && stream->mode[1] != 'a')
					&& stream->mode[0] != 'x' && !(stream_mode_len == 2 && stream->mode[1] != 'x')) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "cannot write to a stream opened in read only mode");
					RETURN_FALSE;
				}
				break;
			default:
				/* not reachable */
				break;
		}

		/* If the stream is filtered, it'll fail here because it's not castable */
		if (FAILURE == php_stream_cast(stream, PHP_STREAM_AS_FD, (void *) &fd, REPORT_ERRORS)) {
			RETURN_FALSE;
		}
		
		bz = BZ2_bzdopen(fd, mode);

		stream = php_stream_bz2open_from_BZFILE(bz, mode, stream);
	}

	if (stream) {
		php_stream_to_zval(stream, return_value);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int bzerrno(resource bz) U
   Returns the error number */
static PHP_FUNCTION(bzerrno)
{
	php_bz2_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_BZ_ERRNO);
}
/* }}} */

/* {{{ proto string bzerrstr(resource bz) U
   Returns the error string */
static PHP_FUNCTION(bzerrstr)
{
	php_bz2_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_BZ_ERRSTR);
}
/* }}} */

/* {{{ proto array bzerror(resource bz) U
   Returns the error number and error string in an associative array */
static PHP_FUNCTION(bzerror)
{
	php_bz2_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_BZ_ERRBOTH);
}
/* }}} */

/* {{{ proto string bzcompress(string source [, int blocksize100k [, int workfactor]]) U
   Compresses a string into BZip2 encoded data */
static PHP_FUNCTION(bzcompress)
{
	zstr		 source;		/* String to compress */
	int		 source_len;
	zend_uchar	 source_type;
	long		 block_size = 4,	/* Block size for compression algorithm */
			 work_factor = 0;	/* Work factor for compression algorithm */
	char		*dest = NULL;		/* Destination to place the compressed data into */
	int		 dest_len = 0;
	int		 error;			/* Error Container */

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t|ll", &source, &source_len, &source_type, &block_size, &work_factor) == FAILURE) {
		return;
	}
	
	/* Malloc dest to dest_len which is the length of the 101% the length of the data plus 600 bytes
	 * This is the largest size the results of the compression could possibly be,
	 * at least that's what the libbz2 docs say (thanks to jeremy@nirvani.net for pointing this out).
	 */
	dest_len   = (int) (source_len + (0.01 * source_len) + 600);

	/* Allocate the destination buffer */
	dest = emalloc(dest_len + 1);
	
	if (source_type == IS_UNICODE) {
		source.s = zend_unicode_to_ascii(source.u, source_len TSRMLS_CC);
		if (!source.s) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Binary or ASCII-Unicode string expected, non-ASCII-Unicode string received");
			RETURN_FALSE;
		}
	}

	error = BZ2_bzBuffToBuffCompress(dest, &dest_len, source.s, source_len, block_size, 0, work_factor);
	if (error != BZ_OK) {
		efree(dest);
		RETVAL_LONG(error);
	} else {
		if (dest_len < source_len) {
			/* Normal case, but it means we over alloc'd */
			dest = erealloc(dest, dest_len + 1);
		} /* else it's not worth a realloc */

		dest[dest_len] = 0;
		RETVAL_STRINGL(dest, dest_len, 0);
	}

	if (source_type == IS_UNICODE) {
		efree(source.s);
	}
}
/* }}} */

/* {{{ proto string bzdecompress(string source [, int small]) U
   Decompresses BZip2 compressed data */
static PHP_FUNCTION(bzdecompress)
{
	char *source, *dest;
	int source_len, error;
	long small = 0;
#if defined(PHP_WIN32)
	unsigned __int64 size = 0;
#else
	unsigned long long size = 0;
#endif
	bz_stream bzs;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S|l", &source, &source_len, &small)) {
		RETURN_FALSE;
	}

	bzs.bzalloc = NULL;
	bzs.bzfree = NULL;

	if (BZ2_bzDecompressInit(&bzs, 0, small) != BZ_OK) {
		RETURN_FALSE;
	}

	bzs.next_in = source;
	bzs.avail_in = source_len;

	/* in most cases bz2 offers at least 2:1 compression, so we use that as our base */
	bzs.avail_out = source_len * 2;
	bzs.next_out = dest = emalloc(bzs.avail_out + 1);
	
	while ((error = BZ2_bzDecompress(&bzs)) == BZ_OK && bzs.avail_in > 0) {
		/* compression is better then 2:1, need to allocate more memory */
		bzs.avail_out = source_len;
		size = (bzs.total_out_hi32 * (unsigned int) -1) + bzs.total_out_lo32;
		dest = erealloc(dest, (size_t) (size + bzs.avail_out + 1));
		bzs.next_out = dest + size;
	}

	if (error == BZ_STREAM_END || error == BZ_OK) {
		size = (bzs.total_out_hi32 * (unsigned int) -1) + bzs.total_out_lo32;
		dest = erealloc(dest, (size_t) (size + 1));
		dest[size] = '\0';
		RETVAL_STRINGL(dest, (int) size, 0);
	} else { /* real error */
		efree(dest);
		RETVAL_LONG(error);
	}

	BZ2_bzDecompressEnd(&bzs);
}
/* }}} */

/* {{{ php_bz2_error()
   The central error handling interface, does the work for bzerrno, bzerrstr and bzerror */
static void php_bz2_error(INTERNAL_FUNCTION_PARAMETERS, int opt)
{ 
	zval         *zstream; /* BZip2 Resource Pointer */
	php_stream   *stream;
	const char   *errstr;  /* Error string */
	int           errnum;  /* Error number */
	struct php_bz2_stream_data_t *self;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zstream) == FAILURE) {
		return;
	}

	php_stream_from_zval(stream, &zstream);

	if (!php_stream_is(stream, PHP_STREAM_IS_BZIP2)) {
		RETURN_FALSE;
	}

	self = (struct php_bz2_stream_data_t *) stream->abstract;
	
	/* Fetch the error information */
	errstr = BZ2_bzerror(self->bz_file, &errnum);
	
	/* Determine what to return */
	switch (opt) {
		case PHP_BZ_ERRNO:
			RETURN_LONG(errnum);
			break;
		case PHP_BZ_ERRSTR:
			RETURN_RT_STRING((char*)errstr, ZSTR_DUPLICATE);
			break;
		case PHP_BZ_ERRBOTH:
			array_init(return_value);
		
			add_ascii_assoc_long     (return_value, "errno",  errnum);
			add_ascii_assoc_rt_string(return_value, "errstr", (char*)errstr, ZSTR_DUPLICATE);
			break;
	}
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
