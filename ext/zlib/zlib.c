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
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Stefan R�hrich <sr@linux.de>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   |          Jade Nicoletti <nicoletti@nns.ch>                           |
   |          Michael Wallner <mike@php.net>                              |
   +----------------------------------------------------------------------+
 */

/* $Id: zlib.c,v 1.226 2009/05/26 08:02:31 jani Exp $ */

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "php.h"
#include "SAPI.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/file.h"

#include "php_zlib.h"

ZEND_DECLARE_MODULE_GLOBALS(zlib);

/* {{{ Memory management wrappers */

static voidpf php_zlib_alloc(voidpf opaque, uInt items, uInt size)
{
	return (voidpf)safe_emalloc(items, size, 0);
}

static void php_zlib_free(voidpf opaque, voidpf address)
{
	efree((void*)address);
}
/* }}} */

/* {{{ php_zlib_output_conflict_check() */
int php_zlib_output_conflict_check(zval *handler_name TSRMLS_DC)
{
	if (php_output_get_level(TSRMLS_C) > 0) {
		PHP_OUTPUT_CONFLICT(PHP_ZLIB_OUTPUT_HANDLER_NAME, return FAILURE);
		PHP_OUTPUT_CONFLICT("ob_gzhandler", return FAILURE);
		PHP_OUTPUT_CONFLICT("mb_output_handler", return FAILURE);
		PHP_OUTPUT_CONFLICT("URL-Rewriter", return FAILURE);
	}
	return SUCCESS;
}
/* }}} */

/* {{{ php_zlib_output_encoding() */
int php_zlib_output_encoding(TSRMLS_D)
{
	zval **enc;
	
	if (!ZLIBG(compression_coding)) {
		zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC);
		
		if (PG(http_globals)[TRACK_VARS_SERVER] && SUCCESS == zend_ascii_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_ACCEPT_ENCODING", sizeof("HTTP_ACCEPT_ENCODING"), (void *) &enc)) {
			convert_to_string(*enc);
			if (strstr(Z_STRVAL_PP(enc), "gzip")) {
				ZLIBG(compression_coding) = PHP_ZLIB_ENCODING_GZIP;
			} else if (strstr(Z_STRVAL_PP(enc), "deflate")) {
				ZLIBG(compression_coding) = PHP_ZLIB_ENCODING_DEFLATE;
			}
		}
	}
	
	return ZLIBG(compression_coding);
}
/* }}} */

/* {{{ php_zlib_output_compression_start() */
void php_zlib_output_compression_start(TSRMLS_D)
{
	zval *zoh, *tmp;
	php_output_handler *h;
	
	switch (ZLIBG(output_compression)) {
		case 0:
			break;
		case 1:
			ZLIBG(output_compression) = PHP_OUTPUT_HANDLER_DEFAULT_SIZE;
			/* break omitted intentionally */
		default:
			MAKE_STD_ZVAL(tmp);
			ZVAL_ASCII_STRING(tmp, PHP_ZLIB_OUTPUT_HANDLER_NAME, ZSTR_DUPLICATE);
			if (	(h = php_zlib_output_handler_init(tmp, ZLIBG(output_compression), PHP_OUTPUT_HANDLER_STDFLAGS TSRMLS_CC)) &&
					(SUCCESS == php_output_handler_start(h TSRMLS_CC))) {
				if (ZLIBG(output_handler) && *ZLIBG(output_handler)) {
					MAKE_STD_ZVAL(zoh);
					ZVAL_ASCII_STRING(zoh, ZLIBG(output_handler), ZSTR_DUPLICATE);
					php_output_start_user(zoh, ZLIBG(output_compression), PHP_OUTPUT_HANDLER_STDFLAGS TSRMLS_CC);
					zval_ptr_dtor(&zoh);
				}
			}
			zval_ptr_dtor(&tmp);
			break;
	}
}
/* }}} */

