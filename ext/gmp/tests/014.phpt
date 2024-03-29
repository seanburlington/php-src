--TEST--
gmp_fact() basic tests
--SKIPIF--
<?php if (!extension_loaded("gmp")) print "skip"; ?>
--FILE--
<?php

var_dump(gmp_strval(gmp_fact(0)));
var_dump(gmp_strval(gmp_fact("")));
var_dump(gmp_strval(gmp_fact("0")));
var_dump(gmp_strval(gmp_fact("-1")));
var_dump(gmp_strval(gmp_fact(-1)));
var_dump(gmp_strval(gmp_fact(1.1)));
var_dump(gmp_strval(gmp_fact(20)));
var_dump(gmp_strval(gmp_fact("50")));
var_dump(gmp_strval(gmp_fact("10")));
var_dump(gmp_strval(gmp_fact("0000")));

$n = gmp_init(12);
var_dump(gmp_strval(gmp_fact($n)));
$n = gmp_init(-10);
var_dump(gmp_strval(gmp_fact($n)));

var_dump(gmp_fact());
var_dump(gmp_fact(1,1));
var_dump(gmp_fact(array()));
var_dump(gmp_strval(gmp_fact(array())));

echo "Done\n";
?>
--EXPECTF--
unicode(1) "1"
unicode(1) "1"
unicode(1) "1"

Warning: gmp_fact(): Number has to be greater than or equal to 0 in %s on line %d
unicode(1) "0"

Warning: gmp_fact(): Number has to be greater than or equal to 0 in %s on line %d
unicode(1) "0"
unicode(1) "1"
unicode(19) "2432902008176640000"
unicode(65) "30414093201713378043612608166064768844377641568960512000000000000"
unicode(7) "3628800"
unicode(1) "1"
unicode(11) "87178291200"

Warning: gmp_fact(): Number has to be greater than or equal to 0 in %s on line %d
unicode(1) "0"

Warning: gmp_fact() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: gmp_fact() expects exactly 1 parameter, 2 given in %s on line %d
NULL
resource(%d) of type (GMP integer)
unicode(1) "1"
Done
