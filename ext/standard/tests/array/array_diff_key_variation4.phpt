--TEST--
Test array_diff_key() function : usage variation - Passing integer indexed array
--FILE--
<?php
/* Prototype  : array array_diff_key(array arr1, array arr2 [, array ...])
 * Description: Returns the entries of arr1 that have keys which are not present in any of the others arguments. 
 * Source code: ext/standard/array.c
 */

echo "*** Testing array_diff_key() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = array(-07 => '-07', 0xA => '0xA'); 

$input_arrays = array(
      'decimal indexed' => array(10 => '10', '-17' => '-17'),
      'octal indexed' => array(-011 => '-011', 012 => '012'),
      'hexa  indexed' => array(0x12 => '0x12', -0x7 => '-0x7', ),
);

// loop through each element of the array for arr1
foreach($input_arrays as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( array_diff_key($input_array, $value) );
      var_dump( array_diff_key($value, $input_array) );
}
?>
===DONE===
--EXPECTF--
*** Testing array_diff_key() : usage variation ***

--decimal indexed--
array(1) {
  [-7]=>
  unicode(3) "-07"
}
array(1) {
  [-17]=>
  unicode(3) "-17"
}

--octal indexed--
array(1) {
  [-7]=>
  unicode(3) "-07"
}
array(1) {
  [-9]=>
  unicode(4) "-011"
}

--hexa  indexed--
array(1) {
  [10]=>
  unicode(3) "0xA"
}
array(1) {
  [18]=>
  unicode(4) "0x12"
}
===DONE===

