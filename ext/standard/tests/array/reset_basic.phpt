--TEST--
Test reset() function : basic functionality
--FILE--
<?php
/* Prototype  : mixed reset(array $array_arg)
 * Description: Set array argument's internal pointer to the first element and return it 
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of reset()
 */

echo "*** Testing reset() : basic functionality ***\n";

$array = array('zero', 'one', 200 => 'two');

echo "\n-- Initial Position: --\n";
echo key($array) . " => " . current($array) . "\n";

echo "\n-- Call to next() --\n";
var_dump(next($array));

echo "\n-- Current Position: --\n";
echo key($array) . " => " . current($array) . "\n";

echo "\n-- Call to reset() --\n";
var_dump(reset($array));
?>
===DONE===
--EXPECTF--
*** Testing reset() : basic functionality ***

-- Initial Position: --
0 => zero

-- Call to next() --
unicode(3) "one"

-- Current Position: --
1 => one

-- Call to reset() --
unicode(4) "zero"
===DONE===
