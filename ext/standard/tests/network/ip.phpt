--TEST--
ip2long() & long2ip() tests
--SKIPIF--
<?php
if (PHP_INT_SIZE != 4) die("skip this test is for 32bit platform only");
?>

--FILE--
<?php

$array = array(
	"127.0.0.1",
	"10.0.0.1",
	"255.255.255.255",
	"255.255.255.0",
	"0.0.0.0",
	"66.163.161.116",
);

foreach ($array as $ip) {
	var_dump($long = ip2long($ip));
	var_dump(long2ip($long));
}

var_dump(ip2long());
var_dump(ip2long(""));
var_dump(ip2long("777.777.777.777"));
var_dump(ip2long("111.111.111.111"));
var_dump(ip2long(array()));

var_dump(long2ip());
var_dump(long2ip(-110000));
var_dump(long2ip(""));
var_dump(long2ip(array()));

echo "Done\n";
?>
--EXPECTF--
int(2130706433)
unicode(9) "127.0.0.1"
int(167772161)
unicode(8) "10.0.0.1"
int(-1)
unicode(15) "255.255.255.255"
int(-256)
unicode(13) "255.255.255.0"
int(0)
unicode(7) "0.0.0.0"
int(1118019956)
unicode(14) "66.163.161.116"

Warning: ip2long() expects exactly 1 parameter, 0 given in %s on line %d
NULL
bool(false)
bool(false)
int(1869573999)

Warning: ip2long() expects parameter 1 to be binary string, array given in %s on line %d
NULL

Warning: long2ip() expects exactly 1 parameter, 0 given in %s on line %d
NULL
unicode(13) "255.254.82.80"
unicode(7) "0.0.0.0"

Warning: long2ip() expects parameter 1 to be binary string, array given in %s on line %d
NULL
Done
