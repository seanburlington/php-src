--TEST--
Test session_encode() function : variation
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php

ob_start();

/* 
 * Prototype : string session_encode(void)
 * Description : Encodes the current session data as a string
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_encode() : variation ***\n";

var_dump(session_start());

$array = array(1,2,3);
$_SESSION["foo"] = &$array;
var_dump(session_encode());
var_dump(session_destroy());

echo "Done";
ob_end_flush();
?>
--EXPECTF--
*** Testing session_encode() : variation ***
bool(true)
unicode(34) "foo|a:3:{i:0;i:1;i:1;i:2;i:2;i:3;}"
bool(true)
Done
