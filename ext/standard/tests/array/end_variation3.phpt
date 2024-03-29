--TEST--
Test end() function : usage variations - Referenced variables
--FILE--
<?php
/* Prototype  : mixed end(array $array_arg)
 * Description: Advances array argument's internal pointer to the last element and return it 
 * Source code: ext/standard/array.c
 */

/*
 * Test how the internal pointer is affected when two variables are referenced to each other
 */

echo "*** Testing end() : usage variations ***\n";

$array1 = array ('zero', 'one', 'two');

echo "\n-- Initial position of internal pointer --\n";
var_dump(current($array1));
end($array1);

// Test that when two variables are referenced to one another
// the internal pointer is the same for both
$array2 = &$array1;
echo "\n-- Position after calling end() --\n";
echo "\$array1: ";
var_dump(current($array1));
echo "\$array2: ";
var_dump(current($array2));
?>
===DONE===
--EXPECTF--
*** Testing end() : usage variations ***

-- Initial position of internal pointer --
unicode(4) "zero"

-- Position after calling end() --
$array1: unicode(3) "two"
$array2: unicode(3) "two"
===DONE===
