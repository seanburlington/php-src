--TEST--
gmp_sub() tests
--SKIPIF--
<?php if (!extension_loaded("gmp")) print "skip"; ?>
--FILE--
<?php

var_dump(gmp_sub());
var_dump(gmp_sub(""));
var_dump(gmp_sub("", ""));
var_dump(gmp_sub("", "", ""));
var_dump(gmp_sub(array(), array()));

var_dump($g = gmp_sub(10000, 10001));
var_dump(gmp_strval($g));
var_dump($g = gmp_sub(10000, -1));
var_dump(gmp_strval($g));
var_dump($g = gmp_sub(10000, new stdclass));
var_dump(gmp_strval($g));
var_dump($g = gmp_sub(new stdclass, 100));
var_dump(gmp_strval($g));

echo "Done\n";
?>
--EXPECTF--
Warning: gmp_sub() expects exactly 2 parameters, 0 given in %s on line %d
NULL

Warning: gmp_sub() expects exactly 2 parameters, 1 given in %s on line %d
NULL
bool(false)

Warning: gmp_sub() expects exactly 2 parameters, 3 given in %s on line %d
NULL

Warning: gmp_sub(): Unable to convert variable to GMP - wrong type in %s on line %d
bool(false)
resource(%d) of type (GMP integer)
unicode(2) "-1"
resource(%d) of type (GMP integer)
unicode(5) "10001"

Warning: gmp_sub(): Unable to convert variable to GMP - wrong type in %s on line %d
bool(false)
unicode(1) "0"

Warning: gmp_sub(): Unable to convert variable to GMP - wrong type in %s on line %d
bool(false)
unicode(1) "0"
Done
