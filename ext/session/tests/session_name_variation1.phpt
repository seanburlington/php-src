--TEST--
Test session_name() function : variation
--INI--
session.save_path=
session.name=PHPSESSID
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php

ob_start();

/* 
 * Prototype : string session_name([string $name])
 * Description : Get and/or set the current session name
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_name() : variation ***\n";

var_dump(session_name("\0"));
var_dump(session_start());
var_dump(session_name());
var_dump(session_destroy());
var_dump(session_name());

var_dump(session_name("\t"));
var_dump(session_start());
var_dump(session_name());
var_dump(session_destroy());
var_dump(session_name());

var_dump(session_name(""));
var_dump(session_start());
var_dump(session_name());
var_dump(session_destroy());
var_dump(session_name());

echo "Done";
ob_end_flush();
?>
--EXPECTF--
*** Testing session_name() : variation ***
unicode(9) "PHPSESSID"
bool(true)
unicode(0) ""
bool(true)
unicode(0) ""
unicode(0) ""
bool(true)
unicode(1) "	"
bool(true)
unicode(1) "	"
unicode(1) "	"
bool(true)
unicode(0) ""
bool(true)
unicode(0) ""
Done