/* {{{ php_zlib_output_handler_init() */
php_output_handler *php_zlib_output_handler_init(zval *handler_name, size_t chunk_size, int flags TSRMLS_DC)
{
	php_output_handler *h = NULL;
	php_zlib_context   *ctx;
	
	if (!ZLIBG(output_compression)) {
		ZLIBG(output_compression) = chunk_size ? chunk_size : PHP_OUTPUT_HANDLER_DEFAULT_SIZE;
	}
	if ((h = php_output_handler_create_internal(handler_name, php_zlib_output_handler, chunk_size, flags TSRMLS_CC))) {
		ctx = (php_zlib_context *) ecalloc(1, sizeof(php_zlib_context));
		ctx->Z.zalloc = php_zlib_alloc;
		ctx->Z.zfree = php_zlib_free;
		php_output_handler_set_context(h, ctx, php_zlib_output_handler_dtor TSRMLS_CC);
	}
	
	return h;
}
/* }}} */

/* {{{ php_zlib_output_handler() */
int php_zlib_output_handler(void **handler_context, php_output_context *output_context)
{
	php_zlib_context *ctx = *(php_zlib_context **) handler_context;
	int flags = Z_SYNC_FLUSH;
	PHP_OUTPUT_TSRMLS(output_context);
	
	if (!php_zlib_output_encoding(TSRMLS_C)) {
		/* "Vary: Accept-Encoding" header sent along uncompressed content breaks caching in MSIE,
			so let's just send it with successfully compressed content or unless the complete
			buffer gets discarded, see http://bugs.php.net/40325;
			
			Test as follows:
			+Vary: $ HTTP_ACCEPT_ENCODING=gzip ./sapi/cgi/php <<<'<?php ob_start("ob_gzhandler"); echo "foo\n";'
			+Vary: $ HTTP_ACCEPT_ENCODING= ./sapi/cgi/php <<<'<?php ob_start("ob_gzhandler"); echo "foo\n";'
			-Vary: $ HTTP_ACCEPT_ENCODING=gzip ./sapi/cgi/php <<<'<?php ob_start("ob_gzhandler"); echo "foo\n"; ob_end_clean();'
			-Vary: $ HTTP_ACCEPT_ENCODING= ./sapi/cgi/php <<<'<?php ob_start("ob_gzhandler"); echo "foo\n"; ob_end_clean();'
		*/
		if (output_context->op != (PHP_OUTPUT_HANDLER_START|PHP_OUTPUT_HANDLER_CLEAN|PHP_OUTPUT_HANDLER_FINAL)) {
			sapi_add_header_ex(ZEND_STRL("Vary: Accept-Encoding"), 1, 1 TSRMLS_CC);
		}
		return FAILURE;
	}
	
	if (output_context->op & PHP_OUTPUT_HANDLER_START) {
		/* start up */
		if (Z_OK != deflateInit2(&ctx->Z, ZLIBG(output_compression_level), Z_DEFLATED, ZLIBG(compression_coding), MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY)) {
			return FAILURE;
		}
	}
	
	if (output_context->op & PHP_OUTPUT_HANDLER_CLEAN) {
		/* free buffers */
		deflateEnd(&ctx->Z);
		
		if (output_context->op & PHP_OUTPUT_HANDLER_FINAL) {
			/* discard */
			return SUCCESS;
		} else {
			/* restart */
			if (Z_OK != deflateInit2(&ctx->Z, ZLIBG(output_compression_level), Z_DEFLATED, ZLIBG(compression_coding), MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY)) {
				return FAILURE;
			}
			ctx->buffer.used = 0;
		}
	} else {
		
		if (output_context->in.used) {
			/* append input */
			if (ctx->buffer.free < output_context->in.used) {
				if (!(ctx->buffer.aptr = erealloc_recoverable(ctx->buffer.data, ctx->buffer.used + ctx->buffer.free + output_context->in.used))) {
					deflateEnd(&ctx->Z);
					return FAILURE;
				}
				ctx->buffer.data = ctx->buffer.aptr;
				ctx->buffer.free += output_context->in.used;
			}
			memcpy(ctx->buffer.data + ctx->buffer.used, output_context->in.data, output_context->in.used);
			ctx->buffer.free -= output_context->in.used;
			ctx->buffer.used += output_context->in.used;
		}
		
		output_context->out.size = PHP_ZLIB_BUFFER_SIZE_GUESS(output_context->in.used);
		output_context->out.data = emalloc(output_context->out.size);
		output_context->out.free = 1;
		output_context->out.used = 0;
		
		ctx->Z.avail_in = ctx->buffer.used;
		ctx->Z.next_in = (Bytef *) ctx->buffer.data;
		ctx->Z.avail_out = output_context->out.size;
		ctx->Z.next_out = (Bytef *) output_context->out.data;
		
		if (output_context->op & PHP_OUTPUT_HANDLER_FINAL) {
			flags = Z_FINISH;
		} else if (output_context->op & PHP_OUTPUT_HANDLER_FLUSH) {
			flags = Z_FULL_FLUSH;
		}
		
		switch (deflate(&ctx->Z, flags)) {
			case Z_OK:
				if (flags == Z_FINISH) {
					deflateEnd(&ctx->Z);
					return FAILURE;
				}
			case Z_STREAM_END:
				if (ctx->Z.avail_in) {
					memmove(ctx->buffer.data, ctx->buffer.data + ctx->buffer.used - ctx->Z.avail_in, ctx->Z.avail_in);
				}
				ctx->buffer.free += ctx->buffer.used - ctx->Z.avail_in;
				ctx->buffer.used = ctx->Z.avail_in;
				output_context->out.used = output_context->out.size - ctx->Z.avail_out;
				break;
			default:
				deflateEnd(&ctx->Z);
				return FAILURE;
		}
		
		php_output_handler_hook(PHP_OUTPUT_HANDLER_HOOK_GET_FLAGS, &flags TSRMLS_CC);
		
		if (!(flags & PHP_OUTPUT_HANDLER_STARTED)) {
			if (SG(headers_sent) || !ZLIBG(output_compression)) {
				deflateEnd(&ctx->Z);
				return FAILURE;
			}
			switch (ZLIBG(compression_coding)) {
				case PHP_ZLIB_ENCODING_GZIP:
					sapi_add_header_ex(ZEND_STRL("Content-Encoding: gzip"), 1, 1 TSRMLS_CC);
					break;
				case PHP_ZLIB_ENCODING_DEFLATE:
					sapi_add_header_ex(ZEND_STRL("Content-Encoding: deflate"), 1, 1 TSRMLS_CC);
					break;
				default:
					deflateEnd(&ctx->Z);
					return FAILURE;
			}
			sapi_add_header_ex(ZEND_STRL("Vary: Accept-Encoding"), 1, 1 TSRMLS_CC);
			php_output_handler_hook(PHP_OUTPUT_HANDLER_HOOK_IMMUTABLE, NULL TSRMLS_CC);
		}
		
		if (output_context->op & PHP_OUTPUT_HANDLER_FINAL) {
			deflateEnd(&ctx->Z);
		}
	}
	return SUCCESS;
}
/* }}} */

