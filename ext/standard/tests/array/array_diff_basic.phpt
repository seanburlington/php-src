--TEST--
Test array_diff() function : basic functionality
--FILE--
<?php
/* Prototype  : array array_diff(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are not present 
 * in any of the others arguments. 
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of array_diff
 */

echo "*** Testing array_diff() : basic functionality ***\n";

//Test indexed array with integers as elements
$array_int1 = array (1, 2, 3, 4);
$array_int2 = array (3, 4, 5, 6);

echo "-- Test indexed array with integers as elements --\n";
var_dump(array_diff($array_int1, $array_int2));
var_dump(array_diff($array_int2, $array_int1));


//Test indexed array with strings as elements
$array_string1 = array ('one', 'two', 'three', 'four');
$array_string2 = array ('three', 'four', 'five', 'six');

echo "-- Test indexed array with strings as elements --\n";
var_dump(array_diff($array_string1, $array_string2));
var_dump(array_diff($array_string2, $array_string1));

//Test associative array with strings as keys and integers as elements
$array_assoc_int1 = array ('one' => 1, 'two' => 2, 'three' => 3, 'four' => 4);
$array_assoc_int2 = array ('three' => 3, 'four' => 4, 'five' => 5, 'six' => 6);

echo "-- Test associative array with strings as keys and integers as elements --\n";
var_dump(array_diff($array_assoc_int1, $array_assoc_int2));
var_dump(array_diff($array_assoc_int2, $array_assoc_int1));

//Test associative array with strings as keys and elements
$array_assoc_str1 = array ('one' => 'un', 'two' => 'deux', 'three' => 'trois', 'four' => 'quatre');
$array_assoc_str2 = array ('three' => 'trois', 'four' => 'quatre', 'five' => 'cinq', 'six' => 'six');

echo "-- Test associative array with strings as keys and integers as elements --\n";
var_dump(array_diff($array_assoc_str1, $array_assoc_str2));
var_dump(array_diff($array_assoc_str2, $array_assoc_str1));

echo "-- Test array_diff with more than 2 arguments --\n";
var_dump(array_diff($array_int1, $array_int2, $array_string1, $array_string2));

echo "Done";
?>
--EXPECT--
*** Testing array_diff() : basic functionality ***
-- Test indexed array with integers as elements --
array(2) {
  [0]=>
  int(1)
  [1]=>
  int(2)
}
array(2) {
  [2]=>
  int(5)
  [3]=>
  int(6)
}
-- Test indexed array with strings as elements --
array(2) {
  [0]=>
  unicode(3) "one"
  [1]=>
  unicode(3) "two"
}
array(2) {
  [2]=>
  unicode(4) "five"
  [3]=>
  unicode(3) "six"
}
-- Test associative array with strings as keys and integers as elements --
array(2) {
  [u"one"]=>
  int(1)
  [u"two"]=>
  int(2)
}
array(2) {
  [u"five"]=>
  int(5)
  [u"six"]=>
  int(6)
}
-- Test associative array with strings as keys and integers as elements --
array(2) {
  [u"one"]=>
  unicode(2) "un"
  [u"two"]=>
  unicode(4) "deux"
}
array(2) {
  [u"five"]=>
  unicode(4) "cinq"
  [u"six"]=>
  unicode(3) "six"
}
-- Test array_diff with more than 2 arguments --
array(2) {
  [0]=>
  int(1)
  [1]=>
  int(2)
}
Done