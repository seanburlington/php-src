--TEST--
Test function fflush() on a zlib stream wrapper
--SKIPIF--
<?php 
if (!extension_loaded("zlib")) {
	print "skip - ZLIB extension not loaded"; 
}
?>
--FILE--
<?php

$filename = "temp.txt.gz";
$h = gzopen($filename, 'w');
$str = b"Here is the string to be written.";
$length = 10;
var_dump(fflush($h));
gzwrite( $h, $str);
gzwrite( $h, $str);
var_dump(fflush($h));
gzclose($h);

$h = gzopen($filename, 'r');
gzpassthru($h);
gzclose($h);
echo "\n";
unlink($filename);
?>
===DONE===
--EXPECT--
bool(true)
bool(true)
Here is the string to be written.Here is the string to be written.
===DONE===