/* {{{ php_zlib_output_handler_dtor() */
void php_zlib_output_handler_dtor(void *opaq TSRMLS_DC)
{
	php_zlib_context *ctx = (php_zlib_context *) opaq;
	
	if (ctx) {
		if (ctx->buffer.data) {
			efree(ctx->buffer.data);
		}
		efree(ctx);
	}
}
/* }}} */

/* {{{ php_zlib_encode() */
int php_zlib_encode(const char *in_buf, size_t in_len, char **out_buf, size_t *out_len, int encoding, int level TSRMLS_DC)
{
	int status;
	z_stream Z;
	
	memset(&Z, 0, sizeof(z_stream));
	Z.zalloc = php_zlib_alloc;
	Z.zfree = php_zlib_free;
	
	if (Z_OK == (status = deflateInit2(&Z, level, Z_DEFLATED, encoding, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY))) {
		*out_len = PHP_ZLIB_BUFFER_SIZE_GUESS(in_len);
		*out_buf = emalloc(*out_len);
		
		Z.next_in = (Bytef *) in_buf;
		Z.next_out = (Bytef *) *out_buf;
		Z.avail_in = in_len;
		Z.avail_out = *out_len;
		
		status = deflate(&Z, Z_FINISH);
		deflateEnd(&Z);
		
		if (Z_STREAM_END == status) {
			/* size buffer down to actual length */
			*out_buf = erealloc(*out_buf, Z.total_out + 1);
			(*out_buf)[*out_len = Z.total_out] = '\0';
			return SUCCESS;
		} else {
			efree(*out_buf);
		}
	}
	
	*out_buf = NULL;
	*out_len = 0;
	
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", zError(status));
	return FAILURE;
}
/* }}} */

