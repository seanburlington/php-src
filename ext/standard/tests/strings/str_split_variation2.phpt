--TEST--
Test str_split() function : usage variations - unexpected values for 'split_length' argument(Bug#42866) 
--SKIPIF--
<?php
if (PHP_INT_SIZE != 4) die("skip this test is for 32bit platform only");
?>
--FILE--
<?php
/* Prototype  : array str_split(string $str [, int $split_length])
 * Description: Convert a string to an array. If split_length is 
                specified, break the string down into chunks each 
                split_length characters long. 
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

echo "*** Testing str_split() : unexpected values for 'split_length' ***\n";

// Initialise function arguments
$str = 'variation2:split_length';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//defining class for object variable
class MyClass
{
  public function __toString()
  {
    return "object";
  }
}

//resource variable
$fp = fopen(__FILE__, 'r');

//different values for 'split_length'
$values = array(

  // float data
  10.5,
  -10.5,
  10.6E-10,
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

  // string data
  "string",
  'string',

  // object data
  new MyClass(),

  // undefined data
  @$undefined_var,

  // unset data
  @$unset_var,

  //resource data
  $fp
);

// loop through each element of $values for 'split_length'
for($count = 0; $count < count($values); $count++) {
  echo "--Iteration ".($count+1)." --\n";
  var_dump( str_split($str, $values[$count]) );
}

//closing resource
fclose($fp);

echo "Done";
?>
--EXPECTF--
*** Testing str_split() : unexpected values for 'split_length' ***
--Iteration 1 --
array(3) {
  [0]=>
  unicode(10) "variation2"
  [1]=>
  unicode(10) ":split_len"
  [2]=>
  unicode(3) "gth"
}
--Iteration 2 --

Warning: str_split(): The length of each segment must be greater than zero in %s on line %d
bool(false)
--Iteration 3 --

Warning: str_split(): The length of each segment must be greater than zero in %s on line %d
bool(false)
--Iteration 4 --

Warning: str_split(): The length of each segment must be greater than zero in %s on line %d
bool(false)
--Iteration 5 --

Warning: str_split() expects parameter 2 to be long, array given in %s on line %d
NULL
--Iteration 6 --

Warning: str_split() expects parameter 2 to be long, array given in %s on line %d
NULL
--Iteration 7 --

Warning: str_split() expects parameter 2 to be long, array given in %s on line %d
NULL
--Iteration 8 --

Warning: str_split() expects parameter 2 to be long, array given in %s on line %d
NULL
--Iteration 9 --

Warning: str_split() expects parameter 2 to be long, array given in %s on line %d
NULL
--Iteration 10 --

Warning: str_split(): The length of each segment must be greater than zero in %s on line %d
bool(false)
--Iteration 11 --

Warning: str_split(): The length of each segment must be greater than zero in %s on line %d
bool(false)
--Iteration 12 --
array(23) {
  [0]=>
  unicode(1) "v"
  [1]=>
  unicode(1) "a"
  [2]=>
  unicode(1) "r"
  [3]=>
  unicode(1) "i"
  [4]=>
  unicode(1) "a"
  [5]=>
  unicode(1) "t"
  [6]=>
  unicode(1) "i"
  [7]=>
  unicode(1) "o"
  [8]=>
  unicode(1) "n"
  [9]=>
  unicode(1) "2"
  [10]=>
  unicode(1) ":"
  [11]=>
  unicode(1) "s"
  [12]=>
  unicode(1) "p"
  [13]=>
  unicode(1) "l"
  [14]=>
  unicode(1) "i"
  [15]=>
  unicode(1) "t"
  [16]=>
  unicode(1) "_"
  [17]=>
  unicode(1) "l"
  [18]=>
  unicode(1) "e"
  [19]=>
  unicode(1) "n"
  [20]=>
  unicode(1) "g"
  [21]=>
  unicode(1) "t"
  [22]=>
  unicode(1) "h"
}
--Iteration 13 --

Warning: str_split(): The length of each segment must be greater than zero in %s on line %d
bool(false)
--Iteration 14 --
array(23) {
  [0]=>
  unicode(1) "v"
  [1]=>
  unicode(1) "a"
  [2]=>
  unicode(1) "r"
  [3]=>
  unicode(1) "i"
  [4]=>
  unicode(1) "a"
  [5]=>
  unicode(1) "t"
  [6]=>
  unicode(1) "i"
  [7]=>
  unicode(1) "o"
  [8]=>
  unicode(1) "n"
  [9]=>
  unicode(1) "2"
  [10]=>
  unicode(1) ":"
  [11]=>
  unicode(1) "s"
  [12]=>
  unicode(1) "p"
  [13]=>
  unicode(1) "l"
  [14]=>
  unicode(1) "i"
  [15]=>
  unicode(1) "t"
  [16]=>
  unicode(1) "_"
  [17]=>
  unicode(1) "l"
  [18]=>
  unicode(1) "e"
  [19]=>
  unicode(1) "n"
  [20]=>
  unicode(1) "g"
  [21]=>
  unicode(1) "t"
  [22]=>
  unicode(1) "h"
}
--Iteration 15 --

Warning: str_split(): The length of each segment must be greater than zero in %s on line %d
bool(false)
--Iteration 16 --

Warning: str_split() expects parameter 2 to be long, Unicode string given in %s on line %d
NULL
--Iteration 17 --

Warning: str_split() expects parameter 2 to be long, Unicode string given in %s on line %d
NULL
--Iteration 18 --

Warning: str_split() expects parameter 2 to be long, Unicode string given in %s on line %d
NULL
--Iteration 19 --

Warning: str_split() expects parameter 2 to be long, Unicode string given in %s on line %d
NULL
--Iteration 20 --

Warning: str_split() expects parameter 2 to be long, object given in %s on line %d
NULL
--Iteration 21 --

Warning: str_split(): The length of each segment must be greater than zero in %s on line %d
bool(false)
--Iteration 22 --

Warning: str_split(): The length of each segment must be greater than zero in %s on line %d
bool(false)
--Iteration 23 --

Warning: str_split() expects parameter 2 to be long, resource given in %s on line %d
NULL
Done
