--TEST--
Test array_shift() function : usage variations - Pass different data types as $stack arg
--FILE--
<?php
/* Prototype  : mixed array_shift(array &$stack)
 * Description: Pops an element off the beginning of the array 
 * Source code: ext/standard/array.c
 */

/*
 * Pass different data types as $stack argument to array_shift() to test behaviour
 */

echo "*** Testing array_shift() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a class
class classA
{
  public function __toString() {
    return "Class A object";
  }
}

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $stack argument
$inputs = array(

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // null data
/*10*/ NULL,
       null,

       // boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,
       
       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ "string",
       'string',
       $heredoc,
       
       // object data
/*21*/ new classA(),

       // undefined data
/*22*/ @$undefined_var,

       // unset data
/*23*/ @$unset_var,

       // resource variable
/*24*/ $fp
);

// loop through each element of $inputs to check the behavior of array_shift()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( array_shift($input) );
  $iterator++;
};

fclose($fp);

echo "Done";
?>
--EXPECTF--
*** Testing array_shift() : usage variations ***

-- Iteration 1 --

Warning: array_shift() expects parameter 1 to be array, integer given in %s on line %d
NULL

-- Iteration 2 --

Warning: array_shift() expects parameter 1 to be array, integer given in %s on line %d
NULL

-- Iteration 3 --

Warning: array_shift() expects parameter 1 to be array, integer given in %s on line %d
NULL

-- Iteration 4 --

Warning: array_shift() expects parameter 1 to be array, integer given in %s on line %d
NULL

-- Iteration 5 --

Warning: array_shift() expects parameter 1 to be array, double given in %s on line %d
NULL

-- Iteration 6 --

Warning: array_shift() expects parameter 1 to be array, double given in %s on line %d
NULL

-- Iteration 7 --

Warning: array_shift() expects parameter 1 to be array, double given in %s on line %d
NULL

-- Iteration 8 --

Warning: array_shift() expects parameter 1 to be array, double given in %s on line %d
NULL

-- Iteration 9 --

Warning: array_shift() expects parameter 1 to be array, double given in %s on line %d
NULL

-- Iteration 10 --

Warning: array_shift() expects parameter 1 to be array, null given in %s on line %d
NULL

-- Iteration 11 --

Warning: array_shift() expects parameter 1 to be array, null given in %s on line %d
NULL

-- Iteration 12 --

Warning: array_shift() expects parameter 1 to be array, boolean given in %s on line %d
NULL

-- Iteration 13 --

Warning: array_shift() expects parameter 1 to be array, boolean given in %s on line %d
NULL

-- Iteration 14 --

Warning: array_shift() expects parameter 1 to be array, boolean given in %s on line %d
NULL

-- Iteration 15 --

Warning: array_shift() expects parameter 1 to be array, boolean given in %s on line %d
NULL

-- Iteration 16 --

Warning: array_shift() expects parameter 1 to be array, Unicode string given in %s on line %d
NULL

-- Iteration 17 --

Warning: array_shift() expects parameter 1 to be array, Unicode string given in %s on line %d
NULL

-- Iteration 18 --

Warning: array_shift() expects parameter 1 to be array, Unicode string given in %s on line %d
NULL

-- Iteration 19 --

Warning: array_shift() expects parameter 1 to be array, Unicode string given in %s on line %d
NULL

-- Iteration 20 --

Warning: array_shift() expects parameter 1 to be array, Unicode string given in %s on line %d
NULL

-- Iteration 21 --

Warning: array_shift() expects parameter 1 to be array, object given in %s on line %d
NULL

-- Iteration 22 --

Warning: array_shift() expects parameter 1 to be array, null given in %s on line %d
NULL

-- Iteration 23 --

Warning: array_shift() expects parameter 1 to be array, null given in %s on line %d
NULL

-- Iteration 24 --

Warning: array_shift() expects parameter 1 to be array, resource given in %s on line %d
NULL
Done
