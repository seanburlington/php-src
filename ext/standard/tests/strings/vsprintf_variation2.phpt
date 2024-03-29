--TEST--
Test vsprintf() function : usage variations - unexpected values for args argument
--FILE--
<?php
/* Prototype  : string vsprintf(string format, array args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vsprintf() when different unexpected values are passed to
 * the '$args' arguments of the function
*/

echo "*** Testing vsprintf() : with unexpected values for args argument ***\n";

// initialising the required variables
$format = '%s';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// declaring a class
class sample
{
  public function __toString() {
  return "object";
  }
}

// Defining resource
$file_handle = fopen(__FILE__, 'r');


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

  // string data
  "string",
  'string',

  // object data
  new sample(),

  // undefined data
  @$undefined_var,

  // unset data
  @$unset_var,

  // resource data
  $file_handle
);

// loop through each element of the array for args
$counter = 1;
foreach($values as $value) {
  echo "\n-- Iteration $counter --\n";
  var_dump( vsprintf($format,$value) );
  $counter++;
};

// closing the resource
fclose($file_handle);

echo "Done";
?>
--EXPECTF--
*** Testing vsprintf() : with unexpected values for args argument ***

-- Iteration 1 --
unicode(1) "0"

-- Iteration 2 --
unicode(1) "1"

-- Iteration 3 --
unicode(5) "12345"

-- Iteration 4 --
unicode(5) "-2345"

-- Iteration 5 --
unicode(4) "10.5"

-- Iteration 6 --
unicode(5) "-10.5"

-- Iteration 7 --
unicode(12) "101234567000"

-- Iteration 8 --
unicode(13) "1.07654321E-9"

-- Iteration 9 --
unicode(3) "0.5"

-- Iteration 10 --

Warning: vsprintf(): Too few arguments in %s on line %d
bool(false)

-- Iteration 11 --

Warning: vsprintf(): Too few arguments in %s on line %d
bool(false)

-- Iteration 12 --
unicode(1) "1"

-- Iteration 13 --
unicode(0) ""

-- Iteration 14 --
unicode(1) "1"

-- Iteration 15 --
unicode(0) ""

-- Iteration 16 --
unicode(0) ""

-- Iteration 17 --
unicode(0) ""

-- Iteration 18 --
unicode(6) "string"

-- Iteration 19 --
unicode(6) "string"

-- Iteration 20 --

Warning: vsprintf(): Too few arguments in %s on line %d
bool(false)

-- Iteration 21 --

Warning: vsprintf(): Too few arguments in %s on line %d
bool(false)

-- Iteration 22 --

Warning: vsprintf(): Too few arguments in %s on line %d
bool(false)

-- Iteration 23 --
unicode(%d) "Resource id #%d"
Done
