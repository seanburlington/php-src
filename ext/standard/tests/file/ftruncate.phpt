--TEST--
ftruncate() tests
--FILE--
<?php

$filename = dirname(__FILE__)."/ftruncate.dat";

file_put_contents($filename, "some test data inside");

$fp = fopen($filename, "r");
var_dump(ftruncate($fp, 10));
fclose($fp);
var_dump(file_get_contents($filename));

$fp = fopen($filename, "w");
var_dump(ftruncate($fp, 10));
fclose($fp);
var_dump(file_get_contents($filename));

file_put_contents($filename, "some test data inside");

$fp = fopen($filename, "a");
var_dump(ftruncate($fp, 10));
fclose($fp);
var_dump(file_get_contents($filename));

$fp = fopen($filename, "a");
var_dump(ftruncate($fp, 0));
fclose($fp);
var_dump(file_get_contents($filename));

file_put_contents($filename, "some test data inside");

$fp = fopen($filename, "a");
var_dump(ftruncate($fp, -1000000000));
fclose($fp);
var_dump(file_get_contents($filename));

@unlink($filename);
echo "Done\n";
?>
--EXPECTF--
bool(false)
string(21) "some test data inside"
bool(true)
string(10) "          "
bool(true)
string(10) "some test "
bool(true)
string(0) ""
bool(false)
string(21) "some test data inside"
Done
