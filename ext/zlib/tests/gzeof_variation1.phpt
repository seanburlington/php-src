--TEST--
Test function gzeof while writing.
--SKIPIF--
<?php 
if (!extension_loaded("zlib")) {
	print "skip - ZLIB extension not loaded"; 
}
?>
--FILE--
<?php

$filename = dirname(__FILE__)."/temp.txt.gz";
$h = gzopen($filename, 'w');
$str = b"Here is the string to be written. ";
$length = 10;
gzwrite( $h, $str );
var_dump(gzeof($h));
gzwrite( $h, $str, $length);
var_dump(gzeof($h));
gzclose($h);
var_dump(gzeof($h));
unlink($filename);
?>
===DONE===
--EXPECTF--
bool(false)
bool(false)

Warning: gzeof(): %d is not a valid stream resource in %s on line %d
bool(false)
===DONE===