/* {{{ inflate_rounds() */
static inline int inflate_rounds(z_stream *Z, size_t max, char **buf, size_t *len)
{
	int status, round = 0;
	php_zlib_buffer buffer = {NULL, NULL, 0, 0, 0};
	
	*buf = NULL;
	*len = 0;
	
	buffer.size = Z->avail_in;
	
	do {
		if ((max && (max < buffer.used)) || !(buffer.aptr = erealloc_recoverable(buffer.data, buffer.size))) {
			status = Z_MEM_ERROR;
		} else {
			buffer.data = buffer.aptr;
			Z->avail_out = buffer.free = buffer.size - buffer.used;
			Z->next_out = (Bytef *) buffer.data + buffer.used;
#if 0
			fprintf(stderr, "\n%3d: %3d PRIOR: size=%7lu,\tfree=%7lu,\tused=%7lu,\tavail_in=%7lu,\tavail_out=%7lu\n", round, status, buffer.size, buffer.free, buffer.used, Z->avail_in, Z->avail_out);
#endif
			status = inflate(Z, Z_NO_FLUSH);
			
			buffer.used += buffer.free - Z->avail_out;
			buffer.free = Z->avail_out;
#if 0
			fprintf(stderr, "%3d: %3d AFTER: size=%7lu,\tfree=%7lu,\tused=%7lu,\tavail_in=%7lu,\tavail_out=%7lu\n", round, status, buffer.size, buffer.free, buffer.used, Z->avail_in, Z->avail_out);
#endif
			buffer.size += (buffer.size >> 3) + 1;
		}
	} while ((Z_BUF_ERROR == status || (Z_OK == status && Z->avail_in)) && ++round < 100);
	
	if (status == Z_OK || status == Z_STREAM_END) {
		buffer.data = erealloc(buffer.data, buffer.used + 1);
		buffer.data[buffer.used] = '\0';
		*buf = buffer.data;
		*len = buffer.used;
	} else if (buffer.data) {
		efree(buffer.data);
	}
	
	return status;
}
/* }}} */

/* {{{ php_zlib_decode() */
int php_zlib_decode(const char *in_buf, size_t in_len, char **out_buf, size_t *out_len, int encoding, size_t max_len TSRMLS_DC)
{
	int status = Z_DATA_ERROR;
	z_stream Z;
	
	memset(&Z, 0, sizeof(z_stream));
	Z.zalloc = php_zlib_alloc;
	Z.zfree = php_zlib_free;

	if (in_len) {
retry_raw_inflate:
		status = inflateInit2(&Z, encoding);
		if (Z_OK == status) {
			Z.next_in = (Bytef *) in_buf;
			Z.avail_in = in_len;
			
			switch (status = inflate_rounds(&Z, max_len, out_buf, out_len)) {
				case Z_OK:
				case Z_STREAM_END:
					inflateEnd(&Z);
					return SUCCESS;
				
				case Z_DATA_ERROR:
					/* raw deflated data? */
					if (PHP_ZLIB_ENCODING_ANY == encoding) {
						inflateEnd(&Z);
						encoding = PHP_ZLIB_ENCODING_RAW;
						goto retry_raw_inflate;
					}
			}
			inflateEnd(&Z);
		}
	}
	
	*out_buf = NULL;
	*out_len = 0;
	
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", zError(status));
	return FAILURE;
}
/* }}} */

/* {{{ proto string zlib_get_coding_type(void) U
   Returns the coding type used for output compression */
