--TEST--
Test array_reverse() function : basic functionality - associative array for 'array' argument
--FILE--
<?php
/* Prototype  : array array_reverse(array $array [, bool $preserve_keys])
 * Description: Return input as a new array with the order of the entries reversed 
 * Source code: ext/standard/array.c
*/

/*
 * Testing array_reverse() with associative array for $array argument
*/

echo "*** Testing array_reverse() : basic functionality ***\n";

// Initialise the array
$array = array("a" => "hello", 123 => "number", 'string' => 'blue', "10" => 13.33);

// Calling array_reverse() with default arguments
var_dump( array_reverse($array) );

// Calling array_reverse() with all possible arguments
var_dump( array_reverse($array, true) );  // expects the keys to be preserved
var_dump( array_reverse($array, false) );  // expects the keys not to be preserved

echo "Done";
?>
--EXPECTF--
*** Testing array_reverse() : basic functionality ***
array(4) {
  [0]=>
  float(13.33)
  [u"string"]=>
  unicode(4) "blue"
  [1]=>
  unicode(6) "number"
  [u"a"]=>
  unicode(5) "hello"
}
array(4) {
  [10]=>
  float(13.33)
  [u"string"]=>
  unicode(4) "blue"
  [123]=>
  unicode(6) "number"
  [u"a"]=>
  unicode(5) "hello"
}
array(4) {
  [0]=>
  float(13.33)
  [u"string"]=>
  unicode(4) "blue"
  [1]=>
  unicode(6) "number"
  [u"a"]=>
  unicode(5) "hello"
}
Done
