--TEST--
COM: mapping a safearray
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded("com_dotnet")) print "skip COM/.Net support not present"; ?>
--FILE--
<?php // $Id: 27974.phpt,v 1.1 2004/04/22 14:29:33 wez Exp $
error_reporting(E_ALL);

try {
$v = new VARIANT(array("123", "456", "789"));
	var_dump($v);
	print $v[0] . "\n";
	print $v[1] . "\n";
	print $v[2] . "\n";
	echo "OK!";
} catch (Exception $e) {
	print $e;
}
?>
--EXPECT--
object(variant)#1 (0) {
}
123
456
789
OK!