PHP_FUNCTION(zlib_get_coding_type)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	switch (ZLIBG(compression_coding)) {
		case PHP_ZLIB_ENCODING_GZIP:
			RETURN_ASCII_STRINGL("gzip", sizeof("gzip") - 1, 1);
		case PHP_ZLIB_ENCODING_DEFLATE:
			RETURN_ASCII_STRINGL("deflate", sizeof("deflate") - 1, 1);
		default:
			RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto array gzfile(string filename [, int use_include_path]) U
   Read and uncompress entire .gz-file into an array */
PHP_FUNCTION(gzfile)
{
	zstr filename;
	int filename_len;
	zend_uchar filename_type;
	int flags = REPORT_ERRORS;
	char buf[8192] = {0};
	long use_include_path = 0;
	php_stream *stream;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t|l", &filename, &filename_len, &filename_type, &use_include_path)) {
		return;
	}

	if (filename_type == IS_UNICODE && SUCCESS != php_stream_path_encode(NULL, &filename.s, &filename_len, filename.u, filename_len, REPORT_ERRORS, NULL)) {
		RETURN_FALSE;
	}

	if (use_include_path) {
		flags |= USE_PATH;
	}
	/* using a stream here is a bit more efficient (resource wise) than php_gzopen_wrapper */
	stream = php_stream_gzopen(NULL, filename.s, "rb", flags, NULL, NULL STREAMS_CC TSRMLS_CC);
	if (filename_type == IS_UNICODE) {
		efree(filename.s);
	}
	if (!stream) {
		/* Error reporting is already done by stream code */
		RETURN_FALSE;
	}

	array_init(return_value);
	while (php_stream_gets(stream, ZSTR(buf), sizeof(buf) - 1)) {
		add_next_index_string(return_value, buf, 1);
	}
	php_stream_close(stream);
}
/* }}} */

/* {{{ proto resource gzopen(string filename, string mode [, int use_include_path]) U
   Open a .gz-file and return a .gz-file pointer */
PHP_FUNCTION(gzopen)
{
	zstr filename;
	char *mode;
	int filename_len, mode_len;
	zend_uchar filename_type;
	int flags = REPORT_ERRORS;
	php_stream *stream;
	long use_include_path = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ts|l", &filename, &filename_len, &filename_type, &mode, &mode_len, &use_include_path) == FAILURE) {
		return;
	}

	if (filename_type == IS_UNICODE && SUCCESS != php_stream_path_encode(NULL, &filename.s, &filename_len, filename.u, filename_len, REPORT_ERRORS, NULL)) {
		RETURN_FALSE;
	}

	if (use_include_path) {
		flags |= USE_PATH;
	}
	stream = php_stream_gzopen(NULL, filename.s, mode, flags, NULL, NULL STREAMS_CC TSRMLS_CC);

	if (filename_type == IS_UNICODE) {
		efree(filename.s);
	}
	if (!stream) {
		RETURN_FALSE;
	}
	php_stream_to_zval(stream, return_value);
}	
/* }}} */

/* {{{ proto int readgzfile(string filename [, int use_include_path]) U
   Output a .gz-file */
PHP_FUNCTION(readgzfile)
{
	zstr filename;
	int filename_len;
	zend_uchar filename_type;
	int flags = REPORT_ERRORS;
	php_stream *stream;
	int size;
	long use_include_path = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t|l", &filename, &filename_len, &filename_type, &use_include_path) == FAILURE) {
		return;
	}

	if (filename_type == IS_UNICODE && SUCCESS != php_stream_path_encode(NULL, &filename.s, &filename_len, filename.u, filename_len, REPORT_ERRORS, NULL)) {
		RETURN_FALSE;
	}

	if (use_include_path) {
		flags |= USE_PATH;
	}
	stream = php_stream_gzopen(NULL, filename.s, "rb", flags, NULL, NULL STREAMS_CC TSRMLS_CC);
	if (filename_type == IS_UNICODE) {
		efree(filename.s);
	}
	if (!stream) {
		RETURN_FALSE;
	}
	size = php_stream_passthru(stream);
	php_stream_close(stream);
	RETURN_LONG(size);
}
/* }}} */

