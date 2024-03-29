--TEST--
gmp_or() basic tests
--SKIPIF--
<?php if (!extension_loaded("gmp")) print "skip"; ?>
--FILE--
<?php

var_dump(gmp_strval(gmp_or("111111", "2222222")));
var_dump(gmp_strval(gmp_or(123123, 435234)));
var_dump(gmp_strval(gmp_or(555, "2342341123")));
var_dump(gmp_strval(gmp_or(-1, 3333)));
var_dump(gmp_strval(gmp_or(4545, -20)));
var_dump(gmp_strval(gmp_or("test", "no test")));

$n = gmp_init("987657876543456");
var_dump(gmp_strval(gmp_or($n, "34332")));
$n1 = gmp_init("987657878765436543456");
var_dump(gmp_strval(gmp_or($n, $n1)));

var_dump(gmp_or($n, $n1, 1));
var_dump(gmp_or(1));
var_dump(gmp_or(array(), 1));
var_dump(gmp_or(1, array()));
var_dump(gmp_or(array(), array()));

echo "Done\n";
?>
--EXPECTF--
unicode(7) "2226831"
unicode(6) "517363"
unicode(10) "2342341163"
unicode(2) "-1"
unicode(3) "-19"
unicode(1) "0"
unicode(15) "987657876576252"
unicode(21) "987658441719689394144"

Warning: gmp_or() expects exactly 2 parameters, 3 given in %s on line %d
NULL

Warning: gmp_or() expects exactly 2 parameters, 1 given in %s on line %d
NULL

Warning: gmp_or(): Unable to convert variable to GMP - wrong type in %s on line %d
bool(false)

Warning: gmp_or(): Unable to convert variable to GMP - wrong type in %s on line %d
bool(false)

Warning: gmp_or(): Unable to convert variable to GMP - wrong type in %s on line %d
bool(false)
Done
