--TEST--
Bug #42976 (Crash when constructor for newInstance() or newInstanceArgs() fails)
--FILE--
<?php

Class C {
	function __construct(&$x) {
		$x = "x.changed";
	}
}

$x = "x.original";
new C($x); // OK
var_dump($x);

$rc = new ReflectionClass('C');
$x = "x.original";
$rc->newInstance($x); // causes crash
var_dump($x);
$x = "x.original";
$rc->newInstanceArgs(array($x)); // causes crash	
var_dump($x);

echo "Done\n";
?>
--EXPECTF--	
unicode(9) "x.changed"

Warning: Parameter 1 to C::__construct() expected to be a reference, value given in %sbug42976.php on line 15

Warning: Invocation of C's constructor failed in %sbug42976.php on line 15
unicode(10) "x.original"

Warning: Parameter 1 to C::__construct() expected to be a reference, value given in %sbug42976.php on line 18

Warning: Invocation of C's constructor failed in %sbug42976.php on line 18
unicode(10) "x.original"
Done
