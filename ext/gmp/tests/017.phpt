--TEST--
gmp_sqrt() basic tests
--SKIPIF--
<?php if (!extension_loaded("gmp")) print "skip"; ?>
--FILE--
<?php

var_dump(gmp_strval(gmp_sqrt(-2)));
var_dump(gmp_strval(gmp_sqrt("-2")));
var_dump(gmp_strval(gmp_sqrt("0")));
var_dump(gmp_strval(gmp_sqrt("2")));
var_dump(gmp_strval(gmp_sqrt("144")));

$n = gmp_init(0);
var_dump(gmp_strval(gmp_sqrt($n)));
$n = gmp_init(-144);
var_dump(gmp_strval(gmp_sqrt($n)));
$n = gmp_init(777);
var_dump(gmp_strval(gmp_sqrt($n)));

var_dump(gmp_sqrt($n, 1));
var_dump(gmp_sqrt());
var_dump(gmp_sqrt(array()));

echo "Done\n";
?>
--EXPECTF--
Warning: gmp_sqrt(): Number has to be greater than or equal to 0 in %s on line %d
unicode(1) "0"

Warning: gmp_sqrt(): Number has to be greater than or equal to 0 in %s on line %d
unicode(1) "0"
unicode(1) "0"
unicode(1) "1"
unicode(2) "12"
unicode(1) "0"

Warning: gmp_sqrt(): Number has to be greater than or equal to 0 in %s on line %d
unicode(1) "0"
unicode(2) "27"

Warning: gmp_sqrt() expects exactly 1 parameter, 2 given in %s on line %d
NULL

Warning: gmp_sqrt() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: gmp_sqrt(): Unable to convert variable to GMP - wrong type in %s on line %d
bool(false)
Done
