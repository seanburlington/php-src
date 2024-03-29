--TEST--
Test strrpos() function : usage variations - unexpected inputs for 'haystack', 'needle' & 'offset' arguments
--FILE--
<?php
/* Prototype  : int strrpos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of last occurrence of 'needle' in 'haystack'.
 * Source code: ext/standard/string.c
*/

/* Test strrpos() function with unexpected inputs for 'haystack', 'needle' & 'offset' arguments */

echo "*** Testing strrpos() function: with unexpected values for haystack, needle & offset ***\n";

// get an unset variable
$unset_var = 'string_val';
unset($unset_var);

// defining a class
class sample  {
  public function __toString() {
    return "object";
  } 
}

//getting the resource
$file_handle = fopen(__FILE__, "r"); 

// array with different values
$values =  array (

  // integer values
  0,
  1,
  12345,
  -2345,

  // float values
  10.5,
  -10.5,
  10.5e10,
  10.6E-10,
  .5,

  // array values
  array(),
  array(0),
  array(1),
  array(1, 2),
  array('color' => 'red', 'item' => 'pen'),

  // boolean values
  true,
  false,
  TRUE,
  FALSE,

  // objects
  new sample(),

  // empty string
  "",
  '',

  // null vlaues
  NULL,
  null,

  //resource
  $file_handle,

  // undefined variable
  @$undefined_var,

  // unset variable
  @$unset_var
);


// loop through each element of the array and check the working of strrpos()
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  var_dump( strrpos($values[$index], $values[$index], $values[$index]) );
  $counter ++;
}

echo "*** Done ***";
?>
--EXPECTF--
*** Testing strrpos() function: with unexpected values for haystack, needle & offset ***
-- Iteration 1 --
bool(false)
-- Iteration 2 --

Warning: strrpos(): Offset is greater than the length of haystack string in %s on line %d
bool(false)
-- Iteration 3 --

Warning: strrpos(): Offset is greater than the length of haystack string in %s on line %d
bool(false)
-- Iteration 4 --

Warning: Needle argument codepoint value out of range (0 - 0x10FFFF) in %s on line %d
bool(false)
-- Iteration 5 --

Warning: strrpos(): Offset is greater than the length of haystack string in %s on line %d
bool(false)
-- Iteration 6 --

Warning: Needle argument codepoint value out of range (0 - 0x10FFFF) in %s on line %d
bool(false)
-- Iteration 7 --

Warning: Needle argument codepoint value out of range (0 - 0x10FFFF) in %s on line %d
bool(false)
-- Iteration 8 --
bool(false)
-- Iteration 9 --
bool(false)
-- Iteration 10 --

Warning: strrpos() expects parameter 3 to be long, array given in %s on line %d
bool(false)
-- Iteration 11 --

Warning: strrpos() expects parameter 3 to be long, array given in %s on line %d
bool(false)
-- Iteration 12 --

Warning: strrpos() expects parameter 3 to be long, array given in %s on line %d
bool(false)
-- Iteration 13 --

Warning: strrpos() expects parameter 3 to be long, array given in %s on line %d
bool(false)
-- Iteration 14 --

Warning: strrpos() expects parameter 3 to be long, array given in %s on line %d
bool(false)
-- Iteration 15 --

Warning: strrpos(): Offset is greater than the length of haystack string in %s on line %d
bool(false)
-- Iteration 16 --
bool(false)
-- Iteration 17 --

Warning: strrpos(): Offset is greater than the length of haystack string in %s on line %d
bool(false)
-- Iteration 18 --
bool(false)
-- Iteration 19 --

Warning: strrpos() expects parameter 3 to be long, object given in %s on line %d
bool(false)
-- Iteration 20 --

Warning: strrpos() expects parameter 3 to be long, Unicode string given in %s on line %d
bool(false)
-- Iteration 21 --

Warning: strrpos() expects parameter 3 to be long, Unicode string given in %s on line %d
bool(false)
-- Iteration 22 --
bool(false)
-- Iteration 23 --
bool(false)
-- Iteration 24 --

Warning: strrpos() expects parameter 3 to be long, resource given in %s on line %d
bool(false)
-- Iteration 25 --
bool(false)
-- Iteration 26 --
bool(false)
*** Done ***
