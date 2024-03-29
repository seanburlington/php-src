--TEST--
Test utf8_decode() function : usage variations  - different types for data
--SKIPIF--
<?php 
if (!extension_loaded("xml")) {
	print "skip - XML extension not loaded"; 
}	 
?>
--FILE--
<?php
/* Prototype  : proto string utf8_decode(string data)
 * Description: Converts a UTF-8 encoded string to ISO-8859-1 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing utf8_decode() : usage variations ***\n";
error_reporting(E_ALL & ~E_NOTICE);

class aClass {
   function __toString() {
       return "Some Ascii Data";
   }
}

// Initialise function arguments not being substituted (if any)

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
      new aClass(),

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,
);

// loop through each element of the array for data

foreach($values as $value) {
      echo "\nArg value $value \n";
      var_dump( utf8_decode($value) );
};

echo "Done";
?>
--EXPECTF--
*** Testing utf8_decode() : usage variations ***

Arg value 0 
unicode(1) "0"

Arg value 1 
unicode(1) "1"

Arg value 12345 
unicode(5) "12345"

Arg value -2345 
unicode(5) "-2345"

Arg value 10.5 
unicode(4) "10.5"

Arg value -10.5 
unicode(5) "-10.5"

Arg value 101234567000 
unicode(12) "101234567000"

Arg value 1.07654321E-9 
unicode(13) "1.07654321E-9"

Arg value 0.5 
unicode(3) "0.5"

Arg value Array 

Warning: utf8_decode() expects parameter 1 to be string (Unicode or binary), array given in %s on line %d
bool(false)

Arg value Array 

Warning: utf8_decode() expects parameter 1 to be string (Unicode or binary), array given in %s on line %d
bool(false)

Arg value Array 

Warning: utf8_decode() expects parameter 1 to be string (Unicode or binary), array given in %s on line %d
bool(false)

Arg value Array 

Warning: utf8_decode() expects parameter 1 to be string (Unicode or binary), array given in %s on line %d
bool(false)

Arg value Array 

Warning: utf8_decode() expects parameter 1 to be string (Unicode or binary), array given in %s on line %d
bool(false)

Arg value  
unicode(0) ""

Arg value  
unicode(0) ""

Arg value 1 
unicode(1) "1"

Arg value  
unicode(0) ""

Arg value 1 
unicode(1) "1"

Arg value  
unicode(0) ""

Arg value  
unicode(0) ""

Arg value  
unicode(0) ""

Arg value Some Ascii Data 
unicode(15) "Some Ascii Data"

Arg value  
unicode(0) ""

Arg value  
unicode(0) ""
Done

