--TEST--
Test mb_encode_mimeheader() function : usage variations - Pass different data types to $str arg
--SKIPIF--
<?php
extension_loaded('mbstring') or die('skip');
function_exists('mb_encode_mimeheader') or die("skip mb_encode_mimeheader() is not available in this build");
?>
--FILE--
<?php
/* Prototype  : string mb_encode_mimeheader
 * (string $str [, string $charset [, string $transfer_encoding [, string $linefeed [, int $indent]]]])
 * Description: Converts the string to MIME "encoded-word" in the format of =?charset?(B|Q)?encoded_string?= 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass different data types to $str argument to see how mb_encode_mimeheader() behaves
 */

echo "*** Testing mb_encode_mimeheader() : usage variations ***\n";

// Initialise function arguments not being substituted
$charset = 'utf-8';
$transfer_encoding = 'B';
$linefeed = "\r\n";
$indent = 2;

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

// unexpected values to be passed to $str argument
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

// loop through each element of $inputs to check the behavior of mb_encode_mimeheader()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( mb_encode_mimeheader($input, $charset, $transfer_encoding, $linefeed, $indent));
  $iterator++;
};

fclose($fp);

echo "Done";
?>
--EXPECTF--
*** Testing mb_encode_mimeheader() : usage variations ***

-- Iteration 1 --
string(1) "0"

-- Iteration 2 --
string(1) "1"

-- Iteration 3 --
string(5) "12345"

-- Iteration 4 --
string(5) "-2345"

-- Iteration 5 --
string(4) "10.5"

-- Iteration 6 --
string(5) "-10.5"

-- Iteration 7 --
string(12) "123456789000"

-- Iteration 8 --
string(13) "1.23456789E-9"

-- Iteration 9 --
string(3) "0.5"

-- Iteration 10 --
string(0) ""

-- Iteration 11 --
string(0) ""

-- Iteration 12 --
string(1) "1"

-- Iteration 13 --
string(0) ""

-- Iteration 14 --
string(1) "1"

-- Iteration 15 --
string(0) ""

-- Iteration 16 --
string(0) ""

-- Iteration 17 --
string(0) ""

-- Iteration 18 --
string(6) "string"

-- Iteration 19 --
string(6) "string"

-- Iteration 20 --
string(11) "hello world"

-- Iteration 21 --
string(14) "Class A object"

-- Iteration 22 --
string(0) ""

-- Iteration 23 --
string(0) ""

-- Iteration 24 --

Warning: mb_encode_mimeheader() expects parameter 1 to be binary string, resource given in %s on line %d
NULL
Done