#define PHP_ZLIB_ENCODE_FUNC(name, default_encoding) \
PHP_FUNCTION(name) \
{ \
	char *in_buf, *out_buf; \
	int in_len; \
	size_t out_len; \
	long level = -1; \
	long encoding = default_encoding; \
	if (default_encoding) { \
		if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S|ll", &in_buf, &in_len, &level, &encoding)) { \
			return; \
		} \
	} else { \
		if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Sl|l", &in_buf, &in_len, &encoding, &level)) { \
			return; \
		} \
	} \
	if (level < -1 || level > 9) { \
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "compression level (%ld) must be within -1..9", level); \
		RETURN_FALSE; \
	} \
	switch (encoding) { \
		case PHP_ZLIB_ENCODING_RAW: \
		case PHP_ZLIB_ENCODING_GZIP: \
		case PHP_ZLIB_ENCODING_DEFLATE: \
			break; \
		default: \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "encoding mode must be either ZLIB_ENCODING_RAW, ZLIB_ENCODING_GZIP or ZLIB_ENCODING_DEFLATE"); \
			RETURN_FALSE; \
	} \
	if (SUCCESS != php_zlib_encode(in_buf, in_len, &out_buf, &out_len, encoding, level TSRMLS_CC)) { \
		RETURN_FALSE; \
	} \
	RETURN_STRINGL(out_buf, out_len, 0); \
}

#define PHP_ZLIB_DECODE_FUNC(name, encoding) \
PHP_FUNCTION(name) \
{ \
	char *in_buf, *out_buf; \
	int in_len; \
	size_t out_len; \
	long max_len = 0; \
	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S|l", &in_buf, &in_len, &max_len)) { \
		return; \
	} \
	if (max_len < 0) { \
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "length (%ld) must be greater or equal zero", max_len); \
		RETURN_FALSE; \
	} \
	if (SUCCESS != php_zlib_decode(in_buf, in_len, &out_buf, &out_len, encoding, max_len TSRMLS_CC)) { \
		RETURN_FALSE; \
	} \
	RETURN_STRINGL(out_buf, out_len, 0); \
}

/* {{{ proto binary zlib_encode(binary data, int encoding[, int level = -1]) U
	Compress data with the specified encoding */
PHP_ZLIB_ENCODE_FUNC(zlib_encode, 0);
/* }}} */

/* {{{ proto binary zlib_decode(binary data[, int max_decoded_len]) U
	Uncompress any raw/gzip/zlib encoded data */
PHP_ZLIB_DECODE_FUNC(zlib_decode, PHP_ZLIB_ENCODING_ANY);
/* }}} */

/* NOTE: The naming of these userland functions was quite unlucky */

/* {{{ proto binary gzdeflate(binary data[, int level = -1[, int encoding = ZLIB_ENCODING_RAW]) U
	Encode data with the raw deflate encoding */
PHP_ZLIB_ENCODE_FUNC(gzdeflate, PHP_ZLIB_ENCODING_RAW);
/* }}} */
/* {{{ proto binary gzencode(binary data[, int level = -1[, int encoding = ZLIB_ENCODING_GZIP]) U
	Encode data with the gzip encoding */
PHP_ZLIB_ENCODE_FUNC(gzencode, PHP_ZLIB_ENCODING_GZIP);
/* }}} */
/* {{{ proto binary gzcompress(binary data[, int level = -1[, int encoding = ZLIB_ENCODING_DEFLATE]) U
	Encode data with the zlib encoding */
PHP_ZLIB_ENCODE_FUNC(gzcompress, PHP_ZLIB_ENCODING_DEFLATE);
/* }}} */
/* {{{ proto binary gzinflate(binary data[, int max_decoded_len]) U
	Decode raw deflate encoded data */
PHP_ZLIB_DECODE_FUNC(gzinflate, PHP_ZLIB_ENCODING_RAW);
/* }}} */
/* {{{ proto binary gzdecode(binary data[, int max_decoded_len]) U
	Decode gzip encoded data */
PHP_ZLIB_DECODE_FUNC(gzdecode, PHP_ZLIB_ENCODING_GZIP);
/* }}} */
/* {{{ proto binary gzuncompress(binary data[, int max_decoded_len]) U
	Decode zlib encoded data */
PHP_ZLIB_DECODE_FUNC(gzuncompress, PHP_ZLIB_ENCODING_DEFLATE);
/* }}} */

#ifdef COMPILE_DL_ZLIB
ZEND_GET_MODULE(php_zlib)
#endif

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_zlib_get_coding_type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_gzfile, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, use_include_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_gzopen, 0, 0, 2)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, use_include_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_readgzfile, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, use_include_path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_zlib_encode, 0, 0, 2)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, encoding)
	ZEND_ARG_INFO(0, level)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_zlib_decode, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, max_decoded_len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_gzdeflate, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, level)
	ZEND_ARG_INFO(0, encoding)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_gzencode, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, level)
	ZEND_ARG_INFO(0, encoding)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_gzcompress, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, level)
	ZEND_ARG_INFO(0, encoding)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_gzinflate, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, max_decoded_len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_gzdecode, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, max_decoded_len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_gzuncompress, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, max_decoded_len)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ php_zlib_functions[] */
