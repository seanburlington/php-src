--TEST--
Test session_cache_limiter() function : variation
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php

ob_start();

/* 
 * Prototype : string session_cache_limiter([string $cache_limiter])
 * Description : Get and/or set the current cache limiter
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_cache_limiter() : variation ***\n";

var_dump(ini_get("session.cache_limiter"));
var_dump(session_start());
var_dump(ini_get("session.cache_limiter"));
var_dump(session_cache_limiter("public"));
var_dump(ini_get("session.cache_limiter"));
var_dump(session_destroy());
var_dump(ini_get("session.cache_limiter"));

echo "Done";
ob_end_flush();
?>
--EXPECTF--
*** Testing session_cache_limiter() : variation ***
unicode(7) "nocache"
bool(true)
unicode(7) "nocache"
unicode(7) "nocache"
unicode(6) "public"
bool(true)
unicode(6) "public"
Done

