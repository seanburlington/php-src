--TEST--
Test decoct() function : usage variations - different data types as $number arg
--INI--
precision=14
--SKIPIF--
<?php
if (PHP_INT_SIZE != 4) die("skip this test is for 32bit platform only");
?>
--FILE--
<?php
/* Prototype  : string decoct  ( int $number  )
 * Description: Returns a string containing an octal representation of the given number argument.
 * Source code: ext/standard/math.c
 */

echo "*** Testing decoct() : usage variations ***\n";
//get an unset variable
$unset_var = 10;
unset ($unset_var);


// heredoc string
$heredoc = <<<EOT
abc
xyz
EOT;

// get a class
class classA
{
}

// get a resource variable
$fp = fopen(__FILE__, "r");

$inputs = array(
       // int data
/*1*/  0,
       1,
       12345,
       -2345,       
       4294967295,  // largest decimal  
       4294967296, 

       // float data
/*7*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // null data
/*12*/ NULL,
       null,

       // boolean data
/*14*/ true,
       false,
       TRUE,
       FALSE,
       
       // empty data
/*18*/ "",
       '',
       array(),

       // string data
/*21*/ "abcxyz",
       'abcxyz',
       $heredoc,
       
       // object data
/*24*/ new classA(),         
       
       // undefined data
/*25*/ @$undefined_var,

       // unset data
/*26*/ @$unset_var,

       // resource variable
/*27*/ $fp
);

// loop through each element of $inputs to check the behaviour of decoct()
$iterator = 1;
foreach($inputs as $input) {
	echo "\n-- Iteration $iterator --\n";
	var_dump(decoct($input));
	$iterator++;
};
fclose($fp);
?>
===Done===
--EXPECTF--
*** Testing decoct() : usage variations ***

-- Iteration 1 --
unicode(1) "0"

-- Iteration 2 --
unicode(1) "1"

-- Iteration 3 --
unicode(5) "30071"

-- Iteration 4 --
unicode(11) "37777773327"

-- Iteration 5 --
unicode(11) "37777777777"

-- Iteration 6 --
unicode(1) "0"

-- Iteration 7 --
unicode(2) "12"

-- Iteration 8 --
unicode(11) "37777777766"

-- Iteration 9 --
unicode(11) "27646215010"

-- Iteration 10 --
unicode(1) "0"

-- Iteration 11 --
unicode(1) "0"

-- Iteration 12 --
unicode(1) "0"

-- Iteration 13 --
unicode(1) "0"

-- Iteration 14 --
unicode(1) "1"

-- Iteration 15 --
unicode(1) "0"

-- Iteration 16 --
unicode(1) "1"

-- Iteration 17 --
unicode(1) "0"

-- Iteration 18 --
unicode(1) "0"

-- Iteration 19 --
unicode(1) "0"

-- Iteration 20 --
unicode(1) "0"

-- Iteration 21 --
unicode(1) "0"

-- Iteration 22 --
unicode(1) "0"

-- Iteration 23 --
unicode(1) "0"

-- Iteration 24 --

Notice: Object of class classA could not be converted to int in %s on line %d
unicode(1) "1"

-- Iteration 25 --
unicode(1) "0"

-- Iteration 26 --
unicode(1) "0"

-- Iteration 27 --
unicode(%d) "%d"
===Done===