const zend_function_entry php_zlib_functions[] = {
	PHP_FE(readgzfile,						arginfo_readgzfile)
	PHP_FALIAS(gzrewind,	rewind,			NULL)
	PHP_FALIAS(gzclose,		fclose,			NULL)
	PHP_FALIAS(gzeof,		feof,			NULL)
	PHP_FALIAS(gzgetc,		fgetc,			NULL)
	PHP_FALIAS(gzgets,		fgets,			NULL)
	PHP_FALIAS(gzgetss,		fgetss,			NULL)
	PHP_FALIAS(gzread,		fread,			NULL)
	PHP_FE(gzopen,							arginfo_gzopen)
	PHP_FALIAS(gzpassthru,	fpassthru,		NULL)
	PHP_FALIAS(gzseek,		fseek,			NULL)
	PHP_FALIAS(gztell,		ftell,			NULL)
	PHP_FALIAS(gzwrite,		fwrite,			NULL)
	PHP_FALIAS(gzputs,		fwrite,			NULL)
	PHP_FE(gzfile,							NULL)
	PHP_FE(gzcompress,						arginfo_gzcompress)
	PHP_FE(gzuncompress,					arginfo_gzuncompress)
	PHP_FE(gzdeflate,						arginfo_gzdeflate)
	PHP_FE(gzinflate,						arginfo_gzinflate)
	PHP_FE(gzencode,						arginfo_gzencode)
	PHP_FE(gzdecode,						arginfo_gzdecode)
	PHP_FE(zlib_encode,						arginfo_zlib_encode)
	PHP_FE(zlib_decode,						arginfo_zlib_decode)
	PHP_FE(zlib_get_coding_type,			arginfo_zlib_get_coding_type)
	{NULL, NULL, NULL}
};
/* }}} */


/* {{{ OnUpdate_zlib_output_compression */
static PHP_INI_MH(OnUpdate_zlib_output_compression)
{
	int status, int_value;
	char *ini_value;
	
	if (new_value == NULL) {
		return FAILURE;
	}
	
	if (!strncasecmp(new_value, "off", sizeof("off"))) {
		new_value = "0";
		new_value_length = sizeof("0");
	} else if (!strncasecmp(new_value, "on", sizeof("on"))) {
		new_value = "1";
		new_value_length = sizeof("1");
	}
	
	int_value = zend_atoi(new_value, new_value_length);
	ini_value = zend_ini_string("output_handler", sizeof("output_handler"), 0);
	
	if (ini_value && *ini_value && int_value) {
		php_error_docref("ref.outcontrol" TSRMLS_CC, E_CORE_ERROR, "Cannot use both zlib.output_compression and output_handler together!!");
		return FAILURE;
	}

	if (stage == PHP_INI_STAGE_RUNTIME) {
		status = php_output_get_status(TSRMLS_C);
		if (status & PHP_OUTPUT_SENT) {
			php_error_docref("ref.outcontrol" TSRMLS_CC, E_WARNING, "Cannot change zlib.output_compression - headers already sent");
			return FAILURE;
		} else if ((status & PHP_OUTPUT_WRITTEN) && int_value) {
			php_error_docref("ref.outcontrol" TSRMLS_CC, E_WARNING, "Cannot enable zlib.output_compression - there has already been output");
			return FAILURE;
		}
	}
	
	status = OnUpdateLong(entry, new_value, new_value_length, mh_arg1, mh_arg2, mh_arg3, stage TSRMLS_CC);
	
	if (stage == PHP_INI_STAGE_RUNTIME && int_value) {
		zval tmp;
		
		INIT_PZVAL(&tmp);
		ZVAL_ASCII_STRING(&tmp, PHP_ZLIB_OUTPUT_HANDLER_NAME, ZSTR_DUPLICATE);
		if (!php_output_handler_started(&tmp TSRMLS_CC)) {
			php_zlib_output_compression_start(TSRMLS_C);
		}
		zval_dtor(&tmp);
	}
	
	return status;
}
/* }}} */

