--TEST--
Test eregi_replace() function : usage variations - unexpected type arg 3
--FILE--
<?php
/* Prototype  : proto string eregi_replace(string pattern, string replacement, string string)
 * Description: Replace regular expression 
 * Source code: ext/standard/reg.c
 * Alias to functions: 
 */

function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
	echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
set_error_handler('test_error_handler');

echo "*** Testing eregi_replace() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$pattern = b'1';
$replacement = b'new value';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//array of values to iterate over
$values = array(

      // int data
      0,
      1,
      12345,
      -2345,

      // float data
      10.5,
      -10.5,
      10.1234567e10,
      10.7654321E-10,
      .5,

      // array data
      array(),
      array(0),
      array(1),
      array(1, 2),
      array('color' => 'red', 'item' => 'pen'),

      // null data
      NULL,
      null,

      // boolean data
      true,
      false,
      TRUE,
      FALSE,

      // empty data
      "",
      '',

      // object data
      new stdclass(),

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,
);

// loop through each element of the array for string

foreach($values as $value) {
      echo "\nArg value $value \n";
      var_dump( eregi_replace($pattern, $replacement, $value) );
};

echo "Done";
?>
--EXPECTF--
*** Testing eregi_replace() : usage variations ***
Error: 8 - Undefined variable: undefined_var, %s(64)
Error: 8 - Undefined variable: unset_var, %s(67)

Arg value 0 
string(1) "0"

Arg value 1 
string(9) "new value"

Arg value 12345 
string(13) "new value2345"

Arg value -2345 
string(5) "-2345"

Arg value 10.5 
string(12) "new value0.5"

Arg value -10.5 
string(13) "-new value0.5"

Arg value 101234567000 
string(28) "new value0new value234567000"

Arg value 1.07654321E-9 
string(29) "new value.0765432new valueE-9"

Arg value 0.5 
string(3) "0.5"
Error: 8 - Array to string conversion, %s(73)

Arg value Array 
Error: 8 - Array to string conversion, %s(74)
string(5) "Array"
Error: 8 - Array to string conversion, %s(73)

Arg value Array 
Error: 8 - Array to string conversion, %s(74)
string(5) "Array"
Error: 8 - Array to string conversion, %s(73)

Arg value Array 
Error: 8 - Array to string conversion, %s(74)
string(5) "Array"
Error: 8 - Array to string conversion, %s(73)

Arg value Array 
Error: 8 - Array to string conversion, %s(74)
string(5) "Array"
Error: 8 - Array to string conversion, %s(73)

Arg value Array 
Error: 8 - Array to string conversion, %s(74)
string(5) "Array"

Arg value  
string(0) ""

Arg value  
string(0) ""

Arg value 1 
string(9) "new value"

Arg value  
string(0) ""

Arg value 1 
string(9) "new value"

Arg value  
string(0) ""

Arg value  
string(0) ""

Arg value  
string(0) ""
Error: 4096 - Object of class stdClass could not be converted to string, %s(73)

Arg value  
Error: 4096 - Object of class stdClass could not be converted to binary string, %s(74)
Error: 8 - Object of class stdClass to string conversion, %s(74)
string(6) "Object"

Arg value  
string(0) ""

Arg value  
string(0) ""
Done
