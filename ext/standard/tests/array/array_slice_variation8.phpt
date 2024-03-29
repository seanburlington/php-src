--TEST--
Test array_slice() function : usage variations - multidimensional arrays
--FILE--
<?php
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length 
 * Source code: ext/standard/array.c
 */

/*
 * Test array_slice when passed 
 * 1. a two-dimensional array as $input argument
 * 2. a sub-array as $input argument
 */

echo "*** Testing array_slice() : usage variations ***\n";

$input = array ('zero', 'one', array('zero', 'un', 'deux'), 9 => 'nine');

echo "\n-- Slice a two-dimensional array --\n";
var_dump(array_slice($input, 1, 3));

echo "\n-- \$input is a sub-array --\n";
var_dump(array_slice($input[2], 1, 2));

echo "Done";
?>

--EXPECTF--
*** Testing array_slice() : usage variations ***

-- Slice a two-dimensional array --
array(3) {
  [0]=>
  unicode(3) "one"
  [1]=>
  array(3) {
    [0]=>
    unicode(4) "zero"
    [1]=>
    unicode(2) "un"
    [2]=>
    unicode(4) "deux"
  }
  [2]=>
  unicode(4) "nine"
}

-- $input is a sub-array --
array(2) {
  [0]=>
  unicode(2) "un"
  [1]=>
  unicode(4) "deux"
}
Done