/* {{{ OnUpdate_zlib_output_handler */
static PHP_INI_MH(OnUpdate_zlib_output_handler)
{
	if (stage == PHP_INI_STAGE_RUNTIME && (php_output_get_status(TSRMLS_C) & PHP_OUTPUT_SENT)) {
		php_error_docref("ref.outcontrol" TSRMLS_CC, E_WARNING, "Cannot change zlib.output_handler - headers already sent");
		return FAILURE;
	}

	return OnUpdateString(entry, new_value, new_value_length, mh_arg1, mh_arg2, mh_arg3, stage TSRMLS_CC);
}
/* }}} */

/* {{{ INI */
PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("zlib.output_compression",      "0", PHP_INI_ALL, OnUpdate_zlib_output_compression,       output_compression,       zend_zlib_globals, zlib_globals)
	STD_PHP_INI_ENTRY("zlib.output_compression_level", "-1", PHP_INI_ALL, OnUpdateLong,                           output_compression_level, zend_zlib_globals, zlib_globals)
	STD_PHP_INI_ENTRY("zlib.output_handler",             "", PHP_INI_ALL, OnUpdate_zlib_output_handler,           output_handler,           zend_zlib_globals, zlib_globals)
PHP_INI_END()
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(zlib)
{
	php_register_url_stream_wrapper("compress.zlib", &php_stream_gzip_wrapper TSRMLS_CC);
	php_stream_filter_register_factory("zlib.*", &php_zlib_filter_factory TSRMLS_CC);
	
	PHP_OUTPUT_ALIAS_REGISTER("ob_gzhandler", php_zlib_output_handler_init);
	PHP_OUTPUT_CONFLICT_REGISTER("ob_gzhandler", php_zlib_output_conflict_check);
	PHP_OUTPUT_CONFLICT_REGISTER(PHP_ZLIB_OUTPUT_HANDLER_NAME, php_zlib_output_conflict_check);
	
	REGISTER_LONG_CONSTANT("FORCE_GZIP", PHP_ZLIB_ENCODING_GZIP, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FORCE_DEFLATE", PHP_ZLIB_ENCODING_DEFLATE, CONST_CS|CONST_PERSISTENT);
	
	REGISTER_LONG_CONSTANT("ZLIB_ENCODING_RAW", PHP_ZLIB_ENCODING_RAW, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_ENCODING_GZIP", PHP_ZLIB_ENCODING_GZIP, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_ENCODING_DEFLATE", PHP_ZLIB_ENCODING_DEFLATE, CONST_CS|CONST_PERSISTENT);
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(zlib)
{
	php_unregister_url_stream_wrapper("zlib" TSRMLS_CC);
	php_stream_filter_unregister_factory("zlib.*" TSRMLS_CC);
	
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(zlib)
{
	ZLIBG(compression_coding) = 0;
	
	php_zlib_output_compression_start(TSRMLS_C);
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(zlib)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "ZLib Support", "enabled");
	php_info_print_table_row(2, "Stream Wrapper", "compress.zlib://");
	php_info_print_table_row(2, "Stream Filter", "zlib.inflate, zlib.deflate");
	php_info_print_table_row(2, "Compiled Version", ZLIB_VERSION);
	php_info_print_table_row(2, "Linked Version", (char *) zlibVersion());
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ php_zlib_module_entry */
zend_module_entry php_zlib_module_entry = {
	STANDARD_MODULE_HEADER,
	"zlib",
	php_zlib_functions,
	PHP_MINIT(zlib),
	PHP_MSHUTDOWN(zlib),
	PHP_RINIT(zlib),
	NULL,
	PHP_MINFO(zlib),
	"2.0",
	PHP_MODULE_GLOBALS(zlib),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
